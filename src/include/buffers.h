#ifndef BUFFERS_H
#define BUFFERS_H


/* don't change these as they are "known" values */
#define BUFFERS_RET_USED (-1)
#define BUFFERS_RET_FAILED (1)
#define BUFFERS_RET_WORKED (0)

typedef struct mail_buffer
{
 struct edit_base *edit_base_copy;

 struct mail_sent *mail;
} mail_buffer;

typedef struct news_buffer
{
 struct edit_base *edit_base_copy;
 
 int post_group;
 char subject[NEWS_SUBJECT_SIZE];
 char name[PLAYER_S_NAME_SZ];
 bitflag anonymous : 1;
} news_buffer;

typedef struct room_buffer
{
 struct edit_base *edit_base_copy;
 
 char edited_room[ROOM_ID_SZ + PLAYER_S_NAME_SZ + 2];
} room_buffer;

typedef struct file_buffer
{
 struct edit_base *edit_base_copy;
} file_buffer;

typedef struct msg_edit_buffer
{
 struct edit_base *edit_base_copy;
 struct msg_file *edited_msg;
} msg_edit_buffer;

typedef struct pager_buffer
{
 struct edit_base *edit_base_copy;
 
 char *searching;
 bitflag search_mode : 1;
 bitflag command_mode : 1;
} pager_buffer;

typedef struct buffers_struct
{
 mail_buffer  *mail_buff;
 news_buffer  *news_buff;
 pager_buffer *pager_buff;
 room_buffer  *room_buff;
 file_buffer  *file_buff;
 msg_edit_buffer  *msg_edit_buff;
 
 struct edit_base *current_edit_base;
 
 int ref_count;
} buffers_struct;

#endif
