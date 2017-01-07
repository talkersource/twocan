#ifndef EXTERN_TIMER_PLAYER_H
#define EXTERN_TIMER_PLAYER_H

extern Timer_q_base timer_queue_player_no_logon;
extern Timer_q_base timer_queue_player_no_move;
extern Timer_q_base timer_queue_player_no_shout;
extern Timer_q_base timer_queue_player_jail;

extern void init_timer_player(void);

extern void cmds_init_timer_player(void);

#endif
