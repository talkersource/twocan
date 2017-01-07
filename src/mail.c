#define MAIL_C
/*
 *  Copyright (C) 1999, 2000 James Antill, John Tobin
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
 * email: james@twocan.org, john@twocan.org
 */
#include "main.h"

Timer_q_base mail_timer_queue;

mail_sent *mail_create_mail_sent(player_tree_node *owner, time_t c_timestamp)
{
 mail_sent *from = XMALLOC(sizeof(mail_sent), MAIL_SENT);
 mail_sent *tmp = NULL;
 
 if (!from)
   return (NULL);

 from->owner = owner;
 from->recipients_start = NULL;
 from->body = NULL;
 from->body_size = 0;
 from->cc = NULL;
 from->subject = NULL;
 from->to = NULL;
 from->number_of_recipients = 0;

 from->c_timestamp = c_timestamp;
 from->m_timestamp = now;
 from->a_timestamp = now;

 from->anonymous = FALSE;
 from->tmp_finnished_editing = TRUE;
 from->tmp_in_core = FALSE;

 if (!(tmp = owner->mail_sent_start))
 {
  from->next = NULL;
  from->prev = NULL;
  owner->mail_sent_start = from;
  
  return (from);
 } 

 while (tmp->next && (difftime(from->c_timestamp, tmp->c_timestamp) > 0))
   tmp = tmp->next;
 
 if (difftime(from->c_timestamp, tmp->c_timestamp) > 0)
 {
  if ((from->next = tmp->next))
    from->next->prev = from;
  
  tmp->next = from;
  from->prev = tmp;
 }
 else if (difftime(from->c_timestamp, tmp->c_timestamp) < 0)
 {
  if ((from->prev = tmp->prev))
    from->prev->next = from;
  else
    owner->mail_sent_start = from;
  
  tmp->prev = from;
  from->next = tmp;
 }
 else
 {
  assert(FALSE);
 } 

 return (from);
}

void mail_destroy_mail_sent(player_tree_node *owner, mail_sent *from)
{
 Timer_q_node *current_timer = NULL;
 
 assert(MALLOC_VALID(from, sizeof(mail_sent), MAIL_SENT));
 
 if ((current_timer = timer_q_find_data(&mail_timer_queue, from)))
 {
  assert_log(current_timer == &from->load_timer);
  timer_q_del_node(&mail_timer_queue, &from->load_timer);
 }
 
 while (from->recipients_start)
   mail_del_mail_recipient(from, (mail_recieved *)from->recipients_start);
 
 if (from->body)
   FREE(from->body);
 if (from->subject)
   FREE(from->subject);
 if (from->to)
   FREE(from->to);
 if (from->cc)
   FREE(from->cc);

 if (from->prev)
   from->prev->next = from->next;
 else
   owner->mail_sent_start = from->next;

 if (from->next)
   from->next->prev = from->prev;

 XFREE(from, MAIL_SENT);
}

mail_recieved *mail_add_mail_recipient(mail_sent *from, player_tree_node *to,
                                       int as_group, int as_cc)
{
 mail_recieved *rcpt = NULL;
 mail_recieved *tmp = NULL;
 player_linked_list *current = NULL;
 
 if ((current = player_link_find(from->recipients_start, to, NULL,
                                 PLAYER_LINK_NAME_ORDERED)))
 {
  rcpt = (mail_recieved *) current;

  assert(rcpt->mail == from);
  assert(!rcpt->read);

  if (!as_group)
  {
   rcpt->grouped = FALSE;
   rcpt->cc_name = as_cc;
  }
  
  return (rcpt);
 }

 if (!(rcpt = XMALLOC(sizeof(mail_recieved), MAIL_RECIEVED)))
   return (NULL);
  
 rcpt->cc_name = as_cc;
 rcpt->deleted = FALSE;
 rcpt->grouped = as_group;
 rcpt->read = FALSE;
 rcpt->replied = FALSE;
 rcpt->mail = from;
 
 ++from->number_of_recipients;

 player_link_add(&from->recipients_start, to, NULL,
                 PLAYER_LINK_NAME_ORDERED, &rcpt->recipient);

 if (!(tmp = to->mail_recieved_start))
 {
  rcpt->next = NULL;
  rcpt->prev = NULL;
  
  to->mail_recieved_start = rcpt;
  
  return (rcpt);
 }
 
 while (tmp->next && (difftime(from->c_timestamp, tmp->mail->c_timestamp) > 0))
   tmp = tmp->next;
 
 if (difftime(from->c_timestamp, tmp->mail->c_timestamp) > 0)
 {
 add_recip_after:
  if ((rcpt->next = tmp->next))
    rcpt->next->prev = rcpt;
  
  tmp->next = rcpt;
  rcpt->prev = tmp;
 }
 else if (difftime(from->c_timestamp, tmp->mail->c_timestamp) < 0)
 {
 add_recip_before:
  if ((rcpt->prev = tmp->prev))
    rcpt->prev->next = rcpt;
  else
    to->mail_recieved_start = rcpt;
  
  tmp->prev = rcpt;
  rcpt->next = tmp;
 }
 else
 {
  while (tmp->next && !difftime(from->c_timestamp, tmp->mail->c_timestamp) &&
         (strcmp(from->owner->lower_name, tmp->mail->owner->lower_name) < 0))
    tmp = tmp->next;
  
  assert(!difftime(from->c_timestamp, tmp->mail->c_timestamp) &&
         (from->owner != tmp->mail->owner));
  
  if (difftime(from->c_timestamp, tmp->mail->c_timestamp) ||
      strcmp(from->owner->lower_name, tmp->mail->owner->lower_name) < 0)
    goto add_recip_before;
  else
    goto add_recip_after;
 }
 
 return (rcpt);
}

void mail_del_mail_recipient(mail_sent *from, mail_recieved *rcpt)
{
 player_tree_node *current = PLAYER_LINK_SAV_GET((&rcpt->recipient));

 assert(MALLOC_VALID(from, sizeof(mail_sent), MAIL_SENT));
 assert(MALLOC_VALID(rcpt, sizeof(mail_recieved), MAIL_RECIEVED));
 
 player_link_del(&from->recipients_start, current, NULL,
                 PLAYER_LINK_NAME_ORDERED, &rcpt->recipient);
 
 if (rcpt->prev)
   rcpt->prev->next = rcpt->next;
 else
   current->mail_recieved_start = rcpt->next;

 if (rcpt->next)
   rcpt->next->prev = rcpt->prev;

 --from->number_of_recipients;
 from->m_timestamp = now;
 
 XFREE(rcpt, MAIL_RECIEVED);
}

unsigned int mail_check_mailbox_size(player_tree_node *saved)
{
 unsigned int mail_count = 0;
 mail_recieved *scan = saved->mail_recieved_start;

 while (scan)
 {
  if (MAIL_VALID_RECV(scan))
    ++mail_count;
  scan = scan->next;
 }
 
 return (mail_count);
}

unsigned int mail_check_mailunread_size(player_tree_node *saved)
{
 unsigned int mail_count = 0;
 mail_recieved *scan = saved->mail_recieved_start;

 while (scan)
 {
  if (MAIL_VALID_RECV(scan) && !scan->read)
    ++mail_count;
  scan = scan->next;
 }
 
 return (mail_count);
}

unsigned int mail_check_mailout_size(player_tree_node *saved)
{
 unsigned int mail_count = 0;
 mail_sent *scan = saved->mail_sent_start;
 
 while (scan)
 {
  if (MAIL_VALID_SENT(scan))
    ++mail_count;
  scan = scan->next;
 }
 
 return (mail_count);
}

int mail_check_mail_new(player_tree_node *saved)
{
 mail_recieved *scan = saved->mail_recieved_start;
 mail_recieved *last = NULL;

 while (scan)
 {
  if (!scan->deleted && scan->mail->tmp_finnished_editing && !scan->read)
    last = scan;
  
  scan = scan->next;
 }

 if (last && (difftime(last->mail->c_timestamp, saved->logoff_timestamp) > 0))
 {
  assert(mail_check_mailunread_size(saved));
  return (TRUE);
 }

 return (FALSE);
}

static mail_sent *mail_find_sent(player *requester,
                                 player_tree_node *current,
                                 unsigned int mail_to_find)
{
 mail_sent *scan = current->mail_sent_start;
 unsigned int count = 0;
 
 while (scan)
 {
  if (!scan->tmp_finnished_editing)
  {
   scan = scan->next;
   continue;
  }
  ++count;
  
  if (count == mail_to_find)
    return (scan);
  
  scan = scan->next;
 }

 if (requester)
   fvtell_player(NORMAL_T(requester), " No such sent mail -- ^S^B%d^s --.\n",
                 mail_to_find);
 return (NULL);
}

static unsigned int mail_find_number(mail_sent *from)
{
 mail_sent *scan = from->owner->mail_sent_start;
 unsigned int count = 0;
 
 while (scan)
 {
  if (!scan->tmp_finnished_editing)
  {
   scan = scan->next;
   continue;
  }
  ++count;
  
  if (scan == from)
    return (count);
  
  scan = scan->next;
 }
 
 assert(FALSE);
 return (UINT_MAX); /* file_read will bodge emty stuff */
}

static mail_recieved *mail_find_recieved(player *requester,
                                         player_tree_node *current,
                                         unsigned int mail_to_find)
{
 mail_recieved *scan = current->mail_recieved_start;
 unsigned int count = 0;
 
 while (scan)
 {
  if (scan->deleted || !scan->mail->tmp_finnished_editing)
  {
   scan = scan->next;
   continue;
  }
  ++count;
  
  if (count == mail_to_find)
    return (scan);

  scan = scan->next;
 }

 if (requester)
   fvtell_player(NORMAL_T(requester),
                 " No such recieved mail -- ^S^B%d^s --.\n", mail_to_find);
 return (NULL);
}

static void mail_tell_player_to_list(player_tree_node *s_from, player *to,
                                     twinkle_info *info,
                                     int flags, time_t timestamp,
                                     mail_sent *from)
{
 int done = FALSE;
 player_linked_list *scan = from->recipients_start;
 
 fvtell_player(ALL_T(s_from, to, info, flags, timestamp), "%s", " To: ");
 if (from->to)
 {
  fvtell_player(ALL_T(s_from, to, info, flags, timestamp), "%s", from->to);
  done = TRUE;
 }

 while (scan)
 {
  player_linked_list *scan_next = PLAYER_LINK_NEXT(scan);
  mail_recieved *tmp = (mail_recieved *)scan;
  
  if (!tmp->cc_name && !tmp->grouped)
  {
   fvtell_player(ALL_T(s_from, to, info, flags, timestamp),
                 ADD_COMMA_FRONT(done, "%s"),
                 PLAYER_LINK_SAV_GET(scan)->name);
   done = TRUE;
  }
  
  scan = scan_next;
 }
 
 assert(done);
 fvtell_player(ALL_T(s_from, to, info, flags, timestamp), "%s", "\n");
}

static void mail_tell_player_cc_list(player_tree_node *s_from, player *to,
                                     twinkle_info *info,
                                     int flags, time_t timestamp,
                                     mail_sent *from)
{
 int done = FALSE;
 player_linked_list *scan = from->recipients_start;
 
 if (from->cc)
 {
  fvtell_player(ALL_T(s_from, to, info, flags, timestamp),
                " Cc: %s", from->cc);
  done = TRUE;
 }

 while (scan)
 {
  player_linked_list *scan_next = PLAYER_LINK_NEXT(scan);
  mail_recieved *tmp = (mail_recieved *)scan;
  
  if (tmp->cc_name && !tmp->grouped)
  {
   fvtell_player(ALL_T(s_from, to, info, flags, timestamp),
                 ADD_COMMA_FRONT(done, "%s"),
                 PLAYER_LINK_SAV_GET(scan)->name);
   done = TRUE;
  }
  
  scan = scan_next;
 }
 
 if (done)
   fvtell_player(ALL_T(s_from, to, info, flags, timestamp), "%s", "\n");
}

static void mail_load_cleanup(mail_sent *scan)
{
 if (scan->owner->flag_tmp_mail_needs_saving || !scan->tmp_in_core)
   return;
 
 if (scan->body)
 {
  FREE(scan->body);
  scan->body = NULL;
  scan->body_size = 0;
 }
 
 if (scan->subject)
 {
  FREE(scan->subject);
  scan->subject = NULL;
 }
 
 if (scan->to)
 {
  FREE(scan->to);
  scan->to = NULL;
 }
 
 if (scan->cc)
 {
  FREE(scan->cc);
  scan->cc = NULL;
 }

 scan->tmp_in_core = FALSE;
}

void mail_load_cleanup_all(player_tree_node *current)
{
 mail_sent *scan = current->mail_sent_start;

 while (scan)
 {
  assert(scan->owner == current);
  
  if (scan->tmp_finnished_editing)
    mail_load_cleanup(scan);

  scan = scan->next;
 }
}

static void mail_load_all(player_tree_node *);

static void timed_mail_cleanup(int timed_type, void *passed_mail_sent)
{
 mail_sent *from = passed_mail_sent;
 int do_save = FALSE;
 int do_cleanup = FALSE;
 int do_retime = FALSE;

 TCTRACE("timed_mail_cleanup");

 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;

 if (timed_type == TIMER_Q_TYPE_CALL_RUN_ALL)
   do_save = TRUE;
 else if (difftime(now, from->a_timestamp) > MAIL_CLEANUP_TIMEOUT_CLEANUP)
 {
  do_save = TRUE;
  do_cleanup = TRUE;
 }
 else if (difftime(now, from->l_timestamp) > MAIL_CLEANUP_TIMEOUT_SYNC_ANYWAY)
 {
  do_save = TRUE;
  do_retime = TRUE;
  from->l_timestamp = now;
 }
 else
   do_retime = TRUE;
 
 if (do_save)
   mail_save(from->owner);

 if (do_cleanup)
   mail_load_cleanup(from);
 else if (do_retime)
 {
  struct timeval tv;
  
  log_assert(from->tmp_in_core);

  gettimeofday(&tv, NULL);

  TIMER_Q_TIMEVAL_ADD_SECS(&tv, MAIL_CLEANUP_TIMEOUT_REDO, 0);

  timer_q_add_static_node(&from->load_timer, &mail_timer_queue,
                          from, &tv, TIMER_Q_FLAG_NODE_SINGLE);
 }
}

static unsigned int internal_mail_load_open(player_tree_node *current,
                                            file_io *io_mail)
{
 char mail_file[sizeof("files/player_data/a/a/.mail") + PLAYER_S_NAME_SZ];
 unsigned int number_of_mails = 0;

 sprintf(mail_file, "%s/%c/%c/%s.mail", "files/player_data",
         *current->lower_name, *(current->lower_name + 1),
         current->lower_name);
 
 if (!file_read_open(mail_file, io_mail))
   return (0);

 file_section_beg("a_saved", io_mail);

 file_section_beg("header", io_mail);
 number_of_mails = file_get_unsigned_int("number_of_mails", io_mail);
 file_section_end("header", io_mail);

 if (!number_of_mails)
 {
  file_section_end("a_saved", io_mail);
  file_read_close(io_mail);
  return (0);
 }
 
 file_section_beg("mails", io_mail);
 
 return (number_of_mails);
}

void mail_load_saved(player_tree_node *current)
{
 file_io io_mail;
 unsigned int number_of_mails = 0;
 unsigned int mail_count = 0;
 
 assert(!current->mail_sent_start);
 
 if (!(number_of_mails = internal_mail_load_open(current, &io_mail)))
   return;

 while (mail_count < number_of_mails)
 {
  char mail_buff[BUF_NUM_TYPE_SZ(int)];
  unsigned int number_of_recipients = 0;
  unsigned int recip_count = 0;
  time_t c_timestamp;
  mail_sent *from = NULL;
  
  sprintf(mail_buff, "%04d", ++mail_count);
  
  file_section_beg(mail_buff, &io_mail);

  c_timestamp = file_get_time_t("c_timestamp", &io_mail);
  if (!(from = mail_create_mail_sent(current, c_timestamp)))
    SHUTDOWN_MEM_ERR();
  
  file_section_beg("flags", &io_mail);

  from->anonymous = file_get_bitflag("anonymous", &io_mail);
  from->have_cc = file_get_bitflag("have_cc", &io_mail);
  from->have_to = file_get_bitflag("have_to", &io_mail);
  
  file_section_end("flags", &io_mail);

  from->m_timestamp = file_get_time_t("m_timestamp", &io_mail);
  
  number_of_recipients = file_get_unsigned_int("number_of_recipients",
                                               &io_mail);

  file_section_beg("recipients", &io_mail);

  while (recip_count < number_of_recipients)
  {
   char recip_buffer[BUF_NUM_TYPE_SZ(int)];
   char name[PLAYER_S_NAME_SZ];
   int flag_cc_name = FALSE;
   int flag_deleted = FALSE;
   int flag_grouped = FALSE;
   int flag_read = FALSE;
   player_tree_node *to = NULL;
   mail_recieved *rcpt = NULL;
   
   sprintf(recip_buffer, "%04d", ++recip_count);

   file_section_beg(recip_buffer, &io_mail);

   file_section_beg("flags", &io_mail);
   
   flag_cc_name = file_get_bitflag("cc_name", &io_mail);
   flag_deleted = file_get_bitflag("deleted", &io_mail);
   flag_grouped = file_get_bitflag("grouped", &io_mail);
   flag_read = file_get_bitflag("read", &io_mail);
   
   file_section_end("flags", &io_mail);

   file_get_string("name", name, PLAYER_S_NAME_SZ, &io_mail);
   
   file_section_end(recip_buffer, &io_mail);

   if ((to = player_find_all(NULL, name, PLAYER_FIND_DEFAULT)))
     if ((rcpt = mail_add_mail_recipient(from, to,
                                         flag_grouped, flag_cc_name)))
     {
      rcpt->deleted = flag_deleted;
      rcpt->read = flag_read;
     }
  }
  
  file_section_end("recipients", &io_mail);

  file_section_end(mail_buff, &io_mail);
 }

 file_section_end("mails", &io_mail);

 file_section_end("a_saved", &io_mail);

 file_read_close(&io_mail);
}

static void internal_mail_load(mail_sent *scan, unsigned int count,
                               file_io *io_mail)
{
 char buffer[BUF_NUM_TYPE_SZ(int)];
 
 if (!timer_q_find_data(&mail_timer_queue, scan))
 {
  struct timeval tv;
  
  gettimeofday(&tv, NULL);
  
  TIMER_Q_TIMEVAL_ADD_SECS(&tv, MAIL_CLEANUP_TIMEOUT_LOAD, 0);
  
  timer_q_add_static_node(&scan->load_timer, &mail_timer_queue,
                          scan, &tv, TIMER_Q_FLAG_NODE_SINGLE);

  scan->l_timestamp = now;
 }
 else
   log_assert(FALSE);
 
 sprintf(buffer, "%04d", count);

 file_section_beg(buffer, io_mail);
 
 if (!(scan->body = file_get_malloc("body", &scan->body_size, io_mail)))
   SHUTDOWN_MEM_ERR();

 if (scan->have_cc)
   if (!(scan->cc = file_get_malloc("group_cc", NULL, io_mail)))
     SHUTDOWN_MEM_ERR();

 if (scan->have_to)
   if (!(scan->to = file_get_malloc("group_to", NULL, io_mail)))
     SHUTDOWN_MEM_ERR();

 if (!(scan->subject = file_get_malloc("subject", NULL, io_mail)))
   SHUTDOWN_MEM_ERR();

 file_section_end(buffer, io_mail);

 scan->tmp_in_core = TRUE;
}

static void mail_load_all(player_tree_node *current)
{
 mail_sent *scan = current->mail_sent_start;
 file_io io_mail;
 unsigned int number_of_mails = UINT_MAX;
 unsigned int mail_count = 0;
 int done = FALSE;

 while (scan && (mail_count < number_of_mails))
 {
  assert(scan->owner == current);

  if (!scan->tmp_finnished_editing)
  {
   scan = scan->next;
   continue;
  }
  
  ++mail_count;
  
  if (!scan->tmp_in_core)
  {
   if (!done)
   {
    if (!(number_of_mails = internal_mail_load_open(current, &io_mail)))
      SHUTDOWN_MEM_ERR();

    file_section_end("mails", &io_mail);
    file_section_end("a_saved", &io_mail);
    
    file_section_beg("mails", &io_mail);

    done = TRUE;
   }
   
   internal_mail_load(scan, mail_count, &io_mail);
  }

  log_assert(scan->body);
  scan->a_timestamp = now;
  scan = scan->next;
 }

 if (done)
 {
  file_section_end("mails", &io_mail);
  file_read_close(&io_mail);
 }
}

static void mail_load(mail_sent *scan, int mail_to_load)
{
 file_io io_mail;
 unsigned int number_of_mails = 0;

 scan->a_timestamp = now;
 
 if (scan->tmp_in_core)
   return;
 
 if (!(number_of_mails = internal_mail_load_open(scan->owner, &io_mail)))
   SHUTDOWN_MEM_ERR();
 
 assert(number_of_mails);
 file_section_end("mails", &io_mail);
 file_section_end("a_saved", &io_mail);
 
 file_section_beg("mails", &io_mail);
 internal_mail_load(scan, mail_to_load, &io_mail);
 file_section_end("mails", &io_mail);
 
 file_read_close(&io_mail);
}

void mail_save(player_tree_node *current)
{
 char mail_file_ren[sizeof("files/player_data/a/a/.mail") + PLAYER_S_NAME_SZ];
 char mail_file[sizeof("files/player_data/a/a/.mail.tmp") + PLAYER_S_NAME_SZ];
 file_io real_io_mail;
 file_io *io_mail = &real_io_mail;
 unsigned int number_of_mails = 0;
 unsigned int mail_count = 0;
 mail_sent *from = current->mail_sent_start;

 if (configure.talker_read_only)
   return;
 
 if (!current->flag_tmp_mail_needs_saving)
   return;

 mail_load_all(current);
 number_of_mails = mail_check_mailout_size(current);
 
 sprintf(mail_file_ren, "%s/%c/%c/%s.mail", "files/player_data",
         *current->lower_name, *(current->lower_name + 1),
         current->lower_name);
 sprintf(mail_file, "%s.tmp", mail_file_ren);
 
 if (!file_write_open(mail_file, MAIL_FILE_VERSION, io_mail))
 {
  /* FIXME: re-introduce...
   * assert(FALSE); */
  return;
 }

 file_section_beg("a_saved", io_mail);
 
 file_section_beg("header", io_mail);
 file_put_unsigned_int("number_of_mails", number_of_mails, io_mail);
 file_section_end("header", io_mail);
 
 file_section_beg("mails", io_mail);

 while (from && (mail_count < number_of_mails))
 {
  char mail_buff[BUF_NUM_TYPE_SZ(int)];
  unsigned int recip_count = 0;
  player_linked_list *rcpt = NULL;
  
  if (!from->tmp_finnished_editing)
  {
   from = from->next;
   continue;
  }

  sprintf(mail_buff, "%04d", ++mail_count);
  
  file_section_beg(mail_buff, io_mail);

  file_put_time_t("c_timestamp", from->c_timestamp, io_mail);
  
  file_section_beg("flags", io_mail);
  
  file_put_bitflag("anonymous", from->anonymous, io_mail);
  
  file_put_bitflag("have_cc", from->have_cc, io_mail);
  file_put_bitflag("have_to", from->have_to, io_mail);
  
  file_section_end("flags", io_mail);

  file_put_time_t("m_timestamp", from->m_timestamp, io_mail);
  
  file_put_unsigned_int("number_of_recipients", from->number_of_recipients,
                        io_mail);
  
  file_section_beg("recipients", io_mail);
  
  rcpt = from->recipients_start;
  while (rcpt && (recip_count < from->number_of_recipients))
  {
   char recip_buffer[BUF_NUM_TYPE_SZ(int)];
   mail_recieved *recip = (mail_recieved *)rcpt;
   
   sprintf(recip_buffer, "%04d", ++recip_count);
   
   file_section_beg(recip_buffer, io_mail);
   
   file_section_beg("flags", io_mail);
   
   file_put_bitflag("cc_name", recip->cc_name, io_mail);
   file_put_bitflag("deleted", recip->deleted, io_mail);
   file_put_bitflag("grouped", recip->grouped, io_mail);
   file_put_bitflag("read", recip->read, io_mail);
   
   file_section_end("flags", io_mail);
   
   file_put_string("name", PLAYER_LINK_SAV_GET(rcpt)->lower_name, 0, io_mail);
   
   file_section_end(recip_buffer, io_mail);
   
   rcpt = PLAYER_LINK_NEXT(rcpt);
  }
  assert(!rcpt && (recip_count == from->number_of_recipients));
  
  file_section_end("recipients", io_mail);
  
  file_section_end(mail_buff, io_mail);

  from = from->next;
 }
 assert((!from || !from->tmp_finnished_editing) &&
        (mail_count == number_of_mails));
 file_section_end("mails", io_mail);

 file_section_end("a_saved", io_mail);
 
 file_section_beg("mails", io_mail);

 mail_count = 0;
 from = current->mail_sent_start;
 while (from && (mail_count < number_of_mails))
 {
  char mail_buff[BUF_NUM_TYPE_SZ(int)];
  
  if (!from->tmp_finnished_editing)
  {
   from = from->next;
   continue;
  }
  
  sprintf(mail_buff, "%04d", ++mail_count);
  
  file_section_beg(mail_buff, io_mail);

  log_assert(from->body);
  assert(strlen(from->body) == from->body_size);
  file_put_string("body", from->body, from->body_size, io_mail);

  if (from->cc)
    file_put_string("group_cc", from->cc, 0, io_mail);
  
  if (from->to)
    file_put_string("group_to", from->to, 0, io_mail);    

  file_put_string("subject", from->subject, 0, io_mail);
  
  file_section_end(mail_buff, io_mail);

  from = from->next;
 }
 assert((!from || !from->tmp_finnished_editing) &&
        (mail_count == number_of_mails));

 file_section_end("mails", io_mail);
 
 if (file_write_close(io_mail))
 {
  rename(mail_file, mail_file_ren);
  
  current->flag_tmp_mail_needs_saving = FALSE;
 }
}

static unsigned int mail_autoreap_mailout(player_tree_node *saved)
{
 unsigned int mail_count = 0;
 mail_sent *scan = saved->mail_sent_start;

 mail_load_all(saved); /* FIXME: do it like rooms so we don't have to
                        * do this */

 while (scan)
 {
  mail_sent *scan_next = scan->next;
  
  if (MAIL_VALID_SENT(scan))
  {
   player_linked_list *rcpt = scan->recipients_start;

   if (difftime(now, scan->c_timestamp) < MK_DAYS(60))
     return (mail_count);

   while (rcpt)
   {
    player_linked_list *rcpt_next = PLAYER_LINK_NEXT(rcpt);
    mail_recieved *tmp = (mail_recieved *)rcpt;

    if (!tmp->deleted)
      break;
    
    rcpt = rcpt_next;
   }
   if (!rcpt)
   {
    scan->owner->flag_tmp_mail_needs_saving = TRUE;
    mail_destroy_mail_sent(saved, scan);
    ++mail_count;
   }
  }
  
  scan = scan_next;
 }
 
 return (mail_count);
}

static void mail_rejoin_func(player *p)
{
 fvtell_player(NORMAL_T(p), "%s",
               " Re-Entering mail mode. Use ^Bhelp mail^N for help\n"
               " To run ^Bnormal commands^N type /<command>\n"
               " Use '^Bend^N' to ^Bleave^N.\n");
}

int mail_command(player *p, const char *str, size_t length)
{
 ICTRACE("mail_command");

 if (MODE_IN_MODE(p, MAIL))
   MODE_HELPER_COMMAND();
 else 
   if (!*str)
   {
    cmds_function tmp_cmd;
    cmds_function tmp_rejoin;
    
    CMDS_FUNC_TYPE_RET_CHARS_SIZE_T(&tmp_cmd, mail_command);
    CMDS_FUNC_TYPE_NO_CHARS(&tmp_rejoin, mail_rejoin_func);
    
    if (mode_add(p, "Mail Mode-> ", MODE_ID_MAIL, 0,
                 &tmp_cmd, &tmp_rejoin, NULL))
      fvtell_player(NORMAL_T(p), "%s", 
                    " Entering mail mode. Use ^Bhelp mail^N for help\n"
                    " To run ^Bnormal commands^N type /<command>\n"
                    " Use '^Bend^N' to ^Bleave^N.\n");
    else
      fvtell_player(SYSTEM_T(p), 
                    " You cannot enter mail mode as you are in too many "
                    "other modes.\n");
    return (TRUE);
   }

 return (cmds_sub_match(p, str, length,
                        &cmds_sub[CMDS_SUB_OFFSET(CMDS_SECTION_MAIL)]));
}

static void mail_view_commands(player *p)
{
 user_cmds_show_section(p, "mail");
}

static void mail_exit_mode(player *p)
{
 assert(MODE_IN_MODE(p, MAIL));
 
 fvtell_player(NORMAL_T(p), "%s", " Leaving mail mode.\n");

 mode_del(p);
}

static void internal_mail_sent_check(player *p, 
                                     player_tree_node *sp, int number_to_show)
{
 mail_sent *scan = sp->mail_sent_start;
 int mail_count = mail_check_mailout_size(sp);
 int count = 0;
 int string_length = 32; /* guess and hope.. */
 
 if (!mail_count)
 {  
  if (p->saved == sp)
    fvtell_player(NORMAL_T(p), "%s", " You have sent no mail.\n");
  else
    fvtell_player(NORMAL_T(p), " %s has sent no mail.\n", sp->name);
  return;
 }

 if (number_to_show && (number_to_show < mail_count))
 {
  while (scan && (count < (mail_count - number_to_show)))
  {
   if (MAIL_VALID_SENT(scan))
     ++count;

   scan = scan->next;
  }
  assert(scan && (count == (mail_count - number_to_show)));
 }
 
 if (mail_count == 1)
   ptell_mid(NORMAL_T(p), "Sent one letter", FALSE);
 else
 {
  char buffer[sizeof("Sent $Number(%d)-Tostr letters%s%s%s") +
             (2 * BUF_NUM_TYPE_SZ(int)) + sizeof(" (showing ") + 1];
  char ascii_num[BUF_NUM_TYPE_SZ(int)];

  if (number_to_show && (number_to_show < mail_count))
    sprintf(ascii_num, "%d", number_to_show);
  else
    ascii_num[0] = 0;  

  sprintf(buffer, "Sent $Number(%d)-Tostr letters%s%s%s",
          mail_count, USE_STRING(ascii_num[0], " (showing "), ascii_num,
          USE_STRING(ascii_num[0], ")"));
  
  ptell_mid(NORMAL_T(p), buffer, FALSE);
 }
 
 while (scan)
 {
  char buffer[BUF_NUM_TYPE_SZ(int) + 2];
  
  if (!scan->tmp_finnished_editing)
  {
   scan = scan->next;
   continue;
  }
  
  sprintf(buffer, "[%d]", ++count);

  mail_load(scan, count);

  fvtell_player(NORMAL_T(p), "%-5s %5lu: %.*s\n",
                buffer, (unsigned long)scan->body_size,
                string_length, scan->subject);

  scan = scan->next;
 }
 
 if (number_to_show && (number_to_show < mail_count))
   ptell_mid(NORMAL_T(p), "Use: sent_all ...to see all letters", TRUE);
 else
   fvtell_player(NORMAL_T(p), "%s", DASH_LEN);

 pager(p, PAGER_DEFAULT);
}

void user_mail_sent_check(player *p, parameter_holder *params)
{
 int number_to_show = 15;
 player_tree_node *sp = NULL;

 switch (params->last_param)
 {
  default:
    if (p->saved->priv_admin)
      TELL_FORMAT(p, "[player] [number-to-show]");
    
    TELL_FORMAT(p, "[number-to-show]");
    
  case 2:
    if (!p->saved->priv_admin)
      TELL_FORMAT(p, "<number-to-show>");
    
    if (!(sp = player_find_all(p, GET_PARAMETER_STR(params, 1),
                               PLAYER_FIND_SC_SU)))
      return;
    get_parameter_shift(params, 1);
    /* FALLTHROUGH */
  case 1:
    if (*(GET_PARAMETER_STR(params, 1) + strspn(GET_PARAMETER_STR(params, 1),
                                                "0123456789")))
    {
     if (p->saved->priv_admin)
       TELL_FORMAT(p, "[player] [number-to-show]");
     else
       TELL_FORMAT(p, "[number-to-show]");
    }
    else
      number_to_show = atoi(GET_PARAMETER_STR(params, 1));
    /* FALLTHROUGH */
  case 0: 
    if (!sp)
      sp = p->saved;

    internal_mail_sent_check(p, sp, number_to_show);
    break;
 }
}

void user_mail_sent_check_all(player *p, parameter_holder *params)
{
 player_tree_node *sp = NULL;
 
 if (params->last_param && p->saved->priv_admin)
 {
  if (!(sp = player_find_all(p, GET_PARAMETER_STR(params, 1),
                             PLAYER_FIND_SC_SU)))
    return;
  get_parameter_shift(params, 1);
 }
 else
   sp = p->saved;
 
 internal_mail_sent_check(p, sp, 0);
}

static void user_mail_sent_info(player *p, parameter_holder *params)
{
 player_tree_node *sp = NULL;
 int mail_to_find = 0;
 mail_sent *from = NULL;
 player_linked_list *scan = NULL;
 char buffer[sizeof("Number of reciepients: %d") + BUF_NUM_TYPE_SZ(int)];
 
 switch (params->last_param)
 {
  default:
    if (p->saved->priv_admin)
      TELL_FORMAT(p, "[player] [number-to-show]");
    else
      TELL_FORMAT(p, "[number-to-show]");
    
  case 2:
    if (!p->saved->priv_admin)
      TELL_FORMAT(p, "<number-to-show>");
    
    if (!(sp = player_find_all(p, GET_PARAMETER_STR(params, 1),
                               PLAYER_FIND_SC_SU)))
      return;
    get_parameter_shift(params, 1);
    /* FALLTHROUGH */
  case 1:
    if (*(GET_PARAMETER_STR(params, 1) + strspn(GET_PARAMETER_STR(params, 1),
                                                "0123456789")))
    {
     if (p->saved->priv_admin)
       TELL_FORMAT(p, "[player] [number-to-show]");
     else
       TELL_FORMAT(p, "[number-to-show]");
    }
    else
      mail_to_find = atoi(GET_PARAMETER_STR(params, 1));
    break;
 }

 if (!sp)
   sp = p->saved;

 if (!(from = mail_find_sent(p, sp, mail_to_find)))
   return;

 sprintf(buffer, "Number of reciepients: %d", from->number_of_recipients);
 ptell_mid(NORMAL_T(p), buffer, FALSE);

 fvtell_player(NORMAL_T(p), " Created: %s\n",
               DISP_TIME_P_STD(from->c_timestamp, p));
 fvtell_player(NORMAL_T(p), " Modified: %s\n",
               DISP_TIME_P_STD(from->m_timestamp, p));
 fvtell_player(NORMAL_T(p), " Accessed: %s\n",
               DISP_TIME_P_STD(from->a_timestamp, p));

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);

 scan = from->recipients_start;
 while (scan)
 {
  mail_recieved *rcpt = (mail_recieved *)scan;
  int done = FALSE;
  
  fvtell_player(NORMAL_T(p), "%*s:",
                PLAYER_S_NAME_SZ - 1, PLAYER_LINK_SAV_GET(scan)->name);

  if (rcpt->cc_name)
  {
   fvtell_player(NORMAL_T(p), "%s", done ? ", cc'd" : " Cc'd");
   done = TRUE;
  }

  if (rcpt->grouped)
  {
   fvtell_player(NORMAL_T(p), "%s", done ? ", grouped" : " Grouped");
   done = TRUE;
  }
  
  if (rcpt->read)
  {
   fvtell_player(NORMAL_T(p), "%s", done ? ", read" : " Read");
   done = TRUE;
  }
  
  if (rcpt->deleted)
  {
   fvtell_player(NORMAL_T(p), "%s", done ? ", deleted" : " Deleted");
   done = TRUE;
  }

  if (rcpt->replied)
  {
   fvtell_player(NORMAL_T(p), "%s", done ? ", replied" : " Replied");
   done = TRUE;
  }

  if (done)
    fvtell_player(NORMAL_T(p), "%s", ".");

  fvtell_player(NORMAL_T(p), "%s", "\n");

  scan = PLAYER_LINK_NEXT(scan);
 }
 
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void internal_mail_check(player *p, 
                                player_tree_node *sp, int number_to_show)
{
 mail_recieved *scan = sp->mail_recieved_start;
 int mail_count = mail_check_mailbox_size(sp);
 int count = 0;
 int string_length = 32; /* guess and hope.. */
 
 if (!mail_count)
 {  
  if (p->saved == sp)
    fvtell_player(NORMAL_T(p), "%s", " You have received no mail.\n");
  else
    fvtell_player(NORMAL_T(p), " %s has received no mail.\n", sp->name);
  return;
 }

 if (number_to_show && (number_to_show < mail_count))
 {
  while (scan && (count < (mail_count - number_to_show)))
  {
   if (MAIL_VALID_RECV(scan))
     ++count;

   scan = scan->next;
  }
  assert(scan && (count == (mail_count - number_to_show)));
 }
  
 if (mail_count == 1)
   ptell_mid(NORMAL_T(p), "Received one letter", FALSE);
 else
 {
  char buffer[sizeof("Received $Number(%d)-Tostr letters%s%s%s") +
             (2 * BUF_NUM_TYPE_SZ(int)) + sizeof(" (showing ") + 1];
  char ascii_num[BUF_NUM_TYPE_SZ(int)];

  if (number_to_show && (number_to_show < mail_count))
    sprintf(ascii_num, "%d", number_to_show);
  else
    ascii_num[0] = 0;

  sprintf(buffer, "Received $Number(%d)-Tostr letters%s%s%s",
          mail_count, USE_STRING(ascii_num[0], " (showing "), ascii_num,
          USE_STRING(ascii_num[0], ")"));
  
  ptell_mid(NORMAL_T(p), buffer, FALSE);
 }

 while (scan)
 {
  char buffer[BUF_NUM_TYPE_SZ(int) + 2];

  if (scan->deleted || !scan->mail->tmp_finnished_editing)
  {
   scan = scan->next;
   continue;
  }
  
  sprintf(buffer, "[%d]", ++count);

  mail_load(scan->mail, mail_find_number(scan->mail));
  
  fvtell_player(NORMAL_T(p), "%s%s %-5s %5lu ",
                scan->read ? " " : "U", scan->replied ? "A" : " ",
                buffer, (unsigned long)scan->mail->body_size);

  if (scan->mail->anonymous && p->saved->priv_admin)
    fvtell_player(NORMAL_T(p), "<%s>", scan->mail->owner->name);
  else
    fvtell_player(NORMAL_T(p), "(%s)", scan->mail->anonymous ? "anonymous" :
                  scan->mail->owner->name);
  
  fvtell_player(NORMAL_T(p), ": %.*s\n", string_length, scan->mail->subject);
  
  scan = scan->next;
 }

 if (number_to_show && (number_to_show < mail_count))
   ptell_mid(NORMAL_T(p), "Use: check_all ...to see all letters", TRUE);
 else
   fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

void user_mail_check(player *p, parameter_holder *params)
{
 int number_to_show = 15;
 player_tree_node *sp = NULL;

 switch (params->last_param)
 {
  default:
    if (p->saved->priv_admin)
      TELL_FORMAT(p, "[player] [number-to-show]");
    
    TELL_FORMAT(p, "[number-to-show]");

  case 2:
    if (!p->saved->priv_admin)
      TELL_FORMAT(p, "[number-to-show]");

    if (!(sp = player_find_all(p, GET_PARAMETER_STR(params, 1),
                               PLAYER_FIND_SC_SU)))
      return;
    get_parameter_shift(params, 1);
    /* FALLTHROUGH */    
  case 1:
    if (*(GET_PARAMETER_STR(params, 1) + strspn(GET_PARAMETER_STR(params, 1),
                                                "0123456789")))
    {
     if (p->saved->priv_admin)
       TELL_FORMAT(p, "[player] [number-to-show]");
     else
       TELL_FORMAT(p, "[number-to-show]");
    }
    else
      number_to_show = atoi(GET_PARAMETER_STR(params, 1));
    /* FALLTHROUGH */
  case 0:
    if (!sp)
      sp = p->saved;
    
    internal_mail_check(p, sp, number_to_show);
    pager(p, PAGER_DEFAULT);
    break;
 }
}

void user_mail_check_all(player *p, parameter_holder *params)
{
 player_tree_node *sp = NULL;
 
 if (params->last_param && p->saved->priv_admin)
 {
  if (!(sp = player_find_all(p, GET_PARAMETER_STR(params, 1),
                             PLAYER_FIND_SC_SU)))
    return;
  get_parameter_shift(params, 1);
 }
 else
   sp = p->saved;

 internal_mail_check(p, sp, 0);
 pager(p, PAGER_DEFAULT);
}

static void mail_priv_inorder(player_tree_node *current, va_list va)
{
 player *p = va_arg(va, player *);
 priv_test_type priv = va_arg(va, priv_test_type);
 int as_cc = va_arg(va, int);
 mail_sent *from = p->buffers->mail_buff->mail;
 
 if (current->priv_banished)
   return;
 if (PRIV_SYSTEM_ROOM(current))
   return;
 
 if ((p->saved != current) && (*priv)(current))
   mail_add_mail_recipient(from, current, TRUE, as_cc);
}

static void add_friends_to_mail(player *p, player_tree_node *owner, int as_cc)
{
 list_node *entry = NULL;
 mail_sent *from = p->buffers->mail_buff->mail;

 if (!player_load(owner))
   return;

 entry = owner->player_ptr->list_self_start; /* ignore tmps */
 
 while (entry)
 {
  if (LIST_FLAG(entry, self, friend) && !LIST_FLAG(entry, self, flag_grouped))
  {
   player_tree_node *tag = player_find_all(p, entry->name,
                                           PLAYER_FIND_DEFAULT);
   
   if (tag && !PRIV_SYSTEM_ROOM(tag))
     mail_add_mail_recipient(from, tag, TRUE, as_cc);
  }
  
  entry = entry->next;
 }
}

static int mail_send_add(player *p, const char *name_list, int as_cc)
{
 player_tree_node *current = NULL;
 mail_sent *from = p->buffers->mail_buff->mail;
 char new_name_list[INPUT_BUFFER_SZ] = {0};
 char *group_list = as_cc ? from->cc : from->to;
 char *name = new_name_list;
 
 qstrcpy(new_name_list, name_list);
 while (name)
 {
  char *end_name = NULL;
  
  if ((end_name = N_strchr(name, ',')))
    *end_name++ = 0;

  lower_case(name);
  
  if (!strcmp(name, "me"))
  {
   if (!mail_add_mail_recipient(from, p->saved, FALSE, as_cc))
   {
    assert(FALSE);
   }
  }
  else if (!strcmp(name, "friends"))
  {
   if (!C_strstr(group_list, "Friends"))
   {
    add_friends_to_mail(p, p->saved, as_cc);
    MAIL_GROUP_NAME_ADD("Friends");
   }
  }
  else if (!strcmp(name, "admin"))
  {
   if (!C_strstr(group_list, "Admin"))
   {
    do_inorder_all(mail_priv_inorder, p, priv_test_admin, as_cc);
    MAIL_GROUP_NAME_ADD("Admin");
   }
  }
  else if (!strcmp(name, "sus"))
  {
   if (!C_strstr(group_list, "Sus"))
   {
    do_inorder_all(mail_priv_inorder, p, priv_test_basic_su, as_cc);
    MAIL_GROUP_NAME_ADD("Sus");
   }
  }
  else if (!strcmp(name, "ministers"))
  {
   if (!C_strstr(group_list, "Ministers"))
   {
    do_inorder_all(mail_priv_inorder, p, priv_test_minister, as_cc);
    MAIL_GROUP_NAME_ADD("Ministers");
   }
  }
  else if (!(current = player_tree_find_exact(name)) ||
           PRIV_SYSTEM_ROOM(current) ||
           !mail_add_mail_recipient(from, current, FALSE, as_cc))
    fvtell_player(NORMAL_T(p),
                  " An error has occured. Can no longer locate player "
                  "-- %s --.\n", name);

  name = end_name;
 }

 return (TRUE);
}

static void mail_check_anon(player *p)
{
 mail_sent *from = p->buffers->mail_buff->mail;
 player_linked_list *scan = from->recipients_start;
 
 assert(p->buffers && p->buffers->mail_buff && p->buffers->mail_buff->mail);

 while (scan)
 {
  player_linked_list *scan_next = PLAYER_LINK_NEXT(scan);
  
  if (PRIV_SYSTEM_ROOM(PLAYER_LINK_SAV_GET(scan)) ||
      PLAYER_LINK_SAV_GET(scan)->priv_banished)
  {
   mail_recieved *rcpt = (mail_recieved *)scan;
   fvtell_player(NORMAL_T(p), "%s", " You can't send mail to a room, "
                 "or someone who is banished.\n");
   mail_del_mail_recipient(from, rcpt);
  }
  else
  {
   if (p->buffers->mail_buff->mail->anonymous &&
       PLAYER_LINK_SAV_GET(scan)->flag_no_anonymous)
   {
    mail_recieved *rcpt = (mail_recieved *)scan;
    fvtell_player(NORMAL_T(p), " %s is not receiving anonymous mail.\n",
                  PLAYER_LINK_SAV_GET(scan)->lower_name);
    mail_del_mail_recipient(from, rcpt);
   }
  }
  
  scan = scan_next;
 }
}

static int internal_mail_inform(player_linked_list *passed_scan, va_list va)
{
 player *p = va_arg(va, player *);
 mail_sent *from = va_arg(va, mail_sent *);
 int do_msg = FALSE;
 player *scan = PLAYER_LINK_GET(passed_scan);

 log_assert(scan);
 
 LIST_SELF_CHECK_FLAG_START(scan, p->saved);
 if (LIST_SELF_CHECK_FLAG_DO(article_inform))
   do_msg = TRUE;
 LIST_SELF_CHECK_FLAG_END();

 if (!do_msg)
   return (TRUE);
  
 fvtell_player(SYSTEM_FT(HILIGHT, scan), 
               "$Bell\n -=> New mail, (%d/%d) from %s; '%s'.\n\n",
               mail_check_mailunread_size(scan->saved),
               mail_check_mailbox_size(scan->saved),
               from->anonymous ? "(anon)" : from->owner->name, from->subject);
 
 return (TRUE);
}

static void mail_edit_cleanup(player *p)
{
 assert(MODE_IN_MODE(p, EDIT));
 buffer_mail_destroy(p);
}

static void mail_edit_quit(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", " Letter NOT posted.\n");

 mail_edit_cleanup(p);
}

static void mail_edit_end(player *p)
{
 mail_sent *from = NULL;
 struct timeval tv;
 
 assert(p->buffers && p->buffers->mail_buff && p->buffers->mail_buff->mail);
 
 from = p->buffers->mail_buff->mail;
 
 if (!p->buffers->mail_buff->mail->number_of_recipients)
 {
  fvtell_player(NORMAL_T(p), "%s", " No one to send the letter to!\n");
  
  mail_edit_cleanup(p);
  return;
 }
 
 mail_check_anon(p);

 if (!(from->body = edit_malloc_dump(p, &from->body_size)))
 {
  P_MEM_ERR(p);
  mail_edit_cleanup(p);
  return;
 }
 
 fvtell_player(NORMAL_T(p), " Sending mail.\n");

 from->tmp_finnished_editing = TRUE;
 from->tmp_in_core = TRUE;
 from->owner->flag_tmp_mail_needs_saving = TRUE;
 
 do_order_misc_on(internal_mail_inform, from->recipients_start, p, from);

 /* FIXME: this should be removed eventually ... but there is a slight
  * problem with the log_assert() on internal_mail_load() going off */
 log_assert(!timer_q_find_data(&mail_timer_queue, from));

 gettimeofday(&tv, NULL);
 
 TIMER_Q_TIMEVAL_ADD_SECS(&tv, MAIL_CLEANUP_TIMEOUT_INIT, 0);
 
 timer_q_add_static_node(&from->load_timer, &mail_timer_queue,
                         from, &tv, TIMER_Q_FLAG_NODE_SINGLE);
 
 /* so buffer_mail_destroy doesn't remove the mail */
 p->buffers->mail_buff->mail = NULL;
 
 mail_edit_cleanup(p);
}

static void mail_parse_cleanup(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", " Quitting mail posting.\n");
 buffer_mail_destroy(p);
}

static void internal_mail_edit_start(player *p, mail_sent *from)
{
 fvtell_player(INFO_T(p->saved, p), " Subject: %.*s\n",
               OUT_LENGTH_MAIL_SUBJECT, from->subject);

 fvtell_player(NORMAL_T(p), "%s", " Enter main text of the letter...\n");
 
 if (edit_start(p, NULL))
 {
  assert(MODE_IN_MODE(p, EDIT));

  edit_limit_characters(p, MAIL_ARTICLE_CHARS_SZ);
  edit_limit_lines(p, MAIL_ARTICLE_LINES_SZ);
  
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_quit, mail_edit_quit);
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_end, mail_edit_end);
 }
 else
   buffer_mail_destroy(p);
}

static void mail_parse_subject(player *p, const char *subject)
{
 size_t subject_size = strnlen(subject, MAIL_SUBJECT_SIZE);
 mail_sent *from = NULL;
 
 ICTRACE("send_mail_got_info");
 
 if (!subject || !*subject)
 {
  fvtell_player(NORMAL_T(p), "%s", " You must enter a subject.\n");
  return;
 }
 
 if (!strcasecmp(subject, ".quit"))
 {
  mode_del(p);
  return;
 }
 
 assert(p->buffers && p->buffers->mail_buff &&
	p->buffers->mail_buff->mail);

 from = p->buffers->mail_buff->mail;
 
 if (!(from->subject = MALLOC(subject_size + 1)))
 {
  P_MEM_ERR(p);
  mode_del(p);
  return;
 }

 memcpy(from->subject, subject, subject_size);
 from->subject[subject_size] = 0;

 CMDS_FUNC_TYPE_NOTHING(&MODE_CURRENT(p).cleanup_func, NULL);
 mode_del(p);

 internal_mail_edit_start(p, from);
}

static int mail_bad_user_list(player *p, const char *str)
{
 char new_name_list[INPUT_BUFFER_SZ];
 char *name, *end_name = 0;
 player_tree_node *person;

 if (!*str)
 {
  fvtell_player(NORMAL_T(p), "%s", " Please enter the mail information, "
                "or use '^S^B.quit^s' to quit sending this mail.\n");
  return (TRUE);
 }
 
 name = new_name_list;
 qstrcpy(new_name_list, str);
 
 while (*name)
 {
  if ((end_name = N_strchr(name, ',')))
  {
   *end_name++ = 0;
  }
  else
    end_name = N_strchr(new_name_list, 0);
  
  lower_case(name);
  
  if (strcmp(name, "friends"))
    if (strcmp(name, "admin"))
      if (strcmp(name, "sus"))
        if (strcmp(name, "ministers"))
        {
         if (!(person = player_find_all(p, name, PLAYER_FIND_VERBOSE |
                                        PLAYER_FIND_SELF)))
           return (TRUE);
         else if (PRIV_SYSTEM_ROOM(person))
         {
          fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is a "
                        "system character.\n", person->name);
          return (TRUE);
         }
        }
  
  name = end_name;
 }
 
 return (FALSE);
}   

static void mail_parse_recv_cc(player *p, const char *str)
{
 cmds_function tmp_cmd;

 ICTRACE("mail_parse_recv_cc");
 assert(MODE_IN_MODE(p, MAIL_2));

 if (*str)
 {
  if (!strcasecmp(str, ".quit"))
  {
   mode_del(p);
   return;
  }

  if (mail_bad_user_list(p, str))
    return;
  
  assert(p->buffers && p->buffers->mail_buff &&
         p->buffers->mail_buff->mail);
  
  if (!mail_send_add(p, str, TRUE))
  {
   P_MEM_ERR(p);
   mode_del(p);
   return;
  }
 }

 if (!*p->buffers->mail_buff->mail->cc)
 {
  FREE(p->buffers->mail_buff->mail->cc);
  p->buffers->mail_buff->mail->cc = NULL;
  p->buffers->mail_buff->mail->have_cc = FALSE;
 }
 
 mail_tell_player_cc_list(NORMAL_T(p), p->buffers->mail_buff->mail);

 CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, mail_parse_subject);
 mode_change(p, " Subject: ", MODE_ID_MAIL_3, 0, &tmp_cmd, NULL, NULL);
}

static void mail_parse_recv_to(player *p, const char *str)
{
 cmds_function tmp_cmd;
  
 ICTRACE("mail_parse_recv_to");

 assert(MODE_IN_MODE(p, MAIL_1));

 if (!strcasecmp(str, ".quit"))
 {
  mode_del(p);
  return;
 }

 if (mail_bad_user_list(p, str))
   return;
 
 assert(p->buffers && p->buffers->mail_buff &&
        p->buffers->mail_buff->mail);
 
 if (!mail_send_add(p, str, FALSE))
 {
  P_MEM_ERR(p);
  mode_del(p);
  return;
 }
 if (!*p->buffers->mail_buff->mail->to)
 {
  FREE(p->buffers->mail_buff->mail->to);
  p->buffers->mail_buff->mail->to = NULL;
  p->buffers->mail_buff->mail->have_to = FALSE;
 }

 mail_tell_player_to_list(NORMAL_T(p), p->buffers->mail_buff->mail);
 
 CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, mail_parse_recv_cc);
 mode_change(p, " Cc: ", MODE_ID_MAIL_2, 0, &tmp_cmd, NULL, NULL);
}

static void internal_user_mail_send_letter(player *p, const char *str,
                                           int anon)
{
 int created = 0;
 unsigned int mail_size = 0;
 parameter_holder params;
 
 get_parameter_init(&params);

 mail_size = mail_check_mailout_size(p->saved);
 if (mail_size >= (unsigned int)p->max_mails)
   mail_size -= mail_autoreap_mailout(p->saved);
 
 if (mail_size >= (unsigned int)p->max_mails)
 {
  fvtell_player(NORMAL_T(p), "%s",
                " Sorry, you have reached your mail limit.\n");
  return;
 }
  
 if ((created = buffer_mail_create(p)) == BUFFERS_RET_FAILED)
 {
  P_MEM_ERR(p);
  return;
 }
 else if (created == BUFFERS_RET_USED)
 {
  fvtell_player(NORMAL_T(p), "%s", 
                " Cannot perform this command whilst already mailing, "
                "sorry.\n");
  return;
 }
 assert(created == BUFFERS_RET_WORKED);
 
 if (anon)
   p->buffers->mail_buff->mail->anonymous = TRUE;
 
 get_parameter_parse(&params, &str, 1);
 
 if (mail_bad_user_list(p, params.last_param ?
                        GET_PARAMETER_STR((&params), 1) : ""))
 {
  cmds_function tmp_cmd1;
  cmds_function tmp_cmd2;
  
  CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd1, mail_parse_recv_to);
  CMDS_FUNC_TYPE_NO_CHARS(&tmp_cmd2, mail_parse_cleanup);
  while (!mode_add(p, " To: ", MODE_ID_MAIL_1, 0, &tmp_cmd1, NULL, &tmp_cmd2))
    mode_del(p);
  return;
 }

 assert(p->buffers && p->buffers->mail_buff && p->buffers->mail_buff->mail);
 
 if (!mail_send_add(p, GET_PARAMETER_STR(&params, 1), FALSE))
 {
  P_MEM_ERR(p);
  buffer_mail_destroy(p);
  return;
 }
 if (!*p->buffers->mail_buff->mail->to)
 {
  FREE(p->buffers->mail_buff->mail->to);
  p->buffers->mail_buff->mail->to = NULL;
  p->buffers->mail_buff->mail->have_to = FALSE;
 }
 
 FREE(p->buffers->mail_buff->mail->cc);
 p->buffers->mail_buff->mail->cc = NULL;
 p->buffers->mail_buff->mail->have_cc = FALSE;
   
 mail_tell_player_to_list(NORMAL_T(p), p->buffers->mail_buff->mail);
    
 if (!*str)
 {
  cmds_function tmp_cmd1;
  cmds_function tmp_cmd2;

  fvtell_player(NORMAL_T(p), "%s", " Please enter a subject header, "
                "or use '^S^B.quit^s' to quit sending this mail.\n");

  CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd1, mail_parse_subject);
  CMDS_FUNC_TYPE_NO_CHARS(&tmp_cmd2, mail_parse_cleanup);
  while (!mode_add(p, " Subject: ", MODE_ID_MAIL_3, 0,
                   &tmp_cmd1, NULL, &tmp_cmd2))
    mode_del(p);
 }
 else
 {
  size_t subject_size = strnlen(str, MAIL_SUBJECT_SIZE - 1);
  
  if (!(p->buffers->mail_buff->mail->subject = MALLOC(subject_size + 1)))
  {
   P_MEM_ERR(p);
   buffer_mail_destroy(p);
   return;
  }
  
  COPY_STR_LEN(p->buffers->mail_buff->mail->subject, str, subject_size);

  internal_mail_edit_start(p, p->buffers->mail_buff->mail);
 }
}

void user_mail_send_letter_post(player *p, const char *str)
{
 internal_user_mail_send_letter(p, str, FALSE);
}

void user_mail_send_letter_apost(player *p, const char *str)
{
 internal_user_mail_send_letter(p, str, TRUE);
}

static void mail_tell_player(player_tree_node *saved, player *p,
                             mail_sent *mail)
{
 ptell_mid(INFO_T(saved, p), "header", TRUE);

 fvtell_player(INFO_T(saved, p), "%s", " From:");
 if (!mail->anonymous || (p->saved == mail->owner) || p->saved->priv_admin)
   fvtell_player(INFO_T(saved, p), " %s", mail->owner->name);
 if (mail->anonymous)
   fvtell_player(INFO_T(saved, p), "%s", " (anonymous)");
 fvtell_player(INFO_T(saved, p), "%s","\n");
 
 mail_tell_player_to_list(NORMAL_T(p), mail);
 mail_tell_player_cc_list(NORMAL_T(p), mail);
 
 fvtell_player(INFO_T(saved, p), " Subject: %.*s\n",
               OUT_LENGTH_MAIL_SUBJECT, mail->subject);
 
 fvtell_player(INFO_T(saved, p), " Date: %s\n",
               DISP_TIME_P_STD(mail->c_timestamp, p));

 ptell_mid(INFO_T(saved, p), "body", TRUE);
 
 fvtell_player(INFO_T(saved, p), "%.*s", OUT_LENGTH_MAIL_POST, mail->body);

 fvtell_player(INFO_T(saved, p), "%s", DASH_LEN);
 
 pager(p, PAGER_DEFAULT);
}

void user_mail_read_sent(player *p, const char *str)
{
 int mail_to_read = 1;
 mail_sent *scan = NULL;
 twinkle_info info;
 
 setup_twinkle_info(&info);
 
 info.returns_limit = UINT_MAX;
 info.allow_fills = TRUE;
 
 if (!*str)
   TELL_FORMAT(p, "<mail_number>");

 mail_to_read = atoi(str);
 if (!(scan = mail_find_sent(p, p->saved, mail_to_read)))
   return;
 
 assert(scan->owner == p->saved);
 
 mail_load(scan, mail_to_read);

 mail_tell_player(p->saved, p, scan);
}

void user_mail_read_letter(player *p, const char *str)
{
 mail_recieved *scan = NULL;
 int mail_to_read = 1;

 twinkle_info info;
 
 setup_twinkle_info(&info);
 
 info.returns_limit = UINT_MAX;
 info.allow_fills = TRUE;

 if (!*str)
   TELL_FORMAT(p, "<mail_number>");

 mail_to_read = atoi(str);
 if (!(scan = mail_find_recieved(p, p->saved, mail_to_read)))
   return;
 
 assert(scan->mail->owner);

 mail_load(scan->mail, mail_find_number(scan->mail));

 mail_tell_player(scan->mail->owner, p, scan->mail);
 if (!scan->read)
 {
  scan->read = TRUE;
  scan->mail->owner->flag_tmp_mail_needs_saving = TRUE;
  scan->mail->m_timestamp = now;
 }
}

void mail_reply_text(player *p,
                     player_tree_node *owner, time_t c_timestamp, 
                     const char *subject, const char *text_body, int anon)
{
 int created = BUFFERS_RET_WORKED;
 size_t subject_size = 0;
 char buffer[sizeof("On %s GMT, %s wrote:\n") + PLAYER_S_NAME_SZ + 128];
 char *body = buffer;
 mail_sent *from = NULL;
 unsigned int mail_size = 0;
 
 mail_size = mail_check_mailout_size(p->saved);
 if (mail_size >= (unsigned int)p->max_mails)
   mail_size -= mail_autoreap_mailout(p->saved);
 
 if (mail_size >= (unsigned int)p->max_mails)
 {
  fvtell_player(NORMAL_T(p), "%s",
                " Sorry, you have reached your mail limit.\n");
  return;
 }
 
 if ((created = buffer_mail_create(p)) == BUFFERS_RET_FAILED)
 {
  P_MEM_ERR(p);
  return;
 }
 else if (created == BUFFERS_RET_USED)
 {
  fvtell_player(NORMAL_T(p), "%s",
                " Cannot reply when already mailing, sorry.\n");
  return;
 }
 assert(created == BUFFERS_RET_WORKED);

 assert(p->buffers && p->buffers->mail_buff && p->buffers->mail_buff->mail);
 
 from = p->buffers->mail_buff->mail;

 FREE(from->to);
 FREE(from->cc);
 from->cc = from->to = NULL; /* can't do group reply, yet */
 from->have_cc = from->have_to = FALSE;
 
 if (anon)
   from->anonymous = TRUE;

 subject_size = strnlen(subject, MAIL_SUBJECT_SIZE - 1);
 if (BEG_CONST_STRCMP("Re: ", subject))
   subject_size += CONST_STRLEN("Re: ");
 
 if (!(from->subject = MALLOC(subject_size + 1)))
 {
  P_MEM_ERR(p);
  buffer_mail_destroy(p);
  return;
 }

 if (!BEG_CONST_STRCMP("Re: ", subject))
 {
  memcpy(from->subject, subject, subject_size);
  from->subject[subject_size] = 0;
 }
 else
   sprintf(from->subject, "Re: %.*s", MAIL_SUBJECT_SIZE - 1, subject);

 mail_add_mail_recipient(from, owner, FALSE, FALSE);

 mail_tell_player_to_list(NORMAL_T(p), from);
 mail_tell_player_cc_list(NORMAL_T(p), from);
 
 fvtell_player(NORMAL_T(p), " Subject: %.*s\n",
               OUT_LENGTH_MAIL_SUBJECT, from->subject);
 
 sprintf(body, "On %s GMT, %s wrote:\n",
         disp_time_std(c_timestamp, 0, TRUE, TRUE), owner->name);

 if (edit_indent_start(p, body, text_body))
 {
  assert(MODE_IN_MODE(p, EDIT));

  edit_limit_characters(p, MAIL_ARTICLE_CHARS_SZ);
  edit_limit_lines(p, MAIL_ARTICLE_LINES_SZ);
  
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_quit, mail_edit_quit);
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_end, mail_edit_end);
 }
 else
   buffer_mail_destroy(p);
}

static void internal_user_mail_reply_letter(player *p, const char *str,
                                            int anon)
{
 mail_recieved *replying_to = NULL;
 int mail_to_reply = -1;
 
 if (!*str)
   TELL_FORMAT(p, "<number>");
 mail_to_reply = atoi(str);
 
 if (!(replying_to = mail_find_recieved(p, p->saved, mail_to_reply)))
   return;

 if (replying_to->mail->anonymous)
 {
  fvtell_player(NORMAL_T(p), "%s", " You cannot reply to anonymous mail.\n");
  return;
 }
 mail_load(replying_to->mail, mail_find_number(replying_to->mail));

 replying_to->replied = TRUE;
 replying_to->mail->owner->flag_tmp_mail_needs_saving = TRUE;
 replying_to->mail->m_timestamp = now;
 
 mail_reply_text(p,
                 replying_to->mail->owner, replying_to->mail->c_timestamp, 
                 replying_to->mail->subject, replying_to->mail->body, anon);
}

void user_mail_reply_letter_reply(player *p, const char *str)
{
 internal_user_mail_reply_letter(p, str, FALSE);
}

void user_mail_reply_letter_areply(player *p, const char *str)
{
 internal_user_mail_reply_letter(p, str, TRUE);
}

void user_mail_delete_sent(player *p, parameter_holder *params)
{
 const char *str = NULL;
 mail_sent *scan = NULL;
 int count = 0;
 
 if (params->last_param != 1)
   TELL_FORMAT(p, "<number(s)|\"all\">");

 str = GET_PARAMETER_STR(params, 1);
 if (!beg_strcasecmp(str, "all"))
 {
  mail_load_all(p->saved); /* FIXME: do it like rooms so we don't have to
                            * do this */

  while ((scan = mail_find_sent(NULL, p->saved, 1)))
  {   
   scan->owner->flag_tmp_mail_needs_saving = TRUE;
   
   mail_destroy_mail_sent(p->saved, scan);

   ++count;
  }
 }
 else
 {
  int store[128];
  mail_sent *to_del[1024];
  int num = 0;
  int size = 0;
  
  mail_load_all(p->saved); /* FIXME: do it like rooms so we don't have to
                            * do this */
  
  str += strcspn(str, "0123456789");
  while (*str && (size < 128))
  {
   num = skip_atoi(&str);

   if (num >= MAIL_DELETE_SZ)
   {
    fvtell_player(NORMAL_T(p),
                  " Only mail numbers ^S^B1^S to ^s%d^s may be deleted.\n",
                  MAIL_DELETE_SZ);
    break;
   }
   
   if ((scan = mail_find_sent(p, p->saved, num)))
   {
    to_del[num - 1] = scan;
    store[size++] = num - 1;
    
    scan->owner->flag_tmp_mail_needs_saving = TRUE;
   }
   
   str += strcspn(str, "0123456789");
  }

  num = 0;
  while (num < size)
  {
   if (!to_del[store[num]])
   {
    ++num;
    continue;
   }
   ++count;
   
   fvtell_player(NORMAL_T(p), " Deleted sent mail number %d.\n",
                 store[num] + 1);

   mail_destroy_mail_sent(p->saved, to_del[store[num]]);

   to_del[store[num]] = NULL;

   ++num;
  }
 }
 
 if (!count)
   fvtell_player(NORMAL_T(p), "%s", " You haven't deleted any sent mail.\n");
 else
   fvtell_player(NORMAL_T(p),
                 " Deleted %d sent mail%s ^S^B%d^s left.\n", count,
                 (count == 1) ? "" : "s", mail_check_mailout_size(p->saved));
}

void user_mail_delete_received(player *p, parameter_holder *params)
{
 const char *str = NULL;
 mail_recieved *scan = NULL;
 int count = 0;
 
 if (params->last_param != 1)
   TELL_FORMAT(p, "<number(s)|\"all\">");

 str = GET_PARAMETER_STR(params, 1);
 if (!beg_strcasecmp(str, "all"))
 {
  while ((scan = mail_find_recieved(NULL, p->saved, 1)))
  {
   mail_load(scan->mail, mail_find_number(scan->mail));
   
   scan->deleted = TRUE;
   scan->mail->owner->flag_tmp_mail_needs_saving = TRUE;
   scan->mail->m_timestamp = now;
   
   ++count;
  }
 }
 else
 {
  int store[128];
  mail_recieved *to_del[1024];
  int num = 0;
  int size = 0;
  
  str += strcspn(str, "0123456789");
  while (*str && (size < 128))
  {
   num = skip_atoi(&str);
   
   if (num >= MAIL_DELETE_SZ)
   {
    fvtell_player(NORMAL_T(p),
                  " Only mail numbers ^S^B1^S to ^s%d^s may be deleted.\n",
                  MAIL_DELETE_SZ);
    break;
   }
   
   if ((scan = mail_find_recieved(p, p->saved, num)))
   {
    mail_load(scan->mail, mail_find_number(scan->mail));

    to_del[num - 1] = scan;
    store[size++] = num - 1;
    
    scan->mail->owner->flag_tmp_mail_needs_saving = TRUE;
   }
   
   str += strcspn(str, "0123456789");
  }

  num = 0;
  while (num < size)
  {
   if (!to_del[store[num]])
   {
    ++num;
    continue;
   }
   ++count;

   fvtell_player(NORMAL_T(p), " Deleted recieved mail number %d.\n",
                 store[num] + 1);

   to_del[store[num]]->deleted = TRUE;
   to_del[store[num]]->mail->m_timestamp = now;

   to_del[store[num]] = NULL;

   ++num;
  }
 }
 
 if (!count)
   fvtell_player(NORMAL_T(p), "%s",
                 " You haven't deleted any recieved mail.\n");
 else
   fvtell_player(NORMAL_T(p),
                 " Deleted %d recieved mail%s ^S^B%d^s left.\n", count,
                 (count == 1) ? "" : "s", mail_check_mailbox_size(p->saved));
}

void user_toggle_anonymous(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->saved->flag_no_anonymous, TRUE,
                       " You will %snot be able to receive anonymous mail.\n",
                       " You will %sbe able to receive anonymous mail.\n",
                       TRUE);
}

static void internal_mail_load_saved(player_tree_node *sp,
                                     va_list ap __attribute__ ((unused)))
{
 if (PRIV_SYSTEM_ROOM(sp))
   return;
 
 mail_load_saved(sp);
}

void init_mail(void)
{
 timer_q_add_static_base(&mail_timer_queue, timed_mail_cleanup,
                         TIMER_Q_FLAG_BASE_DEFAULT);
 
 do_inorder_all(internal_mail_load_saved);
}

void cmds_init_mail(void)
{
 CMDS_BEGIN_DECLS();
 
#define CMDS_SECTION_SUB CMDS_SECTION_MAIL

 CMDS_ADD_SUB("apost", user_mail_send_letter_apost, CONST_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("areply", user_mail_reply_letter_areply, CONST_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("check", user_mail_check, PARSE_PARAMS);
 CMDS_ADD_SUB("check_all", user_mail_check_all, PARSE_PARAMS);
 CMDS_ADD_SUB("commands", mail_view_commands, NO_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("delete_recieved", user_mail_delete_received, PARSE_PARAMS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("delete_sent", user_mail_delete_sent, PARSE_PARAMS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("end", mail_exit_mode, NO_CHARS);
 CMDS_FLAG(no_expand); CMDS_PRIV(mode_mail);
 CMDS_ADD_SUB("noanonymous", user_toggle_anonymous, CONST_CHARS);
 CMDS_ADD_SUB("post", user_mail_send_letter_post, CONST_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("read_recieved", user_mail_read_letter, CONST_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("read_sent", user_mail_read_sent, CONST_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("remove", user_mail_delete_sent, PARSE_PARAMS); /* FIXME: alias */
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("reply", user_mail_reply_letter_reply, CONST_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("sent", user_mail_sent_check, PARSE_PARAMS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("sent_all", user_mail_sent_check_all, PARSE_PARAMS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("sent_info", user_mail_sent_info, PARSE_PARAMS);
 CMDS_PRIV(command_mail);
/* FIXME: CMDS_ADD_SUB("signature", mail_exit_with_sig, NO_CHARS); */
 CMDS_ADD_SUB("view", user_mail_check, PARSE_PARAMS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("view_all", user_mail_check_all, PARSE_PARAMS);
 CMDS_PRIV(command_mail);

#undef CMDS_SECTION_SUB

 CMDS_ADD("mail", mail_command, RET_CHARS_SIZE_T, MISC);
 CMDS_PRIV(command_mail);
}
