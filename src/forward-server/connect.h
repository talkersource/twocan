#ifndef INCLUDE_CONNECT_H
#define INCLUDE_CONNECT_H

extern int connect_to_site(const char *const, int);

extern int send_to_socket(int, const char *, ...)
    __attribute__ ((__format__ (printf, 2, 3)));

extern const char *read_data(char *, size_t, int);
extern const char *read_data_maybe(char *, size_t, int);

#endif
