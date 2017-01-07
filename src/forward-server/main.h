#ifndef MAIN_H
#define MAIN_H


#define _GNU_SOURCE 1

/* #define NDEBUG */

#include "../include/autoconf.h"

#include "../include/main_system.h"

#define MAX_LOG_SIZE (1024 * 1024 * 4)

#undef TRACE_INPUT_TO
#undef TRACE_TIMED

extern time_t now;

#define FIX_NAMESPACE_SYMBOL fwd_autoconf_
#include "fix.h"

#include "assert_loop.h"
#include "connect.h"
#include "display_time.h"
#include "log.h"
#include "safemalloc.h"
#include "structs.h"
#include "tools.h"
#include "trace.h"

#include "extern-assert_loop.h"
#include "extern-display_time.h"
#include "extern-log.h"
#include "extern-safemalloc.h"
#include "extern-tools.h"
#include "extern-trace.h"

#endif
