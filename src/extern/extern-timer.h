#ifndef EXTERN_TIMER_H
#define EXTERN_TIMER_H

#define TIMER_START(obj, timer, secs) do { \
 (obj)-> timer ## _timestamp = now; (obj)-> timer ## _in_seconds = secs; \
 } while (FALSE)
#define TIMER_STOP(obj, timer) (obj)-> timer ## _in_seconds = 0

#define TIMER_IS_ACTIVE(obj, timer) ((obj)-> timer ## _in_seconds)
#define TIMER_TOGO(obj, timer) ((int) \
 ((obj)-> timer ## _in_seconds - difftime(now, (obj)-> timer ## _timestamp)))
#define TIMER_IS_DONE(obj, timer) (TIMER_IS_ACTIVE(obj, timer) && \
 (difftime(now, (obj)-> timer ## _timestamp) > (obj)-> timer ## _in_seconds))

extern global_timers glob_timer;

#if 1 /* using timer_q library now... */

extern Timer_q_base timer_queue_global;

extern struct timeval now_timeval; /* timestamps for timer_q */
extern time_t now; /* current time */

#else

extern timer_node *timer_queue_player_idle;

extern timer_node *timer_queue_player_jail;
extern timer_node *timer_queue_player_logon;
extern timer_node *timer_queue_player_no_logon;
extern timer_node *timer_queue_player_no_move;
extern timer_node *timer_queue_player_no_shout;
extern timer_node *timer_queue_player_nuke;
extern timer_node *timer_queue_player_res;
extern timer_node *timer_queue_player_scripting;

extern timer_node *timer_queue_global;

extern timer_node *timer_queue_room_autos;

extern timer_node *timer_queue_auth_player;

extern timer_node *timer_queue_list_cleanup;
extern timer_node *timer_queue_mail_cleanup;
extern timer_node *timer_queue_news_cleanup;
extern timer_node *timer_queue_player_cleanup;
extern timer_node *timer_queue_room_cleanup;

extern timer_node *timer_queue_channels;

extern timer_node *timer_init_node(timer_node *,
                                   void (*)(int, void *), void *, int);

extern timer_node *timer_find_data(timer_node *, void *);
extern void timer_add_node(timer_node **, timer_node *);
extern void timer_del_node(timer_node **, timer_node *);
/* This deals with not being able to find -- so code looks simpler */
extern void timer_del_data(timer_node **, void *);

extern int timer_run_q(timer_node **);
extern int timer_exec_run_q(timer_node **);

#endif

extern void timer_run_do(void);
extern void timer_exec_run_do(void);

extern void init_timer(void);

#endif
