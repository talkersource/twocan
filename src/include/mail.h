#ifndef MAIL_H
#define MAIL_H

#ifdef MAIL_C
# define MAIL_FILE_VERSION 1

# define MAIL_CLEANUP_TIMEOUT_INIT MK_MINUTES(8)
# define MAIL_CLEANUP_TIMEOUT_LOAD MK_MINUTES(4)
# define MAIL_CLEANUP_TIMEOUT_CLEANUP MK_MINUTES(2)
# define MAIL_CLEANUP_TIMEOUT_SYNC_ANYWAY MK_MINUTES(32)
# define MAIL_CLEANUP_TIMEOUT_REDO MK_MINUTES(4)

# define MAIL_DELETE_SZ 1024 /* number of mails you can specify */
#endif

#define MAIL_ARTICLE_CHARS_SZ 5000
#define MAIL_ARTICLE_LINES_SZ 500

#define MAIL_SUBJECT_SIZE 200

typedef struct mail_recieved
{
 struct player_linked_list recipient;
 
 struct mail_recieved *next;
 struct mail_recieved *prev;

 struct mail_sent *mail;

 bitflag cc_name : 1;
 bitflag deleted : 1;
 bitflag grouped : 1;
 bitflag read : 1;
 bitflag replied : 1;
} mail_recieved;

typedef struct mail_sent
{
 struct mail_sent *next;
 struct mail_sent *prev;

 struct player_linked_list *recipients_start;

 struct player_tree_node *owner;
 
 char *body;

 size_t body_size;
 unsigned int number_of_recipients; 
 time_t c_timestamp; /* creation */
 time_t m_timestamp; /* modified */
 time_t a_timestamp; /* accessed */
 time_t l_timestamp; /* _last_ need saving access going backwards from now */

 char *subject;

 char *to;
 char *cc;

 Timer_q_node load_timer;
 
 bitflag anonymous : 1;
 bitflag have_cc : 1;
 bitflag have_to : 1;
 bitflag tmp_finnished_editing : 1;
 bitflag tmp_in_core : 1;
} mail_sent;

#endif
