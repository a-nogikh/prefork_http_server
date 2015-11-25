#include "utils.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

int is_wspace(char c){
    switch(c){
    case ' ':
    case '\t':
    case '\n':
    case '\r':
        return 1;
    }
    return 0;
}

char *ltrim(char *in){
    while(*in != 0 && is_wspace(*in)) in++;
    return in;
}

char *rtrim(char *in){
    char *tmp = in + strlen(in) - 1;
    while(tmp >= in && is_wspace(*tmp)) *tmp-- = '\0';
    return in;
}

char* copy_till(char *src, char *dest, char till, int limit){
    if (limit <= 0){
        return 0;
    }

    int pos = 0;
    while (*src && *src != till){
        *dest++ = *start++;
        if (pos >= limit){
            break;
        }
    }

    *dest = '\0';
    if (*src == '\0' || *src == till)
    {
        return src;
    }
    return 0;
}

int str_to_sockaddr_ipv4(char *src, sockaddr_in *dst){
    bzero((char *) dst, sizeof(sockaddr_in));
    src = ltrim(rtrim(src));

    char addr[32];
    char port[16];
    src = copy_till(src, addr, ':', 31);
    if (src == 0 || strlen(addr) < 8){
        return 0;
    }

    src++;

    src = copy_till(src, port, '\0', 15);
    if (src == 0 || strlen(port) < 1){
        return 0;
    }

    addr = ltrim(rtrim(addr));
    port = ltrim(rtrim(port));

    if (!inet_pton(AF_INET, addr, &(dst->sin_addr))){
        return 0;
    }

    char *ep;
    int n = strtol(port, ep, 10);
    if (*ep != 0 || n <= 0){
        return 0;
    }

    dst->sin_port = n;
    dst->sin_family = AF_INET;
    return 1;
}

int file_length(FILE *file){
    fseek(f, 0, SEEK_END);
    int s = ftell(f);
    fseek(f, 0, SEEK_SET);
    return s;
}

void die_with_error(char *error_text){
    printf("%s", error_text);
    exit(1);
}
