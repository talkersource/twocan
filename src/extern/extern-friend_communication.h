#ifndef EXTERN_FRIEND_COMMUNICATION_H
#define EXTERN_FRIEND_COMMUNICATION_H

/* for tell and remote */
extern void user_tell_friends(player *, const char *, size_t);
extern void user_remote_friends(player *, const char *, size_t);

extern void cmds_init_friend_coms(void);

#endif
