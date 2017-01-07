#ifndef INTERCOM_H
#define INTERCOM_H

#if 0 /* def USE_INTERCOM --
       * namespace usage should allow us to just include this ...
       * so we aren't in for any nasty supprises */

#define INTERCOM_VERSION "1.1.6" /* this is the version that it is based on */

/*UNCOMMENT THE LINE BELOW TO USE VARARGS*/
/*#define VARARGS*/

#define INTERCOM_TALKER_NAME_SZ 40
#define INTERCOM_TALKER_ABBR_SZ 10
#define INTERCOM_PACKET_SZ 200
#define INTERCOM_DNS_ADDRESS_SZ 80
#define INTERCOM_PLAYER_NAME_SZ 20

typedef struct intercom_net_usage
{
 int chars_in;
 int chars_out;
 /* int established;
    int up_clicks; */
} intercom_net_usage;

struct socket_in_base;
struct socket_out_base;

/* fd; packet_anchor; -- replaced by socket_com */

typedef struct intercom_talker_node
{
 struct intercom_talker_node *next;
 struct intercom_talker_node *prev;

 socket_com_q_node *sock_q_node;
 
 char name[INTERCOM_TALKER_NAME_SZ];
 char abbr[INTERCOM_TALKER_ABBR_SZ];
 
 unsigned char ipv4_address[4]; /* addr/num */
 char dns_address[INTERCOM_DNS_ADDRESS_SZ]; /* dynamic */
 int port;

 int flags;
 time_t timeout;
 signed int validation;
 time_t last_seen;
 intercom_net_usage net_stats;
} intercom_talker_list;

typedef struct intercom_job_list
{
 struct intercom_job_list *next;
 
 long unsigned int job_id;
 long unsigned int job_ref;
 char sender[INTERCOM_PLAYER_NAME_SZ];
 char msg[256];
 time_t timeout;
 int command_type;

 char target[INTERCOM_PLAYER_NAME_SZ];
 char originator[INTERCOM_TALKER_ABBR_SZ];
 
 char destination[INTERCOM_TALKER_ABBR_SZ];
} intercom_job_list;

#if 0
typedef struct intercom_nameban
{
  char name[MAX_NAME];
  short type;
  struct nameban *next;
} intercom_nameban;
#endif

/*
 * Protocol for messages between talker and the intercom
 */
#define INTERCOM_BANISH_SITE 1
#define INTERCOM_OPEN_ALL_LINKS 2
#define INTERCOM_CLOSE_ALL_LINKS 3
#define INTERCOM_UNBAR_LINK 4
#define INTERCOM_CLOSE_LINK 5
#define INTERCOM_ADD_NEW_LINK 6
#define INTERCOM_USER_COMMAND 7
#define INTERCOM_NAME_IGNORED 8
#define INTERCOM_NAME_BLOCKED 9
#define INTERCOM_REQUEST_PORTNUMBER 10
#define INTERCOM_LINK_REQUESTED_UNKNOWN 11
#define INTERCOM_SHOW_LINKS 12
#define INTERCOM_SU_MESSAGE 13

#define INTERCOM_DELETE_LINK 15
#define INTERCOM_PERSONAL_MESSAGE 16
#define INTERCOM_HIGHLIGHT_RETURN 17
#define INTERCOM_HELLO_I_AM 18
#define INTERCOM_REQUEST_VALIDATION_AS 19
#define INTERCOM_VALIDATION_IS 20
#define INTERCOM_BAD_HELLO 21
#define INTERCOM_BAD_VALIDATION 22
#define INTERCOM_PERSONAL_MESSAGE_TAG 23
#define INTERCOM_PERSONAL_MESSAGE_AND_RETURN 24
#define INTERCOM_REPLY_IS 25
#define INTERCOM_NO_SUCH_PLAYER 26
#define INTERCOM_COMMAND_SUCCESSFUL 27
#define INTERCOM_PORTNUMBER_FOLLOWS 28
#define INTERCOM_OPEN_LINK 29
#define INTERCOM_CHANGE_NAME 30
#define INTERCOM_CHANGE_ABBR 31
#define INTERCOM_CHANGE_ADDRESS 32
#define INTERCOM_CHANGE_PORT 33
#define INTERCOM_TALKER_IGNORED 34
#define INTERCOM_TALKER_BLOCKED 35
#define INTERCOM_REQUEST_SERVER_LIST 36
#define INTERCOM_I_KNOW_OF 37
#define INTERCOM_INTERCOM_DIE 38
#define INTERCOM_PING 39
#define INTERCOM_REQUEST_STATS 40
#define INTERCOM_NAME_BANISHED 41
#define INTERCOM_MULTIPLE_NAME_MATCH 42
#define INTERCOM_STARTING_CONNECT 43
#define INTERCOM_SHOW_ALL_LINKS_SHORT 44
#define INTERCOM_HIDE_ENTRY 45
#define INTERCOM_FORMAT_MESSAGE_TAG 46
#define INTERCOM_UNHIDE_ENTRY 47
#define INTERCOM_BARRING_YOU 48
#define INTERCOM_INTERCOM_ROOM_MESSAGE 49
#define INTERCOM_INTERCOM_ROOM_LOOK 50
#define INTERCOM_INTERCOM_ROOM_LIST 51
#define INTERCOM_WE_ARE_MOVING 52
#define INTERCOM_USER_ACTION 53
#define INTERCOM_USE_DYNAMIC 54
/*RESERVED ALL VALUES UP TO &inc 100 FOR CENTRAL INTERCOM DEVELOPMENT*/
/*RESERVED ALL VALUES ABOVE 0xEF FOR CENTRAL DEVELOPMENT*/
#define INTERCOM_END_MESSAGE (0xFE)
#define INTERCOM_INCOMPLETE_MESSAGE (0xFF)




/*Command codes*/
#define INTERCOM_COMMAND_WHO 1
#define INTERCOM_COMMAND_FINGER 2
#define INTERCOM_COMMAND_EXAMINE 3
#define INTERCOM_COMMAND_TELL 4
#define INTERCOM_COMMAND_REMOTE 5
#define INTERCOM_COMMAND_LSU 6
#define INTERCOM_COMMAND_LOCATE 7
/*Ack, buggered up the protocol, have to miss a few out*/
#define INTERCOM_COMMAND_IDLE 10
#define INTERCOM_COMMAND_SAY 11
#define INTERCOM_COMMAND_EMOTE 12
/*RESERVED ALL VALUES UP TO 100 FOR CENTRAL INTERCOM DEVELOPMENT*/

/*User actions*/
#define INTERCOM_ENTER_ROOM 1
#define INTERCOM_LEAVE_ROOM 2

/*List codes*/
#define INTERCOM_LIST_ALL 1
#define INTERCOM_LIST_HIDDEN 2
#define INTERCOM_LIST_UP 3
/*RESERVED ALL VALUES UP TO 50 FOR CENTRAL INTERCOM DEVELOPMENT*/

/*Intercom status*/
#define INTERCOM_HIGHLIGHT (1<<0)
#define INTERCOM_BAR_ALL_CONNECTIONS (1<<1)
#define INTERCOM_PERSONAL_MSG (1<<2)
#define INTERCOM_BOOTING (1<<3)
#define INTERCOM_FORMAT_MSG (1<<4)
/*RESERVED ALL VALUES UP TO (1<<25) FOR CENTRAL INTERCOM DEVELOPMENT*/


/* FD status -- these need to die */
#define INTERCOM_NO_CONNECT_TRIED -23
#define INTERCOM_ERROR_FD -69
#define INTERCOM_BARRED -99
#define INTERCOM_BARRED_REMOTE -123
#define INTERCOM_P_BARRED -999
#define INTERCOM_NO_SYNC_TALKER -9999
/*RESERVED ALL VALUES DOWN TO  -10000 FOR CENTRAL INTERCOM DEVELOPMENT*/


/*talker flags*/
#define INTERCOM_WAITING_CONNECT (1<<0)
#define INTERCOM_HELLO_AFTER_CONNECT (1<<1)
#define INTERCOM_INVIS (1<<2)
#define INTERCOM_VALIDATE_AFTER_CONNECT (1<<3)
#define INTERCOM_FIRST_CONTACT (1<<4)
/*RESERVED ALL VALUES UP TO (1<<25) FOR CENTRAL INTERCOM DEVELOPMENT*/

#endif

#endif
