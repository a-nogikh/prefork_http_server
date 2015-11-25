#include "parent.h"
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

static ServerItem *children;
int used_children = 0;
int server_socket;

void init_server(){
    // setting SIGCHLD handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sigaction(SIGCHLD, &sa, NULL);

    // allocating shared memory
    children = mmap(NULL, (sizeof ServerItem) * (MAX_CHILD_COUNT + 1),
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    int i = 0;
    for(; i < MAX_CHILD_COUNT; i++){
        children[i].pid = 0;
        children[i].state = SERVER_ITEM_DEAD;
    }

    Config *config = config_get();
    bind_server(config);

    int base_fork = config->min_children;
    if (base_fork < 1 || base_fork > MAX_CHILD_COUNT){
        die_with_error("min_children must be between 1 and MAX_CHILD_COUNT");
    }

    for (i = 0; i < base_fork; i++){
        fork_child(children[i]);
    }

    used_children = baseFork;
}

void bind_server(Config *conf){
    sockaddr_in addr;
    if (!str_to_sockaddr_ipv4(config->bind_to, &addr)){
        die_with_error("bind: parse failed");
    }
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        die_with_error("socket failed");
    }

    if (bind(server_socket, (struct sockaddr *) &addr,
          sizeof(addr)) < 0) {
        die_with_error("bind failed");
    }

    listen(server_socket,512);
}

void fork_child(ServerItem *item){
    if (item->state != SERVER_ITEM_DEAD){
        return;
    }

    item->state = SERVER_ITEM_AVAILABLE;
    pid_t pid = fork();
    if (pid < 0){
        die_with_error("Fork failed");
    }
    else if (pid > 0){
        item->pid = pid;
    }
    else if (pid == 0){
        client_process(server_socket, item);
    }
}

void check_children(){
    Config *config = config_get();
    int alive_count = 0, available_count = 0, i =0;

    for(; i < used_children; i++){
        if (children[i].state == SERVER_ITEM_DEAD){
            continue;
        }

        alive_count++;
        if (children[i].state == SERVER_ITEM_AVAILABLE){
            available_count ++;
        }
    }

    int add_count = 0;
    if (alive_count < config->min_children){
        add_count = config->min_children - alive_count;
    }
    if (available_count == 0 && add_count == 0
        && alive_count < config->max_children
        && alive_count + 1 < MAX_CHILD_COUNT){
            add_count = 1;
    }

    for (i=0; i<used_children && add_count > 0; i++){
        if (children[i].state == SERVER_ITEM_DEAD){
            fork_child(children[i]);
            add_count--;
            available_count++;
        }
    }

    if (add_count > 0){
        for(i = used_children; i < (used_children + add_count); i++){
            fork_child(children[i]);
        }
    }
}


void sigchld_handler(int sig)
{
    pid_t p;
    int status;

    while ((p=waitpid(-1, &status, WNOHANG)) != -1)
    {
        // handling death of a child
        int i;
        for(i=0;i<used_children;i++){
            if (children[i].pid == p){
                children[i].state = SERVER_ITEM_DEAD;
                break;
            }
        }
    }
}

