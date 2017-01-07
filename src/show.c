#define SHOW_C
/*
 *  Copyright (C) 1999 James Antill, John Tobin
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
 * email: james@twocan.org, john@twocan.org
 */
#include "main.h"


static int show_flag_func_personal(player *p, const char *str)
{ SHOW_FLAG_FUNC_ON_OFF(personal, "personal"); }
static int show_flag_func_shouts(player *p, const char *str)
{ SHOW_FLAG_FUNC_ON_OFF(shouts, "shouts"); }
static int show_flag_func_room(player *p, const char *str)
{ SHOW_FLAG_FUNC_ON_OFF(room, "room messages"); }
static int show_flag_func_autos(player *p, const char *str)
{ SHOW_FLAG_FUNC_ON_OFF(autos, "autos"); }
static int show_flag_func_socials(player *p, const char *str)
{ SHOW_FLAG_FUNC_ON_OFF(socials, "socials"); }
static int show_flag_func_echo(player *p, const char *str)
{ SHOW_FLAG_FUNC_ON_OFF(echo, "echos"); }

static int show_flag_func_all(player *p, const char *str)
{
 if (!str)
   return (p->flag_show_personal && p->flag_show_shouts && p->flag_show_room &&
           p->flag_show_autos && p->flag_show_socials && p->flag_show_echo);
 
 if (TOGGLE_MATCH_ON(str))
 {
  if (p)
    fvtell_player(NORMAL_T(p), " Set ^S^Ball^s show flags.\n");
  p->flag_show_personal = p->flag_show_shouts = p->flag_show_room =
    p->flag_show_autos = p->flag_show_socials = p->flag_show_echo = TRUE;
 }
 else if (TOGGLE_MATCH_OFF(str))
 {
  if (p)
    fvtell_player(NORMAL_T(p), " Removed ^S^Ball^s show flags.\n");
  p->flag_show_personal = p->flag_show_shouts = p->flag_show_room =
    p->flag_show_autos = p->flag_show_socials = p->flag_show_echo = FALSE;
 }
 else if (TOGGLE_MATCH_TOGGLE(str))
 {
  if (p)
    fvtell_player(NORMAL_T(p), " Done nothing, can't toggle all flags.\n");
 }
 else
 {
  assert(FALSE);
  return (-1);
 }
 
 return (p->flag_show_personal && p->flag_show_shouts && p->flag_show_room &&
         p->flag_show_autos && p->flag_show_socials && p->flag_show_echo);
}


static struct
{
 const char *name;
 int (*func)(player *, const char *);
 bitflag dup : 1;
} show_flags[] =
{
 {"all", show_flag_func_all, FALSE},
 {"autos", show_flag_func_autos, FALSE},
 {"echos", show_flag_func_echo, FALSE},
 {"local", show_flag_func_room, TRUE},
 {"personal", show_flag_func_personal, TRUE},
 {"room", show_flag_func_room, TRUE},
 {"says", show_flag_func_room, FALSE},
 {"shouts", show_flag_func_shouts, FALSE},
 {"socials", show_flag_func_socials, FALSE},
 {"tells", show_flag_func_personal, FALSE},
 {NULL, NULL, TRUE}
};
#define SHOW_FLAG_SZ ((sizeof(show_flags) / sizeof(show_flags[0])) - 1)

static int show_flags_get_offset(const char *str)
{
 int count = 0;

 while (show_flags[count].name)
 {
  int save_cmp = 0;
  
  assert(show_flags[count].func);
  if (!(save_cmp = beg_strcmp(str, show_flags[count].name)))
    return (count);
  else if (save_cmp < 0)
    return (-1);

  ++count;
 }

 return (-1);
}

static void show_flag_change_pre(player *p, parameter_holder *params,
                                 int *flag_offsets, size_t *flag_offset_count)
{
 char *flags = NULL;

 if ((params->last_param != 2) || !*GET_PARAMETER_STR(params, 1))
   TELL_FORMAT(p, "<flag(s)> [on|off|toggle]");

 assert(*GET_PARAMETER_STR(params, 2));
 lower_case(GET_PARAMETER_STR(params, 1));

 flags = GET_PARAMETER_STR(params, 1);
 while (flags)
 {
  int show_flag_offset = -1;
  char *tmp = next_parameter(flags, ',');

  if (tmp)
    *tmp++ = 0;
  
  if ((show_flag_offset = show_flags_get_offset(flags)) == -1)
  {
   *flag_offset_count = 0;
   fvtell_player(NORMAL_T(p),
                 " Bad show flag -- ^S^B%s^s -- no changes made.\n", flags);
   return;
  }

  flag_offsets[*flag_offset_count] = show_flag_offset;
  ++*flag_offset_count;

  if (*flag_offset_count >= SHOW_FLAG_SZ)
  {
   *flag_offset_count = 0;
   fvtell_player(NORMAL_T(p), "%s", " Too many show flags no changes made.\n");
   return;
  }
  
  flags = tmp;
 }
}

static void user_set_show_flags(player *p, parameter_holder *params)
{
 int flag_offsets[SHOW_FLAG_SZ];
 size_t flag_offset_count = 0;
 size_t count = 0;
 const char *cun_toggle = "toggle";
 const char *cun_on = "on";
 const char *cun_off = "off";
 int ret = 0;
 
 switch (params->last_param)
 {
  case 1:
  {
   switch (*GET_PARAMETER_STR(params, 1))
   {
    case '=':
    case '+':
      ++GET_PARAMETER_STR(params, 1);
      --GET_PARAMETER_LENGTH(params, 1);
      get_parameter_parse(params, &cun_on, 2);
      break;
    case '-':
      ++GET_PARAMETER_STR(params, 1);
      --GET_PARAMETER_LENGTH(params, 1);
      get_parameter_parse(params, &cun_off, 2);
      break;
    case '~':
      ++GET_PARAMETER_STR(params, 1);
      --GET_PARAMETER_LENGTH(params, 1);
      
    default:
      get_parameter_parse(params, &cun_toggle, 2);
   }
  }

  default:
  {
   show_flag_change_pre(p, params, flag_offsets, &flag_offset_count);
   if (!flag_offset_count)
     return;
   
   while ((ret != -1) && (count < flag_offset_count))
   {
    ret = (*show_flags[flag_offsets[count++]].func)(p,
                                                 GET_PARAMETER_STR(params, 2));
   }
  }
  break;

  case 0:
    ptell_mid(NORMAL_T(p), "Show tag flags", TRUE);
    while (show_flags[count].name)
    {
     if (!show_flags[count].dup)
       fvtell_player(NORMAL_T(p), "%s: ^S^B%s^s\n", show_flags[count].name,
                     (*show_flags[count].func)(p, NULL) ? "YES" : " NO");
     ++count;
    }
    fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
    break;
 }
}

void cmds_init_show(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("show", user_set_show_flags, PARSE_PARAMS, SETTINGS);
}
