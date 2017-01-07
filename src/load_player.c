#define LOAD_PLAYER_C
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


Timer_q_base load_player_timer_queue;

/* functions.... */


void player_load_cleanup(player *p)
{
 assert(p);

 BTRACE("player_load_cleanup");
 
 if (!p->loaded_player)
   return;
 
 alias_cleanup(p);
 list_player_load_cleanup(p);
 nickname_cleanup(p);
 terminal_unsetup(&p->termcap);

 if (player_list_io_find(p))
   player_list_io_del(p);
 
 output_list_cleanup(&p->output_start);
 output_list_cleanup(&p->output_buffer_tmp);
}

static void timed_player_cleanup(int timed_type, void *passed_player_tree_node)
{
 player_tree_node *sp = passed_player_tree_node;
 int do_save = FALSE;
 int do_cleanup = FALSE;
 int do_retime = FALSE;

 TCTRACE("timed_player_cleanup");

 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;

 assert(P_IS_AVL(sp));
 
 if (timed_type == TIMER_Q_TYPE_CALL_RUN_ALL)
   do_save = TRUE;
 else if (!P_IS_ON(sp) &&
          (difftime(now, sp->a_timestamp) > PLAYER_CLEANUP_TIMEOUT_CLEANUP))
 {
  do_save = TRUE;
  do_cleanup = TRUE;
 }
 else if (difftime(now, sp->l_timestamp) > PLAYER_CLEANUP_TIMEOUT_SYNC_ANYWAY)
 {
  do_save = TRUE;
  do_retime = TRUE;
  sp->l_timestamp = now;
 }
 else
   do_retime = TRUE;
 
 if (do_save)
   player_save(sp->player_ptr);
 
 if (do_cleanup)
 {
  player_load_cleanup(sp->player_ptr);
  XFREE(sp->player_ptr, PLAYER);
  sp->player_ptr = NULL;
 }
 else if (do_retime)
 {
  struct timeval tv;
  
  gettimeofday(&tv, NULL);
  
  TIMER_Q_TIMEVAL_ADD_SECS(&tv, PLAYER_CLEANUP_TIMEOUT_REDO, 0);

  timer_q_add_static_node(&sp->load_timer, &load_player_timer_queue,
                          sp, &tv, TIMER_Q_FLAG_NODE_SINGLE);
 }
}

void player_load_timer_start(player_tree_node *sp)
{
 if (!timer_q_find_data(&load_player_timer_queue, sp))
 {
  struct timeval tv;
  
  gettimeofday(&tv, NULL);
  
  TIMER_Q_TIMEVAL_ADD_SECS(&tv, PLAYER_CLEANUP_TIMEOUT_LOAD, 0);

  timer_q_add_static_node(&sp->load_timer, &load_player_timer_queue,
                          sp, &tv, TIMER_Q_FLAG_NODE_SINGLE);

  sp->l_timestamp = now;
 }
}

int player_load(player_tree_node *sp)
{
 player *p = NULL;
 char player_file[sizeof("files/player_data/a/a/.player") + PLAYER_S_NAME_SZ];
 file_io io_player;
 unsigned int count = 0;
 int tmp = 0;
 char term_name[TERMINAL_NAME_SZ];

 BTRACE("player_load");
 
 if (!sp)
   return (FALSE);

 if (PRIV_SYSTEM_ROOM(sp))
   return (FALSE);
 
 if (P_IS_AVL(sp))
 {
  sp->a_timestamp = now;
  return (TRUE);
 }
 
 sprintf(player_file, "%s/%c/%c/%s.player", "files/player_data",
         *sp->lower_name, *(sp->lower_name + 1), sp->lower_name);
 
 if (!file_read_open(player_file, &io_player))
 {
  assert(FALSE);
  return (FALSE);
 }

 if (sp->player_ptr)
   p = sp->player_ptr;
 else if (!(p = XMALLOC(sizeof(player), PLAYER)))
 {
  file_read_close(&io_player);
  return (FALSE);
 }
 
 memset(p, 0, sizeof(player)); /* FIXME: needs init func */
   
 p->saved = sp;
 sp->player_ptr = p;
 p->loaded_player = TRUE;

 player_load_timer_start(sp);
 
 file_section_beg("a_saved", &io_player);
 
 tmp = file_get_int("total_idle_logon", &io_player);
 assert(tmp == p->saved->total_idle_logon);
 p->saved->total_idle_logon = tmp;
 
 tmp = file_get_int("total_logon", &io_player);
 assert(tmp == p->saved->total_logon);
 p->saved->total_logon = tmp;
 
 file_section_end("a_saved", &io_player);
 
 p->age = file_get_int("age", &io_player);
 
 file_section_beg("aliases", &io_player);

 alias_load(p, &io_player);
 
 file_section_end("aliases", &io_player);

 p->autoexec = file_get_int("autoexec_int", &io_player); /* FIXME: autoexec */
 p->birthday = file_get_time_t("birthday", &io_player);
 file_get_string("connect_msg", p->connect_msg,
                 PLAYER_S_CONNECT_MSG_SZ, &io_player);
 file_get_string("converse_prompt", p->converse_prompt, PROMPT_SZ, &io_player);

 p->saved_default_newsgroup = file_get_int("default_newsgroup", &io_player);
 p->default_newsgroup = p->saved_default_newsgroup;

 file_get_string("description", p->description,
                 PLAYER_S_DESCRIPTION_SZ, &io_player);

 file_get_string("disconnect_msg", p->disconnect_msg,
                 PLAYER_S_DISCONNECT_MSG_SZ, &io_player);
  
 file_section_beg("draughts", &io_player);
 p->flag_draughts_auto_private =
   file_get_bitflag("auto_private", &io_player);
 p->flag_draughts_no_auto_show_my_move =
   file_get_bitflag("no_auto_show_my_move", &io_player);
 p->flag_draughts_no_auto_show_player =
   file_get_bitflag("no_auto_show_player", &io_player);
 p->flag_draughts_no_auto_show_watch =
   file_get_bitflag("no_auto_show_watch", &io_player);
 p->draughts_played = file_get_int("played", &io_player);
 p->draughts_won = file_get_int("won", &io_player);
 file_section_end("draughts", &io_player);
 
 file_get_string("email", p->email, PLAYER_S_EMAIL_SZ, &io_player);
 p->emailed_passwd_timestamp = file_get_time_t("emailed_passwd_timestamp",
                                               &io_player);
 if (FILE_IO_CREATED(&io_player))
   p->emailed_passwd_timestamp = disp_time_create(1990, 1, 1, 1, 0, 0);
 
 file_get_string("enter_msg", p->enter_msg, PLAYER_S_ENTER_MSG_SZ, &io_player);

 file_section_beg("flags", &io_player);

 p->flag_allow_bell = file_get_bitflag("bell_allow", &io_player);
 p->flag_audible_bell = file_get_bitflag("bell_audible", &io_player);
 p->flag_birthday_show_year = file_get_bitflag("birthday_show_year",
                                                &io_player);
 p->flag_crazy_channel_block = file_get_bitflag("crazy_channel_block",
                                                &io_player);
 p->flag_crazy_news = file_get_bitflag("crazy_news", &io_player);
 p->flag_crazy_news_bulletin = file_get_bitflag("crazy_news_bulletin",
                                                &io_player);
 p->flag_follow_block = file_get_bitflag("follow_block", &io_player);
 p->flag_gmt_offset_hide = file_get_bitflag("gmt_offset_hide", &io_player);
 p->flag_idle_warns_block = file_get_bitflag("idle_warns_block", &io_player);
 p->flag_input_keep_going = file_get_bitflag("input_keep_going", &io_player);
 p->flag_just_normal_hilight = file_get_bitflag("just_normal_hilight",
                                                &io_player);
 p->flag_list_show_inorder = file_get_bitflag("list_show_inorder", &io_player);
 p->flag_location_hide = file_get_bitflag("location_hide", &io_player);
 p->flag_marriage_hide = file_get_bitflag("marriage_hide", &io_player);
 p->flag_marriages_block = file_get_bitflag("marriages_block", &io_player);
 p->flag_married = file_get_bitflag("married", &io_player);

 p->flag_no_cmd_matching = file_get_bitflag("no_cmd_matching", &io_player);
 p->flag_no_colour_from_others = file_get_bitflag("no_colour_from_others",
                                                  &io_player);
 p->flag_no_emote_prefix = file_get_bitflag("no_emote_prefix", &io_player); 
 p->flag_no_info_in_who = file_get_bitflag("no_info_in_who", &io_player);
 p->flag_no_net_spouse = file_get_bitflag("no_net_spouse", &io_player);
 p->flag_no_prefix = file_get_bitflag("no_prefix", &io_player);
 p->flag_no_specials_from_others = file_get_bitflag("no_specials_from_others",
                                                    &io_player);
 p->flag_page_on_return = file_get_bitflag("page_on_return", &io_player);
 p->flag_pager_auto_quit = file_get_bitflag("pager_auto_quit", &io_player);
 p->flag_pager_off = file_get_bitflag("pager_off", &io_player);
 p->flag_quiet_edit = file_get_bitflag("quiet_edit", &io_player);
 p->flag_raw_passwd = file_get_bitflag("raw_password", &io_player);
 p->flag_room_enter_brief = file_get_bitflag("room_enter_brief", &io_player);
 p->flag_room_exits_show = file_get_bitflag("room_exits_show", &io_player);
 p->flag_see_echo = file_get_bitflag("see_echo", &io_player);
 p->flag_see_room_events = file_get_bitflag("see_room_events", &io_player);
 p->flag_session_in_who = file_get_bitflag("session_in_who", &io_player);
 p->flag_social_auto_name = file_get_bitflag("social_auto_name", &io_player);
 p->flag_social_auto_remote = file_get_bitflag("social_auto_remote",
                                               &io_player);
 p->flag_spod_channel_block = file_get_bitflag("spod_channel_block",
                                               &io_player);

 file_section_beg("tags", &io_player);
 
 p->flag_show_autos = file_get_bitflag("autos", &io_player);
 p->flag_show_echo = file_get_bitflag("echo", &io_player);
 p->flag_show_room = file_get_bitflag("says", &io_player);
 p->flag_show_shouts = file_get_bitflag("shouts", &io_player);
 p->flag_show_socials = file_get_bitflag("socials", &io_player);
 p->flag_show_personal = file_get_bitflag("tells", &io_player);
 
 file_section_end("tags", &io_player);

 p->flag_trans_to_home = file_get_bitflag("trans_to_home", &io_player);
 p->flag_use_24_clock = file_get_bitflag("use_24_clock", &io_player);
 p->flag_use_birthday_as_age = file_get_bitflag("use_birthday_as_age",
                                                &io_player);
 p->flag_use_long_clock = file_get_bitflag("use_long_clock", &io_player);
 
 file_section_end("flags", &io_player);
 
 file_get_string("follow", p->follow, PLAYER_S_NAME_SZ, &io_player);
 p->gender = file_get_int("gender", &io_player);
 p->gmt_offset = file_get_int("gmt_offset", &io_player);
 file_get_string("ignore_msg", p->ignore_msg,
                 PLAYER_S_IGNORE_MSG_SZ, &io_player);

 p->karma_cutoff = file_get_int("karma_cutoff", &io_player);
 p->karma_used = file_get_int("karma_used", &io_player);

 file_section_beg("list", &io_player);
 
 file_section_beg("coms", &io_player);
 list_load(&p->list_coms_start, LIST_TYPE_COMS, &io_player);
 file_section_end("coms", &io_player);

 file_section_beg("game", &io_player);
 list_load(&p->list_game_start, LIST_TYPE_GAME, &io_player);
 file_section_end("game", &io_player);

 file_section_beg("self", &io_player);
 list_load(&p->list_self_start, LIST_TYPE_SELF, &io_player);
 file_section_end("self", &io_player);
 
 file_section_end("list", &io_player);
 
 file_get_string("married_to", p->married_to, PLAYER_S_NAME_SZ, &io_player);

 p->mask_coms_hit_timestamp = file_get_time_t("mask_coms_hit_timestamp",
                                              &io_player);
 
 /* FIXME: Have a max section or have autos_max etc... and put them with
  * that section ? */
 p->max_autos = file_get_int("max_autos", &io_player);
 p->max_exits = file_get_int("max_exits", &io_player);
 p->max_list_entries = file_get_int("max_list_entries", &io_player);
 p->max_mails = file_get_int("max_mails", &io_player);
 p->max_rooms = file_get_int("max_rooms", &io_player);

 p->su_motdlr = file_get_time_t("motd_su_timestamp", &io_player);
 p->motdlr = file_get_time_t("motd_timestamp", &io_player);
 
 p->nationality = file_get_int("nationality", &io_player);
 
 file_section_beg("nicknames", &io_player);

 nickname_load(p, &io_player);
 
 file_section_end("nicknames", &io_player);
 
 file_section_beg("output_types", &io_player);
 count = 0;
 while (count < OUTPUT_COLOUR_SZ)
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];

  sprintf(buffer, "%04d", count + 1);
  
  p->output[count] = file_get_unsigned_int(buffer, &io_player);

  if (FILE_IO_CREATED(&io_player) &&
      ((count == OUTPUT_COLOUR_CHAN_1) || (count == OUTPUT_COLOUR_CHAN_2) ||
       (count == OUTPUT_COLOUR_CHAN_3)))
  {
   if (count == OUTPUT_COLOUR_CHAN_1)
     tmp = OUTPUT_COLOUR_OLD_MAIN_CHAN;
   else if (count == OUTPUT_COLOUR_CHAN_2)
     tmp = OUTPUT_COLOUR_OLD_SPOD;
   else
     tmp = OUTPUT_COLOUR_OLD_SUS;
   
   p->output[count] = p->output[tmp];
   p->output[tmp] = 0;
  }
  
  ++count;
 }
 file_section_end("output_types", &io_player);

 file_get_string("password", p->passwd, PLAYER_S_PASSWD_SZ, &io_player);
 file_get_string("phone_numbers", p->phone_numbers,
                 PLAYER_S_PHONE_NUMBERS_SZ, &io_player);
 file_get_string("plan", p->plan, PLAYER_S_PLAN_SZ, &io_player);
 file_get_string("prefix", p->prefix, PLAYER_S_PREFIX_SZ, &io_player);
 file_get_string("prompt", p->prompt, PROMPT_SZ, &io_player);

 file_get_string("room_connect", p->room_connect,
                 PLAYER_S_ROOM_CONNECT_SZ, &io_player);

 file_get_string("saved_warn", p->saved_warn, PLAYER_S_WARN_SZ, &io_player);
 file_get_string("signature", p->sig, PLAYER_S_SIG_SZ, &io_player);

 file_section_beg("sps", &io_player);
 p->flag_sps_ai_disabled = file_get_bitflag("ai_disabled", &io_player);
 if (p->flag_sps_ai_disabled)
   p->sps_ai_disabled_till = file_get_time_t("ai_disabled_till", &io_player);
 p->flag_sps_brief = file_get_bitflag("brief", &io_player);
 p->sps_drawn = file_get_int("drawn", &io_player);
 p->sps_played = file_get_int("played", &io_player);
 p->sps_won = file_get_int("won", &io_player);
 file_section_end("sps", &io_player);
 
 file_get_string("su_comment", p->su_comment,
                 PLAYER_S_SU_COMMENT_SZ, &io_player);

 file_section_beg("term", &io_player);
 p->flag_terminal_ansi_colour_override =
   file_get_bitflag("ansi_colour_override", &io_player);
 p->term_height = file_get_unsigned_int("height", &io_player);
 if (!p->automatic_term_name_got)
   file_get_string("name", term_name, TERMINAL_NAME_SZ, &io_player);
 p->term_width = file_get_unsigned_int("width", &io_player);
 file_section_end("term", &io_player);

 terminal_setup(p, term_name);
 
 file_get_string("title", p->title, PLAYER_S_TITLE_SZ, &io_player);

 p->total_cpu.tv_sec = file_get_long("total_cpu_sec", &io_player);
 p->total_cpu.tv_usec = file_get_long("total_cpu_usec", &io_player);

 file_section_beg("ttt", &io_player);
 p->flag_ttt_brief = file_get_bitflag("brief", &io_player);
 p->ttt_drawn = file_get_int("drawn", &io_player);
 p->ttt_played = file_get_int("played", &io_player);
 p->ttt_won = file_get_int("won", &io_player);
 file_section_end("ttt", &io_player);
 
 file_get_string("url", p->url, PLAYER_S_URL_SZ, &io_player);
 
 p->word_wrap = file_get_unsigned_int("word_wrap", &io_player);

 file_read_close(&io_player);
 
 return (TRUE);
}

player_tree_node *player_load_saved(const char *name)
{
 player_tree_node *new_player = player_tree_find_exact(name);
 char player_file[sizeof("files/player_data/a/a/.player") + PLAYER_S_NAME_SZ];
 file_io io_player;
 
 if (new_player)
 {
  assert(FALSE);
  return (NULL);
 }

 if (!(new_player = XMALLOC(sizeof(player_tree_node), PLAYER_TREE_NODE)))
   SHUTDOWN_MEM_ERR();

 sprintf(player_file, "%s/%c/%c/%s.player", "files/player_data",
         *name, *(name + 1), name);
 
 if (!file_read_open(player_file, &io_player))
 {
  fprintf(stderr, " Not loading -- %s -- due to not being able to find .player file.\n", name);
  XFREE(new_player, PLAYER_TREE_NODE);
  return (NULL);
 }

 new_player->left = NULL;
 new_player->right = NULL;
 new_player->next = NULL;
 new_player->prev = NULL;
 
 new_player->multis_start = NULL;
 new_player->channels_start = NULL;

 new_player->rooms_num = 0;
 new_player->rooms_start = NULL;
 
 new_player->mail_sent_start = NULL;
 new_player->mail_recieved_start = NULL;
 
 new_player->player_ptr = NULL;

 new_player->next_num = NULL;
 new_player->prev_num = NULL;

 new_player->list_room_glob_start = NULL;
 new_player->flag_tmp_list_room_glob_in_core = FALSE;
 
 new_player->flag_tmp_player_needs_saving = FALSE;
 new_player->flag_tmp_mail_needs_saving = FALSE;
 new_player->flag_tmp_room_needs_saving = FALSE;

 new_player->logoff_timestamp = now;
 new_player->total_idle_logon = 0;
 new_player->total_logon = 0;
 new_player->last_dns_address[0] = 0;   
 memset(new_player->last_ip_address, 0, 4);

 strcpy(new_player->lower_name, name); 
 player_tree_add(new_player);

 file_section_beg("a_saved", &io_player);

 file_section_beg("bits", &io_player);

 new_player->c_timestamp = file_get_time_t("c_timestamp", &io_player); 
 
 file_section_beg("flags", &io_player);

 new_player->flag_agreed_disclaimer = file_get_bitflag("agreed_disclaimer", &io_player);
 new_player->flag_hide_logon_time = file_get_bitflag("hide_logon_time",
                                                     &io_player);
 new_player->flag_kill_logon_time = file_get_bitflag("kill_logon_time",
                                                     &io_player);
 new_player->flag_no_anonymous = file_get_bitflag("no_anonymous", &io_player);
 new_player->flag_private_email =
   file_get_bitflag("private_email", &io_player);

 file_section_end("flags", &io_player);

 new_player->karma_value = file_get_int("karma_value", &io_player);
 
 file_section_beg("privs", &io_player);

 new_player->priv_admin = file_get_bitflag("admin", &io_player);
 new_player->priv_banished = file_get_bitflag("banished", &io_player);
 new_player->priv_base = file_get_bitflag("base", &io_player);

 new_player->priv_basic_su = file_get_bitflag("basic_su", &io_player);
 new_player->priv_coder = file_get_bitflag("coder", &io_player);
 new_player->priv_command_echo = file_get_bitflag("command_echo", &io_player);
 new_player->priv_command_extern_bug_suggest =
   file_get_bitflag("command_extern_bug_suggest", &io_player);
 if (FILE_IO_CREATED(&io_player))
   new_player->priv_command_extern_bug_suggest = TRUE;
 new_player->priv_command_list = file_get_bitflag("command_list", &io_player);
 new_player->priv_command_mail = file_get_bitflag("command_mail", &io_player);
 new_player->priv_command_room = file_get_bitflag("command_room", &io_player);
 new_player->priv_command_script = file_get_bitflag("command_script",
                                                     &io_player);
 new_player->priv_command_session = file_get_bitflag("command_session",
                                                      &io_player);
 new_player->priv_command_trace = file_get_bitflag("command_trace",
                                                    &io_player);
 new_player->priv_command_warn = file_get_bitflag("command_warn", &io_player);
 new_player->priv_higher_admin = file_get_bitflag("higher_admin", &io_player);
 new_player->priv_lib_maintainer = file_get_bitflag("lib_maintainer",
                                                    &io_player);
 new_player->priv_lower_admin = file_get_bitflag("lower_admin", &io_player);
 new_player->priv_minister = file_get_bitflag("minister", &io_player);
 new_player->priv_no_timeout = file_get_bitflag("no_timeout", &io_player);
 new_player->priv_normal_su = file_get_bitflag("normal_su", &io_player);
 new_player->priv_pretend_su = file_get_bitflag("pretend_su", &io_player);
 new_player->priv_senior_su = file_get_bitflag("senior_su", &io_player);
 new_player->priv_spod = file_get_bitflag("spod", &io_player);
 new_player->priv_su_channel = file_get_bitflag("su_channel", &io_player);
 new_player->priv_system_room = file_get_bitflag("system_room", &io_player);

 file_section_end("privs", &io_player);

 file_section_end("bits", &io_player);
 
 if (!new_player->priv_base)
 {
  assert(PRIV_SYSTEM_ROOM(new_player));
  
  qstrcpy(new_player->name, new_player->lower_name);
  new_player->name[0] = toupper((unsigned char)new_player->name[0]);

  file_section_end("a_saved", &io_player);
  
  file_read_close(&io_player);

  room_load_saved(new_player);
  
  return (new_player);
 }

 file_get_string("capped_name", new_player->name,
                 PLAYER_S_NAME_SZ, &io_player);
 assert(!strcasecmp(new_player->name, new_player->lower_name));
 
 file_get_string("last_dns_address", new_player->last_dns_address,
                 SOCKET_DNS_ADDRESS_SZ, &io_player);
 
 if (FILE_IO_CREATED(&io_player))
   file_get_string("last_host", new_player->last_dns_address,
                   SOCKET_DNS_ADDRESS_SZ, &io_player);
 else
 {
  file_section_beg("last_ip_address", &io_player);
  new_player->last_ip_address[0] = file_get_short("1", &io_player);
  new_player->last_ip_address[1] = file_get_short("2", &io_player);
  new_player->last_ip_address[2] = file_get_short("3", &io_player);
  new_player->last_ip_address[3] = file_get_short("4", &io_player);
  file_section_end("last_ip_address", &io_player);
 }
 
 new_player->logoff_timestamp =
   file_get_time_t("logoff_timestamp", &io_player);

 new_player->total_idle_logon = file_get_int("total_idle_logon", &io_player);
 new_player->total_logon = file_get_int("total_logon", &io_player);
 
 file_section_end("a_saved", &io_player);

 /* FIXME: timeouts... */
 
 if (!new_player->priv_banished)
   no_of_resis++;
 
 file_read_close(&io_player);

 room_load_saved(new_player);

 return (new_player);
}

static void player_load_index(void)
{
 file_io io_index;

 BUILD_FILE_ALPHA_HASH("files/player_data");

 if (file_read_open("files/player_data/index", &io_index))
 {
  unsigned int total = 1;
  unsigned int count = 0;
  
  file_section_beg("header", &io_index);
  
  total = file_get_unsigned_int("total", &io_index);
  
  file_section_end("header", &io_index);
  
  file_section_beg("list", &io_index);
  
  while (count < total)
  {
   char buffer[BUF_NUM_TYPE_SZ(int)];
   char name[PLAYER_S_NAME_SZ];
   
   sprintf(buffer, "%08d", ++count);
   
   file_section_beg(buffer, &io_index);
   
   file_get_string("name", name, PLAYER_S_NAME_SZ, &io_index);
   player_load_saved(name);
   
   file_section_end(buffer, &io_index);
  }
  
  file_section_end("list", &io_index);
  
  file_read_close(&io_index);
 }
}

void init_players(void)
{
 timer_q_add_static_base(&load_player_timer_queue, timed_player_cleanup,
                         TIMER_Q_FLAG_BASE_RUN_ALL |
                         TIMER_Q_FLAG_BASE_INSERT_FROM_END);
 
 player_load_index();
}
