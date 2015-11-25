#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#define SERVER_NAME "Prefork-based server"
#define HTTP_1_0 "HTTP/1.0"

#define END_CLIENT() { item->state = SERVER_ITEM_AVAILABLE; close(client_fd); http_parse_free_request(request); }

#endif // CLIENT_H_INCLUDED
