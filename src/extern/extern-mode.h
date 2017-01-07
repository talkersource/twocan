#ifndef EXTERN_MODE_H
#define EXTERN_MODE_H

#define MODE_INVALID(x) (!(x)->mode_count || \
                         ((x)->mode_count >= MODE_MODES_SZ))
#define MODE_IN_MODE(x, y) (((x)->mode_count < (MODE_MODES_SZ - 1)) && \
 (MODE_CURRENT(x).id == MODE_ID_ ## y))

#define MODE_CURRENT(x) ((x)->modes[(x)->mode_count])

#define MODE_HELPER_COMMAND()  do { \
 if (*str == '/') \
   return (cmds_match(p, str + 1, length - 1, CMDS_MATCH_DEFAULT_LOOP)); \
 } while (FALSE)

/* must be in a mode Ie. mode_count > 0 */
#define MODE_DEL_ALL(x) do { \
 while (!MODE_INVALID(x)) mode_del(x); } while (FALSE)

extern void mode_change(player *, const char *, unsigned int, unsigned int,
                        cmds_function *, cmds_function *, cmds_function *);
extern int mode_add(player *, const char *, unsigned int, unsigned int,
                    cmds_function *, cmds_function *, cmds_function *);
extern int mode_del(player *);

extern void mode_init_base(player *);

extern void cmds_init_mode(void);

#endif
