#ifndef EMAIL_H
#define EMAIL_H

typedef struct email_info
{
 const char *to;
 const char *cc;
 const char *bcc;
 const char *from;
 const char *reply_to;
 const char *subject;
 const char *body;

 void *param;
 int (*func)(FILE *, void *);
} email_info;

#define EMAIL_INFO_INIT() {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

#endif
