#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef SYSTEM_C
# define SYSTEM_STATS_FILE_VERSION 1
# define STATS_SYNC_TIME MK_MINUTES(5)
#endif

typedef struct sys_info
{
 /* Creation date */
 time_t date_stamp;      /* When file created */
 /* Motd's */
 time_t motd;            /* When the motd was last updated */
 time_t su_motd;         /* and the SU_motd */
 /* General (ever) */
 int total_ever_logons;  /* logons EVER (connections) */
 int max_time_logged; /* time up since date_stamp */
 /* Maximums */
 int max_logons;      /* Maximum number of logins in one up session */
 int max_l_timeup;    /* the time we'd been up when this occured */
 int max_uniq_logons;  /* and max number of NEW logins in one up session */
 int max_nl_timeup;   /* the time we'd been up when this occured */
 int max_people;      /* The maximum people on all at once */
 int max_time;        /* The longest up time */
 char longest_spod_name[PLAYER_S_NAME_SZ]; /* Biggest spod */
 int longest_spod_time;
 /* Miscellaneous */
 char shutdown[SHUTDOWN_MSG_SZ];  /* last shutdown msg */
} sys_info;

typedef struct coms_stats
{
  int time_up;
} coms_stats;

/* All the info we would like stored about the program */
extern sys_info system_data;

#endif /* SYSTEM_H */

