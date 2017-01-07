#define TMP_PATCH_C

#include "main.h"

#if 0
void _init (void);
void _fini (void);


void _init (void)
{
 printf("%s", "abcd.\n");
}

void _fini(void)
{
 printf("%s", "pop.\n");
}
#endif

extern void tmp_patch(player *, char *);

void tmp_patch(player *p, char *str)
{
 fvtell_player(NORMAL_T(p),
               " The current function -- %s -- is blob.\n",
               current_command);
}
