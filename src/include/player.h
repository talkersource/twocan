#ifndef PLAYER_H
#define PLAYER_H
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

/* need a better namespace than player_ so use player_s_ for things about
 * the structure */
/* FIXME: need to lower all these and use buffers instead ... */
#define PLAYER_S_NAME_SZ 20
#define PLAYER_S_TITLE_SZ 200
#define PLAYER_S_OUTPUT_TYPE_SZ 4
#define PLAYER_S_COMMENT_SZ 200
#define PLAYER_S_SCRIPT_FILE_SZ (PLAYER_S_NAME_SZ + \
                                 sizeof("log/emergency/%s_emergency.log"))
#define PLAYER_S_TITLE_SZ 200
#define PLAYER_S_PREFIX_SZ 18
#define PLAYER_S_DESCRIPTION_SZ 400
#define PLAYER_S_PLAN_SZ 400
#define PLAYER_S_URL_SZ 255
#define PLAYER_S_PHONE_NUMBERS_SZ 64
#define PLAYER_S_SU_COMMENT_SZ 255
#define PLAYER_S_IDLE_MSG_SZ 200
#define PLAYER_S_IGNORE_MSG_SZ 64
#define PLAYER_S_EMAIL_SZ 64
#define PLAYER_S_PASSWD_SZ 32
#define PLAYER_S_ENTER_MSG_SZ 60
#define PLAYER_S_ROOM_CONNECT_SZ 35
#define PLAYER_S_CONNECT_MSG_SZ 200
#define PLAYER_S_DISCONNECT_MSG_SZ 200
#define PLAYER_S_WARN_SZ 128
#define PLAYER_S_SIG_SZ (80 * 4) /* four lines ... make em be nice :) */

#define NATIONALITY_VOID 0
#define NATIONALITY_BRITISH 1
#define NATIONALITY_AMERICAN 2
#define NATIONALITY_CANADIAN 3
#define NATIONALITY_AUSTRALIAN 4
#define NATIONALITY_OTHER 5

#define GENDER_MALE 1
#define GENDER_FEMALE 2
#define GENDER_PLURAL 3 /* thanks grim ... */
#define GENDER_OTHER 0

#define OUT_LENGTH_COMMUNICATION 256
#define OUT_LENGTH_COMMENT 57
#define OUT_LENGTH_TITLE 59
#define OUT_LENGTH_ROOM_NAME 59
#define OUT_LENGTH_CONNECT_MSGS 59
#define OUT_LENGTH_ENTER_MSG 59
#define OUT_LENGTH_INFO 250 /* eg plan and description */
#define OUT_LENGTH_AUTOMESSAGE 59
#define OUT_LENGTH_NEWS_SUBJECT 128
#define OUT_LENGTH_NEWS_POST 5000
#define OUT_LENGTH_MAIL_SUBJECT 128
#define OUT_LENGTH_MAIL_POST 5000

/* number of times $Return works */
#define OUT_RETURNS_INFO 4

typedef struct tmp_output_list_storage
{
 output_node *output_buffer_tmp;
#if 0
 unsigned int output_type[PLAYER_S_OUTPUT_TYPE_SZ];
 int type_ptr;
#endif
} tmp_output_list_storage;

typedef struct player
{ /* NOTE: top member needs to be capable of holding a ZERO value */
 struct player_tree_node *saved;

 Socket_poll_typedef_offset io_indicator;
 
 struct player_linked_double_list logon_list_io;
 struct player_linked_double_list logon_list_alpha;
 struct player_linked_double_list logon_list_cron;

 struct player_linked_double_list room_list_alpha;
 struct player_linked_double_list room_list_cron;
 
 struct room *location;

 struct input_node *input_start;
 unsigned int input_node_count;
 char input_last_eol;
 char input_doing_iac; /* length of temp stored IAC commands */
 unsigned int input_comp_count; /* number of computer generted inputs run */
 
 struct output_node *output_start;
 struct output_node *output_buffer_tmp;

 int max_aliases;
 int number_of_aliases;
 struct alias_node *aliases_start;
 char *alias_lib_saved_list[ALIAS_LIB_LD_SO_SZ];
 struct alias_lib_node *alias_lib_ldconfig_list[ALIAS_LIB_LD_SO_SZ];
 time_t alias_lib_ld_cache_timestamp;
 
 int max_nicknames;
 int number_of_nicknames;
 struct nickname_node *nicknames_start;

 struct list_node *list_self_tmp_start;
 struct list_node *list_self_start;
 struct list_node *list_coms_tmp_start;
 struct list_node *list_coms_start;
 struct list_node *list_game_start;
 
 unsigned int mode_count;
 struct generic_mode
 { /* this needs to go here due to referencing... */
  const char *prompt;
  unsigned int id;
  unsigned int flags;
  struct cmds_function
  {
   int type;
   
   union cmds_function_union
   {
    void (*player_and_const_chars) (struct player *, const char *);
    void (*player_and_chars_and_length) (struct player *, const char *,
                                         size_t);
    int  (*player_ret_and_chars_and_length) (struct player *,
                                             const char *, size_t);
    void (*player_only) (struct player *);
    void (*player_and_parsed_params) (struct player *,
                                      struct parameter_holder *);
   } func;
  } cmd_func;
  struct cmds_function rejoin_func;
  struct cmds_function cleanup_func;
 } modes[MODE_MODES_SZ];
 
 struct player *ttt_oponent;
 char ttt_op_name[PLAYER_S_NAME_SZ];

 Timer_q_double_node idle_timer;
 
 int idle_logon;
 
 int typed_commands;
 
 int last_commands_index;
 struct last_command_node last_commands[LAST_COMMANDS_SZ];

/* scheduling ... */
 struct timeval schedule_can_go;
 unsigned long int schedule_time;

 /* twinkles... */
 unsigned int twink_dep_msg_command_number;
 long twinkle_counter[OUTPUT_VARIABLES_COUNT_SZ];

 /* specials (wands) ... */
 unsigned int output_type[PLAYER_S_OUTPUT_TYPE_SZ];
 int type_ptr;
 
 time_t last_command_timestamp;
 time_t logon_timestamp;
 time_t logoff_started_timestamp;
 /* logoff_timestamp in tree node */
 
 bitflag loaded_player : 1;
 bitflag is_fully_on : 1;
 bitflag allow_run_commands : 1;
 bitflag output_has_priority : 1;
 bitflag automatic_term_size_got : 1;
 bitflag automatic_term_name_got : 1;
 bitflag passwd_mode : 1;
 bitflag flag_tmp_prompt_do_output : 1;
 bitflag see_raw_twinkles : 1;
 bitflag flag_tmp_dont_do_normal_msgs : 1;
 bitflag flag_tmp_su_channel_off : 1;
 bitflag flag_tmp_su_channel_block : 1;
 bitflag flag_tmp_minister_channel_block : 1;
 bitflag flag_tmp_scripting : 1;
 bitflag draughts_as_checkers : 1; /* should be done on nationality */
 bitflag system_info_only : 1;
 bitflag idle_in_queue : 1;
 bitflag flag_tmp_idle_had_warn : 1;
 bitflag flag_tmp_dont_save_after_this : 1;
 bitflag extra_screen_routines : 1;
 bitflag repeat_prompt : 1;
 bitflag ttt_playing : 1; 
 bitflag ttt_noughts : 1;
 bitflag ttt_my_go : 1;
 bitflag telnet_option_eor_on : 1;
 bitflag telnet_option_bounce_echo_off : 1;
 bitflag telnet_option_passwd_restop : 1;
 bitflag telnet_option_do_echo : 1;
 bitflag alias_list_code : 1; /* comments or code by default */
 bitflag output_compress_do : 1;
 
 /* telnet option stuff... */
 bitflag input_in_sub_buf : 1;
 bitflag telopt_do_echo : 1;
 bitflag telopt_do_eor : 1;
 bitflag telopt_do_sga : 1;
 bitflag telopt_do_naws : 1;
 bitflag telopt_do_compress : 1;

 /* saved values... */
 bitflag flag_no_cmd_matching : 1;
 bitflag flag_use_birthday_as_age : 1;
 bitflag flag_birthday_show_year : 1;

 bitflag flag_terminal_ansi_colour_override : 1;
 bitflag flag_audible_bell : 1;
 bitflag flag_allow_bell : 1;
 bitflag flag_use_24_clock : 1;
 bitflag flag_use_long_clock : 1;
 bitflag flag_follow_block : 1;
 bitflag flag_room_enter_brief : 1;
 bitflag flag_social_auto_name : 1;
 bitflag flag_idle_warns_block : 1;
 bitflag flag_crazy_channel_block : 1;
 bitflag flag_spod_channel_block : 1;
 bitflag flag_married : 1;
 bitflag flag_marriages_block : 1;
 bitflag flag_page_on_return : 1;
 bitflag flag_no_colour_from_others : 1;
 bitflag flag_no_specials_from_others : 1;
 bitflag flag_just_normal_hilight : 1;
 bitflag flag_marriage_hide : 1;
 bitflag flag_no_info_in_who : 1;
 bitflag flag_pager_auto_quit : 1;
 bitflag flag_no_net_spouse : 1;
 bitflag flag_social_auto_remote : 1;
 bitflag flag_input_keep_going : 1;
 bitflag flag_list_show_inorder : 1;
 bitflag flag_gmt_offset_hide : 1;
 bitflag flag_crazy_news : 1;
 bitflag flag_crazy_news_bulletin : 1;
 bitflag flag_show_personal : 1;
 bitflag flag_show_shouts : 1;
 bitflag flag_show_room : 1;
 bitflag flag_show_autos : 1;
 bitflag flag_show_socials : 1;
 bitflag flag_show_echo : 1;
 bitflag flag_location_hide : 1;
 bitflag flag_quiet_edit : 1;
 bitflag flag_trans_to_home : 1;
 bitflag flag_see_echo : 1;
 bitflag flag_no_prefix : 1;
 bitflag flag_no_emote_prefix : 1;
 bitflag flag_pager_off : 1;
 bitflag flag_session_in_who : 1;
 bitflag flag_see_room_events : 1;
 bitflag flag_room_exits_show : 1;
 bitflag flag_mask_coms_block : 1;
 bitflag flag_raw_passwd : 1; /* if their password is in normal text */

 /* games stuff... */
 bitflag flag_draughts_no_auto_show_player : 1;
 bitflag flag_draughts_no_auto_show_my_move : 1;
 bitflag flag_draughts_no_auto_show_watch : 1;
 bitflag flag_draughts_auto_private : 1;
 bitflag flag_sps_brief : 1;
 bitflag flag_sps_ai_disabled : 1;
 bitflag flag_ttt_brief : 1;

 int event;
 int multi_last_used;

 char prompt[PROMPT_SZ];
 char converse_prompt[PROMPT_SZ];
 
 char prompt_output[PROMPT_OUTPUT_SZ];
 unsigned int prompt_length;
 unsigned int prompt_out_length;
 unsigned int prompt_print_length;

 time_t prompt_last_output;
 
 char comment[PLAYER_S_COMMENT_SZ];

 char script_file[PLAYER_S_SCRIPT_FILE_SZ];
 
 struct player *assisted_player; /* either the person you are assisting ...
                                  * or the person assisting you */

 struct buffers_struct *buffers;
 struct draughts_game *draughts_game; /* for draughts game */
 struct sps_struct *sps_cl_mem; /* for sps playing puter */

 /* FIXME: nuked */
 struct player_tree_node *sps_playing; /* for sps game */
 int sps_chosen;  
 
 struct terminal_termcap *termcap;

 unsigned int column;

 unsigned int term_width;
 unsigned int term_height; /* auto set atm. as well as saved */
 
 /* saved values ... */
 unsigned int word_wrap;
 
 int gender;
 
 int gmt_offset;

 time_t birthday;
 int age;

 int max_rooms; /* FIXME: change namespace */
 int max_exits;
 int max_autos;
 int max_list_entries;
 int max_mails;

 int list_newbie_time;
 
 char dns_address[SOCKET_DNS_ADDRESS_SZ];
 unsigned char ip_address[4];

 char title[PLAYER_S_TITLE_SZ];
 
 char prefix[PLAYER_S_PREFIX_SZ];
 
 char description[PLAYER_S_DESCRIPTION_SZ];
 
 char plan[PLAYER_S_PLAN_SZ];
 
 char url[PLAYER_S_URL_SZ];

 char phone_numbers[PLAYER_S_PHONE_NUMBERS_SZ];
 
 char su_comment[PLAYER_S_SU_COMMENT_SZ];

 char idle_msg[PLAYER_S_IDLE_MSG_SZ];
 
 char ignore_msg[PLAYER_S_IGNORE_MSG_SZ];

 char email[PLAYER_S_EMAIL_SZ];
 
 char passwd[PLAYER_S_PASSWD_SZ];
 char passwd_change_tmp[PLAYER_S_PASSWD_SZ];

 char enter_msg[PLAYER_S_ENTER_MSG_SZ];

 char room_connect[PLAYER_S_ROOM_CONNECT_SZ];

 char connect_msg[PLAYER_S_CONNECT_MSG_SZ];
 
 char disconnect_msg[PLAYER_S_DISCONNECT_MSG_SZ];

 char follow[PLAYER_S_NAME_SZ];

 char married_to[PLAYER_S_NAME_SZ];
 
 char saved_warn[PLAYER_S_WARN_SZ];
 
 char sig[PLAYER_S_SIG_SZ];

 int default_newsgroup;
 int saved_default_newsgroup;

 int nationality;

 time_t rep_output_last_used;

 time_t motdlr;
 time_t su_motdlr;
 
 int longest_session;

 int autoexec; /* FIXME: change from an int */

 int draughts_played;
 int draughts_won;
 int draughts_board;

 int sps_played;
 int sps_won;
 int sps_drawn;
 time_t sps_ai_disabled_till;

 int ttt_played;
 int ttt_won;
 int ttt_drawn;
 int ttt_board; /* tmp */

 time_t mask_coms_hit_timestamp; /* saved */
 time_t mask_coms_last_timestamp; /* tmp */
 int mask_coms_type; /* tmp */
 
 struct timeval total_cpu;
 struct timeval this_cpu;
 struct timeval comp_cpu; /* for compression */
 
 /* FIXME: +1 needed for convert */
 unsigned int output[OUTPUT_COLOUR_SZ + 1];

 int karma_used;
 int karma_cutoff;

#if 0
 struct Alarm_node *alarm_start;
 unsigned int alarm_num;
#endif
 char *alarm_str;
 
 time_t emailed_passwd_timestamp;
 
 hangman_type *hangman;
 
#ifdef HAVE_ZLIB_H
 z_stream *output_compress_lib;
 unsigned char *output_compress_buf; /* start of memory */
 unsigned char *output_compress_ptr; /* start of data */
#endif
} player;

#endif
