#ifndef CONFIGURE_H
#define CONFIGURE_H

#define CONFIGURE_FILE_NAME_SZ 128
#define CONFIGURE_FILE_VERSION 1

#define CONFIGURE_DNS_ADDRESS_SZ 128
#define CONFIGURE_EMAIL_SZ 128
#define CONFIGURE_URL_SZ 128
#define CONFIGURE_NAME_SZ 64
#define CONFIGURE_NAME_ABBR_SZ 4 /* 3 chars max */

#define CONFIGURE_INTERFACE_TYPE_ANY 0
#define CONFIGURE_INTERFACE_TYPE_IPV4 1
#define CONFIGURE_INTERFACE_TYPE_IPV6 2
#define CONFIGURE_INTERFACE_TYPE_NAME 3

typedef struct configure_ipv4_node
{
 unsigned char ip_address[4];
} configure_ipv4_node;

typedef struct configure_ipv6_node
{
 int dummy; /* errr ... stuff */
} configure_ipv6_node;

typedef struct configure_name_node
{
 char dns_address[CONFIGURE_DNS_ADDRESS_SZ];
} configure_name_node;

typedef struct configure_interface_node
{
 struct configure_interface_node *next;
 struct configure_interface_node *prev;
 
 int type;

 int port;
 
 union configure_interface_un
 {
  configure_ipv4_node *ipv4;
  configure_ipv6_node *ipv6;
  configure_name_node *name;
 } u;
} configure_interface_node;

typedef struct configure_base
{ /* should probably add more stuff here ... Ie. default max_mails etc. */
 char configure_file_name[CONFIGURE_FILE_NAME_SZ];

 int backups_ammount;
 
 char channels_main_name[CHANNELS_NAME_SZ];
 int channels_main_name_1_1;
 int channels_main_name_2_1;
 int channels_main_name_2_2;

 int channels_players_do_all;
 int channels_players_join;

 char email_extern_bugs[CONFIGURE_EMAIL_SZ];
 char email_extern_suggest[CONFIGURE_EMAIL_SZ];

 char email_from_long[CONFIGURE_EMAIL_SZ];
 char email_from_short[CONFIGURE_EMAIL_SZ];

 bitflag email_sendmail_extern_run : 1;
 bitflag email_sendmail_run : 1;

 char email_to_abuse[CONFIGURE_EMAIL_SZ];
 char email_to_admin[CONFIGURE_EMAIL_SZ];
 char email_to_bugs[CONFIGURE_EMAIL_SZ];
 char email_to_suggest[CONFIGURE_EMAIL_SZ];
 char email_to_sus[CONFIGURE_EMAIL_SZ];
 char email_to_up_down[CONFIGURE_EMAIL_SZ];

 bitflag game_draughts_use : 1;
 bitflag game_hangman_use : 1;
 bitflag game_sps_use : 1;
 bitflag game_ttt_use : 1;
 
 int last_logon_def_show;

 int mask_coms_again_timeout;
 int mask_coms_mask_timeout;
 
 char name_abbr_lower[CONFIGURE_NAME_ABBR_SZ];
 char name_abbr_upper[CONFIGURE_NAME_ABBR_SZ];
 char *name_ascii_art;
 char name_long[CONFIGURE_NAME_SZ];
 char name_short[CONFIGURE_NAME_SZ];

 bitflag output_raw_twinkles : 1;

 char player_name_admin[PLAYER_S_NAME_SZ]; /* FIXME: need list for use in su */

 bitflag prompt_use_priority : 1;
 
 char room_connect_msg[PLAYER_S_CONNECT_MSG_SZ];
 char room_disconnect_msg[PLAYER_S_DISCONNECT_MSG_SZ];
 char room_main[PLAYER_S_NAME_SZ + ROOM_ID_SZ]; /* FIXME: need list */

 bitflag socials_use : 1;
 bitflag socials_xmas_strings : 1;
 
 configure_interface_node *socket_interfaces_start; /* FIXME: need list */
 configure_interface_node *socket_interfaces_end;
 int socket_listen_len;
 bitflag socket_tmp_no_interfaces : 1;

 int sys_nice_value;

 bitflag talker_closed_to_newbies : 1;
 bitflag talker_closed_to_resis : 1;
 bitflag talker_read_only : 1;
 bitflag talker_verbose : 1;
 
 char url_access[CONFIGURE_URL_SZ];
 char url_web[CONFIGURE_URL_SZ];
} configure_base;


#endif
