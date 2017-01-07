#ifndef PLAYER_EVENT_H
#define PLAYER_EVENT_H

/* Order matters due to PLAYER_EVENT_UPGRADE() */

#define PLAYER_EVENT_CHOOSE 0
#define PLAYER_EVENT_OUTPUT 1
#define PLAYER_EVENT_INPUT 2
#define PLAYER_EVENT_RUN_COMMAND 3
#define PLAYER_EVENT_LOGOFF 4
#define PLAYER_EVENT_RECONNECTION 5
#define PLAYER_EVENT_BAD_FD 6

#endif