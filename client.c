#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <errno.h>
#include "client.h"
#include "parent.h"
#include "config.h"
#include "mime.h"
#include "utils.h"


void process_client(int server_socket, ServerItem *item){
    char buffer[1024];
    config *config  = config_get();

    int processed_clients = 0;
    while(1){
        if (processed_clients >= config->child_max_queries){
            exit(0);
        }

        item->state = SERVER_ITEM_AVAILABLE;

        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        int fd = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len);
        if (fd < 0){
            switch(errno){
                case EAGAIN:
                case ECONNABORTED:
                    continue;
                default:
                    die_with_error("accept error");
                    break;
            }
        }

        item->state = SERVER_ITEM_BUSY;

        // now reading
        http_parse_request* request = http_parse_create_request();
        while(1){
            size_t c = read(fd, buffer, 1024);
            if (c <= 0){
                break;
            }
            int used = http_proceed_request(request, buffer, c);
            if (request->state == STATE_ERROR || request->state == STATE_HEADER_DONE){
                break;
            }
        }

        processed_clients++;

        if (request->state != STATE_HEADER_DONE){
            if (request->state == STATE_ERROR && strlen){
                STATUS_400(fd, HTTP_1_0);
                write_error_headers(fd);
                END_CLIENT();
            }
            END_CLIENT();
            continue;
        }

        if (stricmp(request->http_version, "http/1.0") != 0 &&
            stricmp(request->http_version, "http/1.1") != 0){

            STATUS_400(fd, HTTP_1_0);
            write_error_headers(fd);
            END_CLIENT();
            continue;
        }

        if (stricmp(request->method, "get") != 0){
            STATUS_405(fd, HTTP_1_0);
            write_error_headers(fd);
            END_CLIENT();
            continue;
        }

        char *host_param = http_parser_find_param(request, "host");

        config_host *host = NULL;
        if (host_param != NULL){
           host = find_host(host_param);
        }

        if (host_param == NULL || host == NULL){
            STATUS_404(fd, HTTP_1_0);
            write_error_headers(fd);
            END_CLIENT();
            continue;
        }

        if (strcmp(request->path, "/") == 0){
            memset(request->path, '\0', sizeof(request->path));
            strcpy(request->path, "/index.html");
        }

        respond_file(fd, host, request->path);
        END_CLIENT();

        item->state = SERVER_ITEM_AVAILABLE;
    }
}

void respond_file(int fd, config_host *host, char *path){
    char fullpath[512];
    int root_len = strlen(host->root), path_len = strlen(request->path);
    if (root_len < 1 || path_len < 1 || root_len + path_len + 2 >= 512){
        STATUS_400(fd, HTTP_1_0);
        write_error_headers(fd);
        return;
    }
    char *curr = fullpath;
    strcpy(curr, host->root);
    curr += root_len;
    if (host->root[root_len-1] != '/'){
        *curr = '/'; curr++;
    }
    strcpy(curr, path);
    curr += path_len;
    *curr = '\0';

    FILE *file = fopen(curr, "rb");
    if (file == NULL){
        STATUS_404(fd, HTTP_1_0);
        write_error_headers(fd);
        return;
    }

    STATUS_200(fd, HTTP_1_0);
    write_base_headers(fd);

    int len = file_len(file);
    PARAM_CONTENT_LENGTH(fd, len);
    PARAM_CONTENT_TYPE(fd, detect_mime_type(path, file));
    http_empty_line(fd);
    sendfile(fd, file, NULL, len);
}

void write_base_headers(int fd){
    PARAM_DATE(fd, asctime());
    PARAM_SERVER(fd, SERVER_NAME);
    PARAM_CONNECTION(fd, "closed");
}

void write_error_headers(int fd){
    write_base_headers(fd);
    PARAM_CONTENT_LENGTH(fd, 0);
    PARAM_CONTENT_TYPE(fd, "text/html");
    http_empty_line(fd);
}
