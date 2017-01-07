#define SUPER_CHANNEL_C
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


static int construct_lsu_name_list_do(player *scan, va_list ap)
{
 const char *str = va_arg(ap, const char *);
 int *count = va_arg(ap, int *);
 
 /* tell player stuff */
 player_tree_node *from = va_arg(ap, player_tree_node *);
 player *to = va_arg(ap, player *);
 twinkle_info *info = va_arg(ap, twinkle_info *);
 int flags = va_arg(ap, int);
 time_t my_now = va_arg(ap, time_t);

 if (!PRIV_STAFF(scan->saved))
   return (TRUE);
 
 if ((!PRIV_STAFF(to->saved) || (*str == '-')) &&
     (scan->flag_tmp_su_channel_block || scan->flag_tmp_su_channel_off))
   return (TRUE);
 
 ++*count;

 fvtell_player(ALL_T(from, to, info, flags, my_now), "%-20s",
               scan->saved->name);

 if (scan->saved->priv_admin)
   fvtell_player(ALL_T(from, to, info, flags, my_now), "%15s", "<Admin>");
 else if (scan->saved->priv_lower_admin)
   fvtell_player(ALL_T(from, to, info, flags, my_now), "%15s","<Lower Admin>");
 else if (scan->saved->priv_senior_su)
   fvtell_player(ALL_T(from, to, info, flags, my_now), "%15s", "<Senior SU>");
 else if (scan->saved->priv_normal_su)
   fvtell_player(ALL_T(from, to, info, flags, my_now), "%15s", "<Super User>");
 else if (scan->saved->priv_basic_su)
   fvtell_player(ALL_T(from, to, info, flags, my_now), "%15s", "<Basic SU>");
 else if (scan->saved->priv_pretend_su)
   fvtell_player(ALL_T(from, to, info, flags, my_now), "%15s", "<Pretend SU>");
 else if (scan->saved->priv_coder)
   fvtell_player(ALL_T(from, to, info, flags, my_now), "%15s", "{Coder}");
 else if (scan->saved->priv_su_channel)
   fvtell_player(ALL_T(from, to, info, flags, my_now), "%15s", "{Channel}");
 
 if ((*str != '-') && PRIV_STAFF(to->saved) &&
     PRIV_SYSTEM_ROOM(scan->location->owner))
 {
  char buffer[] = "                                                 ";
  sprintf(buffer, "[%s]", scan->location->id);
  fvtell_player(ALL_T(from, to, info, flags, my_now), "%15s", buffer);
 }
 else
   fvtell_player(ALL_T(from, to, info, flags, my_now), "%15s", "");

 if (difftime(now, scan->last_command_timestamp))
   fvtell_player(ALL_T(from, to, info, flags, my_now),
                 " %03.0f:%02.0f idle ",
                 difftime(now, scan->last_command_timestamp) / 60,
                 fmod(difftime(now, scan->last_command_timestamp), 60.0));
 else
   fvtell_player(ALL_T(from, to, info, flags, my_now),
                 " %03d:00 idle ", 0);
   
 if ((*str != '-') && PRIV_STAFF(to->saved))
 {
  if (scan->flag_tmp_su_channel_off)
    if (scan->flag_tmp_su_channel_block)
      fvtell_player(ALL_T(from, to, info, flags, my_now), "%s", " Off D/L");
    else
      fvtell_player(ALL_T(from, to, info, flags, my_now), "%s", " Off L");
  else
    if (scan->flag_tmp_su_channel_block)
      fvtell_player(ALL_T(from, to, info, flags, my_now), "%s", " Off D");
 }

 fvtell_player(ALL_T(from, to, info, flags, my_now), "%s", "\n");

 return (TRUE);
}

void user_lsu(player *p, const char *str)
{
 tmp_output_list_storage tmp_save;
 output_node *tmp = NULL;
 int count = 0;

 INTERCOM_USER_LSU(p, str);

 save_tmp_output_list(p, &tmp_save);

 DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
               (construct_lsu_name_list_do, str, &count,
                INFO_FTP(OUTPUT_BUFFER_TMP, p)));

 tmp = output_list_grab(p);
 load_tmp_output_list(p, &tmp_save);
 
 if (!count)
 {
  assert(!tmp);
  fvtell_player(INFO_TP(p), "%s",
                " Sorry, there are no Super Users connected "
                "at the moment.\n");
  return;
 }

 if (count > 1)
 {
  char buffer[sizeof("There are $Number(%4d     )-Tostr "
                     "Super Users connected")];

  sprintf(buffer, "There are $Number(%4d)-Tostr Super Users connected",
          count);
  
  ptell_mid(INFO_TP(p), buffer, FALSE);
 }
 else
 {
  assert(count == 1);
  ptell_mid(INFO_TP(p), "There is one Super User connected", FALSE);
 }

 output_list_linkin(p, 3, &tmp, INT_MAX);
 fvtell_player(INFO_TP(p), "%s", DASH_LEN);

 pager(p, PAGER_DEFAULT);
}

static void user_su_toggle_on_duty(player *p, const char *str)
{
 player *p2 = p;
 parameter_holder params;
 
 get_parameter_init(&params); 
 
 get_parameter_parse(&params, &str, 3);

 if (p->saved->priv_lower_admin)
   switch (params.last_param)
   {
    case 2:
      /* FALLTHROUGH */
    case 1:
      break;
      
    default:
      TELL_FORMAT(p, "[player] [on|off]");
   }
 else
   switch (params.last_param)
   {
    case 1:
     break;
     
    default:
      TELL_FORMAT(p, "[on|off]");
   }

 if (p->saved->priv_lower_admin && (params.last_param == 2))
 {
  int my_pre_flag = p->flag_tmp_su_channel_block;
  
  if (!(p2 = player_find_on(p, GET_PARAMETER_STR(&params, 1),
                            PLAYER_FIND_SC_SU_ALL)))
    return;

  if (!p2->saved->priv_su_channel)
  {
   fvtell_player(NORMAL_T(p), "%s",
                 " That person's not even on the SU channel.\n");
   return;
  }

  TOGGLE_COMMAND_OFF_ON(p, GET_PARAMETER_STR((&params), 2),
                        p2->flag_tmp_su_channel_block, TRUE,
                        " They are %soff duty.\n",
                        " They are %son duty.\n", TRUE);
  
  if (p2->flag_tmp_su_channel_block != my_pre_flag)
  {
   if (p2->flag_tmp_su_channel_block)
   {
    channels_del_system("staff", p2->saved);
    stats_log_event(p2, STATS_OFF_SU, STATS_SU_FORCED);
   }
   else
   {
    channels_add_system("staff", p2->saved);
    stats_log_event(p2, STATS_ON_SU, STATS_SU_FORCED);
   }
   
   vwlog("duty", " -=> %s forces %s %s duty.\n",
         p->saved->name, p2->saved->name,
         p2->flag_tmp_su_channel_block ? "off" : "on");
   
   fvtell_player(SYSTEM_T(p2),
                 " -=> %s forces you %s duty...%s\n",
                 p->saved->name, /* FIXME: no colour */
                 p2->flag_tmp_su_channel_block ? "off" : "on", "^N");
   
   channels_wall("staff", 3, p2, " -=> %s forces %s %s duty.", 
                 p->saved->name, p2->saved->name,
                 p2->flag_tmp_su_channel_block ? "off" : "on");
  }
 }
 else
 {
  int my_pre_flag = p->flag_tmp_su_channel_block;

  TOGGLE_COMMAND_OFF_ON(p, GET_PARAMETER_STR((&params), 1),
                        p2->flag_tmp_su_channel_block, TRUE,
                        " You are %soff duty.\n",
                        " You are %son duty.\n", TRUE);
  
  if (p->flag_tmp_su_channel_block != my_pre_flag)
  {
   if (p->flag_tmp_su_channel_block)
   {
    stats_log_event(p, STATS_OFF_SU, STATS_NO_EXTRA);
    channels_del_system("staff", p2->saved);
    
    vwlog("duty", " -=> %s%s %sgo%s off duty.\n",
          gender_choose_str(p->gender, "", "", "The ", "The "),
          p->saved->name,
          (p->gender == GENDER_PLURAL) ? "all " : "",
          gender_choose_str(p->gender, "es", "es", "", "es"));
    
    channels_wall("staff", 3, p, " -=> %s%s %sgo%s off duty.",
                  gender_choose_str(p->gender, "", "", "The ", "The "),
                  p->saved->name,
                  (p->gender == GENDER_PLURAL) ? "all " : "",
                  gender_choose_str(p->gender, "es", "es", "", "es"));
   }
   else
   {
    stats_log_event(p, STATS_ON_SU, STATS_NO_EXTRA);
    
    vwlog("duty", " -=> %s%s %sreturn%s to duty.\n",
          gender_choose_str(p->gender, "", "", "The ", "The "),
          p->saved->name, (p->gender == GENDER_PLURAL) ? "all " : "",
          (p->gender == GENDER_PLURAL) ? "" : "s");
    
    channels_wall("staff", 3, p, " -=> %s%s %sreturn%s to duty.",
                 gender_choose_str(p->gender, "", "", "The ", "The "),
                 p->saved->name, (p->gender == GENDER_PLURAL) ? "all " : "",
                 (p->gender == GENDER_PLURAL) ? "" : "s");

    channels_add_system("staff", p2->saved);
   }
  }
 }
}

void user_su_toggle_off_lsu(player *p, const char *str)
{
 int was_on = TRUE;

 CHECK_DUTY(p);

 if (p->flag_tmp_su_channel_off)
   was_on = FALSE;
 
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_tmp_su_channel_off, TRUE,
                       " You are %snot on the lsu list.\n",
                       " You are %son the lsu list.\n", TRUE);

 if (p->flag_tmp_su_channel_off)
 {
  if (was_on)
    stats_log_event(p, STATS_OFF_SU, STATS_NO_EXTRA);
 }
 else
   if (!was_on)
     stats_log_event(p, STATS_ON_SU, STATS_NO_EXTRA);
}

static void user_make_root(player *p)
{
 const char *head_admins[] = {NULL, NULL};
 int count = 0;

 /* can't be bothered writing it to support multiple admins in config atm. */
 head_admins[0] = configure.player_name_admin;
 
 if (p->saved->priv_normal_su)
 {
  fvtell_player(SYSTEM_T(p), " You've got you're aliases wrong, you need to "
                "subscribe to the staff alias library (Eg. alias-sub sus).\n");
  return;
 }
 
 while (head_admins[count])
 {
  if (!strcasecmp(head_admins[count], p->saved->lower_name))
  {
   if (p->saved->priv_base)
   {
    p->saved->priv_admin = p->saved->priv_higher_admin =
      p->saved->priv_su_channel = p->saved->priv_lower_admin =
      p->saved->priv_normal_su = p->saved->priv_senior_su =
      p->saved->priv_basic_su = p->saved->priv_pretend_su =
      p->saved->priv_no_timeout = p->saved->priv_command_warn =
      p->saved->priv_command_trace = p->saved->priv_command_script = TRUE;
    
    player_list_perm_staff_add(p->saved);
    player_list_logon_staff_add(p->saved);
    fvtell_player(SYSTEM_T(p), " Auto promotion to higher admin, done.\n");
    return;
   }
   else
   {
    priv_start_residency(p);
    fvtell_player(SYSTEM_T(p), " Auto promotion to resident, done.\n");
    return;
   }
  }
  
  ++count;
 }

 fvtell_player(SYSTEM_T(p), 
               " Oi, %s, stop being a complete spod, you're not an su "
               "HERE.\n"
               " ^BSu^N should have expanded to the suggest command, "
               "but we had loads of spods spam our logs with convos.\n",
               p->saved->name);
}

void cmds_init_super_channel(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("list_superusers", user_lsu, CONST_CHARS, INFORMATION);

 CMDS_ADD("su", user_make_root, NO_CHARS, HIDDEN);
 
 CMDS_ADD("duty", user_su_toggle_on_duty, CONST_CHARS, SU);
 CMDS_PRIV(su_channel);
 
 CMDS_ADD("lsuperusers_hide", user_su_toggle_off_lsu, CONST_CHARS, SU);
 CMDS_PRIV(senior_su);
}
