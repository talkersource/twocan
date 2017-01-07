#ifndef EXTERN_INTERCOM_H
#define EXTERN_INTERCOM_H

extern time_t now;

extern volatile sig_atomic_t intercom_panic;
extern volatile sig_atomic_t intercom_reboot;

extern void shutdown_error(const char *, ... )
    __attribute__ ((__format__ (printf, 1, 2))) __attribute__ ((__noreturn__));

#endif
