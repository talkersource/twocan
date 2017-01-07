#ifndef MAIN_H
#define MAIN_H

#include "../include/autoconf.h"

#include "main_system.h"

#undef TRACE_INPUT_TO
#undef TRACE_TIMED

#define FIX_NAMESPACE_SYMBOL dns_autoconf_

#include "fix.h"


#include "assert_loop.h"
#include "tools.h"
#include "log.h"
#include "child_com.h"
#include "config.h"
#include "display_time.h"
#include "dns.h"
#include "safemalloc.h"
#include "structs.h"
#include "sys_rlim.h"
#include "trace.h"

#include "extern-assert_loop.h"
#include "extern-child_com.h"
#include "extern-display_time.h"
#include "extern-log.h"
#include "extern-safemalloc.h"
#include "extern-sys_rlim.h"
#include "extern-tools.h"
#include "extern-trace.h"

#endif
