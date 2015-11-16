#include <stdio.h>
#include <stdlib.h>
#include "config.h"

void read_config(){
    FILE *file = fopen("server.cfg", "r");
    config_read_from_file(file);
}

int main()
{
    read_config();

    Config *cf = config_get();
    return 0;
}
