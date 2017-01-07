#ifndef ANGEL_H
#define ANGEL_H

#define ANGEL_MIN_TIME_HALT 4 /* if we die twice in this time, halt */
#define ANGEL_MAX_TIME_HALT MK_MINUTES(32)

/* after which we'll just keep rebooting */
#define ANGEL_TIMEOUT_BOOT MK_MINUTES(4)

#define ANGEL_MAX_REBOOT_LIMIT 4 /* max ammount of reboots before MAX_TIME */

/* how long does the talker wait around for the angel */
#define ANGEL_TALKER_WAIT_BOOT 2
#define ANGEL_TALKER_POLL_BOOT 500 /* the poll timeouts */

#define ANGEL_TALKER_WAIT_INPUT 32

#endif
