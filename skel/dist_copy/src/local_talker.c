#define LOCAL_TALKER_C

#include "main.h"

/* This file is for all local additions to your talker, Ie. you get the
 * namespace of local_talker_*
 *
 * ... for instance if you want a new who put it in here and ...
 * write the function...
 *
 *   void user_local_talker_who(player *p, const char *str) { ... } 
 *
 * Then put this in the cmds_init_local_talker function
 *
 *   CMDS_OVERRIDE(&cmds_alpha[ALPHA_LOWER_OFFSET('w')], "who",
 *                 user_local_talker_who, CONST_CHARS);
 *
 *  If you wish to create new function just look at one of the other
 * source files for an example... (although you'll have to alter the values
 * in cmds_list.h)
 *
 */

void init_local_talker(void)
{
 /* call init functions for local_talker_*.c files */
}

void cmds_init_local_talker(void)
{
 /* call cmd_init functions for local_talker_*.c files */
}

