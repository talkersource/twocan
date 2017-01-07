#ifndef EXTERN_LOG_H
#define EXTERN_LOG_H

#define wlog_open(x) open_wlog(x)
#define wlog_close(x) close_wlog(x)
extern FILE *open_wlog(const char *);
extern void close_wlog(FILE *);
extern void vwlog(const char *, const char *, ...)
    __attribute__ ((__format__ (printf, 2, 3)));
#define wlog(x, y) vwlog(x, "%s\n", y)

extern void log_pid(const char *, int);

extern void init_log(void);

extern void cmds_init_log(void);

#endif
