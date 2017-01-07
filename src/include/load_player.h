#ifndef LOAD_PLAYER_H
#define LOAD_PLAYER_H

#ifdef LOAD_PLAYER_C
/* have to be bigger than the player logon timeouts */
# define PLAYER_CLEANUP_TIMEOUT_LOAD MK_MINUTES(4)
# define PLAYER_CLEANUP_TIMEOUT_CLEANUP MK_MINUTES(2)
/* don't forget that this issues a "Player infomation saved" message */
# define PLAYER_CLEANUP_TIMEOUT_SYNC_ANYWAY MK_MINUTES(64)
# define PLAYER_CLEANUP_TIMEOUT_REDO MK_MINUTES(4)
#endif

#endif
