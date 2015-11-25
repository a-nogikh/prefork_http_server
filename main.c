#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include "parent.h"


#define CHILD_CHECK_INTERVAL_USEC 50000L // 50 ms

void read_config(){
    FILE *file = fopen("server.cfg", "r");
    config_read_from_file(file);
}

int main()
{
    read_config();
    init_server();
    while (1){
        check_children();
        usleep(CHILD_CHECK_INTERVAL_USEC);
    }
    return 0;
}
