#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define HOSTS_LIMIT 128
#define STRING_BUFFER 512

struct Config{
    int min_children;
    int max_children;
    int child_max_queries;
    char *bind_to;
    struct Host{
        char *mask;
        char *root;
    } hosts[HOSTS_LIMIT];
    int hosts_count;
};
typedef struct Config Config;

Config *config_get();
void config_read_from_file(FILE *file);

#endif // CONFIG_H_INCLUDED
