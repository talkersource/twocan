#ifndef MAIN_H
#define MAIN_H

#include "autoconf.h"

#include "main_system.h"

#define FIX_NAMESPACE_SYMBOL intercom_autoconf_

#include "fix.h"

#include "tools.h"

# define MALLOC(x) malloc(x)
# define REALLOC(x) realloc(x)
# define FREE(x) free(x)

# define BTRACE(x)

#include "assert_loop.h"
#include "log.h"
#include "display_time.h"
#if 0
# include "socket_poll.h"
# include "socket_com.h"
# include "socket_com_q.h"
# include "timer_q.h"
#endif

#include "glue_intercom.h"
#include "intercom.h"

#include "extern-assert_loop.h"
#include "extern-display_time.h"
#include "extern-intercom.h"
#include "extern-intercom_signals.h"
#include "extern-log.h"
#if 0
# include "extern-socket_com.h"
# include "extern-socket_com_q.h"
# include "extern-socket_poll.h"
# include "extern-timer_q.h"
#endif
#include "extern-tools.h"

#endif
