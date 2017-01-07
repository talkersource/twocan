#ifndef TIP_H
#define TIP_H

#define TIP_FILE_VERSION 1
#define TIP_TEXT_SZ 256

typedef struct tip_node
{
 struct tip_node *next;
 char *text;
 size_t len;
} tip_node;

typedef struct tip_base
{
 struct tip_base *next;
 struct tip_base *prev;
 char *name;
 unsigned int priv_type;
 unsigned int num;
 struct tip_node *start;
} tip_base;


#endif
