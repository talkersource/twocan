#ifndef PROCESS_INPUT_H
#define PROCESS_INPUT_H

#define INPUT_NODE_QUEUE_SOFT_SZ 50
#define INPUT_NODE_QUEUE_HARD_SZ 80

#define INPUT_BUFFER_SZ 512

typedef struct input_node
{
 struct input_node *next;
 struct input_node *prev;

 char input[INPUT_BUFFER_SZ];
 size_t length;
 
 bitflag in_sub_mode : 1;
 bitflag comp_generated : 1;
 bitflag ready : 1;
} input_node;

#endif
