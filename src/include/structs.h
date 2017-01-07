#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct file
{
 char *where;
 size_t length;
} file;




/* flag list def */
typedef struct flag_list
{
 const char *text;
 int change;
} flag_list;

#endif
