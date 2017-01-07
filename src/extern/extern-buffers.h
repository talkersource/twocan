#ifndef EXTERN_BUFFERS_H
#define EXTERN_BUFFERS_H

extern int buffer_mail_create(player *p);
extern void buffer_mail_destroy(player *p);

extern int buffer_file_create(player *p);
extern void buffer_file_destroy(player *p);

extern int buffer_msg_edit_create(player *p);
extern void buffer_msg_edit_destroy(player *p);

extern int buffer_pager_create(player *p);
extern void buffer_pager_destroy(player *p);

extern int buffer_room_create(player *p);
extern void buffer_room_destroy(player *p);

extern int buffer_news_create(player *p);
extern void buffer_news_destroy(player *p);

#endif
