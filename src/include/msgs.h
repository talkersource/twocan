#ifndef MSGS_H
#define MSGS_H

#define MSGS_TMP_FILE "files/msgs/tmp.msg"

#define MSG_FILE_ERROR 0
#define MSG_FILE_CACHED 1
#define MSG_FILE_READ 2
#define MSG_FILE_WRITTEN 3

typedef struct msg_file
{
 const char *file_name;

 char *text;
 
 struct file_info
 { /* stuff we want from stat structure */
  off_t         msgs_size;     /* total size, in bytes */

  time_t        msgs_atime;    /* time of last access */
  time_t        msgs_mtime;    /* time of last modification */
  time_t        msgs_ctime;    /* time of last change */
 } file_info;
} msg_file;

#endif
