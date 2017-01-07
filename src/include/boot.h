#ifndef BOOT_H
#define BOOT_H

#ifndef INIT_DEBUG
# ifndef NDEBUG
#  define INIT_DEBUG 1
# else
#  define INIT_DEBUG 0
# endif
#endif

typedef struct system_flags
{
 volatile sig_atomic_t bad_panic; /* altered in sig handler */
 volatile sig_atomic_t panic; /* altered in sig handler */
 volatile sig_atomic_t backups_run; /* altered in sig handler */
 volatile sig_atomic_t reload_run; /* altered in sig handler */
 volatile sig_atomic_t shutdown; /* altered in sig handler */
} system_flags;

#endif
