#ifndef EXTERN_PRIVS_H
#define EXTERN_PRIVS_H

#ifdef PRIVS_C

#define RES_DEL_MODE(x) do { \
 int done = mode_del(x); \
 \
 assert(done); \
 if (!done) \
 { \
  fvtell_player(SYSTEM_T(x), " Something went a bit wrong and we couldn't" \
                " make you a resident atm. please speak to an su again.\n"); \
  return; } } while (FALSE)
 

#endif

extern Timer_q_base priv_resident_timer_queue;

extern void priv_start_residency(player *);

extern void init_privs(void);

extern void cmds_init_privs(void);


#endif
