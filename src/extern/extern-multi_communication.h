#ifndef EXTERN_MULTI_COMMUNICATION_H
#define EXTERN_MULTI_COMMUNICATION_H

extern const char *tell_ask_exclaim_me(player *, const char *, size_t);
extern const char *tell_ask_exclaim_group(player *, const char *, size_t);


extern void user_su_tell(player *, const char *, size_t);
extern void user_wake(player *, const char *);
extern void user_shout_emote(player *, const char *);
extern void user_shout_say(player *, const char *, size_t);
extern void user_tell(player *, const char *, size_t);
extern void user_remote_emote(player *, const char *, size_t);
extern void user_remote_echo(player *, const char *);
extern void user_su_warn(player *, const char *);
extern void user_su_wall(player *, const char *);
extern void cmds_init_multi_communication(void);

#endif
