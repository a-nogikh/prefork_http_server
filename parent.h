#ifndef PARENT_H_INCLUDED
#define PARENT_H_INCLUDED
#define MAX_CHILD_COUNT 1000

typedef enum {
    SERVER_ITEM_AVAILABLE,
    SERVER_ITEM_BUSY,
    SERVER_ITEM_DEAD
} ServerItemState;

struct ServerItem{
    ServerItemState state;
    pid_t pid;
};
typedef struct ServerItem ServerItem;

void init_server();
void check_children();

#endif // PARENT_H_INCLUDED
