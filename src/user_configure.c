#define CONFIGURE_C
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

/* NOTE: must use user_configure for file name as autoconf claims the namespace
 * conf* (IIRC)
 */

#include "main.h"

configure_base configure = CONFIGURE_INIT_MAIN();

configure_interface_node *configure_add_interface(int port, int type)
{
 configure_interface_node *inter = NULL;

 inter = XMALLOC(sizeof(configure_interface_node), CONFIGURE_INTERFACE_NODE);
 if (!inter)
   return (NULL);

 switch (type)
 {
  case CONFIGURE_INTERFACE_TYPE_ANY:
    inter->u.ipv4 = NULL;
    break;
  case CONFIGURE_INTERFACE_TYPE_IPV4:
    if (!(inter->u.ipv4 = MALLOC(sizeof(configure_ipv4_node))))
    {
     XFREE(inter, CONFIGURE_INTERFACE_NODE);
     return (NULL);
    }
  case CONFIGURE_INTERFACE_TYPE_NAME:
    if (!(inter->u.name = MALLOC(sizeof(configure_name_node))))
    {
     XFREE(inter, CONFIGURE_INTERFACE_NODE);
     return (NULL);
    }
    break;
            
  case CONFIGURE_INTERFACE_TYPE_IPV6:
  default:
    return (NULL);
    
 }
 
 inter->type = type;
 inter->next = NULL;
 inter->prev = NULL;
 
 inter->port = port;

 if (!configure.socket_interfaces_end)
   configure.socket_interfaces_start = configure.socket_interfaces_end = inter;
 else
 {
  assert(!configure.socket_interfaces_end->next);
  assert(configure.socket_interfaces_start);
  
  configure.socket_interfaces_end->next = inter;
  inter->prev = configure.socket_interfaces_end;
  
  configure.socket_interfaces_end = inter;
 }

 return (inter);
}

void configure_del_interface(configure_interface_node *inter)
{
 if (inter->next)
   inter->next->prev = inter->prev;
 else
   configure.socket_interfaces_end = inter->prev;
 
 if (inter->prev)
   inter->prev->next = inter->next;
 else
   configure.socket_interfaces_start = inter->next;
 
 XFREE(inter, CONFIGURE_INTERFACE_NODE);
}

static void configure_load(void)
{
 file_io local_io_configure;
 file_io *io_configure = &local_io_configure;
 char file_name[sizeof("files/config/%.*s") + CONFIGURE_FILE_NAME_SZ];
 int number_of_interfaces = 0;
 int count = 0;
 
 sprintf(file_name, "files/config/%.*s",
         CONFIGURE_FILE_NAME_SZ, configure.configure_file_name);

 if (!file_read_open(file_name, io_configure))
 {
  char file_name_ren[sizeof("files/config/%.*s.tmp") + CONFIGURE_FILE_NAME_SZ];

  log_assert(FALSE);

  sprintf(file_name_ren, "%s.tmp", file_name);
  
  if (!file_write_open(file_name_ren, CONFIGURE_FILE_VERSION, io_configure))
  {
   log_assert(FALSE);
   return;
  }

  if (file_write_close(io_configure))
  {
   rename(file_name_ren, file_name);
   configure_load();
  }
  else
    log_assert(FALSE);
  
  return;
 }

 CONF_FILE_OP_INT(get, backups_ammount);
 
 CONF_FILE_OP_STR(get, channels_main_name, CHANNELS_NAME_SZ);
 CONF_FILE_OP_INT(get, channels_main_name_1_1);
 CONF_FILE_OP_INT(get, channels_main_name_2_1);
 CONF_FILE_OP_INT(get, channels_main_name_2_2);

 CONF_FILE_OP_INT(get, channels_players_do_all);
 CONF_FILE_OP_INT(get, channels_players_join);
 
 CONF_FILE_OP_STR(get, email_extern_bugs, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(get, email_extern_suggest, CONFIGURE_EMAIL_SZ);

 CONF_FILE_OP_STR(get, email_from_long, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(get, email_from_short, CONFIGURE_EMAIL_SZ);

 CONF_FILE_OP_BIT(get, email_sendmail_extern_run);
 CONF_FILE_OP_BIT(get, email_sendmail_run);

 CONF_FILE_OP_STR(get, email_to_abuse, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(get, email_to_admin, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(get, email_to_bugs, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(get, email_to_suggest, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(get, email_to_sus, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(get, email_to_up_down, CONFIGURE_EMAIL_SZ);

 CONF_FILE_OP_BIT(get, game_draughts_use);
 CONF_FILE_OP_BIT(get, game_hangman_use);
 CONF_FILE_OP_BIT(get, game_sps_use);
 CONF_FILE_OP_BIT(get, game_ttt_use);
 
 CONF_FILE_OP_INT(get, last_logon_def_show);

 CONF_FILE_OP_INT(get, mask_coms_again_timeout);
 CONF_FILE_OP_INT(get, mask_coms_mask_timeout);

 CONF_FILE_OP_STR(get, name_abbr_lower, CONFIGURE_NAME_ABBR_SZ);
 CONF_FILE_OP_STR(get, name_abbr_upper, CONFIGURE_NAME_ABBR_SZ);
 CONF_FILE_OP_MAL(get, name_ascii_art);
 CONF_FILE_OP_STR(get, name_long, CONFIGURE_NAME_SZ);
 CONF_FILE_OP_STR(get, name_short, CONFIGURE_NAME_SZ);

 CONF_FILE_OP_BIT(get, output_raw_twinkles);
 
 CONF_FILE_OP_STR(get, player_name_admin, PLAYER_S_NAME_SZ);

 CONF_FILE_OP_STR(get, room_connect_msg, PLAYER_S_CONNECT_MSG_SZ);
 CONF_FILE_OP_STR(get, room_disconnect_msg, PLAYER_S_DISCONNECT_MSG_SZ);
 CONF_FILE_OP_STR(get, room_main, PLAYER_S_NAME_SZ + ROOM_ID_SZ);

 CONF_FILE_OP_BIT(get, socials_use);
 CONF_FILE_OP_BIT(get, socials_xmas_strings);
  
 file_section_beg("socket_interfaces", io_configure);
 
 file_section_beg("header", io_configure);
 number_of_interfaces = file_get_int("number", io_configure);
 file_section_end("header", io_configure);

 if (configure.socket_tmp_no_interfaces)
   number_of_interfaces = 0;
 
 file_section_beg("interfaces", io_configure);
 while (count < number_of_interfaces)
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];
  int port = 0;
  int type = 0;
  configure_interface_node *inter = NULL;
  
  sprintf(buffer, "%04d", ++count);

  file_section_beg(buffer, io_configure);
  port = file_get_int("port", io_configure);
  type = file_get_int("type", io_configure);
  
  if (!(inter = configure_add_interface(port, type)))
    SHUTDOWN_MEM_ERR();
  assert(type == inter->type);
  
  switch (inter->type)
  {
   case CONFIGURE_INTERFACE_TYPE_ANY:
     break;
     
   case CONFIGURE_INTERFACE_TYPE_IPV4:
     file_section_beg("z", io_configure);
     file_section_beg("ipv4", io_configure);
     inter->u.ipv4->ip_address[0] = file_get_short("1", io_configure);
     inter->u.ipv4->ip_address[1] = file_get_short("2", io_configure);
     inter->u.ipv4->ip_address[2] = file_get_short("3", io_configure);
     inter->u.ipv4->ip_address[3] = file_get_short("4", io_configure);
     file_section_end("ipv4", io_configure);
     file_section_end("z", io_configure);
     break;
     
   case CONFIGURE_INTERFACE_TYPE_NAME:
     file_section_beg("z", io_configure);
     file_get_string("dns_address", inter->u.name->dns_address,
                     CONFIGURE_DNS_ADDRESS_SZ, io_configure);
     file_section_end("z", io_configure);
     break;
     
   default:
     vwlog("error", "Interface type %d\n", inter->type);
     break;
  }
  
  file_section_end(buffer, io_configure);
 }
 file_section_end("interfaces", io_configure);
 file_section_end("socket_interfaces", io_configure);

 if (!configure.socket_interfaces_start)
   if (!configure_add_interface(CONFIGURE_DEFAULT_socket_port,
                                CONFIGURE_INTERFACE_TYPE_ANY))
     SHUTDOWN_MEM_ERR();
 
 CONF_FILE_OP_INT(get, socket_listen_len);

 CONF_FILE_OP_INT(get, sys_nice_value); 
 
 CONF_FILE_OP_BIT(get, talker_closed_to_newbies);
 CONF_FILE_OP_BIT(get, talker_closed_to_resis);
 CONF_FILE_OP_BIT(get, talker_read_only);
 CONF_FILE_OP_BIT(get, talker_verbose);

 CONF_FILE_OP_STR(get, url_access, CONFIGURE_URL_SZ);
 CONF_FILE_OP_STR(get, url_web, CONFIGURE_URL_SZ);
 
 file_read_close(io_configure);
}

void configure_save(int force)
{
 file_io local_io_configure;
 file_io *io_configure = &local_io_configure;
 char file_name[sizeof("files/config/%.*s.tmp") + CONFIGURE_FILE_NAME_SZ];
 char file_name_ren[sizeof("files/config/%.*s") + CONFIGURE_FILE_NAME_SZ];
 configure_interface_node *inter = configure.socket_interfaces_start;
 int count = 0;
 
 if (!force && configure.talker_read_only)
   return;
 
 sprintf(file_name_ren, "files/config/%.*s",
         CONFIGURE_FILE_NAME_SZ, configure.configure_file_name);
 sprintf(file_name, "%s.tmp", file_name_ren);

 if (!file_write_open(file_name, CONFIGURE_FILE_VERSION, io_configure))
 {
  log_assert(FALSE);
  return;
 }

 CONF_FILE_OP_INT(put, backups_ammount);
  
 CONF_FILE_OP_STR(put, channels_main_name, CHANNELS_NAME_SZ);
 CONF_FILE_OP_INT(put, channels_main_name_1_1);
 CONF_FILE_OP_INT(put, channels_main_name_2_1);
 CONF_FILE_OP_INT(put, channels_main_name_2_2);

 CONF_FILE_OP_INT(put, channels_players_do_all);
 CONF_FILE_OP_INT(put, channels_players_join);

 CONF_FILE_OP_STR(put, email_extern_bugs, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(put, email_extern_suggest, CONFIGURE_EMAIL_SZ);

 CONF_FILE_OP_STR(put, email_from_long, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(put, email_from_short, CONFIGURE_EMAIL_SZ);

 CONF_FILE_OP_BIT(put, email_sendmail_extern_run);
 CONF_FILE_OP_BIT(put, email_sendmail_run);
 
 CONF_FILE_OP_STR(put, email_to_abuse, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(put, email_to_admin, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(put, email_to_bugs, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(put, email_to_suggest, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(put, email_to_sus, CONFIGURE_EMAIL_SZ);
 CONF_FILE_OP_STR(put, email_to_up_down, CONFIGURE_EMAIL_SZ);

 CONF_FILE_OP_BIT(put, game_draughts_use);
 CONF_FILE_OP_BIT(put, game_hangman_use);
 CONF_FILE_OP_BIT(put, game_sps_use);
 CONF_FILE_OP_BIT(put, game_ttt_use);

 CONF_FILE_OP_INT(put, last_logon_def_show);

 CONF_FILE_OP_INT(put, mask_coms_again_timeout);
 CONF_FILE_OP_INT(put, mask_coms_mask_timeout);

 CONF_FILE_OP_STR(put, name_abbr_lower, CONFIGURE_NAME_ABBR_SZ);
 CONF_FILE_OP_STR(put, name_abbr_upper, CONFIGURE_NAME_ABBR_SZ);
 CONF_FILE_OP_MAL(put, name_ascii_art);
 CONF_FILE_OP_STR(put, name_long, CONFIGURE_NAME_SZ);
 CONF_FILE_OP_STR(put, name_short, CONFIGURE_NAME_SZ);

 CONF_FILE_OP_BIT(put, output_raw_twinkles);
 
 CONF_FILE_OP_STR(put, player_name_admin, PLAYER_S_NAME_SZ);

 CONF_FILE_OP_STR(put, room_connect_msg, PLAYER_S_CONNECT_MSG_SZ);
 CONF_FILE_OP_STR(put, room_disconnect_msg, PLAYER_S_DISCONNECT_MSG_SZ);
 CONF_FILE_OP_STR(put, room_main, PLAYER_S_NAME_SZ + ROOM_ID_SZ);

 CONF_FILE_OP_BIT(put, socials_use);
 CONF_FILE_OP_BIT(put, socials_xmas_strings);
  
 file_section_beg("socket_interfaces", io_configure);

 file_section_beg("header", io_configure);
 while (inter)
 {
  ++count;
  inter = inter->next;
 }
 file_put_int("number", count, io_configure);
 file_section_end("header", io_configure);
 
 file_section_beg("interfaces", io_configure);

 count = 0;
 inter = configure.socket_interfaces_start;
 while (inter)
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];

  sprintf(buffer, "%04d", ++count);

  file_section_beg(buffer, io_configure);
  file_put_int("port", inter->port, io_configure);
  file_put_int("type", inter->type, io_configure);

  switch (inter->type)
  {
   case CONFIGURE_INTERFACE_TYPE_ANY:
     break;
     
   case CONFIGURE_INTERFACE_TYPE_IPV4:
     file_section_beg("z", io_configure);
     file_section_beg("ipv4", io_configure);
     file_put_short("1", inter->u.ipv4->ip_address[0], io_configure);
     file_put_short("2", inter->u.ipv4->ip_address[1], io_configure);
     file_put_short("3", inter->u.ipv4->ip_address[2], io_configure);
     file_put_short("4", inter->u.ipv4->ip_address[3], io_configure);
     file_section_end("ipv4", io_configure);
     file_section_end("z", io_configure);
     break;
     
   case CONFIGURE_INTERFACE_TYPE_NAME:
     file_section_beg("z", io_configure);
     file_put_string("dns_address", inter->u.name->dns_address, 0, io_configure);
     file_section_end("z", io_configure);
     break;
     
   default:
     vwlog("error", "Interface type %d\n", inter->type);
     break;
  }
  
  file_section_end(buffer, io_configure);
  inter = inter->next;
 }
 
 file_section_end("interfaces", io_configure);
 file_section_end("socket_interfaces", io_configure);

 CONF_FILE_OP_INT(put, socket_listen_len);

 CONF_FILE_OP_INT(put, sys_nice_value);

 CONF_FILE_OP_BIT(put, talker_closed_to_newbies);
 CONF_FILE_OP_BIT(put, talker_closed_to_resis);
 CONF_FILE_OP_BIT(put, talker_read_only);
 CONF_FILE_OP_BIT(put, talker_verbose);
 
 CONF_FILE_OP_STR(put, url_access, CONFIGURE_URL_SZ);
 CONF_FILE_OP_STR(put, url_web, CONFIGURE_URL_SZ);

 if (file_write_close(io_configure))
   rename(file_name, file_name_ren);
}

void init_configure(void)
{
 configure_load();
}

#ifdef TALKER_MAIN_H
static void configure_rejoin_func(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", 
               " Re-Entering configure mode. Use ^Bhelp configure^N for help\n"
               " To run ^Bnormal commands^N type /<command>\n"
               " Use '^Bend^N' to ^Bleave^N.\n");
}

static int configure_command(player *p, const char *str, size_t length)
{
 ICTRACE("configure_command");
 
 if (MODE_IN_MODE(p, CONFIGURE))
   MODE_HELPER_COMMAND();
 else
   if (!*str)
   {
    cmds_function tmp_cmd;
    cmds_function tmp_rejoin;
    
    CMDS_FUNC_TYPE_RET_CHARS_SIZE_T(&tmp_cmd, configure_command);
    CMDS_FUNC_TYPE_NO_CHARS(&tmp_rejoin, configure_rejoin_func);
    
    if (mode_add(p, "Configure Mode-> ", MODE_ID_CONFIGURE, 0,
                 &tmp_cmd, &tmp_rejoin, NULL))
      fvtell_player(NORMAL_T(p), "%s", 
                    " Entering configure mode. Use ^Bhelp configure^N "
                    "for help\n"
                    " To run ^Bnormal commands^N type /<command>\n"
                    " Use '^Bend^N' to ^Bleave^N.\n");
    else
      fvtell_player(SYSTEM_T(p), 
                    " You cannot enter configure mode as you are in too many "
                    "other modes.\n");
    return (TRUE);
   }

 return (cmds_sub_match(p, str, length,
                        &cmds_sub[CMDS_SUB_OFFSET(CMDS_SECTION_CONFIGURE)]));
}

static void configure_exit_mode(player *p)
{
 assert(MODE_IN_MODE(p, CONFIGURE));
 
 fvtell_player(NORMAL_T(p), "%s", " Leaving configure mode.\n");

 mode_del(p);
}

static void configure_view_commands(player *p)
{
 user_cmds_show_section(p, "configure");
}

static void user_configure_show(player *p)
{
 char buf[256];
 
 ptell_mid(NORMAL_T(p), "Configuration", FALSE);

 fvtell_player(NORMAL_T(p), "Configure-File-Name: %s\n",
               configure.configure_file_name);

 fvtell_player(NORMAL_T(p), "Backups-Ammount: %d\n",
               configure.backups_ammount);
 
 fvtell_player(NORMAL_T(p), "Channels-Main-Name: %s\n",
               configure.channels_main_name);
 fvtell_player(NORMAL_T(p), "Channels-1-Letter-Shortcut: '%c'\n",
               configure.channels_main_name_1_1);
 fvtell_player(NORMAL_T(p), "Channels-2-Letter-Shortcut: '%c' '%c'\n",
               configure.channels_main_name_2_1,
               configure.channels_main_name_2_2);

 fvtell_player(NORMAL_T(p), "Channels-Players-Do-All: %d\n",
               configure.channels_players_do_all);
 fvtell_player(NORMAL_T(p), "Channels-Players-Join: %d\n",
               configure.channels_players_join);

 fvtell_player(NORMAL_T(p), "Email-Extern-Bugs: %s\n",
               configure.email_extern_bugs);
 fvtell_player(NORMAL_T(p), "Email-Extern-Suggest: %s\n",
               configure.email_extern_suggest);

 fvtell_player(NORMAL_T(p), "Email-From-Long: %s\n",
               configure.email_from_long);
 fvtell_player(NORMAL_T(p), "Email-From-Short: %s\n",
               configure.email_from_short);

 fvtell_player(NORMAL_T(p), "Email-Sendmail-Extern-Run: %s\n",
               TOGGLE_TRUE_FALSE(configure.email_sendmail_extern_run));
 fvtell_player(NORMAL_T(p), "Email-Sendmail-Run: %s\n",
               TOGGLE_TRUE_FALSE(configure.email_sendmail_run));
 
 fvtell_player(NORMAL_T(p), "Email-To-Abuse: %s\n",
               configure.email_to_abuse);
 fvtell_player(NORMAL_T(p), "Email-To-Admin: %s\n",
               configure.email_to_admin);
 fvtell_player(NORMAL_T(p), "Email-To-Bugs: %s\n",
               configure.email_to_bugs);
 fvtell_player(NORMAL_T(p), "Email-To-Suggest: %s\n",
               configure.email_to_suggest);
 fvtell_player(NORMAL_T(p), "Email-To-Sus: %s\n",
               configure.email_to_sus);
 fvtell_player(NORMAL_T(p), "Email-To-Up/Down: %s\n",
               configure.email_to_up_down);

 fvtell_player(NORMAL_T(p), "Game-Draughts-Use: %s\n",
               TOGGLE_TRUE_FALSE(configure.game_draughts_use));
 fvtell_player(NORMAL_T(p), "Game-Hangman-Use: %s\n",
               TOGGLE_TRUE_FALSE(configure.game_hangman_use));
 fvtell_player(NORMAL_T(p), "Game-Sps-Use: %s\n",
               TOGGLE_TRUE_FALSE(configure.game_sps_use));
 fvtell_player(NORMAL_T(p), "Game-Ttt-Use: %s\n",
               TOGGLE_TRUE_FALSE(configure.game_ttt_use)); 
 
 fvtell_player(NORMAL_T(p), "Last-Logon-Def-Show: %d\n",
               configure.last_logon_def_show);

 fvtell_player(NORMAL_T(p), "Mask-Coms-Again-Timeout: %s\n",
               word_time_short(buf, sizeof(buf),
                               configure.mask_coms_again_timeout,
                               WORD_TIME_DEFAULT));
 fvtell_player(NORMAL_T(p), "Mask-Coms-Mask-Timeout: %s\n",
               word_time_short(buf, sizeof(buf),
                               configure.mask_coms_mask_timeout,
                               WORD_TIME_DEFAULT));

 fvtell_player(NORMAL_T(p), "Name-Abbr-Lower: %s\n",
               configure.name_abbr_lower);
 fvtell_player(NORMAL_T(p), "Name-Abbr-Upper: %s\n",
               configure.name_abbr_upper);

 log_assert(configure.name_ascii_art);
 fvtell_player(NORMAL_T(p), "Name-Ascii-Art: \n%s\n",
               configure.name_ascii_art);
 
 fvtell_player(NORMAL_T(p), "Name-Long: %s\n",
               configure.name_long);
 fvtell_player(NORMAL_T(p), "Name-Short: %s\n",
               configure.name_short);

 fvtell_player(NORMAL_T(p), "Output-Raw-Twinkles: %s\n",
               TOGGLE_TRUE_FALSE(configure.output_raw_twinkles));

 fvtell_player(NORMAL_T(p), "Player-Name-Admin: %s\n",
               configure.player_name_admin);
 
 fvtell_player(NORMAL_T(p), "Room-Connect-Msg: %s\n",
               configure.room_connect_msg);
 fvtell_player(NORMAL_T(p), "Room-Disconnect-Msg: %s\n",
               configure.room_disconnect_msg);

 fvtell_player(NORMAL_T(p), "Room-Main: %s\n",
               configure.room_main);

 fvtell_player(NORMAL_T(p), "Socials-Use: %s\n",
               TOGGLE_TRUE_FALSE(configure.socials_use));
 fvtell_player(NORMAL_T(p), "Socials-Xmas-Strings: %s\n",
               TOGGLE_TRUE_FALSE(configure.socials_xmas_strings));

 socket_interfaces_show(p, "Socket-Interfaces");
 
 fvtell_player(NORMAL_T(p), "Socket-Listen-Length: %d\n",
               configure.socket_listen_len);

 fvtell_player(NORMAL_T(p), "Sys-Nice-Value: %d\n",
               configure.sys_nice_value);

 fvtell_player(NORMAL_T(p), "Talker-Closed-To-Newbies: %s\n",
               TOGGLE_TRUE_FALSE(configure.talker_closed_to_newbies));
 fvtell_player(NORMAL_T(p), "Talker-Closed-To-Residents: %s\n",
               TOGGLE_TRUE_FALSE(configure.talker_closed_to_resis));
 fvtell_player(NORMAL_T(p), "Talker-Read-Only: %s\n",
               TOGGLE_TRUE_FALSE(configure.talker_read_only));
 fvtell_player(NORMAL_T(p), "Talker-Verbose: %s\n",
               TOGGLE_TRUE_FALSE(configure.talker_verbose));

 fvtell_player(NORMAL_T(p), "Url-Access: %s\n",
               configure.url_access);
 fvtell_player(NORMAL_T(p), "Url-Web: %s\n",
               configure.url_web);

 fvtell_player(NORMAL_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p), "%s", DASH_LEN);

 pager(p, PAGER_DEFAULT);
}

static void user_configure_save(player *p, const char *str)
{
 fvtell_player(NORMAL_T(p), "%s",
               " Saved the configure file (this gets done "
               "automatically if you change anything ... but you might "
               "want to do it if you have a default value that you want "
               "saved _OR_ you want to save the _read_only_ variable).\n");
 
 if (!strcasecmp(str, "force"))
   configure_save(TRUE);
 else
   configure_save(FALSE);
}

static void user_configure_configure_file_name(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.configure_file_name, str,
                                "configure file name", CONFIGURE_FILE_NAME_SZ);
 configure_save(FALSE);
}

static void user_configure_name_abbr(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.name_abbr_lower, str,
                                "abbreviated name", CONFIGURE_NAME_ABBR_SZ);
 qstrcpy(configure.name_abbr_upper, configure.name_abbr_lower);
 lower_case(configure.name_abbr_lower);
 upper_case(configure.name_abbr_upper);
 configure_save(FALSE);
}

static void user_configure_name_long(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.name_long, str,
                                "long name", CONFIGURE_NAME_SZ);
 configure_save(FALSE);
}

static void configure_ascii_art_edit_cleanup(player *p)
{ 
 buffer_file_destroy(p);
}

static void configure_ascii_art_edit_quit(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", " Leaving without changes.\n");

 configure_ascii_art_edit_cleanup(p);
}

static void configure_ascii_art_edit_end(player *p)
{
 char *tmp = NULL;
 
 assert(MODE_IN_MODE(p, EDIT));

 if (!(tmp = edit_malloc_dump(p, NULL)))
   P_MEM_ERR(p);
 else
 {
  fvtell_player(NORMAL_T(p), "%s", " Name Ascii Art changed.\n");
  FREE(configure.name_ascii_art);
  configure.name_ascii_art = tmp;
  configure_save(FALSE);
 }

 configure_ascii_art_edit_cleanup(p);
}

static void user_configure_name_ascii_art(player *p)
{
 int created = 0;
 
 if ((created = buffer_file_create(p)) > 0)
   P_MEM_ERR(p);
 else if (created < 0)
   fvtell_player(NORMAL_T(p), "%s",
                 " You cannot edit the  whilst already editing a file.\n");
 else if (edit_start(p, configure.name_ascii_art))
 {
  assert(MODE_IN_MODE(p, EDIT));
  
  edit_limit_lines(p, 12); /* half a screen */
  
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_quit,
                          configure_ascii_art_edit_quit);
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_end,
                          configure_ascii_art_edit_end);
 }
 else
   buffer_file_destroy(p);
}

static void user_configure_name_short(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.name_short, str,
                                "short name", CONFIGURE_NAME_SZ);
 configure_save(FALSE);
}

static void user_configure_output_raw_twinkles(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, configure.output_raw_twinkles, TRUE,
                       " Twinkles will %sbe parsed.\n",
                       " Twinkles will %sbe ignored.\n", TRUE);
 configure_save(FALSE);
}

static void user_configure_player_name_admin(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.player_name_admin, str,
                                "admin player name", CONFIGURE_NAME_SZ);
 if (!player_find_all(p, configure.player_name_admin, PLAYER_FIND_VERBOSE |
                      PLAYER_FIND_SELF))
   return;
 configure_save(FALSE);
}

static void user_configure_sys_nice_value(player *p, const char *str)
{
 USER_CONFIGURE_INT_FUNC(sys_nice_value, "Sys", "nice value", 0, 20);

 /* can't call nice ... as it's cumlative */
 fvtell_player(NORMAL_T(p), "%s", 
               " This change won't take effect untill you reboot.\n");
 configure_save(FALSE);
}

static void user_configure_talker_verbose(player *p, const char *str)
{
 int tmp = configure.talker_verbose;
 TOGGLE_COMMAND_ON_OFF(p, str, configure.talker_verbose, TRUE,
                       " VERBOSE is %son.\n",
                       " VERBOSE is %soff.\n", TRUE);
 if (tmp != configure.talker_verbose)
   configure_save(FALSE);
}

static void user_configure_talker_read_only(player *p, const char *str)
{
 int tmp = configure.talker_read_only;
 TOGGLE_COMMAND_ON_OFF(p, str, configure.talker_read_only, TRUE,
                       " Read only is %son.\n",
                       " Read only is %soff.\n", TRUE);

 if (tmp != configure.talker_read_only)
   configure_save(FALSE);
}

static void user_configure_url_access(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.url_access, str,
                                "url to access the talker", CONFIGURE_URL_SZ);
 configure_save(FALSE);
}

static void user_configure_url_web(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.url_web, str,
                                "url for the web page", CONFIGURE_URL_SZ);
 configure_save(FALSE);
}

void cmds_init_configure(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("configure", configure_command, RET_CHARS_SIZE_T, ADMIN);
 CMDS_PRIV(higher_admin);
 
#define CMDS_SECTION_SUB CMDS_SECTION_CONFIGURE

 CMDS_ADD_SUB("commands", configure_view_commands, NO_CHARS);
 CMDS_ADD_SUB("end", configure_exit_mode, NO_CHARS);
 CMDS_FLAG(no_expand); CMDS_PRIV(mode_configure);
 
 CONF_CMD(show, NO_CHARS);
 CONF_CMD(save, CONST_CHARS);
 
 CONF_CMD(configure_file_name, CONST_CHARS);

 CONF_CMD(backups_ammount, CONST_CHARS);

 CONF_CMD(channels_main_name, PARSE_PARAMS); /* 2 become one + a bit :*/
 CONF_CMD(channels_players_do_all, CONST_CHARS);
 CONF_CMD(channels_players_join, CONST_CHARS);

 CONF_CMD(email_extern_bugs, CONST_CHARS);
 CONF_CMD(email_extern_suggest, CONST_CHARS);

 CONF_CMD(email_from_long, CONST_CHARS);
 CONF_CMD(email_from_short, CONST_CHARS);

 CONF_CMD(email_sendmail_extern_run, CONST_CHARS);
 CONF_CMD(email_sendmail_run, CONST_CHARS);
 
 CONF_CMD(email_to_abuse, CONST_CHARS);
 CONF_CMD(email_to_admin, CONST_CHARS);
 CONF_CMD(email_to_bugs, CONST_CHARS);
 CONF_CMD(email_to_suggest, CONST_CHARS);
 CONF_CMD(email_to_sus, CONST_CHARS);
 CONF_CMD(email_to_up_down, CONST_CHARS);

 CONF_CMD(game_draughts_use, CONST_CHARS);
 CONF_CMD(game_hangman_use, CONST_CHARS);
 CONF_CMD(game_sps_use, CONST_CHARS);
 CONF_CMD(game_ttt_use, CONST_CHARS);

 CONF_CMD(last_logon_def_show, CONST_CHARS);

 CONF_CMD(mask_coms_again_timeout, CONST_CHARS);
 CONF_CMD(mask_coms_mask_timeout, CONST_CHARS);

 CONF_CMD(name_abbr, CONST_CHARS); /* 2 become one ?:*/
 
 CONF_CMD(name_ascii_art, NO_CHARS);
 CONF_CMD(name_long, CONST_CHARS);
 CONF_CMD(name_short, CONST_CHARS);

 CONF_CMD(output_raw_twinkles, CONST_CHARS);
 
 CONF_CMD(player_name_admin, CONST_CHARS);

 CONF_CMD(room_connect_msg, CONST_CHARS);
 CONF_CMD(room_disconnect_msg, CONST_CHARS);
 CONF_CMD(room_main, PARSE_PARAMS);

 CONF_CMD(socials_use, CONST_CHARS);
 CONF_CMD(socials_xmas_strings, CONST_CHARS);
 
 CONF_CMD(socket_interfaces, PARSE_PARAMS);
 CONF_CMD(socket_listen_len, CONST_CHARS);
 
 CONF_CMD(talker_closed_to_newbies, CONST_CHARS);
 CONF_CMD(talker_closed_to_resis, CONST_CHARS);
 CONF_CMD(talker_read_only, CONST_CHARS); /* this might well be a bad idea */
 CONF_CMD(talker_verbose, CONST_CHARS);

 CONF_CMD(sys_nice_value, CONST_CHARS);
 
 CONF_CMD(url_access, CONST_CHARS);
 CONF_CMD(url_web, CONST_CHARS);
 
#undef CMDS_SECTION_SUB
}
#endif
