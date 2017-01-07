#ifndef LOGON_H
#define LOGON_H

/* number of people who can connect at once */
#define TALKER_MAX_PLAYERS_LOGGING_ON 15

/* number of people allowed on the talker */
#define TALKER_MAX_PLAYERS_ON (210 - TALKER_MAX_PLAYERS_LOGGING_ON)

#define LOGON_INIT_TIMEOUT 4 /* seconds */

/* number of player information */
extern int current_players;

/* number of logon's, since boot. */
extern int total_logons;
/* number of seperate logon's since boot */
extern int total_uniq_logons;

/* last time someone entered or left the program */
struct timeval last_entered_left;

#endif
