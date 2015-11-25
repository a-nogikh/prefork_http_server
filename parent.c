#include "parent.h"
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

static ServerItem *children;

void init_server(int socketFd){
    children = mmap(NULL, (sizeof ServerItem) * MAX_CHILD_COUNT,
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int i = 0;
    for(; i < MAX_CHILD_COUNT; i++){
        children[i].pid = 0;
        children[i].state = SERVER_ITEM_DEAD;
    }


}

void add_child(int socketFd){
    int i = 0;
    for(; i < MAX_CHILD_COUNT; i++){
        if (children[i].state != SERVER_ITEM_DEAD){
            continue;
        }

        if (fork() == 0) {
            client_process(children[i]);
        }
        // process error
        break;
    }
}

void check_children(){

}

