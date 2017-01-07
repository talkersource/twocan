#ifndef EXTERN_HELP_H
#define EXTERN_HELP_H

extern const char *help_primary_name; /* for twinkles */
extern const char *help_search_name; /* for twinkles */
extern player *help_shown_by;

extern void init_help(void);

extern void help_destroy_all(void);

#if HELP_DEBUG
extern int help_exists(const char *str);
#endif

extern void user_help(player *, const char *);

extern void cmds_init_help(void);

#endif
