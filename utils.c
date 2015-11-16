#include "utils.h"
#include <stdio.h>

void die_with_error(char *error_text){
        printf("%s", error_text);
        exit(1);
}
