#ifndef EXTERN_STATS_FILES_H
#define EXTERN_STATS_FILES_H

extern void stats_open_file(int);
extern void stats_close_file(char);
extern void stats_log_event(player *, char, int);

extern void init_stats_files(void);

extern void cmds_init_stats_files(void);

#endif
