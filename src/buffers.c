#define BUFFERS_C
/*
 *  Copyright (C) 1999 James Antill, John Tobin
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



static int buffer_main_create(player *p)
{
 if (p->buffers)
   ++p->buffers->ref_count;
 else
   if ((p->buffers = MALLOC(sizeof(buffers_struct))))
   {
    p->buffers->mail_buff = NULL;
    p->buffers->news_buff = NULL;
    p->buffers->pager_buff = NULL;
    p->buffers->room_buff = NULL;
    p->buffers->file_buff = NULL;
    p->buffers->msg_edit_buff = NULL;

    p->buffers->ref_count = 1;
    p->buffers->current_edit_base = NULL;
   }
   else
     return (FALSE);
  
 return (TRUE);
}

static void buffer_main_destroy(player *p)
{
 assert(p->buffers);
 
 if (p->buffers)
 {
  if (--p->buffers->ref_count < 1)
  {
   assert(!p->buffers->ref_count);
   
   FREE(p->buffers);
   
   p->buffers = NULL;
  }
 }
}

int buffer_mail_create(player *p)
{
 mail_sent *from = NULL;
 
 if (p->buffers)
   if (p->buffers->mail_buff)
     return (BUFFERS_RET_USED);
 
 if (!buffer_main_create(p))
   return (BUFFERS_RET_FAILED);
 
 assert(p->buffers);

 if (!(p->buffers->mail_buff = MALLOC(sizeof(mail_buffer))))
   goto malloc_mail_buff_failed;

 assert(p->buffers && p->buffers->mail_buff);

 if (!(from = p->buffers->mail_buff->mail =
       mail_create_mail_sent(p->saved, now)))
   goto create_mail_sent_failed;

 if (!(from->to = MALLOC(MULTI_GROUP_NAME_SIZE)))
   goto create_mail_to_failed;

 if (!(from->cc = MALLOC(MULTI_GROUP_NAME_SIZE)))
   goto create_mail_cc_failed;

 assert(p->buffers && p->buffers->mail_buff && p->buffers->mail_buff->mail);

 from->to[0] = 0;
 from->cc[0] = 0;
 from->have_cc = TRUE;
 from->have_to = TRUE;
 from->tmp_finnished_editing = FALSE;
 from->tmp_in_core = TRUE;
 
 p->buffers->mail_buff->edit_base_copy = p->buffers->current_edit_base;
 p->buffers->current_edit_base = NULL;

 return (BUFFERS_RET_WORKED);

 create_mail_cc_failed:
 FREE(from->to);
 from->to = NULL;
 
 create_mail_to_failed:
 mail_destroy_mail_sent(p->saved, from);
 
 create_mail_sent_failed: 
 FREE(p->buffers->mail_buff);
 p->buffers->mail_buff = NULL;

 malloc_mail_buff_failed:
 buffer_main_destroy(p);
 return (BUFFERS_RET_FAILED);
}

void buffer_mail_destroy(player *p)
{
 assert(p->buffers && p->buffers->mail_buff);
 
 if (p->buffers && p->buffers->mail_buff)
 {
  if (p->buffers->mail_buff->mail)
  {
   assert(!p->buffers->mail_buff->mail->tmp_finnished_editing);
   assert(MALLOC_VALID(p->buffers->mail_buff->mail, sizeof(mail_sent),
                       MAIL_SENT));
   mail_destroy_mail_sent(p->saved, p->buffers->mail_buff->mail);
  }
  
  p->buffers->current_edit_base = p->buffers->mail_buff->edit_base_copy;
  
  FREE(p->buffers->mail_buff);
  p->buffers->mail_buff = NULL;
 
  buffer_main_destroy(p);
 }
}

int buffer_file_create(player *p)
{
 if (p->buffers && p->buffers->file_buff)
   return (BUFFERS_RET_USED);
 
 if (!buffer_main_create(p))
   return (BUFFERS_RET_FAILED);

 assert(p->buffers);

 if (!(p->buffers->file_buff = MALLOC(sizeof(file_buffer))))
   goto malloc_file_buff_failed;

 assert(p->buffers && p->buffers->file_buff);
 
 p->buffers->file_buff->edit_base_copy = p->buffers->current_edit_base;
 p->buffers->current_edit_base = NULL;

 return (BUFFERS_RET_WORKED);
 
 malloc_file_buff_failed:
 buffer_main_destroy(p);
 return (BUFFERS_RET_FAILED);
}

void buffer_file_destroy(player *p)
{
 assert(p->buffers && p->buffers->file_buff);
 if (p->buffers && p->buffers->file_buff)
 {  
  p->buffers->current_edit_base = p->buffers->file_buff->edit_base_copy;
    
  FREE(p->buffers->file_buff);
  p->buffers->file_buff = NULL;
  
  buffer_main_destroy(p);
 }
}

int buffer_msg_edit_create(player *p)
{
 if (p->buffers && p->buffers->msg_edit_buff)
   return (BUFFERS_RET_USED);
 
 if (!buffer_main_create(p))
   return (BUFFERS_RET_FAILED);

 assert(p->buffers);

 if (!(p->buffers->msg_edit_buff = MALLOC(sizeof(msg_edit_buffer))))
   goto malloc_file_buff_failed;

 assert(p->buffers && p->buffers->msg_edit_buff);

 p->buffers->msg_edit_buff->edited_msg = NULL;
 
 p->buffers->msg_edit_buff->edit_base_copy = p->buffers->current_edit_base;
 p->buffers->current_edit_base = NULL;

 return (BUFFERS_RET_WORKED);
 
 malloc_file_buff_failed:
 buffer_main_destroy(p);
 return (BUFFERS_RET_FAILED);
}

void buffer_msg_edit_destroy(player *p)
{
 assert(p->buffers && p->buffers->msg_edit_buff);
 if (p->buffers && p->buffers->msg_edit_buff)
 {  
  p->buffers->current_edit_base = p->buffers->msg_edit_buff->edit_base_copy;
    
  FREE(p->buffers->msg_edit_buff);
  p->buffers->msg_edit_buff = NULL;
  
  buffer_main_destroy(p);
 }
}

int buffer_pager_create(player *p)
{
 if (p->buffers && p->buffers->pager_buff)
   return (BUFFERS_RET_USED);
 
 if (!buffer_main_create(p))
   return (BUFFERS_RET_FAILED);

 assert(p->buffers);

 if (!(p->buffers->pager_buff = MALLOC(sizeof(pager_buffer))))
 {
  buffer_main_destroy(p);
  return (BUFFERS_RET_FAILED);
 }
 
 assert(p->buffers && p->buffers->pager_buff);

 p->buffers->pager_buff->searching = NULL;
 p->buffers->pager_buff->search_mode = FALSE;
 p->buffers->pager_buff->command_mode = FALSE;
 
 p->buffers->pager_buff->edit_base_copy = p->buffers->current_edit_base;
 p->buffers->current_edit_base = NULL;
 
 return (BUFFERS_RET_WORKED);
}

void buffer_pager_destroy(player *p)
{
 assert(p->buffers && p->buffers->pager_buff);
 
 if (p->buffers && p->buffers->pager_buff)
 {
  p->buffers->current_edit_base = p->buffers->pager_buff->edit_base_copy;

  if (p->buffers->pager_buff->searching)
    FREE(p->buffers->pager_buff->searching);
  
  FREE(p->buffers->pager_buff);
  p->buffers->pager_buff = NULL;
  
  buffer_main_destroy(p);
 }
}

int buffer_room_create(player *p)
{
 if (p->buffers && p->buffers->room_buff)
   return (BUFFERS_RET_USED);
 
 if (!buffer_main_create(p))
   return (BUFFERS_RET_FAILED);

 assert(p->buffers);

 if (!(p->buffers->room_buff = MALLOC(sizeof(room_buffer))))
 {
  buffer_main_destroy(p);
  return (BUFFERS_RET_FAILED);
 }

 assert(p->buffers && p->buffers->room_buff);
 
 sprintf(p->buffers->room_buff->edited_room, "%s.%s",
         p->location->owner->lower_name, p->location->id);
 lower_case(p->buffers->room_buff->edited_room);
 
 p->buffers->room_buff->edit_base_copy = p->buffers->current_edit_base;
 p->buffers->current_edit_base = NULL;

 return (BUFFERS_RET_WORKED);
}

void buffer_room_destroy(player *p)
{
 assert(p->buffers && p->buffers->room_buff);
 if (p->buffers && p->buffers->room_buff)
 {
  p->buffers->current_edit_base = p->buffers->room_buff->edit_base_copy;
  
  FREE(p->buffers->room_buff);
  p->buffers->room_buff = NULL;

  buffer_main_destroy(p);
 }
}

int buffer_news_create(player *p)
{
 if (p->buffers && p->buffers->news_buff)
   return (BUFFERS_RET_USED);
 
 if (!buffer_main_create(p))
   return (BUFFERS_RET_FAILED);

 assert(p->buffers);

 if (!(p->buffers->news_buff = MALLOC(sizeof(news_buffer))))
 {
  buffer_main_destroy(p);
  return (BUFFERS_RET_FAILED);
 }

 assert(p->buffers && p->buffers->news_buff);
 
 p->buffers->news_buff->edit_base_copy = p->buffers->current_edit_base;
 p->buffers->current_edit_base = NULL;
 
 return (BUFFERS_RET_WORKED);
}

void buffer_news_destroy(player *p)
{
 assert(p->buffers && p->buffers->news_buff);
 
 if (p->buffers && p->buffers->news_buff)
 {
  p->buffers->current_edit_base = p->buffers->news_buff->edit_base_copy;

  FREE(p->buffers->news_buff);
  p->buffers->news_buff = NULL;
  
  buffer_main_destroy(p);
 }
}
