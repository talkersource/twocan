#define PRIVS_C
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

Timer_q_base priv_resident_timer_queue;

/* FIXME: many namespace violations */

static void priv_flag_admin(player_tree_node *sp, int on_off)
{ sp->priv_admin = on_off; }
static void priv_flag_coder(player_tree_node *sp, int on_off)
{ sp->priv_coder = on_off; }
static void priv_flag_command_echo(player_tree_node *sp, int on_off)
{ sp->priv_command_echo = on_off; }
static void priv_flag_higher_admin(player_tree_node *sp, int on_off)
{ sp->priv_higher_admin = on_off; }
static void priv_flag_command_list(player_tree_node *sp, int on_off)
{ sp->priv_command_list = on_off; }
static void priv_flag_lower_admin(player_tree_node *sp, int on_off)
{ sp->priv_lower_admin = on_off; }
static void priv_flag_minister(player_tree_node *sp, int on_off)
{ sp->priv_minister = on_off; }

static void priv_flag_pretend_su(player_tree_node *sp, int on_off)
{ sp->priv_su_channel = sp->priv_pretend_su = on_off; }
static void priv_flag_basic_su(player_tree_node *sp, int on_off)
{ sp->priv_su_channel = sp->priv_pretend_su = on_off;
  sp->priv_basic_su = sp->priv_command_warn = on_off; }
static void priv_flag_normal_su(player_tree_node *sp, int on_off)
{ sp->priv_su_channel = sp->priv_pretend_su = on_off;
  sp->priv_basic_su = sp->priv_command_warn = on_off;
  sp->priv_normal_su = sp->priv_command_trace = on_off; }

static void priv_flag_no_timeout(player_tree_node *sp, int on_off)
{ sp->priv_no_timeout = on_off; }
static void priv_flag_command_room(player_tree_node *sp, int on_off)
{ sp->priv_command_room = on_off; }
static void priv_flag_command_script(player_tree_node *sp, int on_off)
{ sp->priv_command_script = on_off; }
static void priv_flag_senior_su(player_tree_node *sp, int on_off)
{ sp->priv_senior_su = on_off; }
static void priv_flag_command_session(player_tree_node *sp, int on_off)
{ sp->priv_command_session = on_off; }
static void priv_flag_spod(player_tree_node *sp, int on_off)
{ sp->priv_spod = on_off; }
static void priv_flag_su_channel(player_tree_node *sp, int on_off)
{ sp->priv_su_channel = on_off; }
static void priv_flag_command_trace(player_tree_node *sp, int on_off)
{ sp->priv_command_trace = on_off; }
static void priv_flag_command_mail(player_tree_node *sp, int on_off)
{ sp->priv_command_mail = on_off; }
static void priv_flag_command_warn(player_tree_node *sp, int on_off)
{ sp->priv_command_warn = on_off; }
static void priv_flag_command_extern_bug_suggest(player_tree_node *sp,
                                                 int on_off)
{ sp->priv_command_extern_bug_suggest = on_off; }
static void priv_flag_lib_maintainer(player_tree_node *sp, int on_off)
{ sp->priv_lib_maintainer = on_off; }
static void priv_flag_edit_files(player_tree_node *sp, int on_off)
{ sp->priv_edit_files = on_off; }
static void priv_flag_alter_system_room(player_tree_node *sp, int on_off)
{ sp->priv_system_room = on_off; }

#define PRIV_FLAG_FUNC(x, y, z) {x, priv_flag_ ## y, priv_test_ ## y, z}

static struct
{
 const char *name;
 void (*change_func)(player_tree_node *, int);
 int (*test_func)(player_tree_node *);
 bitflag dup : 1;
} priv_flags[] =
{ /* NOTE: Relies on ASCII */
 PRIV_FLAG_FUNC("admin", admin, FALSE),
 PRIV_FLAG_FUNC("basic su", basic_su, FALSE),
 PRIV_FLAG_FUNC("basic_su", basic_su, TRUE),
 PRIV_FLAG_FUNC("coder", coder, FALSE),
 PRIV_FLAG_FUNC("echo", command_echo, FALSE),
 PRIV_FLAG_FUNC("edit files", edit_files, FALSE),
 PRIV_FLAG_FUNC("edit_files", edit_files, TRUE),
 PRIV_FLAG_FUNC("extern bug/suggest", command_extern_bug_suggest, FALSE),
 PRIV_FLAG_FUNC("extern_bug/suggest", command_extern_bug_suggest, TRUE),
 PRIV_FLAG_FUNC("extern bug suggest", command_extern_bug_suggest, TRUE),
 PRIV_FLAG_FUNC("extern_bug_suggest", command_extern_bug_suggest, TRUE),
 PRIV_FLAG_FUNC("higher admin", higher_admin, FALSE),
 PRIV_FLAG_FUNC("higher_admin", higher_admin, TRUE),
 PRIV_FLAG_FUNC("lib maintainer", lib_maintainer, FALSE),
 PRIV_FLAG_FUNC("lib_maintainer", lib_maintainer, TRUE),
 PRIV_FLAG_FUNC("list", command_list, FALSE),
 PRIV_FLAG_FUNC("local", command_room, FALSE),
 PRIV_FLAG_FUNC("lower admin", lower_admin, FALSE),
 PRIV_FLAG_FUNC("lower_admin", lower_admin, TRUE),
 PRIV_FLAG_FUNC("mail", command_mail, FALSE),
 PRIV_FLAG_FUNC("minister", minister, FALSE),
 PRIV_FLAG_FUNC("no timeout", no_timeout, FALSE),
 PRIV_FLAG_FUNC("no_timeout", no_timeout, TRUE),
 PRIV_FLAG_FUNC("normal su", normal_su, FALSE),
 PRIV_FLAG_FUNC("normal_su", normal_su, TRUE),
 PRIV_FLAG_FUNC("pretend su", pretend_su, FALSE),
 PRIV_FLAG_FUNC("pretend_su", pretend_su, TRUE),
 PRIV_FLAG_FUNC("room", command_room, FALSE),
 PRIV_FLAG_FUNC("script", command_script, FALSE),
 PRIV_FLAG_FUNC("senior su", senior_su, FALSE),
 PRIV_FLAG_FUNC("senior_su", senior_su, TRUE),
 PRIV_FLAG_FUNC("session", command_session, FALSE),
 PRIV_FLAG_FUNC("spod", spod, FALSE),
 PRIV_FLAG_FUNC("su channel", su_channel, FALSE),
 PRIV_FLAG_FUNC("su_channel", su_channel, TRUE),
 PRIV_FLAG_FUNC("system room", alter_system_room, FALSE),
 PRIV_FLAG_FUNC("system_room", alter_system_room, TRUE),
 PRIV_FLAG_FUNC("trace", command_trace, FALSE),
 PRIV_FLAG_FUNC("warn", command_warn, FALSE),
 {NULL, NULL, NULL, FALSE}
};

static int priv_flags_get_offset(const char *str)
{
 int count = 0;

 while (priv_flags[count].name)
 {
  int save_cmp = 0;

  assert(priv_flags[count].change_func && priv_flags[count].test_func);
  if (!(save_cmp = beg_strcmp(str, priv_flags[count].name)))
    return (count);
  else if (save_cmp < 0)
    return (-1);

  ++count;
 }

 return (-1);
}

/* don't forget user_su_say ... */
static void privs_show_all(player *p)
{
 int count = 0;
 
 while (priv_flags[count].name)
 {
  if (!priv_flags[count].dup)
    fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(count, "%s"),
                  priv_flags[count].name);
  ++count;
 }
 
 fvtell_player(NORMAL_T(p), "%s", ".\n");
}

static void user_su_privs_grant(player *p, const char *str)
{
 player *p2 = NULL;
 player_tree_node *sp = NULL;
 int old_spod = FALSE;
 int old_staff = FALSE;
 int old_minister = FALSE;
 int offset = 0;
 parameter_holder params;

 get_parameter_init(&params);
 
 if (!get_parameter_parse(&params, &str, 2))
 {
  TELL_FORMAT_NO_RETURN(p, "<player> <privilege>");

  fvtell_player(SYSTEM_T(p), " Grantable privs are:\n   ");
  privs_show_all(p);
  return;
 }
 
 CHECK_DUTY(p);

 if ((offset = priv_flags_get_offset(GET_PARAMETER_STR((&params), 2))) == -1)
 {
  fvtell_player(NORMAL_T(p), "%s", " Can't find that permission.\n");

  fvtell_player(SYSTEM_T(p), " Grantable privs are:\n   ");
  privs_show_all(p);
  return;
 }
 
 if (!p->saved->priv_higher_admin &&
     !(*priv_flags[offset].test_func)(p->saved))
 {
  fvtell_player(NORMAL_T(p), " The privilege -- ^S^B%s^s -- is one that "
                "you haven't got, therefore you can't grant it.\n",
                priv_flags[offset].name);
  return;
 }
 
 if (!(p2 = player_find_on(p, GET_PARAMETER_STR((&params), 1),
                           PLAYER_FIND_SELF)))
 {  
  if (!(sp = player_find_all(p, GET_PARAMETER_STR((&params), 1),
                             PLAYER_FIND_VERBOSE)))
    return;
  if (!player_load(sp))
  {
   fvtell_player(SYSTEM_T(p), " Player load failed for -- ^S^B%s^s -- this "
                 "probably means they are a system character.\n",
                 sp->name);
   return;
  }
 }
 else
   sp = p2->saved;

 assert(p->saved->priv_base);

 if ((*priv_flags[offset].test_func)(sp))
 {
   fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- already has "
                 "the priv -- ^S^B%s^s --.\n",
                 sp->name, priv_flags[offset].name);
  return;
 } 

 if (P_IS_ON(sp))
   fvtell_player(SYSTEM_T(sp->player_ptr),
                 " -=> %s has granted you the %s permission.\n",
                 p->saved->name, priv_flags[offset].name);

 fvtell_player(NORMAL_T(p), " Granted permission '^S^B%s^s' to %s.\n",
               priv_flags[offset].name, sp->name);

 vwlog("grant", "%s grants %s to %s", p->saved->name,
       priv_flags[offset].name, sp->name);
 
 old_staff = PRIV_STAFF(sp);
 old_spod = sp->priv_spod;
 old_minister = sp->priv_minister;
 (*priv_flags[offset].change_func)(sp, TRUE);

 sp->flag_tmp_player_needs_saving = TRUE;

 if (sp->priv_minister && !old_minister)
   channels_add_system("minister", sp);
 
 if (sp->priv_spod && !old_spod)
 {
  player_list_spod_add(sp);
  channels_add_system("spod", sp);
  if (P_IS_ON(sp))
    fvtell_player(SYSTEM_T(sp->player_ptr), "%s", " Please read: help sp.\n");
 }
 
 if (PRIV_STAFF(sp) && !old_staff)
 {
  sp->player_ptr->list_newbie_time = 0;
  
  player_list_perm_staff_add(sp);
  channels_add_system("staff", sp);
  if (P_IS_ON(sp))
  {
   player_list_logon_staff_add(sp);

   fvtell_player(SYSTEM_T(sp->player_ptr),
                 " Read the appropriate super user files please."
                 "(help %s)\n\n",
                 priv_flags[offset].name);
  }
 }
}

static void user_su_privs_remove(player *p, const char *str)
{
 player *p2 = NULL;
 player_tree_node *sp = NULL;
 int old_spod = FALSE;
 int old_staff = FALSE; 
 int old_minister = FALSE; 
 int offset = 0;
 parameter_holder params;

 get_parameter_init(&params);
 
 if (!get_parameter_parse(&params, &str, 2))
 {
  TELL_FORMAT_NO_RETURN(p, "<player> <privilege>");
  
  fvtell_player(NORMAL_T(p), " Removable privs are:\n   ");

  privs_show_all(p);
  return;
 }

 CHECK_DUTY(p);

 if ((offset = priv_flags_get_offset(GET_PARAMETER_STR(&params, 2))) == -1)
 {
  fvtell_player(NORMAL_T(p), "%s", " Can't find that permission.\n");

  fvtell_player(SYSTEM_T(p), " Removable privs are:\n   ");
  privs_show_all(p);
  return;
 }

 if (!p->saved->priv_higher_admin &&
     !(*priv_flags[offset].test_func)(p->saved))
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You can't remove permissions you haven't got "
                "yourself.\n");
  return;
 }

 if (!(p2 = player_find_on(p, GET_PARAMETER_STR((&params), 1),
                           PLAYER_FIND_SELF)))
 {  
  if (!(sp = player_find_all(p, GET_PARAMETER_STR((&params), 1),
                             PLAYER_FIND_VERBOSE)))
    return;  
  if (!player_load(sp))
  {
   fvtell_player(SYSTEM_T(p), " Player load failed for -- ^S^B%s^s -- this "
                 "probably means they are a system character.\n",
                 sp->name); 
   return;
  }
 }
 else
   sp = p2->saved;

 if (!(*priv_flags[offset].test_func)(sp))
 {
  fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- hasn't got "
                "the priv -- ^S^B%s^s --.\n",
                sp->name, priv_flags[offset].name);
  return;
 }

 assert(p->saved->priv_base);
 
 if (!priv_test_user_check(p->saved, sp, "remove", 1))
   return;
 
 if (P_IS_ON(sp))
   fvtell_player(SYSTEM_T(sp->player_ptr),
                 " -=> %s has removed your %s permission.\n",
                 p->saved->name, priv_flags[offset].name);
 
 fvtell_player(NORMAL_T(p), " Removed permission '^S^B%s^s' from %s.\n",
               priv_flags[offset].name, sp->name);

 vwlog("remove", "%s removes %s from %s", p->saved->name,
       priv_flags[offset].name, sp->name);

 old_staff = PRIV_STAFF(sp);
 old_spod = sp->priv_spod; 
 old_minister = sp->priv_minister;
 (*priv_flags[offset].change_func)(sp, FALSE);

 sp->flag_tmp_player_needs_saving = TRUE;
 
 if (!sp->priv_minister && old_minister)
   channels_del_system("minister", sp);
 
 if (!sp->priv_spod && old_spod)
 {
  player_list_spod_del(sp);
  channels_del_system("spod", sp);
 }
 
 if (!PRIV_STAFF(sp) && old_staff)
 {
  player_list_perm_staff_del(sp);
  channels_del_system("staff", sp);

  sp->player_ptr->list_newbie_time = MK_HOURS(2);
  
  if (P_IS_ON(sp))
    player_list_logon_staff_del(sp);
 }
}

static void make_player_resident(player_tree_node *tobe)
{
 BTRACE("make_player_resident");

 player_tree_add(tobe);

 tobe->priv_base = tobe->priv_command_room =
   tobe->priv_command_list = tobe->priv_command_echo =
   tobe->priv_command_mail = tobe->priv_command_session =
   tobe->priv_command_extern_bug_suggest = TRUE;
 tobe->a_timestamp = now;
 tobe->c_timestamp = now;

 no_of_resis++;
 spodlist_addin_player(tobe);

 player_save_index();

 tobe->flag_tmp_player_needs_saving = TRUE;
 tobe->flag_tmp_mail_needs_saving = TRUE;
 tobe->flag_tmp_room_needs_saving = TRUE;

 channels_add_system(configure.channels_main_name, tobe);

 player_load_timer_start(tobe);
}

static void internal_make_player_resident(player_tree_node *tobe)
{
 player *p = tobe->player_ptr;

 BTRACE("internal_make_player_resident");

 vwlog("resident", "%s.\n", p->saved->name);
 
 player_newbie_del(tobe);

 ++total_uniq_logons; /* last logon will be after talker booted */

 make_player_resident(tobe);
}

static void user_su_make_player(player *p, parameter_holder *params)
{
 player *np = NULL;

 switch (params->last_param)
 {
  default:
    TELL_FORMAT(p, "<name> <password> [email]");
    
  case 2:
  case 3:
    break;
 }
 
 CHECK_DUTY(p);

 lower_case(GET_PARAMETER_STR(params, 1));
 
 if ((GET_PARAMETER_LENGTH(params, 1) > (PLAYER_S_NAME_SZ - 2)) ||
     (GET_PARAMETER_LENGTH(params, 1) < 2))
 {
  fvtell_player(SYSTEM_T(p), "%s%d%s",
                " Name should be between 2 and ",
                PLAYER_S_NAME_SZ - 2,
                " characters.\n");
  return;
 }
  
 if (player_tree_find_exact(GET_PARAMETER_STR(params, 1)) ||
     player_newbie_find_exact(GET_PARAMETER_STR(params, 1)))
 {
  fvtell_player(SYSTEM_T(p), "%s", " That player already exists.\n");
  return;
 }

 if (*(GET_PARAMETER_STR(params, 1) +
       strspn(GET_PARAMETER_STR(params, 1), ALPHABET_LOWER)))
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " Player names have to be alphabetic characters ^Bonly^N.\n");
  return;
 }
 
 np = player_create();

 logon_player_make(np, GET_PARAMETER_STR(params, 1), NULL);

 memset(np->ip_address, 0, 4);
 CONST_COPY_STR_LEN(np->dns_address, " ** made ** ");

 sprintf(np->passwd, "%.*s", PLAYER_S_PASSWD_SZ - 1,
         GET_PARAMETER_STR(params, 2));

 if (params->last_param == 3)
   sprintf(np->email, "%.*s", PLAYER_S_EMAIL_SZ - 1,
           GET_PARAMETER_STR(params, 3));

 np->saved->logoff_timestamp = now;

 make_player_resident(np->saved);
 assert(player_find_load(NULL, np->saved->name, PLAYER_FIND_DEFAULT) == np);
 
 fvtell_player(NORMAL_T(p), " Player created.\n"
               " Name:     %s\n"
               " Email:    %s\n"
               " Password: %s\n",
               np->saved->name, *np->email ? np->email : "(blank)",
               GET_PARAMETER_STR(params, 2));
 
 vwlog("make", "%s creates %s.", p->saved->name, np->saved->name);
}

/* fwd reference -- goes back if passwd's don't match */
static void become_resident_two(player *, const char *);

static void become_resident_three(player *p, const char *str)
{
 ICTRACE("become_resident_three");

 assert(MODE_IN_MODE(p, RES_3));

 if (strcmp(p->passwd, str))
 {
  cmds_function tmp_cmd;
  cmds_function tmp_cleanup;
  
  CMDS_FUNC_TYPE_CONST_CHARS((&tmp_cmd), become_resident_two);
  CMDS_FUNC_TYPE_NO_CHARS((&tmp_cleanup), telopt_ask_passwd_mode_off);
  
  fvtell_player(SYSTEM_T(p), "%s",
                "\n They don't match, try again.\n");

  do
  {
   RES_DEL_MODE(p);
  } while (!mode_add(p, " Enter a password: ", MODE_ID_RES_2, 0,
                     &tmp_cmd, NULL, &tmp_cleanup));
 }
 else
 {
  RES_DEL_MODE(p);
  
  COPY_STR(p->passwd, str, PLAYER_S_PASSWD_SZ);
  p->flag_raw_passwd = TRUE;

  fvtell_player(NORMAL_T(p), "%s", "\n Your password has now been set.\n"
                " Congratulations and welcome! You are now a resident.\n");
  internal_make_player_resident(p->saved);

  player_save(p);
 }
}

static void become_resident_two(player *p, const char *str)
{
 ICTRACE("become_resident_two");

 assert(MODE_IN_MODE(p, RES_2));
 
 if (!*str)
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You cannot set an empty password.\n");
 }
 else
 {
  fvtell_player(NORMAL_T(p), "%s", "\n");
  if (strnlen(str, PLAYER_S_PASSWD_SZ) > (PLAYER_S_PASSWD_SZ - 1))
  {
   fvtell_player(SYSTEM_T(p), "%s",
                 " Password is too long, please try again.\n");
  }
  else
  {
   cmds_function tmp_cmd;
   cmds_function tmp_cleanup;
   
   CMDS_FUNC_TYPE_CONST_CHARS((&tmp_cmd), become_resident_three);
   CMDS_FUNC_TYPE_NO_CHARS((&tmp_cleanup), telopt_ask_passwd_mode_off);

   COPY_STR(p->passwd, str, PLAYER_S_PASSWD_SZ);

   /* don't turn off telopt passwd yet */
   CMDS_FUNC_TYPE_NOTHING((&MODE_CURRENT(p).cleanup_func), NULL);
   
   do
   {
    RES_DEL_MODE(p);
   } while (!mode_add(p, " Enter your password again to verify: ",
                      MODE_ID_RES_3, 0, &tmp_cmd, NULL, &tmp_cleanup));
  }
 }
}

static void become_resident_one(player *p, const char *str)
{
 cmds_function tmp_cmd;
 cmds_function tmp_cleanup;
 
 CMDS_FUNC_TYPE_CONST_CHARS((&tmp_cmd), become_resident_two);
 CMDS_FUNC_TYPE_NO_CHARS((&tmp_cleanup), telopt_ask_passwd_mode_off);
 
 ICTRACE("become_resident_one");
 
 assert(MODE_IN_MODE(p, RES_1));
 
 if (!*str)
 {  
  fvtell_player(NORMAL_T(p), "%s",
                " Your haven't set an email address, you won't get any of our"
                " newsletters/announcements and we cannot change your password"
                " and email it to you if you forget it. If you wish to set an"
                " email address later just use the command:\n"
                " ^S^Bemail^s <address>\n");
  p->email[0] = 0;
 }
 else
 {
  if (email_validate_player(p, str))
  {
   fvtell_player(NORMAL_T(p), "%s", "\n");
   return;
  }
  
  COPY_STR(p->email, str, PLAYER_S_EMAIL_SZ);
  
  fvtell_player(NORMAL_T(p),
                " Email address has been set to: %s\n", p->email);
 }

 do
 {
  RES_DEL_MODE(p);
 } while (!mode_add(p, " Enter a password: ", MODE_ID_RES_2, 0,
                    &tmp_cmd, NULL, &tmp_cleanup));
 
 telopt_ask_passwd_mode_on(p);
}

void priv_start_residency(player *p)
{
 cmds_function tmp_cmd;
 
 CMDS_FUNC_TYPE_CONST_CHARS((&tmp_cmd), become_resident_one);

 qstrcpy(p->script_file, "/dev/null");

 fvtell_player(NORMAL_T(p), "%s",
               " If you don't want to give an email address"
               " then just press return at the prompt. Although _if_ you"
               " set an email address it _must_ be valid.\n");
 
 while (!mode_add(p, " Enter your email address: ", MODE_ID_RES_1, 0,
                  &tmp_cmd, NULL, NULL))
   RES_DEL_MODE(p);
}

static void timed_res_player(int timed_type, void *passed_player)
{
 player *p = passed_player;

 TCTRACE("timed_res_player");

 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;

 if (p->saved->priv_base)
   return;
 
 fvtell_player(SYSTEM_T(p),
               " For your character to become a resident you MUST follow\n"
               " the on screen prompts.\n"
               " After you have done this, if you have any problems\n"
               " just ask %s%s superuser%s for help.\n\n",
               p->assisted_player ? p->assisted_player->saved->name : "a",
               p->assisted_player ? ", or any" : "",
               p->assisted_player ? "," : "");
 
 channels_wall("staff", 3, NULL,
               " -=> %s is now starting residency,"
               " which should take the $Talker-Name Resident total to"
               " %d", p->saved->name, (no_of_resis + 1));

 if (p->assisted_player)
 {
  player *p2 = p->assisted_player;
  p2->assisted_player = NULL;
  p->assisted_player = NULL;
 }

 priv_start_residency(p);
}

static void user_su_tresident(player *p, const char *str)
{
 player *p2 = NULL;
 struct timeval tv;
 
 if (!*str)
   TELL_FORMAT(p, "<newbie>");

 CHECK_DUTY(p);

 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_SU_ALL)))
   return;
 
 if (p2 == p)
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You're already a resident you spanner.\n");
  return;
 }
 
 if (!strcmp(p2->saved->lower_name, "guest"))
 {
  fvtell_player(NORMAL_T(p), "%s",
                "\n The name 'Guest' is reserved because people may use "
                "that when first logging in before using the name they "
                "REALLY want to use. So get this person to choose another "
                "name, THEN make them resident.\n\n");
  return;
 }
  
 if (p2->saved->priv_base)
 {
  fvtell_player(NORMAL_T(p),
                " The player -- ^S^B%s^s -- is already resident.\n",
                p2->saved->name);
  return;
 }
 
 if (timer_q_find_data(&priv_resident_timer_queue, p2))
 {  
  fvtell_player(NORMAL_T(p), " The player -- ^S^B%s^s -- has already been "
                "proposed for residency.\n", p2->saved->name);
  return;
 }

 gettimeofday(&tv, NULL);

 TIMER_Q_TIMEVAL_ADD_MINS(&tv, 2, 0);
 
 if (!timer_q_add_node(&priv_resident_timer_queue, p2, &tv,
                       TIMER_Q_FLAG_NODE_DEFAULT))
 {
  P_MEM_ERR(p);
  return;
 }

 fvtell_player(SYSTEM_T(p2),
               "\n\n -=> %s is validating you for residency\n"
               " -=> This should be done in 2 minutes.\n", p->saved->name);
 
 channels_wall("staff", 3, NULL,
               " -=> %s proposes %s for residency.  [2 minutes]",
               p->saved->name, p2->saved->name);
 channels_wall("staff", 3, NULL, "%s",
               "     Use object <name> <reason> if you have an "
               "objection.");
}

static void user_su_objection(player *p, const char *str)
{
 player *p2 = NULL;
 Timer_q_node *current_timer = NULL;
 parameter_holder params;

 CHECK_DUTY(p);

 get_parameter_init(&params);
 
 if (!get_parameter_parse(&params, &str, 1) || !*str)
   TELL_FORMAT(p, "<newbie> <reason>");
 
 if (!(p2 = player_find_on(p, GET_PARAMETER_STR((&params), 1),
                           PLAYER_FIND_SC_SU_ALL)))
   return;
  
 if (!(current_timer = timer_q_find_data(&priv_resident_timer_queue, p2)))
 {
  fvtell_player(NORMAL_T(p),
                "The player -- ^S^B%s^s -- hasn't been proposed "
                "for residency.\n", p2->saved->name);
  return;
 }
  
 channels_wall("staff", 3, NULL, " -=> %s objects to %s: '^S^B%s^s'.",
               p->saved->name, p2->saved->name, str);

 fvtell_player(SYSTEM_T(p2), "%s",
               "\n Please wait while the SU's process your request "
               "for residency.\n");
 
 timer_q_quick_del_node(current_timer);

 vwlog("resident", "%s objected to %s: %s",
      p->saved->name, p2->saved->name, str);
}

static void user_su_resident(player *p, const char *str)
{
 player *p2 = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<player>");

 CHECK_DUTY(p);
 
 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_SU_ALL)))
   return;
 
 if (p2 == p)
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You're already a resident you spanner.\n");
  return;
 }
 
 if (!strcmp(p2->saved->lower_name, "guest"))
 {
  fvtell_player(NORMAL_T(p), "%s",
                "\n The name 'Guest' is reserved because people may use "
                "that when first logging in before using the name they "
                "REALLY want to use. So get this person to choose another "
                "name, THEN make them resident.\n\n");
  return;
 }

 timer_q_del_data(&priv_resident_timer_queue, p2);

 if (p2->saved->priv_base)
 {
  fvtell_player(NORMAL_T(p), " The player -- ^S^B%s^s -- is already "
                "a resident.\n", p2->saved->name);
  return;
 }
 
 fvtell_player(SYSTEM_T(p2), "\n\n -=> %s %s made you a resident.\n",
               p->saved->name, (p->gender == GENDER_PLURAL) ? "have" : "has");
 
 fvtell_player(SYSTEM_T(p2),
               " For your character to save you MUST follow"
               " the on screen prompts.\n"
               " After you have done this, if you have any problems"
               " just ask %s, or any superuser, for help.\n\n",
               p->saved->name);

 if (p->gender == GENDER_PLURAL) /* FIXME -- wrapping */
   channels_wall("staff", 3, NULL,
                 " -=> All the %s gang up and grant residency to %s,"
                 " which should take the $Talker-Name Resident total to "
                 " %d", p->saved->name, p2->saved->name, (no_of_resis + 1));
 else
   channels_wall("staff", 3, NULL,
                 " -=> %s grants residency to %s,"
                 " which should take the $Talker-Name Resident total to"
                 " %d", p->saved->name, p2->saved->name, (no_of_resis + 1));
 
 vwlog("resident", "%s made %s a resident. (qres)",
       p->saved->name, p2->saved->name);
 
 if (p2->assisted_player)
 {
  player *p3 = p2->assisted_player;
  p2->assisted_player = NULL;
  p3->assisted_player = NULL;
 }

 priv_start_residency(p2);
}

static void user_su_assist_player(player *p, const char *str)
{
 player *p2 = NULL;

 if (!*str)
   TELL_FORMAT(p, "<person>");
 
 CHECK_DUTY(p);
 
 if (p->assisted_player)
 {
  fvtell_player(NORMAL_T(p), " You are already assisting the player "
                "-- ^S^B%s^s --.\n",
                p->assisted_player->saved->name);
  return;
 }
 
 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_SU_ALL)))
   return;
 
 if (p2->saved->priv_base)
 {
  fvtell_player(NORMAL_T(p), " The player -- ^S^B%s^s -- is already "
                "a resident.\n", p2->saved->name);
  return;
 }

 if (p2->assisted_player)
 {
  if (p2->assisted_player != p)
    fvtell_player(NORMAL_T(p), " The player -- ^S^B%s^s -- is already "
                  "being assisted by -- ^S^B%s^s --.\n",
                  p2->saved->name, p2->assisted_player->saved->name);
  return;
 }

 p2->assisted_player = p;
 p->assisted_player = p2;

 if (p->gender == GENDER_PLURAL)
   fvtell_player(SYSTEM_T(p2),
                 "\n -=> %s%s are superusers, and would be more than "
                 "happy to assist you in any problems you may have (including "
                 "gaining residency, type 'help residency' to find out more "
                 "about that).  To talk to %s, type 'tell %s <whatever>\', "
                 "short forms of names usually work as well.\n\n",
                 gender_choose_str(p->gender, "", "", "The ", "The "),
                 p->saved->name,
                 gender_choose_str(p->gender, "him", "her", "them", "it"),
                 p->saved->lower_name);
 else
   fvtell_player(SYSTEM_T(p2),
                 "\n -=> %s%s is a superuser, and would be more than "
                 "happy to assist you in any problems you may have (including "
                 "gaining residency, type 'help residency' to find out more "
                 "about that).  To talk to %s, type 'tell %s <whatever>\', "
                 "short forms of names usually work as well.\n\n",
                 gender_choose_str(p->gender, "", "", "The ", "The "),
                 p->saved->name,
                 gender_choose_str(p->gender, "him", "her", "them", "it"),
                 p->saved->lower_name);

 channels_wall("staff", 3, p, " -=> %s assists %s.",
               p->saved->name, p2->saved->name);
 fvtell_player(NORMAL_T(p), " You assist %s.\n", p2->saved->name);
 
 vwlog("resident", "%s assists %s", p->saved->name, p2->saved->name);
}

static void user_su_unassist_player(player *p, const char *str)
{
 player *p2 = NULL;

 if (!*str)
   TELL_FORMAT(p, "<person>");

 CHECK_DUTY(p);
 
 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_SU_ALL)))
   return;
 
 if (p2->saved->priv_base)
 {
  fvtell_player(NORMAL_T(p), " The player -- ^S^B%s^s -- is already "
                "a resident.\n", p2->saved->name);
  return;
 }

 if (p2->assisted_player != p)
 {
  fvtell_player(SYSTEM_T(p), " That player -- ^S^B%s^s -- is not being "
                "assisted by you.\n",
                p2->saved->name);
  return;
 }
 log_assert(p->assisted_player == p2);
 
 p2->assisted_player = NULL;
 p->assisted_player = NULL;

 fvtell_player(SYSTEM_T(p2),
               "\n -=> %s has unassisted you, if you still want help or "
               "residency, please ask another superuser.\n\n", p->saved->name);

 channels_wall("staff", 3, p, " -=> %s unassists %s.",
               p->saved->name, p2->saved->name);
 fvtell_player(NORMAL_T(p), " You unassist %s.\n", p2->saved->name);
 vwlog("resident", "%s unassists %s", p->saved->name, p2->saved->name);
}

static void user_su_list_newbies(player *p, const char *str)
{
 int newbies_on = player_newbie_number();
 
 assert(newbies_on > -1);

 if (!newbies_on && strcasecmp("full", str) && strcasecmp("all", str))
 {
  fvtell_player(NORMAL_T(p), "%s", " No newbies on at the moment.\n");
  return;
 }
 
 if (newbies_on != 1)
   ptell_mid(NORMAL_T(p), "$Newbie_players newbies on", FALSE);
 else
   ptell_mid(NORMAL_T(p), "1 newbie on", FALSE);
 
 if (!beg_strcasecmp(str, "full") || !beg_strcasecmp(str, "all"))
 {
  player_linked_list *scan = player_list_cron_start();
  assert(scan);
  
  while (scan)
  {
   player_linked_list *scan_next = PLAYER_LINK_NEXT(scan);
   int is_on = (PLAYER_LINK_SAV_GET(scan) && PLAYER_LINK_GET(scan) &&
                P_IS_ON_P(PLAYER_LINK_SAV_GET(scan), PLAYER_LINK_GET(scan)));
     
   assert((scan_next && scan_next->has_prev) ? 
          (PLAYER_LINK_PREV(scan_next) == scan) : TRUE);
   assert((scan->has_prev && PLAYER_LINK_PREV(scan)) ?
          (PLAYER_LINK_NEXT(PLAYER_LINK_PREV(scan)) == scan) : TRUE);

   if (!is_on || !PLAYER_LINK_SAV_GET(scan)->priv_base)
   {
    fvtell_player(NORMAL_T(p), "%-20s ",
                  (PLAYER_LINK_SAV_GET(scan) &&
                   P_IS_ON(PLAYER_LINK_SAV_GET(scan))) ?
                  PLAYER_LINK_SAV_GET(scan)->name : "(logon)");
    
    fvtell_player(NORMAL_T(p), "%-3s", is_on ? "*L" : "*N");
    
    fvtell_player(NORMAL_T(p), "%-37s", PLAYER_LINK_GET(scan)->dns_address);
    
    if (is_on && PLAYER_LINK_GET(scan)->assisted_player)
      fvtell_player(NORMAL_T(p), "[%s]",
                    PLAYER_LINK_GET(scan)->assisted_player->saved->name);

    fvtell_player(NORMAL_T(p), "%s", "\n");
   }
   
   scan = scan_next;
  }
 }
 else
 {
  player_tree_node *scan = player_newbie_start();
  assert(scan);
 
  while (scan)
  {
   assert(P_IS_ON(scan));
   
   fvtell_player(NORMAL_T(p), "%-20s ", scan->name);
   
   if (*str != '-')
   {
    fvtell_player(NORMAL_T(p), "%-3s", P_IS_ON(scan) ? "*N" : "*L");
    
    fvtell_player(NORMAL_T(p), "%-37s", scan->player_ptr->dns_address);
    
    if (scan->player_ptr->assisted_player)
      fvtell_player(NORMAL_T(p), "[%s]",
                    scan->player_ptr->assisted_player->saved->name);
   }
   
   fvtell_player(NORMAL_T(p), "%s", "\n");
   
   scan = scan->next;
  }
 }
 
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 pager(p, PAGER_DEFAULT);
}

static void user_show_privileges(player *p, const char *str)
{
 player *p2 = NULL;
 char buffer[sizeof("Permissions for %s") + PLAYER_S_NAME_SZ];
 
 if (*str && p->saved->priv_normal_su)
 {
  if (!(p2 = player_find_load(p, str, PLAYER_FIND_SC_SU_ALL |
                              PLAYER_FIND_BANISHED)))
    return;

  sprintf(buffer, "Permissions for %s", p2->saved->name);
 }
 else
 {
  sprintf(buffer, "Permissions for you");
  p2 = p;
 }
 ptell_mid(NORMAL_T(p), buffer, FALSE);
 
 if (!p2->saved->priv_base)
 {
  assert(p2->is_fully_on);
  fvtell_player(NORMAL_T(p), "%s",
                "$Merge(1($Line_fill( )) 2( ** NEWBIE ** ))\n");
  fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
  return;
 }
 
 if (p2->saved->priv_banished)
   fvtell_player(NORMAL_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p), "%s",
                 "$Merge(1($Line_fill( )) 2( ** ^S^BBANISHED^s ** ))\n");
 
 fvtell_player(NORMAL_T(p), "Echo command: %s\n",
               TOGGLE_YES_NO(p2->saved->priv_command_echo));
 fvtell_player(NORMAL_T(p), "List commands: %s\n",
               TOGGLE_YES_NO(p2->saved->priv_command_list));
 fvtell_player(NORMAL_T(p), "Room commands: %s\n",
               TOGGLE_YES_NO(p2->saved->priv_command_room));
 fvtell_player(NORMAL_T(p), "Mail commands: %s\n",
               TOGGLE_YES_NO(p2->saved->priv_command_mail));
 fvtell_player(NORMAL_T(p), "Session commands: %s\n",
               TOGGLE_YES_NO(p2->saved->priv_command_session));
 fvtell_player(NORMAL_T(p), "No timeout: %s\n",
               TOGGLE_YES_NO(p2->saved->priv_no_timeout));
 fvtell_player(NORMAL_T(p), "External mails on bug/suggest: %s\n",
               TOGGLE_YES_NO(p2->saved->priv_command_extern_bug_suggest));

 fvtell_player(NORMAL_T(p), "Minister: %s\n",
               TOGGLE_YES_NO(p2->saved->priv_minister));
  
 if (p2->saved->priv_command_script)
   fvtell_player(NORMAL_T(p), "Scripting: %s\n", TOGGLE_YES_NO(TRUE));
 
 if (p2->saved->priv_command_warn)
   fvtell_player(NORMAL_T(p), "Warns: %s\n", TOGGLE_YES_NO(TRUE));
  
 if (p2->saved->priv_command_trace)
   fvtell_player(NORMAL_T(p), "Trace: %s\n", TOGGLE_YES_NO(TRUE));
 
 if (p2->saved->priv_spod)
   fvtell_player(NORMAL_T(p), "Spod: %s\n", TOGGLE_YES_NO(TRUE));
 
 if (p2->saved->priv_su_channel)
   fvtell_player(NORMAL_T(p), "Su channel: %s\n", TOGGLE_YES_NO(TRUE));
 
 if (p2->saved->priv_coder)
   fvtell_player(NORMAL_T(p), "Coder: %s\n", TOGGLE_YES_NO(TRUE));
 
 if (p2->saved->priv_higher_admin)
   fvtell_player(NORMAL_T(p), "Privilege: ^B%s^s\n", "higher administrator");
 else if (p2->saved->priv_admin)
   fvtell_player(NORMAL_T(p), "Privilege: ^S^B%s^s\n", "administrator");
 else if (p2->saved->priv_lower_admin)
   fvtell_player(NORMAL_T(p), "Privilege: ^S^B%s^s\n", "lower administrator");
 else if (p2->saved->priv_senior_su)
   fvtell_player(NORMAL_T(p), "Privilege: ^S^B%s^s\n", "senior superuser");
 else if (p2->saved->priv_normal_su)
   fvtell_player(NORMAL_T(p), "Privilege: ^S^B%s^s\n", "superuser");
 else if (p2->saved->priv_basic_su)
   fvtell_player(NORMAL_T(p), "Privilege: ^S^B%s^s\n", "basic superuser");
 else
   fvtell_player(NORMAL_T(p), "Privilege: %s\n", "resident");
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

void init_privs(void)
{
 timer_q_add_static_base(&priv_resident_timer_queue, timed_res_player,
                         TIMER_Q_FLAG_BASE_DEFAULT);
}

void cmds_init_privs(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("assist", user_su_assist_player, CONST_CHARS, SU);
 CMDS_PRIV(pretend_su);
 
 CMDS_ADD("unassist", user_su_unassist_player, CONST_CHARS, SU);
 CMDS_PRIV(pretend_su);

 CMDS_ADD("grant", user_su_privs_grant, CONST_CHARS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(admin);

 CMDS_ADD("remove", user_su_privs_remove, CONST_CHARS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(admin);

 CMDS_ADD("privilages", user_show_privileges, CONST_CHARS, MISC);
 CMDS_PRIV(base); CMDS_XTRA_SECTION(SU);
 CMDS_ADD("privileges", user_show_privileges, CONST_CHARS, MISC);
 CMDS_PRIV(base); CMDS_XTRA_SECTION(SU);

 CMDS_ADD("list_newbies", user_su_list_newbies, CONST_CHARS, SU);
 CMDS_PRIV(pretend_su);

 CMDS_ADD("make", user_su_make_player, PARSE_PARAMS, ADMIN);
 CMDS_PRIV(admin);

 CMDS_ADD("qresident", user_su_resident, CONST_CHARS, SU);
 CMDS_PRIV(pretend_su);
 
 CMDS_ADD("resident", user_su_tresident, CONST_CHARS, SU);
 CMDS_PRIV(pretend_su);

 CMDS_ADD("object", user_su_objection, CONST_CHARS, SU);
 CMDS_PRIV(pretend_su);
}
