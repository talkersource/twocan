#ifndef EXTERN_BOOT_H
#define EXTERN_BOOT_H

extern system_flags sys_flag; /* internal flags to trigger things */

extern time_t talker_started_on; /* when the talker started */

/* how many resis we have (total) and how many different logins we have had */
extern int no_of_resis;

extern const char *root_dir; /* the root dir of the talker */

extern void init_priv_lists(void);

#endif
