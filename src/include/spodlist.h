#ifndef SPODLIST_H
#define SPODLIST_H

#define SPOD_CHECK_DEF_LEVEL 4
#define SPOD_CHECK_EXTERNAL (1<<0)

#ifdef SPODLIST_C
# define SPOD_CHECK_RECURSE_UP (1<<1)
# define SPOD_CHECK_RECURSE_DOWN (1<<2)
#endif

#endif
