#include "utils.h"
#include <stdio.h>

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

void die_with_error(char *error_text){
        printf("%s", error_text);
        exit(1);
}
