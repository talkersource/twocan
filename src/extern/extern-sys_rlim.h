#ifndef EXTERN_SYS_RLIM_H
#define EXTERN_SYS_RLIM_H

#ifdef HAVE_SETRLIMIT
# define SYS_RLIM_CHANGE(x, y) do { \
 struct rlimit tmp; \
 if (getrlimit(x, &tmp) == -1) { \
  vwlog("error", " Error: getrlimit(%s): %d %s", #x, errno, strerror(errno)); \
  return; } \
 tmp.rlim_cur = y; \
 if (setrlimit(x, &tmp) == -1) { \
  vwlog("error", " Error: setrlimit(%s): %d %s", #x, errno, strerror(errno)); \
  return; } } while (FALSE)
#else
# define SYS_RLIM_CHANGE(x, y) /* do nothing */
#endif

extern void init_sys_rlim(void);

#endif
