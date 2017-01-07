#ifndef EXTERN_CHILD_COM_H
#define EXTERN_CHILD_COM_H

#define CHILD_COM_INIT(x) {x, 0, NULL, 0, 0, 0,0,0, 0, NULL, \
 FALSE, FALSE, ""}

#if 0
# define CHILD_COM_POLL(x, y) do { \
 if (((x)->io_indicator = (y))) { \
 (x)->io_indicator->fd = (x)->input; \
 (x)->io_indicator->events = POLLIN; \
 (x)->io_indicator->revents = 0; } } while (FALSE)
#else
# define CHILD_COM_POLL(x, y) do { \
 if (((x)->io_indicator = (y))) { \
 SOCKET_POLL_INDICATOR((x)->io_indicator)->fd = (x)->input; \
 SOCKET_POLL_INDICATOR((x)->io_indicator)->events = POLLIN; \
 SOCKET_POLL_INDICATOR((x)->io_indicator)->revents = 0; } } while (FALSE)
#endif

extern void child_com_open(child_com *, int, FILE *, pid_t);
extern child_com *child_com_create(const char *, char *[]);

extern void child_com_close(child_com *);
extern void child_com_delete(child_com *);


extern void child_com_send(child_com *, const char *fmt, ...)
    __attribute__((__format__(printf, 2, 3)));
extern int child_com_flush(child_com *);

extern size_t child_com_gather(child_com *, char);
extern size_t child_com_recv(child_com *, char *, size_t);

extern int child_com_waiting_input(child_com *, int);


#endif
