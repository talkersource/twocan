#define COPY_STR_C
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


static void user_copy_str_title(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_ISITS(p->title, str, "title",
                           PLAYER_S_TITLE_SZ, OUT_LENGTH_TITLE);
}

static void user_copy_str_prefix(player *p, const char *str)
{
 const char *tmp = str;
 int use_quotes = FALSE;

 if (player_tree_find_exact(str) || player_newbie_find_exact(str))
   use_quotes = TRUE;

 while (*tmp && !use_quotes)
 {
  if (!isalnum((unsigned char) *tmp))
    use_quotes = TRUE;
  ++tmp;
 }
 
 if (use_quotes)
   sprintf(p->prefix, "%c%.*s%c", '"', PLAYER_S_PREFIX_SZ - 3, str, '"');
 else
   COPY_STR(p->prefix, str, PLAYER_S_PREFIX_SZ);

 fvtell_player(NORMAL_T(p),
               " You change your %sprefix%s string to:\n",
               "^S^B", "^s");

 fvtell_player(NORMAL_FT(RAW_OUTPUT, p), " %s %s\n",
               p->prefix, p->saved->name);
}

static void user_copy_str_phone_numbers(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_RAW(p->phone_numbers, str, "phone numbers",
                         PLAYER_S_TITLE_SZ);
 
 fvtell_player(NORMAL_T(p), "%s",
               " -=> These are only available to your friends atm.\n");
}

static void user_copy_str_description(player *p, const char *str)
{
 twinkle_info info;
 
 setup_twinkle_info(&info);
 
 info.returns_limit = OUT_RETURNS_INFO;
 COPY_STR(p->description, str, PLAYER_S_DESCRIPTION_SZ);

 if (!p->description[0])
 {
  fvtell_player(NORMAL_T(p), " You blank your %sdescription%s string.\n",
                "^S^B", "^s");
  return;
 } 

 fvtell_player(NORMAL_T(p),
               " You change your %sdescription%s string to:\n",
               "^S^B", "^s");

 fvtell_player(ALL_T(p->saved, p, &info, 0, now), " %.*s^N\n",
               OUT_LENGTH_INFO, p->description);
}

static void user_copy_str_plan(player *p, const char *str)
{
 twinkle_info info;
 
 setup_twinkle_info(&info);
 
 info.returns_limit = OUT_RETURNS_INFO;
 COPY_STR(p->plan, str, PLAYER_S_PLAN_SZ);
 
 if (!p->plan[0])
 {
  fvtell_player(NORMAL_T(p), " You blank your %splan%s string.\n",
                "^S^B", "^s");
  return;
 } 

 fvtell_player(NORMAL_T(p),
               " You change your %splan%s string to:\n",
               "^S^B", "^s");
 fvtell_player(ALL_T(p->saved, p, &info, 0, now), " %.*s^N\n",
               OUT_LENGTH_INFO, p->plan);
}

static void user_copy_str_url(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_NORM(p->url, str, "url", PLAYER_S_URL_SZ,
                          OUT_LENGTH_INFO);
}

static void user_copy_str_connect_msg(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_ISITS(p->connect_msg, str, "connect message",
                           PLAYER_S_CONNECT_MSG_SZ,
                           OUT_LENGTH_CONNECT_MSGS);
}

static void user_copy_str_disconnect_msg(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_ISITS(p->disconnect_msg, str, "disconnect message",
                           PLAYER_S_DISCONNECT_MSG_SZ,
                           OUT_LENGTH_CONNECT_MSGS);
}

static void user_copy_str_email(player *p, const char *str)
{
 if (!*str)
 {
  p->email[0] = 0;
  fvtell_player(NORMAL_T(p), "%s",
                " You have blanked your email address.\n"
                " This means that we cannot contact you in anyway.\n"
                "  Ie. If you lose your password.\n");
 }
 else if (!beg_strcasecmp(str, "private"))
 {
  p->saved->flag_private_email = TRUE;
  fvtell_player(NORMAL_T(p), "%s", 
                " Your email is private, only the Admin will be able "
                "to see it.\n");
 }
 else if (!beg_strcasecmp(str, "public"))
 {
  p->saved->flag_private_email = FALSE;
  fvtell_player(NORMAL_T(p), "%s", 
                " Your email address is public, so everyone can see "
                "it.\n");
 }
 else if (!email_validate_player(p, str))
 {
  COPY_STR(p->email, str, PLAYER_S_EMAIL_SZ);
  
  fvtell_player(NORMAL_FT(RAW_OUTPUT, p), 
                " Email address has been changed to: %s\n", p->email);
 }
}

static void user_copy_str_ignore_msg(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_RAW(p->ignore_msg, str, "ignore message",
                         PLAYER_S_IGNORE_MSG_SZ);
}

static void user_copy_str_sig_short(player *p, const char *str)
{
 twinkle_info info;
 
 setup_twinkle_info(&info);
 
 info.returns_limit = OUT_RETURNS_INFO;
 COPY_STR(p->sig, str, PLAYER_S_SIG_SZ);
 
 if (!p->sig[0])
 {
  fvtell_player(NORMAL_T(p), " You blank your %ssignature%s string.\n",
                "^S^B", "^s");
  return;
 } 

 fvtell_player(NORMAL_T(p),
               " You change your %ssignature%s string to:\n",
               "^S^B", "^s");
 fvtell_player(ALL_T(p->saved, p, &info, 0, now), " %.*s^N\n",
               OUT_LENGTH_INFO, p->sig);
}

void cmds_init_copy_str(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("connect_msg", user_copy_str_connect_msg, CONST_CHARS,
          PERSONAL_INFO);
 CMDS_XTRA_SECTION(LOCAL);

 CMDS_ADD("description", user_copy_str_description, CONST_CHARS,
          PERSONAL_INFO);
 CMDS_FLAG(no_beg_space);

 CMDS_ADD("disconnect_msg", user_copy_str_disconnect_msg, CONST_CHARS,
          PERSONAL_INFO);
 CMDS_XTRA_SECTION(LOCAL);
 
 CMDS_ADD("email", user_copy_str_email, CONST_CHARS, PERSONAL_INFO);

 CMDS_ADD("ignoremsg", user_copy_str_ignore_msg, CONST_CHARS, PERSONAL_INFO);
 CMDS_PRIV(command_list);
 
 CMDS_ADD("phone_numbers", user_copy_str_phone_numbers, CONST_CHARS,
          PERSONAL_INFO);
 
 CMDS_ADD("plan", user_copy_str_plan, CONST_CHARS, PERSONAL_INFO);
 CMDS_FLAG(no_beg_space);
 CMDS_ADD("prefix", user_copy_str_prefix, CONST_CHARS, PERSONAL_INFO);
 CMDS_FLAG(no_beg_space); CMDS_FLAG(no_end_space);
 
 CMDS_ADD("signature", user_copy_str_sig_short, CONST_CHARS, PERSONAL_INFO);
 
 CMDS_ADD("title", user_copy_str_title, CONST_CHARS, PERSONAL_INFO);
 CMDS_FLAG(no_beg_space);
 
 CMDS_ADD("url", user_copy_str_url, CONST_CHARS, PERSONAL_INFO);
}
