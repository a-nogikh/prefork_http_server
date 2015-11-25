#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

int is_wspace(char c);
char *rtrim(char *in);
char *ltrim(char *in)
void die_with_error(char *error_text);
char* copy_till(char *src, char *dest, char till, int limit);

#endif // UTILS_H_INCLUDED
