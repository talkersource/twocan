#define DL_LOAD_C
/*
 *  Copyright (C) 1999 James Antill
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * email: james@twocan.org
 */
#include "main.h"

#ifdef USE_DL_LOAD
static void *dl_libs[DL_MAX_LIBS];
static dl_slot dl_patches[DL_MAX_FUNCTIONS];
static int number_of_dl_libs = 0;
static int number_of_dl_functions = 0;

static int find_new_dl_ref(void)
{
 int tmp = 0;

 while ((tmp < DL_MAX_LIBS) && dl_libs[tmp])
   ++tmp;
 
 if (tmp == DL_MAX_LIBS)
   return (-1);
 
 return (tmp);
}

static int find_new_function_ref(void)
{
 int tmp = 0;

 while ((tmp < DL_MAX_FUNCTIONS) && dl_patches[tmp].function)
   ++tmp;
 
 if (tmp == DL_MAX_FUNCTIONS)
   return (-1);
 
 return (tmp);
}

static void user_dl_open(player *p, parameter_holder *params)
{
 int dl_ref = find_new_dl_ref();
 int xtra_flag = 0;
 
 if ((params->last_param != 1) && (params->last_param != 2))
   TELL_FORMAT(p, "<dl-name> [global]");
 
 if (dl_ref < 0)
 {
  fvtell_player(NORMAL_T(p), " Couldn't get a dl reference.\n");
  return;
 }

 if (params->last_param == 2)
 {
  if (TOGGLE_MATCH_ON(GET_PARAMETER_STR(params, 2)))
  {
   xtra_flag =
#ifdef HAVE_DL_RTLD_GLOBAL
     RTLD_GLOBAL
#else
     0
#endif
     ;
  }
  else if (TOGGLE_MATCH_OFF(GET_PARAMETER_STR(params, 2)))
         xtra_flag = 0;
  else
    TELL_FORMAT(p, "<dl-name> [on|off]");
 }
 
 if ((dl_libs[dl_ref] = dlopen(GET_PARAMETER_STR(params, 1),
                               RTLD_LAZY | xtra_flag)))
 {
  fvtell_player(NORMAL_T(p), " New dl library opened (reference %d).\n",
                dl_ref);
  ++number_of_dl_libs;
 }
 else
 {
  const char *tmp = dlerror();
  
  fvtell_player(NORMAL_T(p),
                " The dl library -- ^S^B%s^s -- couldn't be opened (%s).\n",
                GET_PARAMETER_STR(params, 1), tmp);
 }
}

static void user_dl_load_sym(player *p, parameter_holder *params)
{
 int dl_ref = 0;

 if (params->last_param != 2)
 {
  TELL_FORMAT(p, "<dl-reference> <function-name>");
 }
 
 if (*(GET_PARAMETER_STR(params, 1) +
       strspn(GET_PARAMETER_STR(params, 1), "0123456789")))
 {
  fvtell_player(SYSTEM_T(p),
                " The dl library reference needs to be an integer.\n");
  TELL_FORMAT(p, "<dl-reference> <function-name>");
 }

 dl_ref = atoi(GET_PARAMETER_STR(params, 1));
 
 if (!dl_libs[dl_ref])
   fvtell_player(SYSTEM_T(p),
                 " The dl library reference -- ^S^B%d^s -- doesn't exist.\n",
                 dl_ref);
 else
 {
  int func_ref = find_new_function_ref();
  const char *error_str = NULL;
  
  if (dl_ref < 0)
  {
   fvtell_player(NORMAL_T(p), " Couldn't get a dl function slot.\n");
   return;
  }

  dl_patches[func_ref].dl_ref = dl_ref;
  /* this doesn't follow strict ansi standards... blame the people who
     made the API for dlsym ... or the standards committe whichever */
  dl_patches[func_ref].function = (void (*) (player *, const char *))
    dlsym(dl_libs[dl_ref], GET_PARAMETER_STR(params, 2));
  /* TODO: make it able to do all types of user functions via 3rd argument */
  
  if ((error_str = dlerror()))
  {
   dl_patches[func_ref].function = NULL;
   fvtell_player(SYSTEM_T(p),
                 " The dl library sym produced the error: ^S^B%s^s.\n",
                 error_str);
  }
  else
  {
   fvtell_player(NORMAL_T(p), " New dl function made (%d).\n",
                 func_ref);
   ++number_of_dl_functions;
  }
 }
}

static void user_dl_unload_sym(player *p, parameter_holder *params)
{
 int func_ref = 0;

 if (params->last_param != 1)
   TELL_FORMAT(p, "<function-reference>");
 
 if (*(GET_PARAMETER_STR(params, 1) +
       strspn(GET_PARAMETER_STR(params, 1), "0123456789")))
 {
  fvtell_player(SYSTEM_T(p),
                " The dl library reference needs to be an integer.\n");
  TELL_FORMAT(p, "<function-reference>");
 }

 func_ref = atoi(GET_PARAMETER_STR(params, 1));
 
 if ((func_ref >= 0) && (func_ref < DL_MAX_FUNCTIONS) &&
     dl_patches[func_ref].function)
 {
  dl_patches[func_ref].function = NULL;

  fvtell_player(NORMAL_T(p), " dl function removed (%d).\n",
                func_ref);
  --number_of_dl_functions;
 }
 else
   fvtell_player(SYSTEM_T(p),
                 " The dl function reference -- ^S^B%d^s -- doesn't exist.\n",
                 func_ref);
}

static void user_dl_close(player *p, parameter_holder *params)
{
 int dl_ref = 0;

 if (params->last_param != 1)
   TELL_FORMAT(p, "<dl-reference>");
 
 if (*(GET_PARAMETER_STR(params, 1) +
       strspn(GET_PARAMETER_STR(params, 1), "0123456789")))
 {
  fvtell_player(SYSTEM_T(p),
                " The dl library reference needs to be an integer.\n");
  TELL_FORMAT(p, "<dl-reference>");
 }
   
 dl_ref = atoi(GET_PARAMETER_STR(params, 1));
 
 if (!dl_libs[dl_ref])
 {
  fvtell_player(SYSTEM_T(p),
                " The dl library reference -- ^S^B%d^s -- doesn't exist.\n",
                dl_ref);
 }
 else
 {
  int tmp = 0;
  
  dlclose(dl_libs[dl_ref]);
  dl_libs[dl_ref] = NULL;
  
  --number_of_dl_libs;

  while (tmp < DL_MAX_FUNCTIONS)
  {
   if (dl_patches[tmp].function && (dl_patches[tmp].dl_ref == dl_ref))
   {
    dl_patches[tmp].function = NULL;
    --number_of_dl_functions;
   }

   ++tmp;
  }

  fvtell_player(NORMAL_T(p), " The dl reference -- %d -- has been deleted.\n",
                dl_ref);  
 }
}

/* starts user_ even though it isn't really a user function...
   it's a gateway to lots of user functions... */
static void user_dl_patch(player *p, const char *str)
{
 int func_ref = 0;

 str += strspn(str, " ");
 func_ref = atoi(str);
 
 if ((func_ref >= 0) && (func_ref < DL_MAX_FUNCTIONS) &&
     dl_patches[func_ref].function)
 {
  str += strspn(str, "0123456789");
  str += strspn(str, " ");
  
  (*dl_patches[func_ref].function)(p, str);
 }
 else
   fvtell_player(NORMAL_T(p),
                 " The function -- %s -- is currently disabled.\n",
                 current_command);
}

static void user_dl_list(player *p, const char *str)
{
 if (!beg_strcasecmp(str, "functions"))
 {
  if (number_of_dl_functions)
  {
   int tmp = 0;
   
   fvtell_player(NORMAL_T(p),
                 " Here is a list of the currently loaded "
                 "dl functions (%d):\n", number_of_dl_functions);
   while (tmp < DL_MAX_FUNCTIONS)
   {
    if (dl_patches[tmp].function)
      fvtell_player(NORMAL_T(p), "     %d\n", tmp);
    ++tmp;
   }
  }
  else
   fvtell_player(NORMAL_T(p), " There are no dl functions loaded.\n"); 
 }
 else if (!beg_strcasecmp(str, "libs"))
 {
  if (number_of_dl_libs)
  {
   int tmp = 0;
   
   fvtell_player(NORMAL_T(p),
                 " Here is a list of the currently loaded dl libs (%d):\n",
                 number_of_dl_libs);
   while (tmp < DL_MAX_LIBS)
   {
    if (dl_libs[tmp])
      fvtell_player(NORMAL_T(p), "     %d\n", tmp);
    ++tmp;
   }
  }
  else
   fvtell_player(NORMAL_T(p), " There are no dl libs loaded.\n");
 }
 else
   TELL_FORMAT(p, "<\"functions\"|\"libs\">");
}

void cmds_init_dl(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("dl_close", user_dl_close, PARSE_PARAMS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(coder);
 CMDS_ADD("dl_list", user_dl_list, CONST_CHARS, ADMIN);
 CMDS_PRIV(coder);
 CMDS_ADD("dl_load_sym", user_dl_load_sym, PARSE_PARAMS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(coder);
 CMDS_ADD("dl_open", user_dl_open, PARSE_PARAMS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(coder);
 CMDS_ADD("dl_patch_cmd", user_dl_patch, CONST_CHARS, ADMIN);
 CMDS_ADD("dl_unload_sym", user_dl_unload_sym, PARSE_PARAMS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(coder);
}

#else

void cmds_init_dl(void)
{}

#endif
