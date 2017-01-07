#ifndef INTERCOM_GLUE_H
#define INTERCOM_GLUE_H

#define INTERCOM_INIT() /* do nothing */
#define INTERCOM_POLL() /* do nothing */
#define INTERCOM_SHUTDOWN() /* do nothing */

#define INTERCOM_USER_FINGER(p, str) /* do nothing */
#define INTERCOM_USER_EXAMINE(p, str) /* do nothing */
#define INTERCOM_USER_WHO(p, str) IGNORE_PARAMETER(str)

#define INTERCOM_USER_TELL(p, name, str, len) /* do nothing */
#define INTERCOM_USER_REMOTE(p, name, str, len) /* do nothing */

#define INTERCOM_USER_LSU(p, str) /* do nothing */


extern void cmds_init_intercom(void);

#endif
