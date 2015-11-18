#include "http_parser.h"
#include "utils.h"
#include <string>

http_parse_request* create_request(){
    http_parse_request *item = (http_parse_request *)malloc(sizeof(http_parse_request));
    item->header_curr = item->header;
    item->header_to = item->header;
    item->seq_empty_line_count = 0;
    item->params = NULL;
    item->state = STATE_PROCESSING_REQUEST_LINE;
    return item;
}

/*
Process yet another block of input socket data.
newbuf - block of data of newBufSize size

returns number of used newbuf characters (unused data corresponds to message body)
*/
int http_proceed_request(http_parse_request *request, char *newbuf, int newBufSize){
    if (request->state != STATE_PROCESSING_REQUEST_LINE &&
        request->state != STATE_PROCESSING_HEADER){
        return 0;
    }

    char *headEnd = request->header + HEADER_SIZE_LIMIT - 1,
            *origTo = request->header_to;

    // copy input data to request header buffer
    while (request->header_to < headEnd && newBufSize-- > 0){
        *request->header_to++ = *newbuf++;
    }

    char *lineEnd = request->header_curr, *lastLineEnd = origTo;
    while (lineEnd + 1 <= headEnd && *lineEnd != '\0'){
        if (*(lineEnd) != '\r' || *(lineEnd + 1) != '\n'){
            lineEnd++;
            continue;
        }

        lastLineEnd = lineEnd + 1;
        int lineSize = lineEnd - request->header_curr - 1;
        if (lineSize <= 0){ // empty line
            if(request->state == STATE_PROCESSING_HEADER
                     && request->seq_empty_line_count == 0){
                request->seq_empty_line_count++;
            }else if (request->state == STATE_PROCESSING_HEADER
                      && request->seq_empty_line_count == 1){
                // end of header
                request->state = STATE_HEADER_DONE;
                return max(lineEnd + 1 - origTo, 0);
            }else{
                STOP_ERROR(request);
            }
            request->header_curr = lineEnd + 1;
            lineEnd += 2;
        }else{ // non-empty line
            if (request->state == STATE_PROCESSING_REQUEST_LINE
                && !http_query_request(request->header_curr, lineEnd, request)){
                STOP_ERROR(request);
            }else if (request->state == STATE_PROCESSING_HEADER
                && !http_query_keyvalue(request->header_curr, lineEnd, request)){
                STOP_ERROR(request);
            }
            request->header_curr = lineEnd + 1;
            request->seq_empty_line_count = 0;
            lineEnd += 2;
        }
    }


    return max(0, lastLineEnd - origTo);
}

int http_query_request(char *start, char *end, http_parse_request *request){
    // @TODO
}

int http_query_keyvalue(char *start, char *end, http_parse_request *request){
    start = ltrim(start);
    char *keyEnd = strstr(buf, ":"), *key, *value;
    if (!keyEnd || keyEnd > end){
        return 0;
    }

    int keyLen = keyEnd - answer + 1;
    key = (char *)malloc(sizeof(char)*(keyLen + 1));
    memcpy(key, start, keyLen);
    key[keyLen] = '\0';
    key = rtrim(key);

    start = keyEnd + 1;
    while(start < end && is_wspace(start)) start++;

    int valueLen = end - start;
    if (valueLen <= 0){
        value = strdup("");
    }else{
        value = (char *)malloc(sizeof(char)*(valueLen+1));
        memcpy(value, start, valueLen);
        value[valueLen] = '\0';
    }

    http_param_list *listItem = (http_param_list *)malloc(sizeof(http_param_list));
    listItem->key = key;
    listItem->value = value;
    listItem->next = request->params;
    request->params = listItem;

    return 1;
}

void free_request(http_parse_request *request){
    http_param_list *currParam = request->params;
    while (currParam != NULL){
        http_param_list *next = currParam->next;
        free(currParam->key);
        free(currParam->value);
        free(currParam);
        currParam = next;
    }
    free(request);
}
