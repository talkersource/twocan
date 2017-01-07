#ifndef STATS_FILES_H
#define STATS_FILES_H

#define STATS_FILENAME_SZ 100

#define STATS_OPEN_NEW '&'
#define STATS_CLOSE_NEW '@'
#define STATS_BOOT '^'
#define STATS_CRASH '!'
#define STATS_SHUT '_'
#define STATS_BACKUP '='
#define STATS_RESI_OFF '<'
#define STATS_RESI_ON '>'
#define STATS_ON_SU '}'
#define STATS_OFF_SU '{'
#define STATS_STILL_UP '#'

/* int for extra info */
#define STATS_NO_EXTRA 0
#define STATS_SU_FORCED 1
#define STATS_RESI_ON_RECONNECT 2
#define STATS_RESI_OFF_IDLEOUT 3
#define STATS_RESI_OFF_FORCED 4

#endif /* STATS_FILES_H */


