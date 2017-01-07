#ifndef MAIN_H
#define MAIN_H

#include "autoconf.h"

#undef _GNU_SOURCE
#define _GNU_SOURCE 1

#include "main_system.h"

#define FIX_NAMESPACE_SYMBOL timer_q_autoconf_

#include "fix.h"

#include "assert_loop-def.h"
#include "assert_loop-extern.h"

#include "timer_q.h"

extern int timer_q__walk_base_empty;


#include "tools-def.h"
#include "tools-extern.h"

#endif
