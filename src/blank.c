#define BLANK_C
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


static void user_su_blank(player *p, const char *str)
{
 const char *blanked_type_name = NULL;
 unsigned int blanked_type_len = 1;
 char *blanked_type_it = NULL;
 int blanked_type_check = TRUE;
 player *p2 = NULL;
 parameter_holder params;

 get_parameter_init(&params);

 CHECK_DUTY(p);
 
 if (!get_parameter_parse(&params, &str, 2))
   goto show_format;

 if (!(p2 = player_find_load(p, GET_PARAMETER_STR(&params, 1),
                             PLAYER_FIND_SC_SU_ALL)))
   return;
 
 lower_case(GET_PARAMETER_STR(&params, 2));
 
 switch (*GET_PARAMETER_STR(&params, 2))
 {
 case 'c':
  if (*(GET_PARAMETER_STR(&params, 2) + 1) == 'o')
  {
   if (*(GET_PARAMETER_STR(&params, 2) + 2) == 'm')
   {
    if (beg_strcmp(GET_PARAMETER_STR(&params, 2), "comment"))
      break;
    
    str = ""; /* FIXME: user_configuyre entry for whether to do it */
    blanked_type_name = "Comment";
    blanked_type_len = PLAYER_S_COMMENT_SZ;
    blanked_type_it = p2->comment;
    break;
   }
   else
   {
    if (beg_strcmp(GET_PARAMETER_STR(&params, 2), "connect_msg") &&
        beg_strcmp(GET_PARAMETER_STR(&params, 2), "connect_message") &&
        beg_strcmp(GET_PARAMETER_STR(&params, 2), "connect msg") &&
        beg_strcmp(GET_PARAMETER_STR(&params, 2), "connect message"))
      break;

    str = "";
    blanked_type_name = "Connect message";
    blanked_type_len = PLAYER_S_CONNECT_MSG_SZ;
    blanked_type_it = p2->connect_msg;
   }
  }
  break;
  
 case 'd':
  if (*(GET_PARAMETER_STR(&params, 2) + 1) == 'e')
  {
   if (beg_strcmp(GET_PARAMETER_STR(&params, 2), "description"))
     break;

   str = "";
   blanked_type_name = "Description";
   blanked_type_len = PLAYER_S_DESCRIPTION_SZ;
   blanked_type_it = p2->description;
  }
  else
  {
   if (beg_strcmp(GET_PARAMETER_STR(&params, 2), "disconnect_msg") &&
       beg_strcmp(GET_PARAMETER_STR(&params, 2), "disconnect_message") &&
       beg_strcmp(GET_PARAMETER_STR(&params, 2), "disconnect msg") &&
       beg_strcmp(GET_PARAMETER_STR(&params, 2), "disconnect message"))
     break;

   str = "";
   blanked_type_name = "Disconnect message";
   blanked_type_len = PLAYER_S_DISCONNECT_MSG_SZ;
   blanked_type_it = p2->disconnect_msg;
  }
  break;
  
 case 'e':
   if (*(GET_PARAMETER_STR(&params, 2) + 1) == 'm')
   {
    if (beg_strcmp(GET_PARAMETER_STR(&params, 2), "email"))
      break;
    
    blanked_type_name = "Email";
    blanked_type_len = PLAYER_S_EMAIL_SZ;
    blanked_type_it = p2->email;
   }
   else
   {
    if (beg_strcmp(GET_PARAMETER_STR(&params, 2), "enter_msg") &&
        beg_strcmp(GET_PARAMETER_STR(&params, 2), "enter_message") &&
        beg_strcmp(GET_PARAMETER_STR(&params, 2), "enter msg") &&
        beg_strcmp(GET_PARAMETER_STR(&params, 2), "enter message"))
      break;

    str = "";
    blanked_type_name = "Enter message";
    blanked_type_len = PLAYER_S_ENTER_MSG_SZ;
    blanked_type_it = p2->enter_msg;
   }
   break;
    
 case 'i':
   if (beg_strcmp(GET_PARAMETER_STR(&params, 2), "ignore_msg") &&
       beg_strcmp(GET_PARAMETER_STR(&params, 2), "ignore_message") &&
       beg_strcmp(GET_PARAMETER_STR(&params, 2), "ignore msg") &&
       beg_strcmp(GET_PARAMETER_STR(&params, 2), "ignore message"))
     break;

   str = "";
   blanked_type_name = "Ignore message";
   blanked_type_len = PLAYER_S_IGNORE_MSG_SZ;
   blanked_type_it = p2->ignore_msg;
   break;

 case 'p':
  if (*(GET_PARAMETER_STR(&params, 2) + 1) == 'a')
  {
   if (beg_strcmp(GET_PARAMETER_STR(&params, 2), "password") &&
       beg_strcmp(GET_PARAMETER_STR(&params, 2), "passwd"))
     break;
   
   blanked_type_name = "Password";
   blanked_type_len = PLAYER_S_PASSWD_SZ;
   blanked_type_it = p2->passwd;
  }
  else if (*(GET_PARAMETER_STR(&params, 2) + 1) == 'l')
  {
   if (beg_strcmp(GET_PARAMETER_STR(&params, 2), "plan"))
     break;

   str = "";
    blanked_type_name = "Plan";
    blanked_type_len = PLAYER_S_PLAN_SZ;
    blanked_type_it = p2->plan;
  }
  else
  {
   if (beg_strcmp(GET_PARAMETER_STR(&params, 2), "prefix"))
     break;

   str = "";
    blanked_type_name = "Prefix";
    blanked_type_len = PLAYER_S_PREFIX_SZ;
    blanked_type_it = p2->prefix;
  }
  break;

 case 't':
   if (beg_strcmp(GET_PARAMETER_STR(&params, 2), "title"))
     break;

   str = "";
   blanked_type_name = "Title";
   blanked_type_len = PLAYER_S_TITLE_SZ;
   blanked_type_it = p2->title;
   break;

 case 'u':
   if (beg_strcmp(GET_PARAMETER_STR(&params, 2), "url"))
     break;

   str = "";
   blanked_type_name = "Url";
   blanked_type_len = PLAYER_S_URL_SZ;
   blanked_type_it = p2->url;
   break;

  case 's':
   if (beg_strcmp(GET_PARAMETER_STR(&params, 2), "su_comment") &&
       beg_strcmp(GET_PARAMETER_STR(&params, 2), "su comment"))
     break;
   
   blanked_type_name = "Su Comment";
   blanked_type_len = PLAYER_S_SU_COMMENT_SZ;
   blanked_type_it = p2->su_comment;
   blanked_type_check = FALSE;
   break;
   
  default:
    ; /* shouldn't get here - do nothing */
 }

 if (!blanked_type_name)
 {
 show_format: /* if you don't enter enough params */
  fvtell_player(NORMAL_T(p), "%s%s%s", " Blankable strings are:\n"
                "   comment, connect message, description, "
                "disconnect message, email, enter message, ignore message, "
                "password, plan, prefix, title, url",
                PRIV_STAFF(p->saved) ? ", su comment" : "", ".\n");
  TELL_FORMAT(p, "<player> <type> [string]");
 }
 
 if (blanked_type_check && (priv_test_check(p->saved, p2->saved) < 1))
 {
  fvtell_player(SYSTEM_T(p),
                " The player -- ^S^B%s^s -- has enough "
                "privileges that you cannot blank their -- ^S^B%s^s --.\n",
                p2->saved->name, blanked_type_name);
  
  if (P_IS_ON(p2->saved))
    fvtell_player(SYSTEM_T(p2),
		  " -=> %s%s tried to ^S^Bblank your %s^s, but failed.\n",
                  gender_choose_str(p->gender, "", "", "The ", "The "),
                  p->saved->name, blanked_type_name);

  vwlog("blanked", "%s tried to %s %s's %s (%s).", p->saved->name,
        str[0] ? "changed" : "blanked", p2->saved->name,
        blanked_type_name, blanked_type_it);
  return;
 }

 /* Woo hoo ... major hacks */
 if (p2->su_comment == blanked_type_it)
 {
  if (p == p2)
  {
   fvtell_player(SYSTEM_T(p), "%s",
                 " You cannot blank you're own su comment.\n");
   return;
  }
  
#if PLAYER_S_SU_COMMENT_SZ <= PLAYER_S_NAME_SZ
# error "This doesn't work"
#endif
  if (*str)
  {
   char buf[sizeof("$Date(%s)") + 256];
   size_t len = sprintf(buf, "$Date(%s)", disp_time_cmp_string(now));
   sprintf(p2->su_comment, "%s %s: %.*s", p->saved->name, buf,
           (PLAYER_S_SU_COMMENT_SZ - 4) - PLAYER_S_NAME_SZ - (int)len, str);
   goto after_normal_blank_copy;
  }
 }
 else if (p2->passwd == blanked_type_it)
 {
  if (!*str)
  {
   fvtell_player(SYSTEM_T(p), "%s",
                 " You have to supply a value for someone's passwd.\n");   
   return;
  }
  p2->flag_raw_passwd = TRUE;
 }

 COPY_STR(blanked_type_it, str, blanked_type_len);
 
 p2->saved->flag_tmp_player_needs_saving = TRUE;

 after_normal_blank_copy:
 fvtell_player(SYSTEM_T(p),
               " The player '^S^B%s^s' has had their '^S^B%s^s' blanked",
               p2->saved->name, blanked_type_name);

 if (blanked_type_it[0])
   fvtell_player(SYSTEM_T(p), ", with '^S^B%s^s'.\n", blanked_type_it);
 else
   fvtell_player(SYSTEM_T(p), "%s", ".\n");
 
 if (P_IS_ON_P(p2->saved, p2))
   fvtell_player(NORMAL_T(p2), " -=> You have had your %s string %s, (%s).\n",
                 blanked_type_name,
                 str[0] ? "changed" : "blanked", blanked_type_it);
 
 vwlog("blanked", " %s %s %s's %s string (%s).\n", p->saved->name,
       str[0] ? "changed" : "blanked", p2->saved->name,
       blanked_type_name, blanked_type_it);
}

void cmds_init_blank(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("blank", user_su_blank, CONST_CHARS, SU);
 CMDS_PRIV(normal_su);
}
