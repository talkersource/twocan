#define PRIV_TEST_C
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

const char *priv_test_names[] =
{
 "none",
 "newbie",
 "resident",
 "su channel",
 "normal su",
 "lower admin",
 "admin",
 "minister",
 "spod",
 "coder",
 "mail command",
 "list command",
 "room command",
 NULL
};

int priv_test_int(player_tree_node *sp, int priv)
{
 switch (priv)
 {
  default:
    log_assert(FALSE);
    break;

  case PRIV_TEST_NONE:
    return (TRUE);
    
  case PRIV_TEST_NEWBIE:
    return (!sp->priv_base);
  case PRIV_TEST_RESIDENT:
    return (sp->priv_base);
  case PRIV_TEST_SU_CHAN:
    return (sp->priv_su_channel);
  case PRIV_TEST_SU_NORM:
    return (sp->priv_normal_su);
  case PRIV_TEST_LWR_ADMIN:
    return (sp->priv_lower_admin);
  case PRIV_TEST_ADMIN:
    return (sp->priv_admin);
  case PRIV_TEST_MINISTER:
    return (sp->priv_minister);
  case PRIV_TEST_SPOD:
    return (sp->priv_spod);
  case PRIV_TEST_CODER:
    return (sp->priv_coder);
  case PRIV_TEST_CMD_MAIL:
    return (sp->priv_command_mail);
  case PRIV_TEST_CMD_LIST:
    return (sp->priv_command_list);
  case PRIV_TEST_CMD_ROOM:
    return (sp->priv_command_room);
 }
 
 return (FALSE);
}

int priv_test_parse_int(const char *priv)
{
 if (!BEG_CONST_STRCMP("none", priv))
   return (PRIV_TEST_NONE);
 
 if (!BEG_CONST_STRCMP("newbie", priv))
   return (PRIV_TEST_NEWBIE);

 if (!BEG_CONST_STRCMP("resident", priv))
   return (PRIV_TEST_RESIDENT);
 
 if (!BEG_CONST_STRCMP("su_channel", priv))
   return (PRIV_TEST_SU_CHAN);
 if (!BEG_CONST_STRCMP("su channel", priv))
   return (PRIV_TEST_SU_CHAN);
 
 if (!BEG_CONST_STRCMP("normal su", priv))
   return (PRIV_TEST_SU_NORM);
 if (!BEG_CONST_STRCMP("normal_su", priv))
   return (PRIV_TEST_SU_NORM);
 if (!BEG_CONST_STRCMP("su", priv))
   return (PRIV_TEST_SU_NORM);

 if (!BEG_CONST_STRCMP("lower admin", priv))
   return (PRIV_TEST_LWR_ADMIN);
 if (!BEG_CONST_STRCMP("lower_admin", priv))
   return (PRIV_TEST_LWR_ADMIN);

 if (!BEG_CONST_STRCMP("admin", priv))
   return (PRIV_TEST_ADMIN);

 if (!BEG_CONST_STRCMP("minister", priv))
   return (PRIV_TEST_MINISTER);
 
 if (!BEG_CONST_STRCMP("spod", priv))
   return (PRIV_TEST_SPOD);
 
 if (!BEG_CONST_STRCMP("coder", priv))
   return (PRIV_TEST_CODER);
 
 if (!BEG_CONST_STRCMP("mail command", priv))
   return (PRIV_TEST_CMD_MAIL);
 if (!BEG_CONST_STRCMP("mail_command", priv))
   return (PRIV_TEST_CMD_MAIL);
 if (!BEG_CONST_STRCMP("command mail", priv))
   return (PRIV_TEST_CMD_MAIL);
 if (!BEG_CONST_STRCMP("command_mail", priv))
   return (PRIV_TEST_CMD_MAIL);
 
 if (!BEG_CONST_STRCMP("list command", priv))
   return (PRIV_TEST_CMD_LIST);
 if (!BEG_CONST_STRCMP("list_command", priv))
   return (PRIV_TEST_CMD_LIST);
 if (!BEG_CONST_STRCMP("command list", priv))
   return (PRIV_TEST_CMD_LIST);
 if (!BEG_CONST_STRCMP("command_list", priv))
   return (PRIV_TEST_CMD_LIST);
 
 if (!BEG_CONST_STRCMP("room command", priv))
   return (PRIV_TEST_CMD_ROOM);
 if (!BEG_CONST_STRCMP("room_command", priv))
   return (PRIV_TEST_CMD_ROOM);
 if (!BEG_CONST_STRCMP("command room", priv))
   return (PRIV_TEST_CMD_ROOM);
 if (!BEG_CONST_STRCMP("command_room", priv))
   return (PRIV_TEST_CMD_ROOM);

 return (PRIV_TEST_NONE);
}

int priv_test_string(player *p, const char *str, int priv_override)
{
 output_node *out_node = NULL;
 tmp_output_list_storage tmp_save;
 twinkle_info info;
 int flags = 0;

 if (priv_override)
   flags = OUTPUT_VARIABLES_NO_CHECKS;
 
 setup_twinkle_info(&info);
 
 info.returns_limit = UINT_MAX;
 info.allow_fills = TRUE;

 save_tmp_output_list(p, &tmp_save);

 fvtell_player(ALL_T(p->saved, p, &info, flags, now), "%s", str);

 out_node = output_list_grab(p);

 output_list_cleanup(&out_node);

 load_tmp_output_list(p, &tmp_save);

 return (info.counter[0]);
}

/* this sees if p1 has more privs than p2
   returns:
   1 if p1 has more
   0 if they are equal
   -1 if p2 has more */
int priv_test_check(player_tree_node *p1, player_tree_node *p2)
{
 /* you always have more privs than yourself,
  * hence you can always do things to yourself */
 if (p1 == p2)
   return (1);
 
 PRIV_TEST_CHECK(p1, p2, higher_admin);
 PRIV_TEST_CHECK(p1, p2, admin);
 PRIV_TEST_CHECK(p1, p2, lower_admin);
 PRIV_TEST_CHECK(p1, p2, senior_su);
 PRIV_TEST_CHECK(p1, p2, coder);
 PRIV_TEST_CHECK(p1, p2, normal_su);
 PRIV_TEST_CHECK(p1, p2, basic_su);
 PRIV_TEST_CHECK(p1, p2, pretend_su);
 PRIV_TEST_CHECK(p1, p2, su_channel);

 return (0);
}

int priv_test_user_check(player_tree_node *p1, player_tree_node *p2,
                         const char *msg, int priv_needed)
{
 int ret = priv_test_check(p1, p2);

 if (ret >= priv_needed)
   return (TRUE);

 if (P_IS_ON(p1))
   fvtell_player(SYSTEM_T(p1->player_ptr),
                 " The player -- ^S^B%s^s -- has enough "
                 "privileges that you cannot -- ^S^B%s^s -- them.\n",
                 p2->name, msg);
 
 if (P_IS_ON(p2))
   fvtell_player(SYSTEM_T(p2->player_ptr),
                 " -=> %s%s tried to ^S^B%s^s you, but failed.\n",
                 P_IS_AVL(p1) ?
                  gender_choose_str(p1->player_ptr->gender, "", "",
                                    "The ", "The ") : "",
                 p1->name, msg);

 vwlog("priv_test", "%s tried to -- %s -- %s\n", p1->name, msg, p2->name);
 
 return (FALSE);
}


int priv_test_none(player_tree_node *sp)
{ IGNORE_PARAMETER(sp); return (TRUE); }

int priv_test_higher_admin(player_tree_node *sp)
{ return (sp->priv_higher_admin); }
int priv_test_admin(player_tree_node *sp)
{ return (sp->priv_admin); }
int priv_test_lower_admin(player_tree_node *sp)
{ return (sp->priv_lower_admin); }
int priv_test_senior_su(player_tree_node *sp)
{ return (sp->priv_senior_su); }
int priv_test_normal_su(player_tree_node *sp)
{ return (sp->priv_normal_su); }
int priv_test_coder(player_tree_node *sp)
{ return (sp->priv_coder); }
int priv_test_basic_su(player_tree_node *sp)
{ return (sp->priv_basic_su); }
int priv_test_pretend_su(player_tree_node *sp)
{ return (sp->priv_pretend_su); }
int priv_test_su_channel(player_tree_node *sp)
{ return (sp->priv_su_channel); }
int priv_test_spod(player_tree_node *sp)
{ return (sp->priv_spod); }
int priv_test_minister(player_tree_node *sp)
{ return (sp->priv_minister); }
int priv_test_alter_system_room(player_tree_node *sp)
{/* this is not the same as PRIV_SYSTEM_ROOM */ return (sp->priv_system_room);}
int priv_test_no_timeout(player_tree_node *sp)
{ return (sp->priv_no_timeout); }
int priv_test_base(player_tree_node *sp)
{ return (sp->priv_base); }
int priv_test_newbie(player_tree_node *sp)
{ return (!sp->priv_base); }

int priv_test_coder_and_admin(player_tree_node *sp)
{ return (sp->priv_coder && sp->priv_admin); }
int priv_test_coder_admin(player_tree_node *sp)
{ return (sp->priv_coder || sp->priv_admin); }
int priv_test_coder_lower_admin(player_tree_node *sp)
{ return (sp->priv_coder || sp->priv_admin); }
int priv_test_coder_senior_su(player_tree_node *sp)
{ return (sp->priv_coder || sp->priv_senior_su); }
int priv_test_coder_normal_su(player_tree_node *sp)
{ return (sp->priv_coder || sp->priv_normal_su); }
int priv_test_coder_pretend_su(player_tree_node *sp)
{ return (sp->priv_coder || sp->priv_pretend_su); }
int priv_test_spod_pretend_su(player_tree_node *sp)
{ return (sp->priv_spod || sp->priv_pretend_su); }
int priv_test_spod_minister_coder_normal_su(player_tree_node *sp)
{ return (sp->priv_spod || sp->priv_minister ||
          sp->priv_coder || sp->priv_normal_su); }
int priv_test_lib_maintainer(player_tree_node *sp)
{ return (sp->priv_lib_maintainer); }
int priv_test_edit_files(player_tree_node *sp)
{ return (sp->priv_edit_files); }

int priv_test_command_echo(player_tree_node *sp)
{ return (sp->priv_command_echo); }
int priv_test_command_jotd_edit(player_tree_node *sp)
{ return (sp->priv_command_jotd_edit); }
int priv_test_command_list(player_tree_node *sp)
{ return (sp->priv_command_list); }
int priv_test_command_mail(player_tree_node *sp)
{ return (sp->priv_command_mail); }
int priv_test_command_extern_bug_suggest(player_tree_node *sp)
{ return (sp->priv_command_extern_bug_suggest); }
int priv_test_command_marriage_channel(player_tree_node *sp)
{ return (sp->priv_coder || (P_IS_AVL(sp) && sp->player_ptr->flag_married)); }
int priv_test_command_room(player_tree_node *sp)
{ return (sp->priv_command_room); }
int priv_test_command_room_link(player_tree_node *sp)
{
 if (!P_IS_ON(sp) || !sp->priv_command_room)
   return (FALSE);
 
 if (sp == sp->player_ptr->location->owner)
   return (TRUE);

 if (PRIV_SYSTEM_ROOM(sp->player_ptr->location->owner) && sp->priv_system_room)
   return (TRUE);

 LIST_ROOM_CHECK_FLAG_START(sp->player_ptr->location, sp);
 if (LIST_ROOM_CHECK_FLAG_DO(link))
   return (TRUE);
 LIST_ROOM_CHECK_FLAG_END();

 return (FALSE); }
int priv_test_command_room_lock(player_tree_node *sp)
{
 if (!P_IS_ON(sp) || !sp->priv_command_room)
   return (FALSE);
 
 if (sp == sp->player_ptr->location->owner)
   return (TRUE);
 
 if (PRIV_SYSTEM_ROOM(sp->player_ptr->location->owner) && sp->priv_system_room)
   return (TRUE);

 LIST_ROOM_CHECK_FLAG_START(sp->player_ptr->location, sp);
 if (LIST_ROOM_CHECK_FLAG_DO(key))
   return (TRUE);
 LIST_ROOM_CHECK_FLAG_END();
 
 return (FALSE); }
int priv_test_command_room_bolt(player_tree_node *sp)
{
 if (!P_IS_ON(sp) || !sp->priv_command_room)
   return (FALSE);
 
 if (sp == sp->player_ptr->location->owner)
   return (TRUE);
 
 if (PRIV_SYSTEM_ROOM(sp->player_ptr->location->owner) && sp->priv_system_room)
   return (TRUE);

 LIST_ROOM_CHECK_FLAG_START(sp->player_ptr->location, sp);
 if (LIST_ROOM_CHECK_FLAG_DO(bolt))
   return (TRUE);
 LIST_ROOM_CHECK_FLAG_END();
 
 return (FALSE); }
int priv_test_command_room_alter(player_tree_node *sp)
{
 if (!P_IS_ON(sp) || !sp->priv_command_room)
   return (FALSE);
 
 if (sp == sp->player_ptr->location->owner)
   return (TRUE);
 
 if (PRIV_SYSTEM_ROOM(sp->player_ptr->location->owner) && sp->priv_system_room)
   return (TRUE);

 LIST_ROOM_CHECK_FLAG_START(sp->player_ptr->location, sp);
 if (LIST_ROOM_CHECK_FLAG_DO(alter))
   return (TRUE);
 LIST_ROOM_CHECK_FLAG_END();
 
 return (FALSE); }
int priv_test_command_room_grant(player_tree_node *sp)
{
 if (!P_IS_ON(sp) || !sp->priv_command_room)
   return (FALSE);
 
 if (sp == sp->player_ptr->location->owner)
   return (TRUE);
 
 if (PRIV_SYSTEM_ROOM(sp->player_ptr->location->owner) && sp->priv_system_room)
   return (TRUE);

 LIST_ROOM_CHECK_FLAG_START(sp->player_ptr->location, sp);
 if (LIST_ROOM_CHECK_FLAG_DO(grant))
   return (TRUE);
 LIST_ROOM_CHECK_FLAG_END();
 
 return (FALSE); }
int priv_test_command_session(player_tree_node *sp)
{ return (sp->priv_command_session); }
int priv_test_command_script(player_tree_node *sp)
{ return (sp->priv_command_script); }
int priv_test_command_trace(player_tree_node *sp)
{ return (sp->priv_command_trace); }
int priv_test_command_warn(player_tree_node *sp)
{ return (sp->priv_command_warn); }


int priv_test_configure_socials(player_tree_node *sp)
{ IGNORE_PARAMETER(sp); return (configure.socials_use); }
int priv_test_configure_game_draughts(player_tree_node *sp)
{ IGNORE_PARAMETER(sp); return (configure.game_draughts_use); }
int priv_test_configure_game_hangman(player_tree_node *sp)
{ IGNORE_PARAMETER(sp); return (configure.game_hangman_use); }
int priv_test_configure_game_sps(player_tree_node *sp)
{ IGNORE_PARAMETER(sp); return (configure.game_sps_use); }
int priv_test_configure_game_ttt(player_tree_node *sp)
{ IGNORE_PARAMETER(sp); return (configure.game_ttt_use); }
int priv_test_configure_games(player_tree_node *sp)
{ return (priv_test_configure_game_draughts(sp) ||
          priv_test_configure_game_hangman(sp) ||
          priv_test_configure_game_sps(sp) || 
          priv_test_configure_game_ttt(sp) ||
          FALSE); }

int priv_test_configure_game_draughts_base(player_tree_node *sp)
{ return (configure.game_draughts_use && sp->priv_base); }
int priv_test_configure_game_hangman_base(player_tree_node *sp)
{ return (configure.game_hangman_use && sp->priv_base); }
int priv_test_configure_game_sps_base(player_tree_node *sp)
{ return (configure.game_sps_use && sp->priv_base); }
int priv_test_configure_game_ttt_base(player_tree_node *sp)
{ return (configure.game_ttt_use && sp->priv_base); }
int priv_test_configure_games_base(player_tree_node *sp)
{ return (priv_test_configure_games(sp) && sp->priv_base); }


int priv_test_mode_mail(player_tree_node *sp)
{ return (P_IS_ON(sp) && MODE_IN_MODE(sp->player_ptr, MAIL)); }
int priv_test_mode_news(player_tree_node *sp)
{ return (P_IS_ON(sp) && MODE_IN_MODE(sp->player_ptr, NEWS)); }
int priv_test_mode_newsgroup(player_tree_node *sp)
{ return (P_IS_ON(sp) && MODE_IN_MODE(sp->player_ptr, NEWSGROUP)); }
int priv_test_mode_room(player_tree_node *sp)
{ return (P_IS_ON(sp) && MODE_IN_MODE(sp->player_ptr, ROOM)); }
int priv_test_mode_channels(player_tree_node *sp)
{ return (P_IS_ON(sp) && MODE_IN_MODE(sp->player_ptr, CHANNELS)); }
int priv_test_mode_check(player_tree_node *sp)
{ return (P_IS_ON(sp) && MODE_IN_MODE(sp->player_ptr, CHECK)); }
int priv_test_mode_chlim(player_tree_node *sp)
{ return (P_IS_ON(sp) && MODE_IN_MODE(sp->player_ptr, CHLIM)); }
int priv_test_mode_draughts(player_tree_node *sp)
{ return (P_IS_ON(sp) && MODE_IN_MODE(sp->player_ptr, DRAUGHTS)); }
int priv_test_mode_stats(player_tree_node *sp)
{ return (P_IS_ON(sp) && MODE_IN_MODE(sp->player_ptr, STATS)); }
int priv_test_mode_configure(player_tree_node *sp)
{ return (P_IS_ON(sp) && MODE_IN_MODE(sp->player_ptr, CONFIGURE)); }



int priv_test_command_room_list(player_tree_node *sp)
{ return (sp->priv_command_room && sp->priv_command_list); }




int list_self_priv_test_friend(list_node *entry)
{ return (LIST_FLAG(entry, self, friend)); }
int list_self_priv_test_grab(list_node *entry)
{ return (LIST_FLAG(entry, self, grab)); }
