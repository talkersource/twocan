#ifndef EXTERN_KARMA_H
#define EXTERN_KARMA_H

/* available = hours of real logon time. */
#define KARMA_TRANS_FUNC(available) (available -= 12)

extern void cmds_init_karma(void);

#endif
