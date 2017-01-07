#ifndef EXTERN_EMAIL_H
#define EXTERN_EMAIL_H

extern void email_generic(const email_info *);

extern void email_report_local(const player *, const char *, const char *,
                               int (*) (FILE *, void *), void *,
                               const char *);

extern int email_validate_player(player *, const char *);

extern void user_configure_email_extern_bugs(player *, const char *);
extern void user_configure_email_extern_suggest(player *, const char *);

extern void user_configure_email_from_long(player *, const char *);
extern void user_configure_email_from_short(player *, const char *);

extern void user_configure_email_sendmail_run(player *, const char *);
extern void user_configure_email_sendmail_extern_run(player *, const char *);

extern void user_configure_email_to_abuse(player *, const char *);
extern void user_configure_email_to_admin(player *, const char *);
extern void user_configure_email_to_bugs(player *, const char *);
extern void user_configure_email_to_suggest(player *, const char *);
extern void user_configure_email_to_sus(player *, const char *);
extern void user_configure_email_to_up_down(player *, const char *);


extern void cmds_init_email(void);

#endif
