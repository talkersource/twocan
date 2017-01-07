#ifndef TIMER_H
#define TIMER_H

#if 0

#define DECL_CRAZY_TIMER(timer) \
 time_t timer ## _timestamp; unsigned long timer ## _in_seconds

#define TIMER_TYPE_RUN 1
#define TIMER_TYPE_DEL 2
#define TIMER_TYPE_EXEC_RUN 3

typedef struct timer_node
{
 struct timer_node *next;
 
 void *data;
 void (*func)(int, void *);

 DECL_CRAZY_TIMER(timer);

 bitflag do_free : 1;
 bitflag has_prev : 1;
} timer_node;

typedef struct timer_double_node
{
 timer_node s;
 timer_node *prev;
} timer_double_node;

typedef struct global_timers
{
 DECL_CRAZY_TIMER(shutdown);
 struct timer_double_node angel;
 struct timer_double_node channels;
 struct timer_double_node stats;
 struct timer_double_node text_objs;
} global_timers;

#else

typedef struct global_timers
{
 time_t shutdown_timestamp;
 unsigned long shutdown_in_seconds;
} global_timers;

#endif

#endif
