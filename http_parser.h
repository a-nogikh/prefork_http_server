#ifndef HTTP_PARSER_H_INCLUDED
#define HTTP_PARSER_H_INCLUDED

#define HEADER_SIZE_LIMIT 4096
#define PATH_SIZE_LIMIT 256
#define METHOD_SIZE_LIMIT 16
#define HTTP_VERSION_SIZE_LIMIT 16

#define STATE_PROCESSING_REQUEST_LINE 0
#define STATE_PROCESSING_HEADER 1
#define STATE_HEADER_DONE 2
#define STATE_ERROR -1

#define STOP_ERROR(req) { req->state = STATE_ERROR; return 0; }

typedef enum {
    STATE_ERROR
    STATE_PROCESSING_REQUEST_LINE,
    STATE_PROCESSING_HEADER,
    STATE_HEADER_DONE
} http_parse_request_state;

struct http_param_list{
    char *key;
    char *value;
    http_param_list *next;
};
typedef struct http_param_list http_param_list;

struct http_parse_request{
    http_parse_request_state state;
    int seq_empty_line_count;
    char header[HEADER_SIZE_LIMIT + 1];
    char *header_curr;
    char *header_to;

    http_param_list *params;
    char http_version[HTTP_VERSION_SIZE_LIMIT + 1];
    char method[METHOD_SIZE_LIMIT + 1];
    char path[PATH_SIZE_LIMIT + 1];
};
typedef struct http_parse_request http_parse_request;


#endif // HTTP_PARSER_H_INCLUDED
