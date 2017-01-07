#define PASSWD_C
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

int passwd_check(player *p, const char *entered)
{
 player_load(p->saved);
#ifdef USE_CRYPT_MIGRATION
 if (!p->saved->player_ptr->flag_raw_passwd)
 {
  char key[9];
  char *tmp;
  
  COPY_STR(key, entered, sizeof(key));
  
  tmp = crypt(key, p->saved->lower_name);
  
  return (!strncmp(tmp, p->saved->player_ptr->passwd, 11));
 }
 else
#endif
 {
  p->saved->player_ptr->flag_raw_passwd = TRUE;
  /* you can enter somthing bigger than what is saved...
   * Because you might have entered a 20 char password and
   * we don't save all of it */
  return (!beg_strcmp(p->saved->player_ptr->passwd, entered));
 }
}

static void passwd_cleanup(player *p)
{
 telopt_ask_passwd_mode_off(p);
}

static void passwd_cmd1(player *, const char *);
static void passwd_cmd2(player *p, const char *str)
{
 ICTRACE("passwd_cmd2");
 
 fvtell_player(NORMAL_T(p), "%s", "\n");
 
 assert(MODE_IN_MODE(p, PASSWD_3));

 if (beg_strcmp(p->passwd_change_tmp, str))
 {
  cmds_function tmp_cmd;

  CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, passwd_cmd1);

  fvtell_player(NORMAL_T(p), "%s", 
                "\n The passwords entered do not match.\n"
                " Password not changed.\n");
  
  mode_change(p, " Enter a new password: ", MODE_ID_PASSWD_2, 0,
              &tmp_cmd, NULL, NULL);
  return;
 }

 mode_del(p);
 
 COPY_STR(p->passwd, str, PLAYER_S_PASSWD_SZ);
 p->flag_raw_passwd = TRUE;

 fvtell_player(NORMAL_T(p), "%s", "\n Password has now been changed.\n");
}

static void passwd_cmd1(player *p, const char *str)
{
 ICTRACE("passwd_cmd1");
 
 fvtell_player(NORMAL_T(p), "%s", "\n");
 
 assert(MODE_IN_MODE(p, PASSWD_2));
 
 if (*str)
 {
  cmds_function tmp_cmd;

  CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, passwd_cmd2);

  COPY_STR(p->passwd_change_tmp, str, PLAYER_S_PASSWD_SZ);

  mode_change(p, " Enter password again to verify: ", MODE_ID_PASSWD_3, 0,
              &tmp_cmd, NULL, NULL);
 }
 else
 {
  fvtell_player(NORMAL_T(p), "%s",
                " Password has NOT been changed.\n");
  mode_del(p);
 }
}

static void passwd_validate(player *p, const char *str)
{
 ICTRACE("passwd_validate");
 
 fvtell_player(NORMAL_T(p), "%s", "\n");

 assert(MODE_IN_MODE(p, PASSWD_1));
 
 if (!passwd_check(p, str))
 {
  fvtell_player(NORMAL_T(p), "%s", " That's the wrong password!\n");
  mode_del(p);
 }
 else
 {
  cmds_function tmp_cmd;

  CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, passwd_cmd1);

  mode_change(p, " Enter a new password: ", MODE_ID_PASSWD_2, 0,
              &tmp_cmd, NULL, NULL);
 }
}

static void user_passwd_change(player *p)
{
 if (!p->saved->priv_base)
 {
  fvtell_player(NORMAL_T(p), "%s", 
                " You may only set a password once resident.\n"
                " To become a resident, please ask a superuser.\n");
  return;
 }

 telopt_ask_passwd_mode_on(p);

 if (p->passwd[0])
 {
  cmds_function tmp_cmd1;
  cmds_function tmp_cmd2;

  CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd1, passwd_validate);
  CMDS_FUNC_TYPE_NO_CHARS(&tmp_cmd2, passwd_cleanup);
  
  while (!mode_add(p, " Please enter your current password: ",
                   MODE_ID_PASSWD_1, 0, &tmp_cmd1, NULL, &tmp_cmd2))
    mode_del(p);
 }
 else
 {
  cmds_function tmp_cmd1;
  cmds_function tmp_cmd2;

  CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd1, passwd_cmd1);
  CMDS_FUNC_TYPE_NO_CHARS(&tmp_cmd2, passwd_cleanup);
  
  fvtell_player(SYSTEM_T(p), " You currently don't have a password set.\n");
  
  while (!mode_add(p, " Enter a new password: ", MODE_ID_PASSWD_2, 0,
                   &tmp_cmd1, NULL, &tmp_cmd2))
    mode_del(p);
 }
}

void cmds_init_passwd(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("password", user_passwd_change, NO_CHARS, SETTINGS);
}
