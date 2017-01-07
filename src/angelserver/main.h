#ifndef MAIN_H
#define MAIN_H

#include "../include/autoconf.h"

#include "main_system.h"

#define FIX_NAMESPACE_SYMBOL angel_autoconf_

#include "fix.h"

/* for speed */
#undef TRACE_INPUT_TO
#undef TRACE_TIMED

#include "config.h"


#include "tools.h"

#include "angel.h"
#include "assert_loop.h"
#include "child_com.h"
#include "config.h"
#include "copy_str.h"
#include "disk.h"
#include "display_time.h"
#include "log.h"
#include "safemalloc.h"
#include "structs.h"
#include "trace.h"

#include "user_configure.h"

#include "extern-assert_loop.h"
#include "extern-child_com.h"
#include "extern-copy_str.h"
#include "extern-disk.h"
#include "extern-display_time.h"
#include "extern-log.h"
#include "extern-safemalloc.h"
#include "extern-tools.h"
#include "extern-toggle.h"
#include "extern-trace.h"

#include "extern-user_configure.h"

#endif
