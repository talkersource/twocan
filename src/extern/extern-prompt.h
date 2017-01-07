#ifndef EXTERN_PROMPT_H
#define EXTERN_PROMPT_H

extern void prompt_update(player *, const char *);
extern void prompt_add_input(player *, const char *, size_t);
extern void prompt_del_input(player *, size_t);
extern char *prompt_do_output(player *, unsigned int *);
extern char *prompt_next_output(player *, unsigned int *);

extern void user_configure_prompt_use_priority(player *, const char *);

extern void cmds_init_prompt(void);

#endif
