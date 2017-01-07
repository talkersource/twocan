#ifndef LAST_COMMAND_H
#define LAST_COMMAND_H

#define LAST_COMMANDS_SZ 32

typedef struct last_command_node
{
 const char *command_name;
 int number_of_times;
 int min_timediff;
 int max_timediff;
 time_t timestamp;
} last_command_node;

#endif
