#ifndef DL_LOAD_H
#define DL_LOAD_H

#ifdef DL_LOAD_C

#define DL_MAX_LIBS 16
#define DL_MAX_FUNCTIONS 64 /* may be worth having more ? */

typedef struct
{
 /* FIXME: function types out of cmds_list.h */
 void (*function)(player *, const char *);
 int dl_ref;
} dl_slot;

#endif


#endif
