#ifndef PARENT_H_INCLUDED
#define PARENT_H_INCLUDED

#define SERVER_ITEM_RUNNING 1
#define SERVER_ITEM_AVAILABLE 0


struct ServerItem{
    int state;
    pid_t pid;
};
typedef struct ServerItem ServerItem;

#endif // PARENT_H_INCLUDED
