#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include "child.h"
#include "http_parser.h"
#include "parent.h"
#include "config.h"
#include "mime.h"
#include "http_writer.h"
#include "utils.h"

void write_file(FILE *file_d, int socket_d);
void write_error_headers(int fd);
void write_base_headers(int fd);
void respond_file(int fd, config_host *host, char *path);

void process_client(int server_socket, server_item *item){
    prctl(PR_SET_PDEATHSIG, SIGHUP);

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
            http_proceed_request(request, buffer, c);
            if (request->state == STATE_ERROR || request->state == STATE_HEADER_DONE){
                break;
            }
        }
        processed_clients++;

        if (request->state != STATE_HEADER_DONE){
            if (request->state == STATE_ERROR){
                STATUS_400(fd, HTTP_1_0);
                write_error_headers(fd);
                END_CLIENT(item, fd, request);
            }
            END_CLIENT(item, fd, request);
            continue;
        }

        if (stricmp(request->http_version, "http/1.0") != 0 &&
            stricmp(request->http_version, "http/1.1") != 0){

            STATUS_400(fd, HTTP_1_0);
            write_error_headers(fd);
            END_CLIENT(item, fd, request);
            continue;
        }

        if (stricmp(request->method, "get") != 0){
            STATUS_405(fd, HTTP_1_0);
            write_error_headers(fd);
            END_CLIENT(item, fd, request);
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
            END_CLIENT(item, fd, request);
            continue;
        }

        if (strcmp(request->path, "/") == 0){
            memset(request->path, '\0', sizeof(request->path));
            strcpy(request->path, "/index.html");
        }

        // chop off part after ?
        char *tmp = request->path;
        while (*tmp != '\0' && *tmp != '?' && *tmp != '#') tmp++;
        *tmp = '\0';

        respond_file(fd, host, request->path);
        END_CLIENT(item, fd, request);

        item->state = SERVER_ITEM_AVAILABLE;
    }
}

void respond_file(int fd, config_host *host, char *path){
    char fullpath[512];
    int root_len = strlen(host->root), path_len = strlen(path);
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

    FILE *file = fopen(fullpath, "rb");
    if (file == NULL){
        STATUS_404(fd, HTTP_1_0);
        write_error_headers(fd);
        return;
    }

    STATUS_200(fd, HTTP_1_0);
    write_base_headers(fd);

    int len = file_length(file);
    PARAM_CONTENT_LENGTH(fd, len);
    PARAM_CONTENT_TYPE(fd, detect_mime_type(path, file));
    http_empty_line(fd);
    write_file(file, fd);
}

void write_file(FILE *file_d, int socket_d){
    char data[SEND_BUFFER_SIZE];

    size_t n = 0;
    while ( (n = fread(data, sizeof(char), SEND_BUFFER_SIZE, file_d)) > 0)
    {
        write(socket_d, data, n);
    }
}

void write_base_headers(int fd){
    PARAM_DATE(fd, current_time());
    PARAM_SERVER(fd, SERVER_NAME);
    PARAM_CONNECTION(fd, "closed");
}

void write_error_headers(int fd){
    write_base_headers(fd);
    PARAM_CONTENT_LENGTH(fd, 0);
    PARAM_CONTENT_TYPE(fd, "text/html");
    http_empty_line(fd);
}

