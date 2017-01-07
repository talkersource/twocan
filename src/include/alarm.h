#ifndef ALARM_H
#define ALARM_H

typedef struct Alarm_node
{
 struct Alarm_node *next;
 struct Alarm_node *prev;
 
 char *alarm_str;
 Timer_q_double_node node;
} Alarm_node;

#endif

