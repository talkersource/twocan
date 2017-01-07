#ifndef PLAYER_TREE_H
#define PLAYER_TREE_H

#ifdef PLAYER_TREE_C
/* internal values .... */

/* for balance */
# define EQUAL_HEIGHT 0
# define RIGHT_HEAVY 1
# define LEFT_HEAVY (-1)

# define FULL_PROB   512
# define HALF_PROB   256
# define SMALL_PROB   8
# define LEVELS_MOST  10

#endif

typedef struct player_tree_node
{
 /* FIXME: player_linked_list */
 struct player_tree_node *next;
 struct player_tree_node *prev;
 
 /* tree stuff */
 struct player_tree_node *left;
 struct player_tree_node *right;

 signed char balance;

 time_t c_timestamp;
 time_t a_timestamp; /* auto cleanup -- player */
 time_t l_timestamp; /* auto cleanup -- player */
 Timer_q_node load_timer;

 time_t list_a_timestamp; /* auto cleanup -- room glob list */ 
 time_t list_l_timestamp; /* auto cleanup -- room glob list */
 Timer_q_node list_load_timer;

 struct list_node *list_room_glob_start;
 
 struct room *rooms_start;

 struct mail_recieved *mail_recieved_start;
 struct mail_sent *mail_sent_start;

 struct multi_node *multis_start;
 struct channels_node *channels_start;

 struct player *player_ptr;
 
 /* spodlist stuff */
 /* FIXME: player_linked_list */
 struct player_tree_node *next_num;
 struct player_tree_node *prev_num;

 unsigned int spod_number;
 unsigned int spod_offset;

 /* number of rooms */
 unsigned int rooms_num;
 
 /* NOTE: logon time for the player... Only accurate if they are
    not logged on */
 int total_logon;
 /* NOTE: idle time for the player... Only accurate if they are
    not logged on */
 int total_idle_logon;

 int karma_value;
 
 time_t logoff_timestamp;
 
 bitflag flag_private_email : 1;
 bitflag flag_no_anonymous : 1;
 bitflag flag_agreed_disclaimer : 1;
 bitflag flag_hide_logon_time : 1;
 bitflag flag_kill_logon_time : 1;
 
 bitflag priv_banished : 1;
 
 bitflag priv_base : 1;

 bitflag priv_command_echo : 1;
 bitflag priv_command_jotd_edit : 1;
 bitflag priv_command_list : 1;
 bitflag priv_command_mail : 1;
 bitflag priv_command_room : 1;
 bitflag priv_command_script : 1;
 bitflag priv_command_session : 1;
 bitflag priv_command_trace : 1;
 bitflag priv_command_warn : 1;
 bitflag priv_command_extern_bug_suggest : 1;
 
 bitflag priv_spod : 1;
 bitflag priv_minister : 1;
 bitflag priv_coder : 1;
 bitflag priv_lib_maintainer : 1;
 bitflag priv_edit_files : 1;
 bitflag priv_system_room : 1;
 bitflag priv_no_timeout : 1;
 
 bitflag priv_su_channel : 1;
 bitflag priv_pretend_su : 1;
 bitflag priv_basic_su : 1;
 bitflag priv_normal_su : 1;
 bitflag priv_senior_su : 1;
 bitflag priv_lower_admin : 1;
 bitflag priv_admin : 1;
 bitflag priv_higher_admin : 1;

 bitflag flag_tmp_list_room_glob_in_core : 1;
 bitflag flag_tmp_no_logon : 1;

 bitflag flag_tmp_player_needs_saving : 1;
 bitflag flag_tmp_mail_needs_saving : 1;
 bitflag flag_tmp_room_needs_saving : 1;
 
 char lower_name[PLAYER_S_NAME_SZ];
 char name[PLAYER_S_NAME_SZ];

 char last_dns_address[SOCKET_DNS_ADDRESS_SZ];
 unsigned char last_ip_address[4];
} player_tree_node;

#endif
