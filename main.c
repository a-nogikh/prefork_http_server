#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include "parent.h"
#include <sys/prctl.h>
#include <signal.h>

#define CHILD_CHECK_INTERVAL_USEC 50000L // 50 ms

void read_config(){
    FILE *file = fopen("server.cfg", "r");
    config_read_from_file(file);
}

static void quit_handler(int signo) {
    stop_server();
    exit(0);
}

int main()
{
    read_config();
    init_server();

    signal(SIGHUP, quit_handler);
    signal(SIGINT, quit_handler);
    signal(SIGSTOP, quit_handler);
    signal(SIGABRT, quit_handler);
    signal(SIGKILL, quit_handler);
    signal(SIGSEGV, quit_handler);
    signal(SIGTERM, quit_handler);

    while (1){
        check_children();
        usleep(CHILD_CHECK_INTERVAL_USEC);
    }

    signal(SIGQUIT, quit_handler);

    return 0;
}

