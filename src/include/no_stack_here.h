#ifndef NO_STACK_HERE_H
#define NO_STACK_HERE_H

/* this will kill on compile */
#define stack do { const int a = 1; a(++a++); } while (FALSE)

#endif
