#ifndef EXTERN_MAIL_H
#define EXTERN_MAIL_H

#ifdef MAIL_C
# define MAIL_GROUP_NAME_ADD(x) strcat(group_list, *group_list ? ", " x : x)

# define MAIL_VALID_SENT(x) ((x)->tmp_finnished_editing)
# define MAIL_VALID_RECV(x) (!(x)->deleted && \
                             (x)->mail->tmp_finnished_editing)

#endif

/* global functions... */
extern mail_sent *mail_create_mail_sent(player_tree_node *, time_t);
extern void mail_destroy_mail_sent(player_tree_node *, mail_sent *);
extern mail_recieved *mail_add_mail_recipient(mail_sent *,
                                              player_tree_node *,
                                              int, int); /* news */
extern void mail_del_mail_recipient(mail_sent *, mail_recieved *);

extern unsigned int mail_check_mailbox_size(player_tree_node *);
extern unsigned int mail_check_mailunread_size(player_tree_node *);
extern unsigned int mail_check_mailout_size(player_tree_node *);
extern int mail_check_mail_new(player_tree_node *);

extern void mail_load_saved(player_tree_node *);
extern void mail_load_cleanup_all(player_tree_node *);
extern void mail_save(player_tree_node *);

extern void mail_reply_text(player *,
                            player_tree_node *, time_t, 
                            const char *, const char *, int);

extern void user_mail_delete_received(player *, parameter_holder *);
extern void user_mail_delete_sent(player *, parameter_holder *);

extern int mail_command(player *, const char *, size_t);

extern void user_mail_read_sent(player *, const char *);
extern void user_mail_sent_check(player *, parameter_holder *);
extern void user_mail_sent_check_all(player *, parameter_holder *);
extern void user_mail_check(player *, parameter_holder *);
extern void user_mail_check_all(player *, parameter_holder *);
extern void user_mail_send_letter_post(player *, const char *);
extern void user_mail_send_letter_apost(player *, const char *);
extern void user_mail_reply_letter_areply(player *, const char *);
extern void user_mail_reply_letter_reply(player *, const char *);
extern void user_mail_read_letter(player *, const char *);

extern void user_toggle_mail_inform(player *, const char *);
extern void user_toggle_anonymous(player *, const char *);

extern void init_mail(void);

extern void cmds_init_mail(void);

#endif
