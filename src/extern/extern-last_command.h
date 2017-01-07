#ifndef EXTERN_LAST_COMMAND_H
#define EXTERN_LAST_COMMAND_H

extern void last_command_add(player *, const char *);
extern void last_command_clear(player *);

extern void cmds_init_last_command(void);

#endif
