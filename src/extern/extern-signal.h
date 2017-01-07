#ifndef EXTERN_SIGNAL_H
#define EXTERN_SIGNAL_H

#ifndef HAVE_SIGCONTEXT

# define S_IGNAL_CRASH_INFO(x)
# define S_IGNAL_CRASH_PARAMS(num, con)  int num

#else

# ifdef S_IGNAL_INFO_LONG
#  define S_IGNAL_CRASH_INFO(x)  vwlog(x, \
       "\n gs            = %4hx, __gsh = %4hx" \
       "\n fs            = %4hx, __fsh = %4hx" \
       "\n es            = %4hx, __esh = %4hx" \
       "\n ds            = %4hx, __dsh = %4hx" \
       "\n edi           = %8lx" \
       "\n esi           = %8lx" \
       "\n ebp           = %8lx" \
       "\n esp           = %8lx" \
       "\n ebx           = %8lx" \
       "\n edx           = %8lx" \
       "\n ecx           = %8lx" \
       "\n eax           = %8lx" \
       "\n trapno        = %8lu" \
       "\n err           = %8lx" \
       "\n eip           = %8lx" \
       "\n cs            = %4hx, __csh = %4hx" \
       "\n eflags        = %8lx" \
       "\n esp_at_signal = %8lx" \
       "\n ss            = %4hx, __ssh = %4hx" \
       "\n fp_stat       = %8lx" \
       "\n oldmask       = %8lx" \
       "\n cr2 = %8lx", \
       (unsigned short) con.gs, (unsigned short) con.__gsh, \
       (unsigned short) con.fs, (unsigned short) con.__fsh, \
       (unsigned short) con.es, (unsigned short) con.__esh, \
       (unsigned short) con.ds, (unsigned short) con.__dsh, \
       (unsigned long) con.edi, \
       (unsigned long) con.esi, \
       (unsigned long) con.ebp, \
       (unsigned long) con.esp, \
       (unsigned long) con.ebx, \
       (unsigned long) con.edx, \
       (unsigned long) con.ecx, \
       (unsigned long) con.eax, \
       (unsigned long) con.trapno, \
       (unsigned long) con.err, \
       (unsigned long) con.eip, \
       (unsigned short) con.cs, (unsigned short) con.__csh, \
       (unsigned long) con.eflags, \
       (unsigned long) con.esp_at_signal, \
       (unsigned short) con.ss, (unsigned short) con.__ssh, \
       (unsigned long) con.fpstate ? con.fpstate->status : 0, \
       (unsigned long) con.oldmask, \
       (unsigned long) con.cr2)

# else
    /* short version... */
#  define S_IGNAL_CRASH_INFO(x)  vwlog(x, " eip = %lx\n cr2 = %lx", \
       (unsigned long) con.eip, \
       (unsigned long) con.cr2)
# endif
    
# define S_IGNAL_CRASH_PARAMS(num, con) int num , struct sigcontext con
#endif

#ifdef S_IGNAL_C
# define S_IGNAL_SETUP(x)  do { \
 if (sigaction(x, &sa, NULL) == -1) \
 vwlog("error", " ** Error: sigaction(%s)\n", #x); } while (FALSE)
#endif
    
extern void init_signals(void);
    
#endif
