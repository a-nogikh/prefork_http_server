#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "client.h"
#include "parent.h"
#include "config.h"


void client_process(int server_socket, ServerItem *item){
    char buffer[1024];
    Config *config  = config_get();
    while(1){
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        int client_fd = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_fd < 0){
            switch(errno){
                case EAGAIN:
                case ECONNABORTED:
                    continue;
                default:
                    die_with_error("accept error");
                    break;
            }
        }

        // now reading
        http_parse_request* request = create_request();
        while(1){
            size_t c = read(client_fd, buffer, 1024);
            if (c <= 0){
                break;
            }
            int used = http_proceed_request(request, buffer, c);
            if (request->state == STATE_ERROR || request->state == STATE_HEADER_DONE){
                break;
            }
        }

        if (request->state != STATE_HEADER_DONE){
            free_request(request);
            close(client_fd);
            continue;
        }

        // somehow process data

    }
}
