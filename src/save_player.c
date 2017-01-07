#define SAVE_PLAYER_C
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



/* functions... */


static void internal_player_save_index(player_tree_node *scan, va_list va)
{
 unsigned int *count = va_arg(va, unsigned int *);
 file_io *io_index = va_arg(va, file_io *);
 char buffer[BUF_NUM_TYPE_SZ(int)];

 sprintf(buffer, "%08d", ++*count);

 file_section_beg(buffer, io_index);
 
 file_put_string("name", scan->lower_name, 0, io_index);

 file_section_end(buffer, io_index);
}

static void internal_get_total(player_tree_node *scan, va_list va)
{
 unsigned int *count = va_arg(va, unsigned int *);

 IGNORE_PARAMETER(scan);
 
 ++*count;
}

void player_save_index(void)
{
 file_io io_index;
 
 if (file_write_open("files/player_data/index.tmp", 1, &io_index))
 {
  unsigned int count = 0;
  unsigned int total = 0;

  do_inorder_all(internal_get_total, &total); /* FIXME: need running counter */
  
  file_section_beg("header", &io_index);
  
  file_put_unsigned_int("total", total, &io_index);

  file_section_end("header", &io_index);

  file_section_beg("list", &io_index);

  do_inorder_all(internal_player_save_index, &count, &io_index);
  
  file_section_end("list", &io_index);

  if (file_write_close(&io_index))
    rename("files/player_data/index.tmp", "files/player_data/index");
 }
}

void player_save(player *p)
{
 player_tree_node *sp = p->saved;
 char player_file[sizeof("files/player_data/a/a/.player.tmp") + PLAYER_S_NAME_SZ];
 char player_file_ren[sizeof("files/player_data/a/a/.player") + PLAYER_S_NAME_SZ];
 file_io io_player;
 int count = 0;
 
 if (!sp)
 {
  assert(FALSE);
  return;
 }

 if (configure.talker_read_only)
   return;

 log_assert(P_IS_AVL(p->saved)); /* triggered */
 if (!p->saved->flag_tmp_player_needs_saving &&
     !P_IS_ON_P(p->saved, p))
   return;

 if (PRIV_SYSTEM_ROOM(p->saved))
 {
  assert(FALSE); /* this might get called, not sure */
  return;
 }
 
 sprintf(player_file_ren, "%s/%c/%c/%s.player", "files/player_data",
         *sp->lower_name, *(sp->lower_name + 1), sp->lower_name);
 sprintf(player_file, "%s.tmp", player_file_ren);
 
 if (!file_write_open(player_file, 1, &io_player))
 {
  log_assert(FALSE);
  return;
 }

 file_section_beg("a_saved", &io_player);

 file_section_beg("bits", &io_player);

 file_put_time_t("c_timestamp", sp->c_timestamp, &io_player);
 
 file_section_beg("flags", &io_player);

 file_put_bitflag("agreed_disclaimer", sp->flag_agreed_disclaimer, &io_player);
 file_put_bitflag("hide_logon_time", sp->flag_hide_logon_time, &io_player);
 file_put_bitflag("kill_logon_time", sp->flag_kill_logon_time, &io_player);
 file_put_bitflag("no_anonymous", sp->flag_no_anonymous, &io_player);
 file_put_bitflag("private_email", sp->flag_private_email, &io_player);

 file_section_end("flags", &io_player);

 file_put_int("karma_value", sp->karma_value, &io_player);
 
 file_section_beg("privs", &io_player);

 file_put_bitflag("admin", sp->priv_admin, &io_player);
 file_put_bitflag("banished", sp->priv_banished, &io_player);
 file_put_bitflag("base", sp->priv_base, &io_player);
 file_put_bitflag("basic_su", sp->priv_basic_su, &io_player);
 file_put_bitflag("coder", sp->priv_coder, &io_player);
 file_put_bitflag("command_echo", sp->priv_command_echo, &io_player);
 file_put_bitflag("command_list", sp->priv_command_list, &io_player);
 file_put_bitflag("command_mail", sp->priv_command_mail, &io_player);
 file_put_bitflag("command_room", sp->priv_command_room, &io_player);
 file_put_bitflag("command_script", sp->priv_command_script, &io_player);
 file_put_bitflag("command_session", sp->priv_command_session, &io_player);
 file_put_bitflag("command_trace", sp->priv_command_trace, &io_player);
 file_put_bitflag("command_warn", sp->priv_command_warn, &io_player);
 file_put_bitflag("higher_admin", sp->priv_higher_admin, &io_player);
 file_put_bitflag("lib_maintainer", sp->priv_lib_maintainer, &io_player);

 file_put_bitflag("lower_admin", sp->priv_lower_admin, &io_player);
 file_put_bitflag("minister", sp->priv_minister, &io_player);
 file_put_bitflag("no_timeout", sp->priv_no_timeout, &io_player);
 file_put_bitflag("normal_su", sp->priv_normal_su, &io_player);
 file_put_bitflag("pretend_su", sp->priv_pretend_su, &io_player);
 file_put_bitflag("senior_su", sp->priv_senior_su, &io_player);
 file_put_bitflag("spod", sp->priv_spod, &io_player);
 file_put_bitflag("su_channel", sp->priv_su_channel, &io_player);
 file_put_bitflag("system_room", sp->priv_system_room, &io_player);

 file_section_end("privs", &io_player);

 file_section_end("bits", &io_player);

 assert(sp->priv_base);

 assert(sp->player_ptr == p);
 
 file_put_string("capped_name", sp->name, 0, &io_player);
 assert(!strcasecmp(sp->name, sp->lower_name));
 
 file_put_string("last_dns_address", sp->last_dns_address, 0, &io_player);
 
 file_section_beg("last_ip_address", &io_player);
 file_put_short("1", sp->last_ip_address[0], &io_player);
 file_put_short("2", sp->last_ip_address[1], &io_player);
 file_put_short("3", sp->last_ip_address[2], &io_player);
 file_put_short("4", sp->last_ip_address[3], &io_player);
 file_section_end("last_ip_address", &io_player);

 if (P_IS_ON_P(p->saved, p))
 {
  file_put_time_t("logoff_timestamp", now, &io_player);
  
  file_put_int("total_idle_logon", sp->total_idle_logon + p->idle_logon,
               &io_player);
  file_put_int("total_logon", sp->total_logon +
               floor(difftime(now, p->logon_timestamp)), &io_player);
 }
 else
 {
  file_put_time_t("logoff_timestamp", sp->logoff_timestamp, &io_player);
  
  file_put_int("total_idle_logon", sp->total_idle_logon, &io_player);
  file_put_int("total_logon", sp->total_logon, &io_player);
 }
 
 file_section_end("a_saved", &io_player);

 file_put_int("age", p->age, &io_player);
 
 file_section_beg("aliases", &io_player);

 alias_save(p, &io_player);
 
 file_section_end("aliases", &io_player);

 file_put_int("autoexec_int", p->autoexec, &io_player); /* FIXME: autoexec */
 file_put_time_t("birthday", p->birthday, &io_player);
 file_put_string("connect_msg", p->connect_msg, 0, &io_player);
 file_put_string("converse_prompt", p->converse_prompt, 0, &io_player);

 file_put_int("default_newsgroup", p->saved_default_newsgroup, &io_player);

 file_put_string("description", p->description, 0, &io_player);

 file_put_string("disconnect_msg", p->disconnect_msg, 0, &io_player);
  
 file_section_beg("draughts", &io_player);
 file_put_bitflag("auto_private",  p->flag_draughts_auto_private, &io_player);
 file_put_bitflag("no_auto_show_my_move",
                  p->flag_draughts_no_auto_show_my_move, &io_player);
 file_put_bitflag("no_auto_show_player",
                  p->flag_draughts_no_auto_show_player, &io_player);
 file_put_bitflag("no_auto_show_watch",
                  p->flag_draughts_no_auto_show_watch, &io_player);
 file_put_int("played",  p->draughts_played, &io_player);
 file_put_int("won",  p->draughts_won, &io_player);
 file_section_end("draughts", &io_player);
 
 file_put_string("email", p->email, 0, &io_player);
 file_put_time_t("emailed_passwd_timestamp", p->emailed_passwd_timestamp,
                 &io_player);
 file_put_string("enter_msg", p->enter_msg, 0, &io_player);

 file_section_beg("flags", &io_player);

 file_put_bitflag("bell_allow", p->flag_allow_bell, &io_player);
 file_put_bitflag("bell_audible", p->flag_audible_bell, &io_player);

 file_put_bitflag("birthday_show_year", p->flag_birthday_show_year,
                  &io_player);

 file_put_bitflag("crazy_channel_block", p->flag_crazy_channel_block,
                  &io_player);
 file_put_bitflag("crazy_news", p->flag_crazy_news, &io_player);
 file_put_bitflag("crazy_news_bulletin", p->flag_crazy_news_bulletin,
                                                &io_player);
 file_put_bitflag("follow_block", p->flag_follow_block, &io_player);
 file_put_bitflag("gmt_offset_hide", p->flag_gmt_offset_hide, &io_player);
 file_put_bitflag("idle_warns_block", p->flag_idle_warns_block, &io_player);
 file_put_bitflag("input_keep_going", p->flag_input_keep_going, &io_player);
 file_put_bitflag("just_normal_hilight", p->flag_just_normal_hilight,
                  &io_player);
 file_put_bitflag("list_show_inorder", p->flag_list_show_inorder, &io_player);
 file_put_bitflag("location_hide", p->flag_location_hide, &io_player);
 file_put_bitflag("marriage_hide", p->flag_marriage_hide, &io_player);
 file_put_bitflag("marriages_block", p->flag_marriages_block, &io_player);
 file_put_bitflag("married", p->flag_married, &io_player);

 file_put_bitflag("no_cmd_matching", p->flag_no_cmd_matching, &io_player);
 file_put_bitflag("no_colour_from_others", p->flag_no_colour_from_others,
                  &io_player);
 file_put_bitflag("no_emote_prefix", p->flag_no_emote_prefix, &io_player); 
 file_put_bitflag("no_info_in_who", p->flag_no_info_in_who, &io_player);
 file_put_bitflag("no_net_spouse", p->flag_no_net_spouse, &io_player);
 file_put_bitflag("no_prefix", p->flag_no_prefix, &io_player);
 file_put_bitflag("no_specials_from_others", p->flag_no_specials_from_others,
                  &io_player);
 file_put_bitflag("page_on_return", p->flag_page_on_return, &io_player);
 file_put_bitflag("pager_auto_quit", p->flag_pager_auto_quit, &io_player);
 file_put_bitflag("pager_off", p->flag_pager_off, &io_player);
 file_put_bitflag("quiet_edit", p->flag_quiet_edit, &io_player);
 file_put_bitflag("raw_password", p->flag_raw_passwd, &io_player);
 file_put_bitflag("room_enter_brief", p->flag_room_enter_brief, &io_player);
 file_put_bitflag("room_exits_show", p->flag_room_exits_show, &io_player);
 file_put_bitflag("see_echo", p->flag_see_echo, &io_player);
 file_put_bitflag("see_room_events", p->flag_see_room_events, &io_player);
 file_put_bitflag("session_in_who", p->flag_session_in_who, &io_player);
 file_put_bitflag("social_auto_name", p->flag_social_auto_name, &io_player);
 file_put_bitflag("social_auto_remote", p->flag_social_auto_remote,
                  &io_player);
 file_put_bitflag("spod_channel_block", p->flag_spod_channel_block,
                  &io_player);
 
 file_section_beg("tags", &io_player);
 
 file_put_bitflag("autos", p->flag_show_autos, &io_player);
 file_put_bitflag("echo", p->flag_show_echo, &io_player);
 file_put_bitflag("says", p->flag_show_room, &io_player);
 file_put_bitflag("shouts", p->flag_show_shouts, &io_player);
 file_put_bitflag("socials", p->flag_show_socials, &io_player);
 file_put_bitflag("tells", p->flag_show_personal, &io_player);
 
 file_section_end("tags", &io_player);

 file_put_bitflag("trans_to_home", p->flag_trans_to_home, &io_player);
 file_put_bitflag("use_24_clock", p->flag_use_24_clock, &io_player);
 file_put_bitflag("use_birthday_as_age", p->flag_use_birthday_as_age,
                  &io_player);
 file_put_bitflag("use_long_clock", p->flag_use_long_clock, &io_player);
 
 file_section_end("flags", &io_player);
 
 file_put_string("follow", p->follow, 0, &io_player);
 file_put_int("gender",  p->gender, &io_player);
 file_put_int("gmt_offset",  p->gmt_offset, &io_player);
 file_put_string("ignore_msg", p->ignore_msg, 0, &io_player);

 file_put_int("karma_cutoff", p->karma_cutoff, &io_player);
 file_put_int("karma_used", p->karma_used, &io_player);

 file_section_beg("list", &io_player);

 file_section_beg("coms", &io_player);
 list_save(&p->list_coms_start, LIST_TYPE_COMS, &io_player);
 file_section_end("coms", &io_player);

 file_section_beg("game", &io_player);
 list_save(&p->list_game_start, LIST_TYPE_GAME, &io_player);
 file_section_end("game", &io_player);

 file_section_beg("self", &io_player);
 list_save(&p->list_self_start, LIST_TYPE_SELF, &io_player);
 file_section_end("self", &io_player);

 file_section_end("list", &io_player);

 file_put_string("married_to", p->married_to, 0, &io_player);

 file_put_time_t("mask_coms_hit_timestamp", p->mask_coms_hit_timestamp,
                 &io_player);

 /* FIXME: see load_player comment */
 file_put_int("max_autos",  p->max_autos, &io_player);
 file_put_int("max_exits",  p->max_exits, &io_player);
 file_put_int("max_list_entries",  p->max_list_entries, &io_player);
 file_put_int("max_mails",  p->max_mails, &io_player);
 file_put_int("max_rooms",  p->max_rooms, &io_player);
 
 file_put_time_t("motd_su_timestamp",  p->su_motdlr, &io_player);
 file_put_time_t("motd_timestamp",  p->motdlr, &io_player);
 
 file_put_int("nationality",  p->nationality, &io_player);
 
 file_section_beg("nicknames", &io_player);

 nickname_save(p, &io_player);
 
 file_section_end("nicknames", &io_player);
 
 file_section_beg("output_types", &io_player);
 count = 0;
 while (count < OUTPUT_COLOUR_SZ)
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];

  sprintf(buffer, "%04d", ++count);
  
  file_put_unsigned_int(buffer, p->output[count - 1], &io_player);
 }
 file_section_end("output_types", &io_player);

 file_put_string("password", p->passwd, 0, &io_player);
 file_put_string("phone_numbers", p->phone_numbers, 0, &io_player);
 file_put_string("plan", p->plan, 0, &io_player);

 file_put_string("prefix", p->prefix, 0, &io_player);
 file_put_string("prompt", p->prompt, 0, &io_player);

 file_put_time_t("rep_output_last_used", p->rep_output_last_used, &io_player);
  
 file_put_string("room_connect", p->room_connect, 0, &io_player);

 file_put_string("saved_warn", p->saved_warn, 0, &io_player);
 file_put_string("signature", p->sig, 0, &io_player);

 file_section_beg("sps", &io_player);
 file_put_bitflag("ai_disabled", p->flag_sps_ai_disabled, &io_player);
 if (p->flag_sps_ai_disabled)
   file_put_time_t("ai_disabled_till",  p->sps_ai_disabled_till, &io_player);
 file_put_bitflag("brief",  p->flag_sps_brief, &io_player);
 file_put_int("drawn",  p->sps_drawn, &io_player);
 file_put_int("played",  p->sps_played, &io_player);
 file_put_int("won",  p->sps_won, &io_player);
 file_section_end("sps", &io_player);
 
 file_put_string("su_comment", p->su_comment, 0, &io_player);

 file_section_beg("term", &io_player);
 file_put_bitflag("ansi_colour_override",
                  p->flag_terminal_ansi_colour_override, &io_player);
 file_put_unsigned_int("height",  p->term_height, &io_player);
 if (p->termcap)
   file_put_string("name",  p->termcap->name, 0, &io_player);
 file_put_unsigned_int("width",  p->term_width, &io_player);
 file_section_end("term", &io_player);

 file_put_string("title", p->title, 0, &io_player);

 file_put_long("total_cpu_sec", p->total_cpu.tv_sec, &io_player);
 file_put_long("total_cpu_usec", p->total_cpu.tv_usec, &io_player);

 file_section_beg("ttt", &io_player);
 file_put_bitflag("brief", p->flag_ttt_brief, &io_player);
 file_put_int("drawn",  p->ttt_drawn, &io_player);
 file_put_int("played",  p->ttt_played, &io_player);
 file_put_int("won",  p->ttt_won, &io_player);
 file_section_end("ttt", &io_player);
 
 file_put_string("url", p->url, 0, &io_player);
 file_put_unsigned_int("word_wrap",  p->word_wrap, &io_player);
 
 if (file_write_close(&io_player))
   rename(player_file, player_file_ren);
}

void player_save_all(player *p)
{
 player_save(p);
 mail_save(p->saved);
 room_save(p->saved);
}

void user_player_save(player *p)
{
#ifdef PLAYER_SAVE_USER_ALLOW
  player_save_all(p);
#endif
  fvtell_player(NORMAL_T(p), "%s", " Character Saved.\n");
}
