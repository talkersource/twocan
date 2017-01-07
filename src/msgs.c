#define MSGS_C
/*
 *  Copyright (C) 1999 James Antill
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * email: james@twocan.org
 */
#include "main.h"

/* gloabls vars... */
msg_file msg_motd = MSG_INIT("files/msgs/motd.msg"); /* stat'd */
msg_file msg_sumotd = MSG_INIT("files/msgs/sumotd.msg");

msg_file msg_full = MSG_INIT("files/msgs/full.msg");
msg_file msg_full_logon = MSG_INIT("files/msgs/full_logon.msg");
msg_file msg_auth_blocked_name = MSG_INIT("files/msgs/auth_blocked_name.msg");
msg_file msg_auth_blocked_net = MSG_INIT("files/msgs/auth_blocked_net.msg");
msg_file msg_auth_no_newbies = MSG_INIT("files/msgs/"
                                        "auth_tmp_block_newbies.msg");
msg_file msg_auth_no_residents = MSG_INIT("files/msgs/"
                                          "auth_tmp_block_residents.msg");

msg_file msg_connect = MSG_INIT("files/msgs/connect.msg");

msg_file msg_disclaimer = MSG_INIT("files/msgs/disclaimer.msg");

msg_file msg_jotd = MSG_INIT("files/msgs/jotd.msg");
msg_file msg_wotw = MSG_INIT("files/msgs/wotw.msg");

msg_file msg_newbie_start = MSG_INIT("files/msgs/newbie_start.msg");
msg_file msg_newbie_finish = MSG_INIT("files/msgs/newbie_finish.msg");

static int msg_bad_file(msg_file *m_file, int line_num)
{
 vwlog("error", " Error reading file (%d): %s", line_num, m_file->file_name);
 
 m_file->text = MALLOC(sizeof(char));
 *m_file->text = 0;
 m_file->file_info.msgs_size = 0; /* to make sure */
 
 return (MSG_FILE_ERROR);
}

int msg_load_file(msg_file *m_file)
{
 int fd = 0;
 struct stat file_info;
 char *text = NULL;
 
 assert(m_file);
 
 if ((stat(m_file->file_name, &file_info) == -1) ||
     !S_ISREG(file_info.st_mode))
 {
  if (m_file->text)
    return (MSG_FILE_CACHED); /* guess and hope... */
  else
    return (msg_bad_file(m_file, __LINE__));
 }
 
 if (m_file->text &&
     !difftime(m_file->file_info.msgs_mtime, file_info.st_mtime) &&
     (m_file->file_info.msgs_size == file_info.st_size))
 {
  m_file->file_info.msgs_atime = file_info.st_atime;
  m_file->file_info.msgs_ctime = file_info.st_ctime;
  
  return (MSG_FILE_CACHED);
 }
 
 if ((fd = open(m_file->file_name, O_RDONLY)) == -1)
 {
  if (m_file->text)
    return (MSG_FILE_CACHED); /* guess and hope... */
  else
    return (msg_bad_file(m_file, __LINE__));
 }

 text = MALLOC(file_info.st_size + sizeof(char));

 /* reentrant signal stuff means this must work... */
 if (read(fd, text, file_info.st_size) != file_info.st_size)
 {
  close(fd);

  FREE(text);
  if (m_file->text)
    return (MSG_FILE_CACHED); /* guess and hope... */
  else
    return (msg_bad_file(m_file, __LINE__));
 }
 close(fd);
 
 if (configure.talker_verbose)
   vwlog("msgs", "Loaded file: %s\n", m_file->file_name);
 
 m_file->file_info.msgs_size = file_info.st_size;
 m_file->file_info.msgs_atime = file_info.st_atime;
 m_file->file_info.msgs_ctime = file_info.st_ctime;
 m_file->file_info.msgs_mtime = file_info.st_mtime;

 if (m_file->text)
   FREE(m_file->text);

 m_file->text = text;
 m_file->text[MSG_STRLEN(m_file)] = 0;
 
 return (MSG_FILE_READ);
}

void msgs_load(void)
{
 msg_load_file(&msg_motd);
 system_data.motd = msg_motd.file_info.msgs_mtime;
 
 msg_load_file(&msg_sumotd);
 system_data.su_motd = msg_sumotd.file_info.msgs_mtime;

 msg_load_file(&msg_full);
 msg_load_file(&msg_full_logon);
 msg_load_file(&msg_auth_blocked_name);
 msg_load_file(&msg_auth_blocked_net);
 msg_load_file(&msg_auth_no_newbies);
 msg_load_file(&msg_auth_no_residents); 

 msg_load_file(&msg_connect);
 msg_load_file(&msg_disclaimer);
 
 msg_load_file(&msg_jotd);
 msg_load_file(&msg_wotw);

 msg_load_file(&msg_newbie_start);
 msg_load_file(&msg_newbie_finish);
}

void msgs_destroy(void)
{
 FREE(msg_motd.text);
 msg_motd.text = NULL;
 FREE(msg_sumotd.text);
 msg_sumotd.text = NULL;

 FREE(msg_full.text);
 msg_full.text = NULL;
 FREE(msg_full_logon.text);
 msg_full_logon.text = NULL;
 FREE(msg_auth_blocked_name.text);
 msg_auth_blocked_name.text = NULL;
 FREE(msg_auth_blocked_net.text);
 msg_auth_blocked_net.text = NULL;
 FREE(msg_auth_no_newbies.text);
 msg_auth_no_newbies.text = NULL;
 FREE(msg_auth_no_residents.text);
 msg_auth_no_residents.text = NULL;
 
 FREE(msg_connect.text);
 msg_connect.text = NULL;
 FREE(msg_disclaimer.text);
 msg_disclaimer.text = NULL;
 
 FREE(msg_jotd.text);
 msg_jotd.text = NULL;
 FREE(msg_wotw.text);
 msg_wotw.text = NULL;
 
 FREE(msg_newbie_start.text);
 msg_newbie_start.text = NULL;
 FREE(msg_newbie_finish.text);
 msg_newbie_finish.text = NULL;
}

int msg_write_file(msg_file *m_file, const char *str, size_t length)
{
 FILE *fp = NULL;

 if (length == (size_t)MSG_STRLEN(m_file))
 {
  if (!memcmp(m_file->text, str, MSG_STRLEN(m_file)))
    return (MSG_FILE_CACHED);
 }
 else
 {
  void *ptr = REALLOC(m_file->text, length + 1);

  if (!ptr)
    return (MSG_FILE_ERROR);
  m_file->text = ptr;

  m_file->file_info.msgs_size = length;
 }

 memcpy(m_file->text, str, MSG_STRLEN(m_file));
 m_file->file_info.msgs_mtime = now;

 if (configure.talker_read_only)
   return (MSG_FILE_WRITTEN);

 if ((fp = fopen(MSGS_TMP_FILE, "wb")))
 {
  int ret = fwrite(str, sizeof(char), MSG_STRLEN(m_file), fp);
  
  fclose(fp);
  
  if ((size_t)ret != MSG_STRLEN(m_file))
    return (MSG_FILE_ERROR);

  rename(MSGS_TMP_FILE, m_file->file_name);
 }
 
 return (MSG_FILE_WRITTEN);
}

int msg_edit_sync_file(msg_file *m_file, edit_base *base)
{
 FILE *fp = NULL;
 edit_line_node *scan = NULL;
 char *tmp = NULL;
 
 if (EDIT_SIZE(base) == MSG_STRLEN(m_file))
 {
  int cmp = 0;
  
  scan = base->lines_start;
  tmp = m_file->text;
  while (scan)
  {
   if ((cmp = memcmp(tmp, scan->line, scan->length)))
     break;
   tmp += scan->length;

   if (!base->is_raw)
   {
    if ((*tmp != '\n') && scan->next)
    {
     cmp = 1;
     break;
    }
    ++tmp;
   }
   
   scan = scan->next;
  }
  assert((size_t)(tmp - m_file->text) == EDIT_SIZE(base));

  if (!cmp)
    return (MSG_FILE_CACHED);
 }
 else
 {
  void *ptr = REALLOC(m_file->text, EDIT_SIZE(base) + 1);

  if (!ptr)
    return (MSG_FILE_ERROR);
  m_file->text = ptr;

  m_file->file_info.msgs_size = EDIT_SIZE(base);
 }

 scan = base->lines_start;
 tmp = m_file->text;
 m_file->text[MSG_STRLEN(m_file)] = 0;
 while (scan)
 {
  memcpy(tmp, scan->line, scan->length);
  tmp += scan->length;

  if (!base->is_raw)
    *tmp++ = '\n';
  
  scan = scan->next;
 }
 assert((size_t)(tmp - m_file->text) == EDIT_SIZE(base));
 assert(m_file->text[MSG_STRLEN(m_file)] == 0);

 m_file->file_info.msgs_mtime = now;

 if (configure.talker_read_only)
   return (MSG_FILE_WRITTEN);

 if ((fp = fopen(MSGS_TMP_FILE, "wb")))
 {
  int ret = fwrite(m_file->text, sizeof(char), MSG_STRLEN(m_file), fp);
  
  if (((size_t)ret != MSG_STRLEN(m_file)) || fclose(fp))
    return (MSG_FILE_ERROR);

  rename(MSGS_TMP_FILE, m_file->file_name);
 }
 else
   return (MSG_FILE_ERROR);
 
 return (MSG_FILE_WRITTEN);
}

static void user_su_msgs_reload(player *p, parameter_holder *params)
{
 if (params->last_param != 1)
   TELL_FORMAT(p, "[ help | messages | all ]\n");
 
 if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "help"))
 {
  fvtell_player(NORMAL_T(p), "%s", " Re-Loaded help.\n");
  init_help();
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "messages") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 1), "msgs"))
 {
  fvtell_player(NORMAL_T(p), "%s", " Re-Loaded messages.\n");
  MSGS_RELOAD();
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "all"))
 {
  fvtell_player(NORMAL_T(p), "%s", " Re-Loaded help and messages.\n");
  init_help();
  MSGS_RELOAD();
 }
 else
   TELL_FORMAT(p, "[ help | messages | all ]\n");
}

static void msg_edit_cleanup(player *p)
{ 
 buffer_msg_edit_destroy(p);
}

static void msg_edit_quit(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", " Leaving without changes.\n");

 msg_edit_cleanup(p);
}

static void msg_edit_end(player *p)
{
 msg_file *msg = p->buffers->msg_edit_buff->edited_msg;

 assert(MODE_IN_MODE(p, EDIT));
 
 if (!msg_edit_sync_file(msg, EDIT_BASE(p)))
   fvtell_player(NORMAL_T(p), " Could not write the msg file - ^S^B%s^s -.\n",
                 msg->file_name);
 else
   fvtell_player(NORMAL_T(p), " Msg file ^S^B%s^s changed.\n",
                 msg->file_name);
 
 msg_edit_cleanup(p);
}

static void user_su_msgs_edit(player *p, parameter_holder *params)
{
 msg_file *msg = NULL;
 int created = 0;
 
 if (params->last_param != 1)
   TELL_FORMAT(p, "<msg file>");

 lower_case(GET_PARAMETER_STR(params, 1));
 
 if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "motd"))
   msg = &msg_motd;
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "sumotd"))
   msg = &msg_sumotd;
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "full"))
   msg = &msg_full;
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "full_logon"))
   msg = &msg_full_logon;
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "auth_blocked_name"))
   msg = &msg_auth_blocked_name;
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "auth_blocked_net"))
   msg = &msg_auth_blocked_net;
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "auth_no_newbies"))
   msg = &msg_auth_no_newbies;
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "auth_no_residents"))
   msg = &msg_auth_no_residents;
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "connect"))
   msg = &msg_connect;
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "disclaimer"))
   msg = &msg_disclaimer;
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "jotd"))
   msg = &msg_jotd;
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "wotw"))
   msg = &msg_wotw;
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "newbie_start"))
   msg = &msg_newbie_start;
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "newbie_finish"))
   msg = &msg_newbie_finish;
 else
 {
  fvtell_player(SYSTEM_T(p), " Invalid msg file -- ^S^B%s^s --.\n",
                GET_PARAMETER_STR(params, 1));
  
  fvtell_player(SYSTEM_T(p), " Valid msg files are: motd, sumotd, full, "
                "full_logon, auth_blocked_name, auth_blocked_net, "
                "auth_no_newbies, auth_no_residents, "
                "connect, disclaimer, jotd, wotw, newbie_start, "
                "newbie_finish\n");
  
  TELL_FORMAT(p, "<msg file>");
  return;
 }
 
 if ((created = buffer_msg_edit_create(p)) > 0)
   P_MEM_ERR(p);
 else if (created < 0)
   fvtell_player(NORMAL_T(p), "%s",
                 " You cannot edit a msg file whilst already editing one.\n");
 else if (edit_start(p, msg->text))
 {
  assert(MODE_IN_MODE(p, EDIT));

  p->buffers->msg_edit_buff->edited_msg = msg;
  
  edit_limit_characters(p, 5000);
  edit_limit_lines(p, 24);
  
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_quit, msg_edit_quit);
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_end, msg_edit_end);
 }
}

void cmds_init_msgs(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("reload", user_su_msgs_reload, PARSE_PARAMS, ADMIN);
 CMDS_PRIV(coder_lower_admin);

 CMDS_ADD("edit", user_su_msgs_edit, PARSE_PARAMS, ADMIN);
 CMDS_PRIV(lower_admin);
}
