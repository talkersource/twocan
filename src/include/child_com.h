#ifndef CHILD_COM_H
#define CHILD_COM_H

#define CHILD_COM_DEFAULT_SIZE 1024

typedef struct child_com
{
 size_t buffer_size; /* so you can do... child_com abcd = {123}; */

 int input;
 FILE *output;

 Socket_poll_typedef_offset io_indicator;
 
 pid_t pid;
 
 time_t c_timestamp; /* time created */
 time_t s_timestamp; /* last time sent */
 time_t r_timestamp; /* last time recv */
 
 size_t used;
 const char *cached_terminator;

 bitflag output_waiting : 1;
 bitflag bad : 1;

 char buffer[1024]; /* for using C struct hack... */
} child_com;

#endif
