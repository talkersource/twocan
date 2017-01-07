#ifndef EXTERN_MSGS_H
#define EXTERN_MSGS_H

extern msg_file msg_motd;
extern msg_file msg_sumotd;

extern msg_file msg_full;
extern msg_file msg_full_logon;
extern msg_file msg_auth_blocked_name;
extern msg_file msg_auth_blocked_net;
extern msg_file msg_auth_no_newbies;
extern msg_file msg_auth_no_residents;

extern msg_file msg_connect;

extern msg_file msg_disclaimer;

extern msg_file msg_jotd;
extern msg_file msg_wotw;

extern msg_file msg_newbie_start;
extern msg_file msg_newbie_finish;

#define MSG_INIT(x) {x, NULL, {0, /* time_t's */ 0, 0, 0}}

/* sizeof(size_t) >= sizeof(off_t) */
#define MSG_STRLEN(x) ((size_t) (x)->file_info.msgs_size)
#define MSG_RELOAD_FILE(x) msg_load_file(x)
#define MSGS_RELOAD() msgs_load()

extern int msg_load_file(msg_file *);

extern int msg_write_file(msg_file *, const char *, size_t);
extern int msg_edit_sync_file(msg_file *, edit_base *);

extern void msgs_load(void);
extern void msgs_destroy(void);

extern void cmds_init_msgs(void);

#endif
