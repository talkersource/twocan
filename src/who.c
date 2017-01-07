#define WHO_C
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


static int construct_title_name_list_do(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);

 fvtell_player(INFO_T(scan->saved, p), "%s%s%.*s^N\n",
               scan->saved->name, isits1(scan->title),
               OUT_LENGTH_TITLE, isits2(scan->title));
 
 return (TRUE);
}

static void user_title_who(player *p)
{ 
 if (current_players == 1)
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " There are ^S^Bno other^s players logged "
                "on, at the moment.\n");
  return;
 }

 ptell_mid(INFO_TP(p), "^S^B$Current_players-Tostr_cap people^s on", FALSE);

 DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
               (construct_title_name_list_do, p));

 fvtell_player(INFO_TP(p), "%s", DASH_LEN);
 
 pager(p, PAGER_DEFAULT);
}

static void user_default_who(player *p, const char *str)
{
 INTERCOM_USER_WHO(p, str);
 
 if (p->flag_session_in_who)
   user_session_show(p);
 else
   user_title_who(p);
}

void user_short_who(player *p, const char *str)
{
 int count = 0;
 int out_flags = 0;
 
 if (!current_players)
 { /* for login swho */
  fvtell_player(SYSTEM_T(p), "%s", " There are ^S^Bno^s players logged on, "
                "at the moment.\n");
  return;
 }
 
 if (current_players == 1)
   if (p->is_fully_on)
   {
    fvtell_player(SYSTEM_T(p), "%s",
                  " There are ^S^Bno other^s players logged "
                  "on, at the moment.\n");
    return;
   }
   else /* for login swho */
     ptell_mid(NORMAL_T(p), "^S^BOne player^s on", FALSE);
 else
   ptell_mid(NORMAL_T(p),
             "^S^B$Current_players-Tostr_cap players^s on", FALSE);
 
 DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
               (construct_name_list_do,
                current_players, &count,
                ((str && (*str == '-')) ? 0 : CONSTRUCT_NAME_USE_PREFIX),
                (priv_test_list_type)NULL, (priv_test_list_type)NULL,
                &out_flags, NORMAL_FT(RAW_OUTPUT, p)));
 
 if (out_flags & CONSTRUCT_NAME_OUT_MID_LINE)
   fvtell_player(NORMAL_FT(RAW_OUTPUT, p), "%s", "\n");

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);

 pager(p, PAGER_DEFAULT);
}

void user_friend_who(player *p, const char *str)
{
 tmp_output_list_storage tmp_save;
 int count = 0;
 int out_flags = 0;
 
 if (!current_players)
 {
  log_assert(FALSE);
  fvtell_player(SYSTEM_T(p), "%s", " ** Error: swho\n");
  return;
 }
 
 if (current_players == 1)
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " There are ^S^Bno other^s players logged "
                "on, at the moment.\n");
  return;
 }

 save_tmp_output_list(p, &tmp_save);
 DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
               (construct_name_list_do,
                current_players, &count,
                ((str && (*str == '-')) ? 0 : CONSTRUCT_NAME_USE_PREFIX) |
                CONSTRUCT_NAME_NO_ME | CONSTRUCT_NAME_USE_LIST_ENT_ME,
                list_self_priv_test_friend, (priv_test_list_type)NULL,
                &out_flags, NORMAL_FT(OUTPUT_BUFFER_TMP | RAW_OUTPUT, p)));

 if (out_flags & CONSTRUCT_NAME_OUT_MID_LINE)
   fvtell_player(NORMAL_FT(RAW_OUTPUT | OUTPUT_BUFFER_TMP, p), "%s", "\n");
 
 if (!count)
 {
  output_node *tmp = output_list_grab(p);
  
  fvtell_player(SYSTEM_T(p), "%s",
                " You have ^S^Bno friends^s logged on, at the moment.\n");
  output_list_cleanup(&tmp);
  load_tmp_output_list(p, &tmp_save);
  return;
 }
 else
 {
  output_node *tmp = output_list_grab(p);
  load_tmp_output_list(p, &tmp_save);
  
  if (count == 1)
    ptell_mid(NORMAL_T(p), "^S^BOne friend^s on", FALSE);
  else
  {
   char buffer[BUF_NUM_TYPE_SZ(int) +
              sizeof("^S^B$Num(%d)-Tostr_cap friends^s on")];

   sprintf(buffer, "^S^B$Num(%d)-Tostr_cap friends^s on", count);
   ptell_mid(NORMAL_T(p), buffer, FALSE);
  }
  output_list_linkin(p, 3, &tmp, INT_MAX);
  fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 }

 pager(p, PAGER_DEFAULT);
}

static void user_toggle_info_in_who(player *p, const char *str)
{
 TOGGLE_COMMAND_OFF_ON(p, str, p->flag_no_info_in_who, TRUE,
                       " You will not %ssee whether people have colour/wands "
                       "in the who command.\n",
                       " You will %ssee whether people have colour/wands "
                       "in the who command.\n", TRUE);
}

void cmds_init_who(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("fwho", user_friend_who, CONST_CHARS, INFORMATION);
 CMDS_PRIV(base);
 
 CMDS_ADD("swho", user_short_who, CONST_CHARS, INFORMATION);
 CMDS_ADD("twho", user_title_who, NO_CHARS, INFORMATION);
 
 CMDS_ADD("who", user_default_who, CONST_CHARS, INFORMATION);
 CMDS_ADD("who_info", user_toggle_info_in_who, CONST_CHARS, SETTINGS);
}
