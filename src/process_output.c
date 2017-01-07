#define PROCESS_OUTPUT_C
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

/* forward reference... Yes, functions do call each other :*/
static output_node *normal_char_output(player *, int, char,
                                       int *, output_node *);
static output_node *output_string(player_tree_node *, player *,
				  twinkle_info *, int, time_t,
                                  const char **input,
				  int, int *, output_node *);
static output_node *room_variables(player_tree_node *, player *,
				   twinkle_info *, int, time_t,
                                   room *, const char **,
				   int *, output_node *);
static output_node *output_return(player *, int, int, int *, output_node *);

#define DEFAULT_TWINKLE_SETUP \
 {FALSE/* used twinkles */, TRUE/* allow fills */, FALSE /* output not me */, \
  0/* number of returns */, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} /* twinkle counters */}
void setup_twinkle_info(twinkle_info *info)
{ /* hash define used in vfvtell_player and output_string */
 twinkle_info the_info = DEFAULT_TWINKLE_SETUP;
 assert(info);

 if (!info) /* safety check */
   return;

 /* setup defaults values */
 *info = the_info;
}

/* new_output_node needs this */
static void destroy_output_node(output_node **head, output_node *togo)
{
 assert(head && MALLOC_VALID(*head, sizeof(output_node), OUTPUT_NODE));
 assert(togo);
  
 if (togo->prev)
   togo->prev->next = togo->next;
 else
 {
  assert(*head == togo);
  *head = togo->next;
 }
 
 if (togo->next)
   togo->next->prev = togo->prev;
 
 XFREE(togo, OUTPUT_NODE);
}

static output_node *new_output_node(player *p, int flags, time_t timestamp,
                                    output_node *from, int skip_to_end)
{
 output_node **head = NULL;
 output_node *tmp = 0;
 output_node *new_node = 0;
 int count = 0;

 BTRACE("new_output_node");

 if (flags & OUTPUT_BUFFER_TMP)
   head = &p->output_buffer_tmp;
 else
   head = &p->output_start;
 
 if (!(new_node = XMALLOC(sizeof(output_node), OUTPUT_NODE)))
   return (NULL);

 if (from)
   tmp = from;
 else
   tmp = *head;

 if (tmp && skip_to_end)
   while (tmp->next && (difftime(timestamp, tmp->timestamp) >= 0))
   {
    tmp = tmp->next;
    count++;
   }

 if (count > MAX_OUTPUT_LIST)
   /* note: can be more than MAX nodes and they just havn't been counted */
   while (--count > (MAX_OUTPUT_LIST - 1))
     destroy_output_node(head, *head);
  
 if (tmp && (difftime(timestamp, tmp->timestamp) >= 0))
 {
  new_node->next = tmp->next;
  new_node->prev = tmp;
  tmp->next = new_node;
  if (new_node->next)
    new_node->next->prev = new_node;
 }
 else
 {
  if ((new_node->next = tmp))
  {
   if ((new_node->prev = tmp->prev))
     new_node->prev->next = new_node;
   else
     *head = new_node;

   tmp->prev = new_node;
  }
  else
  {
   new_node->prev = 0;
   *head = new_node;
  }
 }
  
 new_node->length = new_node->output_bit = 0;
 new_node->end_repeat_list = new_node->full_buffer = new_node->has_return
   = new_node->output_already = FALSE;
 new_node->flags = flags;
 new_node->timestamp = timestamp;

 return (new_node);
}

void save_tmp_output_list(player *p, tmp_output_list_storage *tmp)
{
 tmp->output_buffer_tmp = p->output_buffer_tmp;
 p->output_buffer_tmp = NULL;

#if 0
 /* can't do output in load */
 tmp->type_ptr = p->type_ptr;
 while (count < tmp->type_ptr)
 {
  tmp->output_type[count] = p->output_type[count];
  ++count;
 }
#endif
}

void load_tmp_output_list(player *p, tmp_output_list_storage *tmp)
{
 assert(!p->output_buffer_tmp); /* should have done a grab */
 p->output_buffer_tmp = tmp->output_buffer_tmp;

#if 0
 /* can't do output here */
 p->type_ptr = tmp->type_ptr;
 while (count < tmp->type_ptr)
 {
  p->output_type[count] = tmp->output_type[count];
  ++count;
 }
#endif
}

output_node *output_list_grab(player *p)
{
 output_node *tmp = NULL;

 assert(p);
 tmp = p->output_buffer_tmp;
 p->output_buffer_tmp = NULL;
 
 return (tmp);
}

static int output_node_print_copy(player *p, int max_print_length,
                                  int *print_length,
                                  output_node *from, output_node **to,
                                  int offset)
{
 int length = 0;
  
 assert(((max_print_length > 0) || (from->flags & OUTPUT_BUFFER_NON_PRINT)) &&
        (offset >= 0) && from);
 
 assert(!(from->flags & OUTPUT_REPEAT));

 if (from->length <= offset)
 {
  assert(!from->length);
  return (0);
 }

 length = from->length - offset;
 if (from->flags & OUTPUT_BUFFER_NON_PRINT)
 {  
  *to = output_raw(p, (*to)->flags | OUTPUT_BUFFER_NON_PRINT,
                   from->start + offset, length, *to);
#ifndef NDEBUG
  if (from->has_return)
  { /* from has a return in it as below */
   assert(((*to)->start[(*to)->length - 1] == '\n') &&
          ((*to)->start[(*to)->length - 2] == '\r'));
  }
#endif

  (*to)->has_return = TRUE;
  *to = new_output_node(p, (*to)->flags, (*to)->timestamp, *to, FALSE);
 }
 else
 {
  if (max_print_length < length)
    length = max_print_length - offset;
  
  *print_length += length;
  *to = output_raw(p, (*to)->flags & ~OUTPUT_BUFFER_NON_PRINT,
                   from->start + offset, length, *to);
 }

 return (length);
}

static output_node *output_list_print_copy(player *p, int max_print_length,
                                           output_node *from, output_node *to,
                                           int from_begining)
{
 assert(p && from && to && (max_print_length > 0));
 BTRACE("output_list_print_copy");

 assert(!(from->flags & OUTPUT_REPEAT));

 if (!from_begining && (max_print_length != INT_MAX))
 { /* we want to print the LAST max_length items */
  int length_left = output_list_print_length(p, from, 0, INT_MAX);
    
  if (length_left > max_print_length)
  { /* skip to the node we should be starting at */
   int offset = (length_left - max_print_length);
   int print_length = 0; 

   while ((from->flags & OUTPUT_BUFFER_NON_PRINT) ||
          (offset >= from->length))
   {
    if (!(from->flags & OUTPUT_BUFFER_NON_PRINT))
      offset -= from->length;
        
    from = from->next;
    assert(!(from->flags & OUTPUT_REPEAT));
   }
   
   if (offset > 0)
   {
    print_length = 0;
    output_node_print_copy(p, max_print_length, &print_length,
                           from, &to, offset);
    DECREMENT_LENGTH(max_print_length, print_length);
    assert(!(from->flags & OUTPUT_REPEAT));
    from = from->next;
   }
  }
 }
 
 BTRACE("output_list_print_copy (main copy)");

 while (from && ((max_print_length > 0) ||
                 (from->flags & OUTPUT_BUFFER_NON_PRINT)))
 {
  int print_length = 0;
  
  output_node_print_copy(p, max_print_length, &print_length,
                         from, &to, 0);
  DECREMENT_LENGTH(max_print_length, print_length);
  
  from = from->next;
 }
 assert(!(max_print_length && from));

 return (to);
}

output_node *output_list_copy(player *p, output_node *one,
                              int max_print_length,
                              int flags, int from_begining)
{
 tmp_output_list_storage tmp_save;
 output_node *out_node = NULL;
 
 assert(MALLOC_VALID(one, sizeof(output_node), OUTPUT_NODE));

 save_tmp_output_list(p, &tmp_save);
  
 if (!one)
 {
  load_tmp_output_list(p, &tmp_save);
  return (NULL);
 }

 if (!(out_node = new_output_node(p, flags | OUTPUT_BUFFER_TMP,
                                  one->timestamp, NULL, TRUE)))
 {
  load_tmp_output_list(p, &tmp_save);
  return (NULL);
 }
 
 output_list_print_copy(p, max_print_length, one, out_node, from_begining);
 out_node = output_list_grab(p);

 load_tmp_output_list(p, &tmp_save);
 return (out_node);
}

static int output_get_repeat_len(player *p, output_node **rep_node,
                                 int cur_indent, int max_length)
{
 int length_left = 0;
 int rep_length = 0;

 assert(rep_node &&
        MALLOC_VALID(*rep_node, sizeof(output_node), OUTPUT_NODE) &&
        ((*rep_node)->flags & OUTPUT_REPEAT) && p);
 
 while (*rep_node)
 {
  assert((*rep_node)->flags & OUTPUT_REPEAT);
  rep_length += (*rep_node)->length;
  if ((*rep_node)->end_repeat_list)
    break;
  *rep_node = (*rep_node)->next;
 }
 
 if (p->term_width)
   length_left = (p->term_width - (p->column + cur_indent));
 else
   if (p->column + cur_indent)
     length_left = 79 - ((p->column + cur_indent) % 79);
   else
     length_left = 79;
 
 if (length_left > max_length)
   length_left = max_length;

 if (rep_length > length_left)
   return (0);
 else
   return (length_left - (length_left % rep_length));
}

static output_node *output_node_repeat_expand(player *p, output_node *rep_node,
                                              output_node *const pre_rep_node,
                                              output_node **pre_rep_node_next,
                                              int cur_indent, int max_length)
{
 int length_left = 0;
 int rep_length = 0;
 output_node *rep_start = rep_node;
 output_node *rep_finish = NULL;
 
 assert(rep_node && (rep_node->flags & OUTPUT_REPEAT) && p &&
        pre_rep_node_next && (*pre_rep_node_next == rep_node));
 
 rep_node->prev = NULL;
 while (rep_node)
 {
  assert(rep_node->flags & OUTPUT_REPEAT);
  rep_length += rep_node->length;
  rep_finish = rep_node;
  if (rep_node->end_repeat_list)
    break;
  rep_node = rep_node->next;
 }

 if (rep_finish->next)
   rep_finish->next->prev = pre_rep_node;
 
 *pre_rep_node_next = rep_finish->next;

 rep_finish->next = NULL;
 
 if (p->term_width)
   length_left = (p->term_width - (p->column + cur_indent));
 else
   if (p->column + cur_indent)
     length_left = 79 - ((p->column + cur_indent) % 79);
   else
     length_left = 79;
 
 if (length_left > max_length)
   length_left = max_length;

 if (rep_length <= length_left)
 { /* link repeat nodes in */
  int count = rep_length;
  tmp_output_list_storage tmp_save;
  int flags = OUTPUT_BUFFER_TMP | (rep_start->flags & ~OUTPUT_REPEAT);
  output_node *out_node = NULL;
  output_node *out_node_start = NULL;

  save_tmp_output_list(p, &tmp_save);

  if (!(out_node  = new_output_node(p, flags,
                                    rep_start->timestamp, NULL, TRUE)))
  {
   load_tmp_output_list(p, &tmp_save);
   return (NULL);
  }
  out_node_start = out_node;
  
  while (count <= length_left)
  {
   output_node *tmp_rep = rep_start;

   while (tmp_rep)
   {
    flags &= ~OUTPUT_BUFFER_NON_PRINT;
    flags |= (tmp_rep->flags & OUTPUT_BUFFER_NON_PRINT);
    out_node = output_raw(p, flags, tmp_rep->start, tmp_rep->length,
                          out_node);

    tmp_rep = tmp_rep->next;
   }
   
   count += rep_length;
  }

  out_node->full_buffer = TRUE; /* make sure the last one goes out */

  out_node_start->prev = pre_rep_node;
  if ((out_node->next = *pre_rep_node_next))
    (*pre_rep_node_next)->prev = out_node;
  *pre_rep_node_next = out_node_start;

  p->output_buffer_tmp = NULL;
  load_tmp_output_list(p, &tmp_save);
 }
 else if (!*pre_rep_node_next)
 {
  int flags = OUTPUT_BUFFER_TMP | (rep_start->flags & ~OUTPUT_REPEAT);
  tmp_output_list_storage tmp_save;
  
  save_tmp_output_list(p, &tmp_save);
  *pre_rep_node_next = new_output_node(p, flags,
                                       rep_start->timestamp, NULL, TRUE);
  p->output_buffer_tmp = NULL;
  load_tmp_output_list(p, &tmp_save);
  /* zero length node to act as placeholder */
 }
 
 (*pre_rep_node_next)->prev = pre_rep_node;
 
 output_list_cleanup(&rep_start);
 
 return (*pre_rep_node_next);
}

static int output_wrap_left(player *p, int number)
{
 assert(p);
 
 if (p->term_width)
 {
  int tmp = (p->column + number) - p->term_width;

  if (tmp <= 0)
    return (0);
  else
    return (tmp); /* this is how many chars are over the wrap point */
 }
 else
   return (0);
}

static int output_back_char(output_node *out_node, int i)
{
 if (!out_node)
   return (-1);
 
 if (out_node->flags & OUTPUT_BUFFER_NON_PRINT)
   return (output_back_char(out_node->prev, i));
 
 if (out_node->length > i)
   return (out_node->start[out_node->length - i - 1]);
 else
   return (output_back_char(out_node->prev, i - out_node->length));
}

static int do_wrap_output(player *p, int flags, int the_char,
                          int offset, output_node *out_node)
{
 unsigned int i = 0;
 
 if (flags & SPLIT_ON_PUNCTUATION)
   while ((i <= p->word_wrap) && (isalnum(the_char) || (the_char == -1)))
     the_char = output_back_char(out_node, offset + i++);
 else    
   while ((i <= p->word_wrap) && ((the_char != ' ') || (the_char == -1)))
     the_char = output_back_char(out_node, offset + i++);

 
 if (i && (i <= p->word_wrap)) /* if zero put wrapping chars */
   return (offset + i);
 else
   /* splits word up */
   return (offset + 1);
}

static int get_indent(player *p, int flags, int went_through,
                      int wrapped_length, output_node *out_node)
{
 int offset = out_node->length - wrapped_length;
 
 p->column += went_through - wrapped_length;

 assert(offset < out_node->length);
 offset = do_wrap_output(p, flags, (unsigned char) out_node->start[offset],
                         wrapped_length, out_node);
 assert(offset);
 return (offset - 1 + (flags & WRAPPING_SPACES));
}

/* output_list_print_length with an expander for repeat nodes */
static int output_list_repeat_expand(player *p, output_node **head,
                                     output_node **ret_tail,
                                     int flags,
                                     int max_print_length)
{
 int print_length = 0;
 int print_indent = 0;
 int saved_column = p->column;
 output_node *tail = *head;

 BTRACE("output_list_repeat_expand");
  
 while (tail)
 {
  assert(tail->length <= OUTPUT_LINE_SZ);
  
  if (tail->length && !(tail->flags & OUTPUT_BUFFER_NON_PRINT))
  {
   int length = tail->length;
   int wrapped_length = 0;
   
   if (tail->flags & OUTPUT_REPEAT)
   {
    tail = output_node_repeat_expand(p, tail, tail->prev,
                                     tail->prev ? &(tail->prev->next): head,
                                     print_indent,
                                     max_print_length);
    assert(tail);
    continue;
   }

   if (length > max_print_length)
     length = max_print_length;

   print_length += length;
   DECREMENT_LENGTH(max_print_length, length);

   print_indent += length;     
   if (!(flags & OUTPUT_BUFFER_TMP) &&
       (wrapped_length = output_wrap_left(p, print_indent)))
   {
    print_indent = get_indent(p, flags, print_indent, wrapped_length, tail);
    p->column = 0;
    
    print_length += (flags & WRAPPING_SPACES);
    DECREMENT_LENGTH(max_print_length, (flags & WRAPPING_SPACES));
   }
   
   /* this is a little more complex so that $Line_fill(*)$Return will work */
   if (!max_print_length && (tail->length != length))
   {
    output_node *chopped_off = tail->next;
    
    tail->length = length;
    tail->next = NULL;
    
    if (chopped_off)
    {
     chopped_off->prev = NULL;
     output_list_cleanup(&chopped_off);
    }
    assert(!chopped_off);
    break;
   }
  }
  else
    if (tail->has_return)
    {
     print_indent = 0;
     p->column = 0;
    }
  
  if (!tail->next)
    break;
  
  tail = tail->next;
 }

 if (ret_tail)
   *ret_tail = tail;
 assert(!tail->next);
 
 p->column = saved_column;
 return (print_length);
}

int output_list_length(player *p, output_node *head, int flags, int max_length)
{
 output_node *tmp = head;
 int length = 0;
 int print_indent = 0;
 int saved_column = p->column;
 
 assert(MALLOC_VALID(head, sizeof(output_node), OUTPUT_NODE));
 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
 
 while (tmp && (max_length > 0))
 {
  int tmp_len = tmp->length;
  
  if (tmp->flags & OUTPUT_REPEAT)
    tmp_len = output_get_repeat_len(p, &tmp, print_indent, max_length);
  
  assert(tmp->length >= 0);
  if (max_length < tmp_len)
    tmp_len = max_length;
  
  DECREMENT_LENGTH(max_length, tmp_len);
  length += tmp_len;

  if (!(tmp->flags & OUTPUT_BUFFER_NON_PRINT))
  {
   int wrapped_length = 0;

   print_indent += tmp_len;
   if ((wrapped_length = output_wrap_left(p, print_indent)))
   {
    int added_length = (flags & WRAPPING_SPACES) + 2; /* for \r\n */
    if (added_length > max_length)
      added_length = max_length;
    
    print_indent = get_indent(p, flags, print_indent, wrapped_length, tmp);
    p->column = 0;
    
    DECREMENT_LENGTH(max_length, added_length);
    length += added_length;
   }
  }
  else if (tmp->has_return)
  {
   p->column = 0;
   print_indent = 0;
  }

  tmp = tmp->next;
 }
 assert(!(max_length && tmp));

 p->column = saved_column;
 return (length);
}

int output_list_print_length(player *p, output_node *head,
                             int flags, int max_print_length)
{
 output_node *tmp = head;
 int print_length = 0;
 int print_indent = 0;
 int saved_column = p->column;
 
 assert(MALLOC_VALID(head, sizeof(output_node), OUTPUT_NODE));
 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
 
 BTRACE("output_list_print_length");

 while (tmp && (max_print_length > 0))
 {
  if (!(tmp->flags & OUTPUT_BUFFER_NON_PRINT))
  {
   int wrapped_length = 0;
   int length = tmp->length;
  
   assert(length >= 0);

   if (tmp->flags & OUTPUT_REPEAT)
     length = output_get_repeat_len(p, &tmp, print_indent, max_print_length);
   else
     if (max_print_length < length)
       length = max_print_length;
  
   DECREMENT_LENGTH(max_print_length, length);
   print_length += length;
   print_indent += length;
   
   if ((wrapped_length = output_wrap_left(p, print_indent)))
   {
    int added_spaces = (flags & WRAPPING_SPACES);
    if (added_spaces > max_print_length)
      added_spaces = max_print_length;

    print_indent = get_indent(p, flags, print_indent, wrapped_length, tmp);
    p->column = 0;
    
    DECREMENT_LENGTH(max_print_length, added_spaces);
    print_length += added_spaces;
   }
  }
  else if (tmp->has_return)
  {
   p->column = 0;
   print_indent = 0;
  }
  
  tmp = tmp->next;
 }
 assert(!(max_print_length && tmp));

 p->column = saved_column;
 return (print_length);
}

int output_list_print_indent(player *p, output_node *head,
                             int flags, int max_print_indent)
{
 output_node *tmp = head;
 int print_indent = 0;
 int saved_column = p->column;
 
 assert(MALLOC_VALID(head, sizeof(output_node), OUTPUT_NODE));
 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
 
 while (tmp && (max_print_indent > 0))
 {
  int length = tmp->length;
  
  assert(length >= 0);
  
  if (!(tmp->flags & OUTPUT_BUFFER_NON_PRINT))
  {
   int wrapped_length = 0;
   
   if (tmp->flags & OUTPUT_REPEAT)
     length = output_get_repeat_len(p, &tmp, print_indent, max_print_indent);
   else
     if (max_print_indent < length)
       length = max_print_indent;
   
   DECREMENT_LENGTH(max_print_indent, length);
   print_indent += length;

   if ((wrapped_length = output_wrap_left(p, print_indent)))
   {
    int added_spaces = (flags & WRAPPING_SPACES);
    if (added_spaces > max_print_indent)
      added_spaces = max_print_indent;

    print_indent = get_indent(p, flags, print_indent, wrapped_length, tmp);
    p->column = 0;
    
    DECREMENT_LENGTH(max_print_indent, added_spaces);
   }
  }
  else if (tmp->has_return)
  {
   print_indent = 0;
   p->column = 0;
  }
  
  if (tmp->has_return)
    print_indent = 0;
  
  tmp = tmp->next;
 }
 assert(!tmp || !max_print_indent);

 p->column = saved_column;
 return (print_indent);
}

int output_list_lines(player *p, output_node *head, int flags,
                      int max_lines, int max_length)
{
 output_node *tmp = head;
 int lines = 0;
 int print_indent = 0;
 int saved_column = p->column;

 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
  
 while (tmp && (max_lines > 0) && (max_length > 0))
 {
  if (!(tmp->flags &  OUTPUT_BUFFER_NON_PRINT))
  {
   int wrapped_length = 0;
   int tmp_len = tmp->length;

   if (tmp_len > max_length)
     tmp_len = max_length;
   
   DECREMENT_LENGTH(max_length, tmp_len); /* maximum printable chars */
   print_indent += tmp_len;
   if ((wrapped_length = output_wrap_left(p, print_indent)))
   {
    int added_spaces = (flags & WRAPPING_SPACES);
    if (added_spaces > max_length)
      added_spaces = max_length;
    
    print_indent = get_indent(p, flags, print_indent, wrapped_length, tmp);
    p->column = 0;
    DECREMENT_LENGTH(max_length, added_spaces);
    lines++;
    DECREMENT_LENGTH(max_lines, 1);
   }   
  }
  else if (tmp->has_return)
  {
   p->column = 0;
   lines++;
   DECREMENT_LENGTH(max_lines, 1);
  }
  
  tmp = tmp->next;
 }
 assert(!(max_lines && tmp && max_length));

 p->column = saved_column;
 return (lines);
}

int output_list_into_buffer(player *p, output_node *head, char *output,
                            int max_length)
{
 output_node *tmp = head;
 int length = 0;
 
 assert(MALLOC_VALID(head, sizeof(output_node), OUTPUT_NODE) && output);

 while (tmp)
 {
  if (length >= max_length)
    break;
  
  if (tmp->length > 0)
  {
   int use_length = tmp->length;

   if (tmp->flags & OUTPUT_REPEAT)
   {
    int count = 0;
    output_node *tmp_end = tmp;
    
    use_length = output_get_repeat_len(p, &tmp_end, length, max_length);
    while ((count < use_length))
    {
     memcpy(output + length, tmp->start, tmp->length);
     count += tmp->length;
     length += tmp->length;
    }
    assert((count == use_length) && (tmp == tmp_end));
   }
   else
   {
    if ((length + tmp->length) > max_length)
      use_length = (max_length - length);
    
    memcpy(output + length, tmp->start, use_length);
    length += use_length;
   }
  }
  assert(tmp->length >= 0);
  
  tmp = tmp->next;
 }

 return (length);
}

output_node *output_list_merge(player *p, output_node **one,
                               output_node **two, int add_spaces,
                               int max_middle)
{
 tmp_output_list_storage tmp_save;
 int length_one = 0;
 int length_two = 0;
 int length_diff = 0;
 int tmp_count = 0;
 int int_max = INT_MAX;
 output_node *out_node = NULL;
 output_node *tmp = NULL;
 
 assert(one && MALLOC_VALID(*one, sizeof(output_node), OUTPUT_NODE));
 assert(two && MALLOC_VALID(*two, sizeof(output_node), OUTPUT_NODE));
 assert(p && !p->column);

 save_tmp_output_list(p, &tmp_save);
 length_one = output_list_repeat_expand(p, one, NULL, 0, INT_MAX);
 length_two = output_list_repeat_expand(p, two, NULL, 0, max_middle);
 if (add_spaces)
   length_two += 2;
 
 if (!((tmp = *one) && *two))
 {
  load_tmp_output_list(p, &tmp_save);
  return (NULL);
 }

 length_diff = (length_one - length_two);
 tmp_count = (length_diff >> 1);

 if (!(out_node = new_output_node(p, OUTPUT_BUFFER_TMP,
                                  tmp->timestamp, NULL, TRUE)))
 {
  load_tmp_output_list(p, &tmp_save);
  return (NULL);
 }
 
 if ((length_two > length_one) || !tmp_count)
 {
  output_list_print_copy(p, INT_MAX, *two, out_node, TRUE);
  out_node = output_list_grab(p);
  load_tmp_output_list(p, &tmp_save);
  return (out_node);
 }

 out_node = output_list_print_copy(p, tmp_count, *one, out_node, TRUE);
 if (add_spaces)
 {
  out_node = normal_char_output(p, out_node->flags, ' ', &int_max, out_node);
  out_node = output_list_print_copy(p, length_two - 2, *two, out_node, TRUE);
  out_node = normal_char_output(p, out_node->flags, ' ', &int_max, out_node);
 }
 else
   out_node = output_list_print_copy(p, length_two, *two, out_node, TRUE);
 
 assert(int_max == INT_MAX);
 
 tmp_count = length_one - length_two - tmp_count;
 
 output_list_print_copy(p, tmp_count, *one, out_node, FALSE);
 
 out_node = output_list_grab(p);

 /* NOTE: there can't be any wrapping or this wouldn't work 
 assert(output_list_print_length(p, out_node, 0, length_one + 1) ==
 length_one); */
 
 load_tmp_output_list(p, &tmp_save);
 return (out_node);
}

output_node *output_list_join(output_node **one, output_node **two)
{
 output_node *tmp = NULL;
 
 assert(one && MALLOC_VALID(*one, sizeof(output_node), OUTPUT_NODE));
 assert(two && MALLOC_VALID(*two, sizeof(output_node), OUTPUT_NODE));

 if (!(tmp = *one)) /* someone passed an empty output_list to link in */
   return (*two);

 while (tmp->next)
   tmp = tmp->next;
 
 if (*two)
 {
  tmp->next = *two;
  (*two)->prev = tmp;
  tmp->full_buffer = TRUE;
 }
 
 *two = NULL;
 
 return (*one);
}

static char output_list_get_char(output_node *one, int count)
{
 int node_count = 0;
 assert(one && MALLOC_VALID(one, sizeof(output_node), OUTPUT_NODE));

 if (count < 0)
   count = 0;
 
 while (one && count)
 {
  if (!(one->flags & OUTPUT_BUFFER_NON_PRINT))
  {
   node_count = 0;
   
   while ((node_count < one->length) && --count)
     ++node_count;     
  }

  if (count)
    one = one->next;
 }

 if (one)
   return (one->start[node_count]);
 else
   return (0);
}

long output_list_toint(output_node *one)
{
 int number = 0;
 int isnegative = FALSE;
 
 assert(one && MALLOC_VALID(one, sizeof(output_node), OUTPUT_NODE));

 while (one)
 {
  if (!(one->flags & OUTPUT_BUFFER_NON_PRINT))
  {
   int count = 0;

   while ((count < one->length) && (one->start[count] == ' '))
     ++count;

   if (!number && (count < one->length) && (one->start[count] == '-'))
     isnegative = TRUE;
     
   while ((count < one->length) && isdigit((unsigned char) one->start[count]))
   {
    number = (number * 10) + TONUMB(one->start[count]);
    ++count;
   }
   assert((count == one->length) ||
          !isdigit((unsigned char) one->start[count]));
   if (count < one->length)
     return (isnegative ? -number : number);
  }
  
  one = one->next;
 }

 return (isnegative ? -number : number);
}

time_t output_list_totime(output_node *one)
{
 struct tm *the_time = gmtime(&now);
 int something_there = FALSE;
 int number = 0;
 
 assert(one && MALLOC_VALID(one, sizeof(output_node), OUTPUT_NODE));

 while (one)
 {
  if (!(one->flags & OUTPUT_BUFFER_NON_PRINT))
  {
   int count = 0;

   while (count < one->length)
   {
    if (!something_there)
      while ((count < one->length) && (one->start[count] == ' '))
        ++count;

    while ((count < one->length) && isdigit((unsigned char) one->start[count]))
    {
     something_there = TRUE;
     number = (number * 10) + TONUMB(one->start[count]);
     ++count;
    }
    assert((count == one->length) ||
           !isdigit((unsigned char) one->start[count]));
    
    if (count >= one->length)
      continue;
    
    if (!something_there)
      return (mktime(the_time));
    
    switch (one->start[count])
    {
     case '/': /* year */
       the_time->tm_year = number - 1900; /* years since 1900 */
       break;
     case '.': /* month */
       the_time->tm_mon = number - 1; /* goes from 0 .. 11 */
       break;
     case '-': /* day of month */
       the_time->tm_mday = number;
       break;
     case ':': /* hour */
       the_time->tm_hour = number;
       break;
     case '+': /* minute */
       the_time->tm_min = number;
       break;
     case ' ': /* seconds -- also done below... */
       the_time->tm_sec = number;
       break;
       
     default:
       return (-1);
    }
    number = 0;
    something_there = FALSE;
    count++;
   }
  }
  
  one = one->next;
 }
 
 if (something_there) /* process_output nodes aren't null terminated */
   the_time->tm_sec = number;

 return (mktime(the_time));
}

int output_list_cmp(output_node *one, output_node *two)
{
 int cmp_save = 0;
 int one_offset = 0;
 int two_offset = 0;
 
 assert(one && MALLOC_VALID(one, sizeof(output_node), OUTPUT_NODE));
 assert(two && MALLOC_VALID(two, sizeof(output_node), OUTPUT_NODE));
  
 while (one && two && !cmp_save)
 {
  if (!(one->flags & OUTPUT_BUFFER_NON_PRINT) && (one_offset < one->length))
    if (!(two->flags & OUTPUT_BUFFER_NON_PRINT) && (two_offset < two->length))
      while ((one_offset < one->length) && (two_offset < two->length) &&
             !cmp_save)
        cmp_save = (one->start[one_offset++] - two->start[two_offset++]);
    else
    {
     two = two->next;
     two_offset = 0;
    }
  else
  {
   one = one->next;
   one_offset = 0;
  }
 }
 
 if (!cmp_save)
 {
  if (!(one || two))
  {
   if (one)
     return (1);
   else
     return (-1);
  }
 }
 
 return (cmp_save);
}

int output_list_case_cmp(output_node *one, output_node *two)
{
 int cmp_save = 0;
 int one_offset = 0;
 int two_offset = 0;
 
 assert(one && MALLOC_VALID(one, sizeof(output_node), OUTPUT_NODE));
 assert(two && MALLOC_VALID(two, sizeof(output_node), OUTPUT_NODE));
  
 while (one && two && !cmp_save)
 {
  if (!(one->flags & OUTPUT_BUFFER_NON_PRINT) && (one_offset < one->length))
    if (!(two->flags & OUTPUT_BUFFER_NON_PRINT) && (two_offset < two->length))
      while ((one_offset < one->length) && (two_offset < two->length) &&
             !cmp_save)
        cmp_save = (tolower((unsigned char) one->start[one_offset++]) -
                    tolower((unsigned char) two->start[two_offset++]));
    else
    { 
     two = two->next;
     two_offset = 0;
    }
  else
  {
   one = one->next;
   one_offset = 0;   
  }
 }

 if (!cmp_save)
 {
  if (!(one || two))
  {
   if (one)
     return (1);
   else
     return (-1);
  }
 }
 
 return (cmp_save);
}

int output_list_number_cmp(output_node *one, output_node *two)
{
 long number_one = 0;
 long number_two = 0;
 
 assert(one && MALLOC_VALID(one, sizeof(output_node), OUTPUT_NODE));
 assert(two && MALLOC_VALID(two, sizeof(output_node), OUTPUT_NODE));
  
 number_one = output_list_toint(one);
 number_two = output_list_toint(two);

 if (number_one > number_two)
   return (1);
 else
   if (number_one < number_two)
     return (-1);
   else
     return (0);
}

static void output_list_rot13(output_node *one)
{
 assert(one && MALLOC_VALID(one, sizeof(output_node), OUTPUT_NODE));

 while (one)
 {
  if (!(one->flags & OUTPUT_BUFFER_NON_PRINT))
  {
   int count = 0;
   
   while (count < one->length)
   {
    if (isalpha((unsigned char) one->start[count]))
    {
     if (isupper((unsigned char) one->start[count]))
     {
      int offset = ALPHA_UPPER_OFFSET(one->start[count]);
      offset += 13;
      offset %= 26;
      one->start[count] = ALPHA_UPPER_CHAR(offset);
      }
     else
     {
      int offset = ALPHA_LOWER_OFFSET(one->start[count]);
      offset += 13;
      offset %= 26;
      one->start[count] = ALPHA_LOWER_CHAR(offset);
     }
    }
   
    ++count;
   }
  }
  
  one = one->next;
 }
}

static output_node *output_list_linkin_after(player *p, int flags,
                                             output_node **head,
                                             output_node **link_after,
                                             output_node *link_prev,
                                             int max_print_length)
{
 output_node *tail = NULL;
 int const_int_max = INT_MAX; /* for output_return */
 int linkin_print_length = 0;
 
 assert(head && MALLOC_VALID(*head, sizeof(output_node), OUTPUT_NODE));
 assert(link_after && (max_print_length >= 0));
 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
 assert(!*link_after || ((*link_after)->prev == link_prev));
 assert(!link_prev || (*link_after == link_prev->next));
 
 BTRACE("output_list_linkin_after");
 
 linkin_print_length = output_list_repeat_expand(p, head, &tail,
                                                 flags, max_print_length);
 if (linkin_print_length >= max_print_length)
 {
  assert(linkin_print_length == max_print_length);
  max_print_length = 0;
 }
 else
   DECREMENT_LENGTH(max_print_length, linkin_print_length);
 
 /* do wrapping for linkin */
 if (!(flags & OUTPUT_BUFFER_TMP))
 {
  output_node *tmp = *head;
  int went_through = 0;
  int wrapped_length = 0;
   
  while (tmp)
  {
   assert(tmp->length <= OUTPUT_LINE_SZ);
   
   if (!(tmp->flags & OUTPUT_BUFFER_NON_PRINT) && tmp->length)
   {
    int went_through_tmp = went_through + tmp->length;
    
    assert(!(tmp->flags & OUTPUT_REPEAT));
    if ((wrapped_length = output_wrap_left(p, went_through_tmp)))
    {
     int offset = tmp->length - wrapped_length;

     p->column += went_through + offset;
     linkin_print_length -= went_through_tmp;

     assert(offset < tmp->length);
     offset = do_wrap_output(p, flags, (unsigned char) tmp->start[offset],
                             wrapped_length, tmp);

     assert(offset);
     linkin_print_length += (offset - 1);

     tmp = output_return(p, flags, offset, &const_int_max, tmp);
     went_through = p->column;
     p->column = 0;
     
     tmp->full_buffer = TRUE;
    }
    else
      went_through = went_through_tmp;
   }
   else
     if (tmp->has_return)
     {
      linkin_print_length -= went_through;
      p->column = went_through = 0;
     }

   tmp = tmp->next;
  }
  p->column += went_through;
  linkin_print_length -= went_through;

  assert(!linkin_print_length);

  while (tail->next)
    tail = tail->next; /* could have wrapped at the end */
 } /* end of wrapping, while loop */

 /* linkin the list */
 assert(!tail->next);

 if (((*head)->prev = link_prev))
   link_prev->full_buffer = TRUE;

 if ((tail->next = *link_after))
 {
  assert(link_prev == tail->next->prev);
  tail->next->prev = tail;
 }
 
 *link_after = *head;

#ifndef NDEBUG
 {
  output_node *tmp = p->output_start;

  while (tmp)
  {
   assert(!tmp->next || (tmp->next->prev == tmp));
   assert((!tmp->prev && (p->output_start == tmp)) ||
          (tmp->prev->next == tmp));
   tmp = tmp->next;
  }

  tmp = p->output_buffer_tmp;
  while (tmp)
  {
   assert(!tmp->next || (tmp->next->prev == tmp));
   assert((!tmp->prev && (p->output_buffer_tmp == tmp)) ||
          (tmp->prev->next == tmp));
   tmp = tmp->next;
  }
 }
#endif
 
 *head = NULL;

 return (tail);
}

void output_list_linkin(player *p, int flags, output_node **head, int length)
{
 output_node **tmp = NULL;

 assert(p && head &&
        MALLOC_VALID(*head, sizeof(output_node), OUTPUT_NODE));
 assert(!(flags & OUTPUT_BUFFER_TMP));

 /* FIXME: assert(player_list_io_find(p)); */
 
 if (flags & OUTPUT_BUFFER_TMP)
   tmp = &p->output_buffer_tmp;
 else
   tmp = &p->output_start;

 if (*tmp)
 {
  output_node *out_node = NULL;
  
  while (*tmp && (difftime((*head)->timestamp, (*tmp)->timestamp) >= 0))
    /* find the first node in a given time period */
    tmp = &(*tmp)->next;

   /* ANSI magic */
  out_node = (output_node *) (((char *)tmp) - offsetof(output_node, next));
  output_list_linkin_after(p, flags, head, tmp, out_node, length);
 }
 else
   output_list_linkin_after(p, flags, head, tmp, NULL, length);
}

void output_list_cleanup(output_node **head)
{
 output_node *tmp = NULL;

 assert(head);
 
 if (!*head)
   return;
 
 assert(MALLOC_VALID(*head, sizeof(output_node), OUTPUT_NODE));

 tmp = *head;
 while (tmp)
 {
  output_node *tmp_next = tmp->next;
  
  destroy_output_node(head, tmp);
  tmp = tmp_next;
 }

 *head = NULL;
}

static int output_space_left(output_node *out_node, int length)
{
 assert(out_node);
 
 if ((out_node->length + length) <= OUTPUT_LINE_SZ)
   return (TRUE);
 else
   return (FALSE);
}

static output_node *first_output_node(player *p, int flags, time_t timestamp,
				      output_node *from)
{
 output_node *tmp = NULL;
 output_node *works = NULL;

 assert(p);

 if (from)
   tmp = from;
 else
   if (flags & OUTPUT_BUFFER_TMP)
     tmp = p->output_buffer_tmp;
   else
     tmp = p->output_start;

 if (tmp)
   while (tmp->next && (difftime(timestamp, tmp->timestamp) > 0))
     /* find the first node in a given time period */
     tmp = tmp->next;

 /* find the last node from a time period */
 while (tmp && (difftime(timestamp, tmp->timestamp) >= 0))
 {
  if (!OUTPUT_NODE_NON_WRITABLE(tmp))
    works = tmp;
  
  tmp = tmp->next;
 }
 
 if (works)
 {
  works->timestamp = timestamp;
  return (works);
 }
 else
   return (new_output_node(p, flags, timestamp, from, TRUE));
}

output_node *output_raw(player *p, int flags, const char *output,
                        int length, output_node *out_node)
{
 assert(length > 0);
 
 if (!out_node)
   return (NULL);
 
 if (!OUTPUT_NODE_EQUIV(flags, out_node->flags) ||
     ((out_node->flags & OUTPUT_REPEAT) && out_node->end_repeat_list))
 { /* if changing sets of flags, Ie. from/to non printable chars */
  if (out_node->length)
  {
   out_node->full_buffer = TRUE;
   if ((out_node->flags & OUTPUT_REPEAT) && !(flags & OUTPUT_REPEAT))
     out_node->end_repeat_list = TRUE;
   if (!(out_node = new_output_node(p, flags, out_node->timestamp,
                                    out_node, FALSE)))
     return (NULL);
  }
  else
  { /* there isn't anything in the node yet */
   assert(!out_node->end_repeat_list); /* this can't be the reason */
   out_node->flags &= (OUTPUT_BUFFER_NON_PRINT | OUTPUT_REPEAT |OUTPUT_BYTES);
   out_node->flags |= (flags & (OUTPUT_BUFFER_NON_PRINT | OUTPUT_REPEAT |
                                OUTPUT_BYTES));
  }
 }
 
 if (flags & OUTPUT_PRIORITY)
   p->output_has_priority = TRUE;
 if (!(flags & OUTPUT_BYTES))
   output_maybe_delete_line(p);
 
 while (!output_space_left(out_node, length))
 {
  while (output_space_left(out_node, 1))
  {
   *(out_node->start + out_node->length++) = *output++;
   --length;
  }
  
  out_node->full_buffer = TRUE;
  
  if (!(out_node = new_output_node(p, flags, out_node->timestamp,
                                   out_node, FALSE)))
    return (NULL);
 }  
 
 memcpy(out_node->start + out_node->length, output, length);
 out_node->length += length;
 
 return (out_node);
}

static output_node *output_spaces(player *p, int flags, int number,
                                  int *length, output_node *out_node)
{
 while (number-- > 0)
   out_node = normal_char_output(p, flags, ' ', length, out_node);
 return (out_node);
}

static output_node *output_return(player *p, int flags, int from_end,
				  int *length, output_node *out_node)
{
 output_node *new_node = NULL;
 int wrapping_spaces = (flags & WRAPPING_SPACES);
 
 assert(p && length && (*length >= 0));
 
 if (!out_node)
   return (NULL);

 if (*length < wrapping_spaces)
   wrapping_spaces = *length;
   
 if (from_end > 0)
 {
  --from_end;

  assert(!(flags & OUTPUT_BUFFER_TMP));
  p->column = from_end;

  if (from_end > 0)  
  {
   if (from_end > out_node->length)
   { /* we are wrapping chars as well */
    output_node *old_out_node = out_node; /* this is the new output node */
    
    while (out_node->prev && ((out_node->flags & OUTPUT_BUFFER_NON_PRINT) ||
                              (out_node->length < from_end)))
    {
     assert(out_node->length <= OUTPUT_LINE_SZ);
     
     if (!(out_node->flags & OUTPUT_BUFFER_NON_PRINT))
       from_end -= out_node->length;
     
     out_node = out_node->prev;
    }
    assert((from_end <= out_node->length) &&
           !(out_node->flags & OUTPUT_BUFFER_NON_PRINT));
    
    new_node = new_output_node(p, flags, out_node->timestamp, out_node, FALSE);
    assert(!(new_node->flags & OUTPUT_BUFFER_NON_PRINT) &&
           !(out_node->flags & OUTPUT_BUFFER_NON_PRINT));

    if (wrapping_spaces)
      new_node = output_spaces(p, flags, wrapping_spaces, length, new_node);

    out_node->length -= from_end;
    new_node = output_raw(p, new_node->flags,
                          out_node->start + out_node->length,
                          from_end, new_node);
    new_node->full_buffer = TRUE;
    
    new_node = old_out_node; /* put new_node back at the end of the list */
   }
   else
   { /* wrap the chars to the new node */
    assert(from_end);
    new_node = new_output_node(p, flags, out_node->timestamp, out_node, FALSE);
    if (wrapping_spaces)
      new_node = output_spaces(p, flags, wrapping_spaces, length, new_node);
    out_node->length -= from_end;
    new_node = output_raw(p, new_node->flags,
                          out_node->start + out_node->length,
                          from_end, new_node);
   }
  }
  else
  {
   new_node = new_output_node(p, flags, out_node->timestamp, out_node, FALSE);
   if (wrapping_spaces)
     new_node = output_spaces(p, flags, wrapping_spaces, length, new_node);
  }
 }
 else
 {
  new_node = new_output_node(p, flags, out_node->timestamp, out_node, FALSE);

  if (!(flags & OUTPUT_BUFFER_TMP))
    p->column = 0;
  
  if (p->flag_just_normal_hilight || (flags & (RAW_SPECIALS | HILIGHT)))
    out_node = colour_write(p, flags, EVERYTHING_OFF, out_node);
 }

 if (!out_node)
   return (0);

 out_node = output_raw(p, flags | OUTPUT_BUFFER_NON_PRINT,
                       "\r\n", 2, out_node);
 out_node->has_return = TRUE;
 
 return (new_node);
}

static int do_divide(long *num, unsigned int base)
{
 int char_offset = (((unsigned) *num) % base);
 
 *num = (((unsigned) *num) / base);
 
 return (char_offset);
}

/* used to write out a number */
static output_node *output_number(player *p,
                                  int *length, output_node *out_node,
				  int output_flags, long num, int size,
				  int precision, int flags)
{
 char sign = 0;
 char tmp[BUF_NUM_TYPE_SZ(long)];
 const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
 int base = (flags & BASE_MASK);
 unsigned int i = 0;

 assert(p && length && (*length >= 0));

 if (*length <= 0)
   return (out_node);
 
 if (flags & LEFT)
   flags &= ~ZEROPAD;
 
 if (flags & LARGE)
   digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
 
 switch (base)
 {
  case 10:
  case 16:
  case 8:
    break;
 
  default:
    return (normal_char_output(p, output_flags, '0', length, out_node));
 }
 
 
 if (flags & SIGN)
 {
  if (num < 0)
  {
   sign = '-';
   num = -num;
  }
  else
    if (flags & PLUS)
      sign = '+';
    else
      if (flags & SPACE)
	sign = ' ';
      else
	size++;
  size--;
 }

 if (flags & SPECIAL)
 {
  if (base == 16)
    size -= 2;
  else
    if (base == 8)
      --size;
 }
 
 while ((num >= 0) && (i < (sizeof(tmp) - 1)))
 {
  /* this writes the number in, backwards */
  tmp[i++] = digits[do_divide(&num, base)];
  if (!num)
    num = -1;
 }

 assert(i < (sizeof(tmp) - 1)); /* debug check */
 
 if ((signed)i > precision)
   precision = i; /* upgrade the precision if it is a long number */
 
 size -= precision;
 
 if (!(flags & (ZEROPAD | LEFT)))
   if (size > 0)
   { /* right justify number */
    out_node = output_spaces(p, output_flags, size, length, out_node);
    size = 0;
   }
 
 if (sign)
   out_node = normal_char_output(p, output_flags, sign, length, out_node);
 
 if (flags & SPECIAL)
   switch (base)
   {
    case 16:
      out_node = normal_char_output(p, output_flags, '0', length, out_node);
      out_node = normal_char_output(p, output_flags, digits[33], /* x */
                                    length, out_node);
      break;
     
    case 8:
      out_node = normal_char_output(p, output_flags, '0', length, out_node);
    default:
      break;
   }

 if (!(flags & LEFT))
   while (size-- > 0) /* right justify with zero's */
     out_node = normal_char_output(p, output_flags, '0', length, out_node);
 
 while (precision-- > (signed)i) /* make number the desired length */
   out_node = normal_char_output(p, output_flags, '0', length, out_node);
 
 while (i-- > 0) /* output number */
   out_node = normal_char_output(p, output_flags, tmp[i], length, out_node);

 if (size > 0) /* left justify number */
   out_node = output_spaces(p, output_flags, size, length, out_node);

 return (out_node);
}

static output_node *normal_char_output(player *p, int flags, char input,
                                       int *length, output_node *out_node)
{
 assert(p && length && (*length >= 0));
 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
  
 if (*length <= 0)
   return (out_node);

 if (!(flags & OUTPUT_BUFFER_TMP))
 { /* NOTE: no wrapping is done for tmp buffers, it's done on linkin */
  if (output_wrap_left(p, 1))
  {
   int i = do_wrap_output(p, flags, (unsigned char) input, 0, out_node);

   assert(output_wrap_left(p, 1) == 1);
   out_node = output_return(p, flags, i, length, out_node);
                                    
   if (*length <= 0)
     return (out_node); /* we wrapped and don't have room any more */
  }
  
  p->column++;
 }
 
 DECREMENT_LENGTH(*length, 1);
 return (output_raw(p,  flags & ~OUTPUT_BUFFER_NON_PRINT,
                    &input, 1, out_node));
}

/* parse colours, hilights, underlines and flashing */
static output_node *special_char_output(player_tree_node *from, player *p,
					int flags,
					const char **input,
					int *length, output_node *out_node)
{
 int colour = TRUE;
 int specials = TRUE;

 BTRACE("special_char_output");
 
 if (p->flag_just_normal_hilight || (flags & EAT_ALL_SPECIALS))
   colour = specials = FALSE;
 else
   if (from && (p->saved != from))
   {
    if (p->flag_no_colour_from_others)
      colour = FALSE;
    if (p->flag_no_specials_from_others)
      specials = FALSE;
   }
 
 ++*input;

 switch(**input)
 {
  default:
    --*input; /* don't skip the second char */
  case '^':
    out_node = normal_char_output(p, flags, '^', length, out_node);
    break;
  
  case '0':
    if (colour)
      out_node = colour_write(p, flags, FOREGROUND_OFF, out_node);
    break;
  
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8': /* Change the colour of the text */
    if (colour)
      out_node = colour_write(p, flags, TONUMB(**input), out_node);
    break;
  
  case '9':
    ++*input;
    if (isdigit((unsigned char) **input))
      if (**input == '0')
      {
       if (colour)
         out_node = colour_write(p, flags, BACKGROUND_OFF, out_node);
      }
      else
      { /* Change the colour of the background for the text */
       if (colour)
         out_node = colour_write(p, flags, (TONUMB(**input) << 4), out_node);
      }
    else
    {
     out_node = normal_char_output(p, flags, '^', length, out_node);
     out_node = normal_char_output(p, flags, '9', length, out_node);
     --*input;
    }
    break;
  
  case 'L': /* flash on and off */
  case 'F': /* flash on and off */
  case 'K': /* flash on and off */
    if (specials)
      out_node = colour_write(p, flags, FLASHING_ON, out_node);
    break;
  case 'l': /* stop -- flash on and off */
  case 'f': /* stop -- flash on and off */
  case 'k': /* stop -- flash on and off */
    if (specials)
      out_node = colour_write(p, flags, FLASHING_OFF, out_node);
    break;
  
  case 'B': /* bold, hilight */
  case 'H': /* bold, hilight */
    if (specials)
      out_node = colour_write(p, flags, BOLD_ON, out_node);    
    break;
  case 'b': /* stop -- bold, hilight */
  case 'h': /* stop -- bold, hilight */
    if (specials)
      out_node = colour_write(p, flags, BOLD_OFF, out_node);    
    break;

  case 'D':
    if (specials)
      out_node = colour_write(p, flags, DIM_ON, out_node);
    break;
  case 'd':
    if (specials)
      out_node = colour_write(p, flags, DIM_OFF, out_node);
    break;
    
  case 'U': /* underline */
    if (specials)
      out_node = colour_write(p, flags, UNDERLINE_ON, out_node);
    break;
  case 'u': /* underline */
    if (specials)
      out_node = colour_write(p, flags, UNDERLINE_OFF, out_node);
    break;

  case 'I': /* inverse */
    if (specials)
      out_node = colour_write(p, flags, INVERSE_ON, out_node);
    break;
  case 'i': /* inverse */
    if (specials)
      out_node = colour_write(p, flags, INVERSE_OFF, out_node);
    break;
  
  case 'S':
    if (specials || colour)
      out_node = colour_write(p, flags, SAVE_OUTPUT_TYPE, out_node);
    break;
  case 's':
    if (specials || colour)
      out_node = colour_write(p, flags, RESTORE_OUTPUT_TYPE, out_node);
    break;
  
  case 'R': /* restore colour, with a load... Ie. ^R00 */
    ++*input;
    if (!(flags & EAT_ALL_SPECIALS) &&
        (isdigit((unsigned char) **input) &&
         isdigit((unsigned char) *(*input + 1))))
      out_node = colour_load(p, flags, 
                             (TONUMB(**input) * 10) + TONUMB(*(*input + 1)), 
                             out_node);

    ++*input;
    break;
   
  case 'r': /* restore colour */
    if (!(flags & EAT_ALL_SPECIALS))
      out_node = colour_write(p, flags, RESET_OUTPUT, out_node);  
    break;
    /* turn off everything */
  case 'N':
    if (specials || colour)
      out_node = colour_write(p, flags, CLEAR_OUTPUT, out_node);
    break;
  case 'n':
    if (specials || colour)
      out_node = colour_write(p, flags, EVERYTHING_OFF, out_node);
    break;
 }

 ++*input;
 
 return (out_node);
}

static int get_output_flags(const char **input)
{
 int flags = 0;

 assert(input && *input);
 
 while (TRUE)
 {
  switch (**input)
  {
   case '-':
     flags |= LEFT;
     break;
    
   case '+':
     flags |= PLUS;
     break;
    
   case ' ':
     flags |= SPACE;
     break;
    
   case '#':
     flags |= SPECIAL;
     break;
    
   case '0':
     flags |= ZEROPAD;
     break;
    
   default:
     return (flags);
  }
  ++*input;
 }

 assert(FALSE);
 return (flags);
}

/* output formatting... insperation from vsprintf */
static int get_output_information(const char **input, int *size,
				  int *object_size)
{
 const char *tmp = 0;
 int flags = 0;
 int tmp_size = 0; /* used before we know formatting is correct */
 int tmp_object_size = 0; /* used before we know formatting is correct */
 int base = 10;

 tmp = *input;

 /* done here so that if they don't do the rest correctly then it
    doesn't affect anything */
 ++tmp;

 if (*tmp != '(') /* the format has to be correct */
   return (base);

 ++tmp;

 flags |= get_output_flags(&tmp);
 
 if (isdigit((unsigned char) *tmp))
   tmp_size = skip_atoi(&tmp);
 else
   tmp_size = *size;

 if ((*tmp == '.') && isdigit((unsigned char) *(++tmp)))
   tmp_object_size = skip_atoi(&tmp);
 else
   tmp_object_size = *object_size;

 switch (*tmp)
 {
  case 'o':
    base = 8;
    ++tmp;
    break;
    
  case 'X':
    flags |= LARGE;
  case 'x':
    base = 16;
    ++tmp;
    break;
    
  case 'B':
    ++tmp;
    if (isdigit((unsigned char) *tmp))
    {
     int tmp_base = skip_atoi(&tmp);
     if (tmp_base > 16)
       base = 16;
     else
       if (base > 1)
         base = tmp_base;
    }
    break;
    
  default:
    break;
 }

 if (*tmp == ')')
 {
  *input = tmp + 1;

  if ((tmp_size > 1200) /* make sure they can't spam 4.5 billion spaces */
      && !(tmp_size == *size))
    *size = 1200;
  else
    *size = tmp_size;
  
  if ((tmp_object_size > 1200)
      /* make sure they can't spam 4.5 billion zero's */
      && !(tmp_object_size == *object_size))
    *object_size = 1200;
  else
    *object_size = tmp_object_size;
 }
 else
   return (10); /* default base | default flags */
 
 return (base | flags);
}

static output_node *out_node_after(player *p, twinkle_info *info,
                                   int output_flags, output_node *buffer,
                                   int *output_length, int size,
                                   int string_size, int flags,
                                   output_node *out_node)
{
 int ignore_length = INT_MAX;
 int len = 0;
 
 if (output_flags & OUTPUT_BUFFER_TMP)
   len = output_list_print_length(p, buffer, output_flags, string_size);
 else
   len = output_list_repeat_expand(p, &buffer, NULL, output_flags,
                                   string_size);

 assert(len <= string_size);

 info->used_twinkles = TRUE;
 DECREMENT_LENGTH(*output_length, len);

 if (!(flags & LEFT) &&
     (size > len))
 {
  out_node = output_spaces(p, output_flags, size - len,
                           &ignore_length, out_node);
  size = len;
 }

 out_node = output_list_linkin_after(p, output_flags, &buffer,
                                     &out_node->next, out_node, len);
 assert(!buffer);
 
 if (size > len)
   out_node = output_spaces(p, output_flags, size - len,
                            &ignore_length, out_node);

 return (out_node);
}

static void fntell_player(player_tree_node *, player *, twinkle_info *, int, 
                          time_t, int, int, const char *);

static output_node *string_variables(player_tree_node *from, player *p, 
                                     twinkle_info *info, int output_flags,
                                     time_t timestamp,
                                     int input_length,
                                     const char *output,
                                     const char **input,
                                     int *output_length, 
                                     output_node *out_node)
{
 tmp_output_list_storage tmp_save;
 output_node *buffer = NULL;
 int size = 0;
 int string_size = INT_MAX;
 int flags = 0;
 
 assert(p && input && *input && output &&
        output_length && (*output_length >= 0));
 assert(!(NOT_USED_FLAGS_TELL_PLAYER & output_flags));
 
 BTRACE("string_variables");
 
 if (**input == '-') /* means they want to do output info on the string */
   flags = get_output_information(input, &size, &string_size);

 assert(string_size >= 0);
 
 if (string_size > *output_length)
   string_size = *output_length;

 if (!input_length)
   return (out_node);
 
 save_tmp_output_list(p, &tmp_save);
 fntell_player(ALL_T(from, p, info, output_flags | OUTPUT_BUFFER_TMP,
                     timestamp), input_length, string_size, output);
 buffer = output_list_grab(p);
 load_tmp_output_list(p, &tmp_save);

 return (out_node_after(p, info, output_flags, buffer, output_length,
                        size, string_size, flags, out_node));
}

static output_node *out_node_variables(player *p, twinkle_info *info,
                                       int output_flags, output_node *buffer,
                                       const char **input,
                                       int *output_length, 
                                       output_node *out_node)
{
 int size = 0;
 int string_size = INT_MAX;
 int flags = 0;
 
 if (**input == '-') /* means they want to do output info on the out_node */
   flags = get_output_information(input, &size, &string_size);
 
 assert(string_size >= 0);
 
 if (string_size > *output_length)
   string_size = *output_length;

 return (out_node_after(p, info, output_flags, buffer, output_length,
                        size, string_size, flags, out_node));
}

static output_node *number_variables(player_tree_node *from, player *p,
                                     twinkle_info *info, 
				     int output_flags, time_t timestamp,
                                     long number, const char **input,
                                     int *output_length, 
				     output_node *out_node)
{
 int size = 0;
 int numb_size = 0;
 int flags = SIGN; /* make sure -/+ prefix's go on */

 assert(p && input && *input && output_length && *output_length);
 
 BTRACE("number_variables");
 assert(!(NOT_USED_FLAGS_TELL_PLAYER & output_flags));
 
 info->used_twinkles = TRUE;
 if (**input == '-') /* means they want to do output info on the number */
 {
  if (!BEG_CONST_STRCMP("-Tostr", *input))
  {
   char buffer[256];
   int use_cap = FALSE;

   SKIP_MATCHED_TO(input, "-Tostr");

   if (!BEG_CONST_STRCMP("_cap", *input))
   {
    SKIP_MATCHED_TO(input, "_cap");
    use_cap = TRUE;
   }
   
   word_number_def(buffer, 256, number, use_cap);
   return (string_variables(from, p, info, output_flags, timestamp, INT_MAX,
                            buffer, input, output_length, out_node));
  }
  
  flags |= get_output_information(input, &size, &numb_size);
 }
 else
   flags |= 10; /* add the default base for the number */
  
 return (output_number(p, output_length, out_node, output_flags,
		       number, size, numb_size, flags));
}

static const char *tag_skip_prefix(const char *input)
{
 input += strspn(input, " \n\t");

 if (*input != '(')
   return (NULL);

 ++input;

 return (input);
}

static int tag_get_bracket(const char *input)
{
 const char *const beg_input = input;
 int open_brackets = 1; /* for the first one we are expecting */
 
 assert(input);

 if (!(input = tag_skip_prefix(input)))
   return (0);
 --input;
 
 while (*input)
 {
  ++input;
  
  if (!open_brackets)
    return (input - beg_input);
  
  switch (*input)
  {
   case '\\':
     if ((input[1] == '(') || (input[1] == ')'))
       ++input;
     else if (input[1] == '\\')
       ++input;
     break;

   case '(': /* this shouldn't be able to overflow, on user input anyway */
     ++open_brackets;
     break;
   case ')':
     --open_brackets;
     break;
    
   default:
     break;
  }
 }

 return (0);
}

static int tag_get_name(const char **tag_names, const char **passed_input,
                        int in_length)
{ /* NOTE: it might be worth "compiling" the tags into an array as this
   * function would dramaticaly speed up, and there are a few functions
   * which do multiple lookups now */
 const char **cur_name = tag_names;
 const char *real_local_input = NULL;
 const char **input = &real_local_input;
 int skip = 0;  

 assert(passed_input && *passed_input && in_length && cur_name);

 real_local_input = *passed_input;
 skip = strspn(*input, ", \n\t");
 
 if (skip < in_length)
 {
  *input += skip;
  in_length -= skip;
 }
 else
 {
  assert(FALSE);
  in_length = 0;
  return (-1);
 }
 
 while (**input && (in_length > 0))
 {
  const char *input_save = *input;
  
  while (*cur_name)
  {
   int tmp = beg_strcmp(*cur_name, *input);
   
   if (!tmp)
   {/* found match to one of the tags */
    int tag_int = 0;
    
    input_save = *input;
    *input += strlen(*cur_name); /* FIXME: could speed this up */

    if ((tag_int = tag_get_bracket(*input)))
    {
     const char *the_str = tag_skip_prefix(*input);
     assert(the_str);
     tag_int -= (the_str - *input) + 1;
     *passed_input = the_str;
     return (tag_int);
    }
    else
      *input = input_save;
   }
   else
     if (tmp > 0)
       /* skip to next tag */
       break;
     
   ++cur_name;
  }
  
  if ((*input = C_strnchr(*input, '(', in_length - (*input - input_save))))
    /* skip the non-matching tag */
  {
   int tmp_len = tag_get_bracket(*input);
   *input += tmp_len;
   in_length -= (*input - input_save);
   assert(in_length >= 0);
   assert(tmp_len);
  }
  else
    return (-1);
  
  cur_name = tag_names;

  skip = strspn(*input, ", \n\t");
  
  if (skip < in_length)
  {
   *input += skip;
   in_length -= skip;
  }
  else
    in_length = 0;
 }

 assert(!*input || !in_length);
  
 return (-1);
}

static int tag_get_def_name(const char **tag_names, const char **passed_input,
                            int in_length)
{
 int ret = tag_get_name(tag_names, passed_input, in_length);

 if (ret == -1)
 {
  *passed_input = "";
  return (0);
 }

 return (ret);
}

/* return (NULL) = success */
static const char *tag_valid_tags(const char **tag_names, const char *input,
                                  int length)
{
 int skip = 0;
  
 assert(tag_names && input);
 
 --length;

 if (skip <= length) /* could be zero length */
 {
  input += skip;
  length -= skip;
 }
 else
 {
  assert(0);
  return (NULL);
 }
 
 while (*input && (length > 0)) /* may or maynot be null terminated :0 */
 {
  const char **cur_name = tag_names;
  
  while (*cur_name)
  {
   int tmp = beg_strcmp(*cur_name, input);
   
   if (!tmp)
   {/* found match to one of the tags */
    int tmp_tag = 0;
    const char *input_save = input;
    
    input += strlen(*cur_name);

    /* must be within length, or tag_get_bracket would have failed */
    if ((tmp_tag = tag_get_bracket(input)))
    {
     assert(tmp_tag);
     cur_name = tag_names;
     
     input += tmp_tag;
     length -= (input - input_save);
     
     assert(length >= 0);
     break; /* skips the ++cur_name */
    }
    else
      input = input_save;
   }
   else if (tmp > 0)
     return (input); /* valid list MUST be alphabetical */
   ++cur_name;
  }
  if (!*cur_name)
    return (input); /* tag didn't match any of the valid tags */

  if (length < 0)
  {
   assert(!length);
   return (NULL);
  }
  
  skip = strspn(input, ", \n\t");

  if (skip < length)
  {
   input += skip;
   length -= skip;
  }
  else
    length = 0;
 }
 assert(!*input || !length);

 return (NULL);
}

/* NOTE: input and output can NOT overlap */
static int tag_valid_number_tags(const char **tag_names, int beg_number,
                                 const char *input, int length,
                                 char *error_output)
{
 int skip = 0;
  
 assert(tag_names && input && error_output);
 
 --length;
 
 skip = strspn(input, ", \n\t");
 
 if (skip <= length) /* could be zero length */
 {
  input += skip;
  length -= skip;
 }
 else
 {
  assert(0);
  return (FALSE);
 }
 
 while (*input && (length > 0))
 {
  const char **cur_name = tag_names;
  
  if (isdigit((unsigned char) *input))
  {
   const char *input_start = input;
   int number_tag = skip_atoi(&input);
   int tmp_tag = 0;
   
   if ((number_tag < beg_number) || (length < 0) ||
       !(tmp_tag = tag_get_bracket(input)))
   {
    sprintf(error_output, "%d", number_tag);
    /* number tags MUST be possitive */
    return (FALSE);
   }

   assert(tmp_tag);
   cur_name = tag_names;
    
   input += tmp_tag;
   length -= (input - input_start);
   
   assert(length >= 0);
   break;
  }
  else
  {
   while (*cur_name)
   {
    int tmp = beg_strcmp(*cur_name, input);
    
    if (!tmp)
    {
     int tmp_tag = 0;
     const char *input_save = input;
     
     input += strlen(*cur_name);
     
     if ((tmp_tag = tag_get_bracket(input)))
     {
      assert(tmp_tag);
      cur_name = tag_names;
      
      input += tmp_tag;
      length -= (input - input_save);
      
      assert(length >= 0);
      break;
     }
     else
       input = input_save;
    }
    else if (tmp > 0)
    {
     const char *end_bad_tag = C_strchr(input, '(');
     int real_length = end_bad_tag - input;
     int err_length = ((real_length < MAX_TAG_SIZE) ? real_length :
                       MAX_TAG_SIZE);
     
     assert(end_bad_tag);
     
     COPY_STR_LEN(error_output, input, err_length);

     return (FALSE);
    }
    ++cur_name;
   }
   
   if (!*cur_name)
   {
    const char *end_bad_tag = C_strchr(input, '(');
    int real_length = end_bad_tag - input;
    int err_length = (real_length < MAX_TAG_SIZE) ? real_length : MAX_TAG_SIZE;
    
    assert(end_bad_tag);

    COPY_STR_LEN(error_output, input, err_length);
    
    return (FALSE); /* tag didn't match any of the valid tags */
   }
  }

  if (length < 0)
    return (FALSE);
  
  skip = strspn(input, ", \n\t");
  
  if (skip < length)
  {
   input += skip;
   length -= skip;
  }
  else
    length = 0;
 }
  
 return (TRUE);
}

static output_node *error_invalid_tag(player_tree_node *from, player *p,
				      twinkle_info *info,
                                      int flags, time_t timestamp,
                                      const char *error,
                                      const char *name,
                                      int *length, output_node *out_node)
{
 const char *beg_msg = "<";
 const char *mid_msg = " is not a valid ";
 const char *end_msg = " tag>";

 out_node = output_string(from, p, info, flags | RAW_OUTPUT, timestamp,
                          &beg_msg, INT_MAX, length, out_node);
 out_node = output_string(from, p, info, flags | RAW_OUTPUT, timestamp,
                          &error, INT_MAX, length, out_node);
 out_node = output_string(from, p, info, flags | RAW_OUTPUT, timestamp,
                          &mid_msg, INT_MAX, length, out_node);
 out_node = output_string(from, p, info, flags | RAW_OUTPUT, timestamp,
                          &name, INT_MAX, length, out_node);
 return (output_string(from, p, info, flags | RAW_OUTPUT, timestamp,
                       &end_msg, INT_MAX, length, out_node)); 
}

static output_node *error_no_tags(player_tree_node *from, player *p,
				  twinkle_info *info,
                                  int flags, time_t timestamp,
                                  const char *name,
                                  int *length, output_node *out_node)
{
 const char *beg_msg = "<";
 const char *end_msg = " is a tag variable>";

 out_node = output_string(from, p, info, flags | RAW_OUTPUT, timestamp,
                          &beg_msg, INT_MAX, length, out_node);
 out_node = output_string(from, p, info, flags | RAW_OUTPUT, timestamp,
                          &name, INT_MAX, length, out_node);
 return (output_string(from, p, info, flags | RAW_OUTPUT, timestamp,
                       &end_msg, INT_MAX, length, out_node));
}

static int check_tag_string(player_tree_node *from, player *p,
			    twinkle_info *info, int flags, time_t timestamp,
                            const char **input, const char **start_tags,
                            const char **valid_tags, const char *name,
                            int *output_length, output_node **out_node)
{
 int tmp_len = 0;

 *start_tags = *input;

 if ((tmp_len = tag_get_bracket(*input)))
 {
  const char *tmp = NULL;
  int actual_length = 0;

  *input += tmp_len;
  
  *start_tags = tag_skip_prefix(*start_tags);
  assert(*start_tags);
  *start_tags += strspn(*start_tags, ", \n\t");
  actual_length = (*input - *start_tags) - 1;
  
  tmp = tag_valid_tags(valid_tags, *start_tags, actual_length + 1);
  if (!tmp)
    return (actual_length); /* it worked */
  else
  {
   const char *end_bad_tag = tmp + strcspn(tmp, "()");
   int length = (end_bad_tag - tmp);
   char error_buffer[MAX_TAG_SIZE + 1];
   
   if (length > MAX_TAG_SIZE)
     length = MAX_TAG_SIZE;

   if (length) /* just to make sure */
     memcpy(error_buffer, tmp, length);
   error_buffer[length] = 0;
   *out_node = error_invalid_tag(from, p, info, flags, timestamp,
                                 error_buffer, name, output_length, *out_node);
   info->used_twinkles = TRUE;
  }
 }
 else
 {
  info->used_twinkles = TRUE;
  *out_node = error_no_tags(from, p, info, flags, timestamp, name,
                            output_length, *out_node);
 }
 
 return (-1);
}

static long buffer_toint(player_tree_node *from, player *p,
                         twinkle_info *info, int flags, time_t timestamp,
                         int len, const char *str_num)
{
 tmp_output_list_storage tmp_save;
 output_node *number_list = NULL;
 long the_number = 0;
 
 save_tmp_output_list(p, &tmp_save);
 if (len)
   fntell_player(ALL_T(from, p, info, flags | OUTPUT_BUFFER_TMP, timestamp),
                 len, INT_MAX, str_num);   
 else
   fvtell_player(ALL_T(from, p, info, flags | OUTPUT_BUFFER_TMP, timestamp),
                 "%s", str_num);
 
 number_list = output_list_grab(p);
 
 the_number = output_list_toint(number_list);
 
 output_list_cleanup(&number_list);
 load_tmp_output_list(p, &tmp_save);
 
 return (the_number);
}

static time_t buffer_totime(player_tree_node *from, player *p,
                            twinkle_info *info, int flags, time_t timestamp,
                            int len, const char *str_time)
{
 tmp_output_list_storage tmp_save;
 output_node *time_list = NULL;
 time_t the_time = (time_t)-1;
 
 save_tmp_output_list(p, &tmp_save);
 if (len)
   fntell_player(ALL_T(from, p, info, flags | OUTPUT_BUFFER_TMP, timestamp),
                 len, INT_MAX, str_time);   
 else
   fvtell_player(ALL_T(from, p, info, flags | OUTPUT_BUFFER_TMP, timestamp),
                 "%s", str_time);
 
 time_list = output_list_grab(p);

 the_time = output_list_totime(time_list);
 
 output_list_cleanup(&time_list);
 load_tmp_output_list(p, &tmp_save);

 /* FIXME: gmt -> user localtime problem ? .. always convert to GMT on output
 * and localtime on input ? */
 
 return (the_time);
}

static output_node *max_number_tag_variables(player_tree_node *from, player *p,
					     twinkle_info *info,
                                             int flags, time_t timestamp,
                                             int object,
                                             const char **input,
                                             const char *name,
                                             int *length,
                                             output_node *out_node)
{ 
 const char *tag_list_base[] = {"b", "base", NULL};
 const char *tag_list_max[] = {"m", "max", "maximum", NULL};
 const char *tag_list_valid[] = {"b", "base", DEFAULT_TAG_STRING,
                                 "m", "max", "maximum", NULL};
 char max_number_buffer[BUF_NUM_TYPE_SZ(int)];
 const char *tag_list_number[] = {max_number_buffer, NULL};
 int all_len = 0;
 const char *input_start = *input;
 
 BTRACE("max_number_tag_variables");
 
 /* not calling check_tag_string, doing it by hand as max_numb is a
    little different */
 if ((all_len = tag_get_bracket(*input)))
 {
  char error_buffer[MAX_TAG_SIZE + 1];
  int base_max_numb = object;
  int beg_len = 0;
  
  *input = tag_skip_prefix(*input);
  beg_len = all_len - ((*input - input_start) + 1);
  
  if (tag_valid_number_tags(tag_list_valid, 0, *input, beg_len, error_buffer))
  {
   const char *tmp = *input;
   int name_len = 0;
   
   if ((name_len = tag_get_name(tag_list_base, &tmp, beg_len)) >= 0)
     base_max_numb = buffer_toint(ALL_T(from, p, info, flags | OUTPUT_IN_TAG,
                                        timestamp),
                                  name_len, tmp);
   tmp = *input;
   name_len = 0;
   *input = input_start + all_len;
   
   if (object > base_max_numb)
     name_len = tag_get_name(tag_list_max, &tmp, beg_len);
   else
   {
    sprintf(max_number_buffer, "%d", object);
    name_len = tag_get_name(tag_list_number, &tmp, beg_len);
   }
   
   CHECK_TAG_STRING_END(&tmp, beg_len, name_len);
   
   return (string_variables(from, p, info, flags | OUTPUT_IN_TAG, timestamp,
                            name_len, tmp, input, length, out_node));
  }
  else
  {
   info->used_twinkles = TRUE;
   return (error_invalid_tag(from, p, info, flags, timestamp,
                             error_buffer, name, length, out_node));
  }
 }
 else
 {
  info->used_twinkles = TRUE;
  return (error_no_tags(from, p, info, flags, timestamp, name,
                        length, out_node));
 }
}

static output_node *probability_tag_variables(player_tree_node *from,player *p,
                                              twinkle_info *info,
                                              int flags, time_t timestamp,
					      int use_saved,
                                              const char **input,
                                              const char *name,
                                              int *length,
                                              output_node *out_node)
{
 const char *tag_list_base[] = {"b", "base", NULL};
 const char *tag_list_valid[] = {"b", "base", DEFAULT_TAG_STRING, NULL};
 char probability_buffer[BUF_NUM_TYPE_SZ(int)];
 const char *tag_list_prob[] = {probability_buffer, NULL};
 int all_len = 0;
 const char *input_start = *input;

 BTRACE("probability_tag_variables");

 /* not calling check_tag_string, doing it by hand as probabilities are a
    little different */
 if ((all_len = tag_get_bracket(*input)))
 {
  char error_buffer[MAX_TAG_SIZE + 1];
  int beg_len = 0;
  
  *input = tag_skip_prefix(*input);
  beg_len = all_len - ((*input - input_start) + 1);
  
  if (tag_valid_number_tags(tag_list_valid, 1, *input, beg_len, error_buffer))
  {
   int output_prob = 0;
   const char *tmp = *input;
   int name_len = 0;
   int base_prob = 2;
   
   if ((name_len = tag_get_name(tag_list_base, &tmp, beg_len)) >= 0)
   {
    base_prob = buffer_toint(ALL_T(from, p, info, flags | OUTPUT_IN_TAG,
                                   timestamp),
                             name_len, tmp);
    if (base_prob < 1)
      base_prob = 1;
   }
   tmp = *input;
   name_len = 0;
   *input = input_start + all_len;

   /* 0 isn't valid but base_prob is */
   output_prob = get_random_num(1, base_prob);
   
   if (use_saved) /* we want it to be the same for multiple ppl */
   {
    static unsigned int command_number_save = 0;
    static player *output_to_save = NULL;
    static int output_prob_save[MAX_PROB_IMBEDDING] = {0};
    static int output_count_save = 0;
    static struct { bitflag writting : 1; } are = {0};
    
    if ((command_number_save == current_command_number) &&
        (output_to_save == p))
    {
     if (are.writting) /* the first player sets up the probabilities */
       output_prob_save[output_count_save] = output_prob;
     else
       output_prob = output_prob_save[output_count_save];
     
     if (output_count_save < (MAX_PROB_IMBEDDING - 1))
       ++output_count_save;
    }
    else
    {
     if (command_number_save == current_command_number)
     { /* next player for the current_command */
      assert(output_to_save != p);
      assert(are.writting);
      
      are.writting = FALSE;
      output_prob = output_prob_save[0];
     }
     else
     {
      are.writting = TRUE;
      command_number_save = current_command_number;
      output_prob_save[0] = output_prob;
     }
     output_count_save = 1;
     output_to_save = p;
    }
   }
   
   sprintf(probability_buffer, "%d", output_prob);
   name_len = tag_get_name(tag_list_prob, &tmp, beg_len);
   
   CHECK_TAG_STRING_END(&tmp, beg_len, name_len);
   
   return (string_variables(from, p, info, flags | OUTPUT_IN_TAG, timestamp,
                            name_len, tmp, input, length, out_node));
  }
  else
  {
   info->used_twinkles = TRUE;
   return (error_invalid_tag(from, p, info, flags, timestamp,
                             error_buffer, name, length, out_node));
  }
 }
 else
 {
  info->used_twinkles = TRUE;
  return (error_no_tags(from, p, info, flags, timestamp, name,
                        length, out_node));
 }
}

static unsigned int seconds_variables_two(const char **input)
{
 unsigned int option = 0;

 MOD_STICK(input, option, ALL);
 
 if (!BEG_CONST_STRCMP("_years", *input))
 {
  SKIP_MATCHED_TO(input, "_years");
  option |= WORD_TIME_YEARS;
  MOD_STICK(input, option, YEARS);
 }    

 if (!BEG_CONST_STRCMP("_weeks", *input))
 {
  SKIP_MATCHED_TO(input, "_weeks");
  option |= WORD_TIME_WEEKS;
  MOD_STICK(input, option, WEEKS);
 }

 if (!BEG_CONST_STRCMP("_days", *input))
 {
  SKIP_MATCHED_TO(input, "_days");
  option |= WORD_TIME_DAYS;
  MOD_STICK(input, option, DAYS);
 }
 
 if (!BEG_CONST_STRCMP("_hours", *input))
 {
  SKIP_MATCHED_TO(input, "_hours");
  option |= WORD_TIME_HOURS;
  MOD_STICK(input, option, HOURS);
 }

 if (!BEG_CONST_STRCMP("_minutes", *input))
 {
  SKIP_MATCHED_TO(input, "_minutes");
  option |= WORD_TIME_MINUTES;
  MOD_STICK(input, option, MINUTES);
 }
 
 if (!BEG_CONST_STRCMP("_seconds", *input))
 {
  SKIP_MATCHED_TO(input, "_seconds");
  option |= WORD_TIME_SECONDS;
  MOD_STICK(input, option, SECONDS);
 }

 if (option)
   return (option);
 else
   /* if they havn't selected anything ... this is default */
   return (WORD_TIME_ALL);
}


static output_node *seconds_variables(player_tree_node *from, player *p,
				      twinkle_info *info,
                                      int flags, time_t timestamp, 
                                      long object, const char **input,
                                      int *length, output_node *out_node)
{
 int cmp_sve = -1;
 const char *tmp = "<Bad Formatting (Seconds)>";
 char word_time_buffer[1024 * 2];
 const char *ret = NULL;
 
 BTRACE("seconds_variables");

 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));

 if (islower((unsigned char)**input))
   cmp_sve = 1;
 
 if (**input == '?')
 {
  ++*input;
  return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                           "Long, Short", input, length, out_node));
 }

 if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Long", *input)))
 {  
  SKIP_MATCHED_TO(input, "Long");
  ret = word_time_long(word_time_buffer, sizeof(word_time_buffer),
                       object, seconds_variables_two(input));
 }
 else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Short", *input)))
 {  
  SKIP_MATCHED_TO(input, "Short");
  ret = word_time_short(word_time_buffer, sizeof(word_time_buffer),
                        object, seconds_variables_two(input));
 }
 else
 {
  out_node = output_string(from, p, info, flags, timestamp, &tmp, INT_MAX,
			   length, out_node);
  return (out_node);
 }
 
 return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                          ret, input, length, out_node));
}

static const char *internal_time_variables(player *p, time_t object,
                                           const char **input)
{
 int cmp_sve = -1;
 const char *tmp = NULL;
 
 switch (**input)
 {
  case '1':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("12", *input)))
    {
     SKIP_MATCHED_TO(input, "12");
     tmp = disp_time_hour_min(object, 0, FALSE);
    }
    break;
   
  case '2':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("24", *input)))
    {
     SKIP_MATCHED_TO(input, "24");
     tmp = disp_time_hour_min(object, 0, TRUE);
    }
    break;
   
  case 'A':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Ampm", *input)))
    {
     SKIP_MATCHED_TO(input, "Ampm");
     tmp = disp_time_ampm_string(object);
    }
    break;

  case 'C':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Clock", *input)))
    {
     SKIP_MATCHED_TO(input, "Clock");
     tmp = disp_time_hour_min(object, 0, p->flag_use_24_clock);     
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Cmp", *input)))
    {
     SKIP_MATCHED_TO(input, "Cmp");
     tmp = disp_time_cmp_string(object);
    }
    break;   
   
  case 'D':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Date", *input)))
    {
     SKIP_MATCHED_TO(input, "Date");
     tmp = disp_date_std(object, 0, p->flag_use_long_clock);
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Day", *input)))
    {
     if (!BEG_CONST_STRCMP("Day_number", *input))
     {
      SKIP_MATCHED_TO(input, "Day_number");
      tmp = disp_time_week_day_number(object);
     }
     else if (!BEG_CONST_STRCMP("Day_name", *input))
     {
      SKIP_MATCHED_TO(input, "Day_name");
      if (!BEG_CONST_STRCMP("_short", *input))
      {
       SKIP_MATCHED_TO(input, "_short");
       tmp = disp_time_day_name_short(object);
      }
      else
        tmp = disp_time_day_name(object);
     }
     else if (!BEG_CONST_STRCMP("Day_month", *input))
     {
      SKIP_MATCHED_TO(input, "Day_month");
      tmp = disp_time_month_day_number(object);
     }
     else if (!(cmp_sve = BEG_CONST_STRCMP("Day_year", *input)))
     {
      SKIP_MATCHED_TO(input, "Day_year");
      tmp = disp_time_year_day_number(object);
     }
     else if (!(cmp_sve = BEG_CONST_STRCMP("Day_week", *input)))
     {
      SKIP_MATCHED_TO(input, "Day_week");
      tmp = disp_time_week_day_number(object);
     }
     else
     {
      SKIP_MATCHED_TO(input, "Day");
      tmp = disp_time_month_day_number(object);
     }
    }
    break;

  case 'H':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Hour", *input)))
    {
     SKIP_MATCHED_TO(input, "Hour");
     if (!(cmp_sve = BEG_CONST_STRCMP("_12", *input)))
     {
      SKIP_MATCHED_TO(input, "_12");
      tmp = disp_time_hour_number_12(object);
     }
     else
     {
      if (!(cmp_sve = BEG_CONST_STRCMP("_24", *input)))
        SKIP_MATCHED_TO(input, "_24");
      tmp = disp_time_hour_number_24(object);
     }
    }
    break;
   
  case 'M':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Minute", *input)))
    {
     SKIP_MATCHED_TO(input, "Minute");
     tmp = disp_time_minute_number(object);
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Month", *input)))
    {
     if (!BEG_CONST_STRCMP("Month_day", *input))
     {
      SKIP_MATCHED_TO(input, "Month_day");
      tmp = disp_time_month_day_number(object);
     }
     else
       if (!BEG_CONST_STRCMP("Month_number", *input))
       {
        SKIP_MATCHED_TO(input, "Month_number");
        tmp = disp_time_month_number(object);
       }
       else if (!BEG_CONST_STRCMP("Month_name", *input))
       {
        SKIP_MATCHED_TO(input, "Month_name");

        if (!BEG_CONST_STRCMP("_short", *input))
        {
         SKIP_MATCHED_TO(input, "_short");
         tmp = disp_time_month_name_short(object);
        }
        else
          tmp = disp_time_month_name(object);
       }
       else
       {
        SKIP_MATCHED_TO(input, "Month");
        tmp = disp_time_month_number(object);
       }
    }
    break;
    
  case 'S':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Second", *input)))
    {
     SKIP_MATCHED_TO(input, "Second");
     tmp = disp_time_second_number(object);
    }
    break;   

  case 'T':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Time", *input)))
    {
     SKIP_MATCHED_TO(input, "Time");
     tmp = disp_time_std(object, 0,
                         p->flag_use_24_clock, p->flag_use_long_clock);
    }
    break;
      
  case 'W':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Week", *input)))
    {
     SKIP_MATCHED_TO(input, "Week");
     if (!(cmp_sve = BEG_CONST_STRCMP("_number", *input)))
       SKIP_MATCHED_TO(input, "_number");

     tmp = disp_time_week_number(object);
    }
    break;

  case 'Y':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Year", *input)))
    {
     if (!(cmp_sve = BEG_CONST_STRCMP("Year_day", *input)))
     {
      SKIP_MATCHED_TO(input, "Year_day");
      tmp = disp_time_year_day_number(object);
     }
     else
     {
      SKIP_MATCHED_TO(input, "Year");
      tmp = disp_time_year_number(object);
     }
    }
    break;

  default:
    break;
 }

 return (tmp);
}
                                   
static output_node *time_variables(player_tree_node *from, player *p,
				   twinkle_info *info,
                                   int flags, time_t timestamp, 
                                   time_t object, const char **input,
                                   int *length, output_node *out_node)
{
 int cmp_sve = -1;
 const char *tmp = NULL;
 
 BTRACE("time_variables");

 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
  
 if (islower(**input))
   cmp_sve = 1;

 if (**input == '?')
 {
  ++*input;
  return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                           "12, 24, Ampm, Clock, Cmp, Day_number, Day_name, Day_name_short, Day_month, Day_year, Day_week, Hour_12, Hour_24, Minute, Month_day, Month_number, Month_name, Month_name_short, Second, Week_number, Year, Year_day",
                           input, length, out_node));
 }
    
 if (**input != '(')
 {
  if (**input)
  {
   if ((tmp = internal_time_variables(p, object, input)))
   {
    if (!*(tmp + strspn(tmp, "0123456789")))
      return (number_variables(from, p, info, flags, timestamp,
                               atol(tmp), input, length, out_node));

    return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                             tmp, input, length, out_node));
   }
  }
 }
 else
 {
  char time_buffer[256]; /* FIXME: magic number */
  size_t buffer_used = 0;
  int name_len = 0;
  
  if ((name_len = tag_get_bracket(*input)))
  {
   const char *in_buffer = tag_skip_prefix(*input);
   int count = name_len - ((in_buffer - *input) + 1);
   
   *input += name_len;
   
   while ((count > 0) && *in_buffer)
   {
    const char *save_in_buffer = in_buffer;
    
    if ((tmp = internal_time_variables(p, object, &in_buffer)))
    {
     int tmp_length = strlen(tmp);
     
     assert(tmp_length);
     
     if ((buffer_used + tmp_length) < sizeof(time_buffer))
       memcpy(time_buffer + buffer_used, tmp, tmp_length);
     
     buffer_used += tmp_length;
     count -= (in_buffer - save_in_buffer);
    }
    else
    { /* nothing is invalid here */
     if ((buffer_used + 1) < sizeof(time_buffer))
     {
      if ((*in_buffer == '\\') && (count != 1) && in_buffer[1] &&
          ((in_buffer[1] == '\\') || (in_buffer[1] == '(') ||
           (in_buffer[1] == ')')))
        ++in_buffer;
      time_buffer[buffer_used++] = *in_buffer;
     }
     
     ++in_buffer;
     --count;
    }
   }
   
   time_buffer[buffer_used] = 0;
   if (buffer_used)
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              time_buffer, input, length, out_node));
  }
 }
  
 tmp = "<Bad Formatting (Time)>";
 info->used_twinkles = TRUE;
 out_node = output_string(from, p, info, flags, timestamp, &tmp, INT_MAX,
                          length, out_node);

 return (out_node);
 
}

static output_node *toggle_variables(player_tree_node *from, player *p,
                                     twinkle_info *info, int flags,
                                     time_t timestamp, int object,
                                     const char **input, int *length,
                                     output_node *out_node)
{
 const char *tmp = NULL;

 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
  
 if (!(BEG_CONST_STRCMP("_off", *input) && BEG_CONST_STRCMP("_no", *input)))
 {
  if (*(*input + 1) == 'n')
    SKIP_MATCHED_TO(input, "_no");
  else
    SKIP_MATCHED_TO(input, "_off");

  if (object)
    tmp = "N";
  else
    tmp = "Y";
 }
 else
 {
  if (!(BEG_CONST_STRCMP("_on", *input) && BEG_CONST_STRCMP("_yes", *input)))
  {
   if (*(*input + 1) == 'y')
     SKIP_MATCHED_TO(input, "_yes"); 
   else
     SKIP_MATCHED_TO(input, "_on"); 
  }
  
  if (object)
    tmp = "Y";
  else
    tmp = "N";
 }

 assert(tmp);
 return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                          tmp, input, length, out_node));
}

static void output_variables_player_load(player_tree_node *object)
{
 static unsigned int command_number_save = 0;
 static unsigned int count = 0;

 if (P_IS_AVL(object))
   return;
 
 if (command_number_save != current_command_number)
 {
  count = 0;
  command_number_save = current_command_number;
 }
 
 if (count < OUTPUT_VARIABLES_PLAYER_LOADS)
 {
  if (player_load(object))
    ++count;
 }
}

static output_node *toggle_player_variables(player_tree_node *from, player *p,
                                            twinkle_info *info,
                                            int flags, time_t timestamp,
                                            player_tree_node *object,
                                            const char **input,
                                            int *length, output_node *out_node)
{
 int cmp_sve = -1;
 const char *tmp = "<** ERROR **>";
 
 BTRACE("toggle_player_variables");
 
 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
 
 CHECK_OBJECT(object, p, flags, "<Bad toggle>", length, out_node);
 
 switch (**input)
 {
  case '?':
    ++*input;
    return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                             "Anonymous, Duty, Email_show, Follow_allowed, Hiding, Ignoreing_emote_prefix, Ignoreing_prefix, Ignoreing_rooms, Ignoreing_shouts, Ignoreing_tells, Logged, Lsu_list, New_mail, Newbie, Quiet_edit, Room_enter, Session_in_who, Show_exits",
                             input, length, out_node));
    
  case 'A':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Anonymous", *input)))
    { /* anyone can see this, as it's helpfull */
     SKIP_MATCHED_TO(input, "Anonymous");
     return (toggle_variables(from, p, info, flags, timestamp,
                              !object->flag_no_anonymous,
                              input, length, out_node));
    }
    break;

  case 'D':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Duty", *input)))
    {
     SKIP_MATCHED_TO(input, "Duty");
     if (PRIV_STAFF(p->saved) && P_IS_ON(object))
       return (toggle_variables(from, p, info, flags, timestamp,
                                !object->player_ptr->flag_tmp_su_channel_block,
                                input, length, out_node));
     else
       return (toggle_variables(from, p, info, flags, timestamp, FALSE,
                                input, length, out_node));
    }
    break;

  case 'E':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Email_show", *input)))
    { /* anyone can see this, as it's helpfull */
     SKIP_MATCHED_TO(input, "Email_show");
     return (toggle_variables(from, p, info, flags, timestamp,
                              !object->flag_private_email,
                              input, length, out_node));
    }
    break;
   
  case 'F':
    if ((cmp_sve < 1) &&
        !(cmp_sve = BEG_CONST_STRCMP("Follow_allowed", *input)))
    { /* anyone can see this, as it's helpfull */
     SKIP_MATCHED_TO(input, "Follow_allowed");
     
     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       return (toggle_variables(from, p, info, flags, timestamp,
                                !object->player_ptr->flag_follow_block,
                                input, length, out_node));
     else
       return (toggle_variables(from, p, info, flags, timestamp, FALSE,
                                input, length, out_node));
    }
    break;
   
  case 'H':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Hiding", *input)))
    {
     SKIP_MATCHED_TO(input, "Hiding");
     
     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       return (toggle_variables(from, p, info, flags, timestamp,
                                object->player_ptr->flag_location_hide,
                                input, length, out_node));
     else
       return (toggle_variables(from, p, info, flags, timestamp, FALSE,
                                input, length, out_node));
    }
    break;

  case 'I':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Ign", *input)))
    { /* anyone can see these, as it's helpfull */
     /* prefix, epreif, room descrip, shouts, tells */
     if (!BEG_CONST_STRCMP("Ignoreing", *input))
       SKIP_MATCHED_TO(input, "Ignoreing");
     else
       SKIP_MATCHED_TO(input, "Ign");
     
     cmp_sve = -1;
     if (((cmp_sve < 1) &&
          !(cmp_sve = BEG_CONST_STRCMP("_emote_prefix", *input))) ||
         ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("_eprefix", *input))))
     {
      if (*(*input + 2) == 'p')
        SKIP_MATCHED_TO(input, "_eprefix");
      else
        SKIP_MATCHED_TO(input, "_emote_prefix");

      output_variables_player_load(object);
      
      if (P_IS_AVL(object))
        return (toggle_variables(from, p, info, flags, timestamp,
                                 object->player_ptr->flag_no_emote_prefix,
                                 input, length, out_node));
      else
        return (toggle_variables(from, p, info, flags, timestamp, FALSE,
                                 input, length, out_node));
     }
     else if ((cmp_sve < 1) &&
              !(cmp_sve = BEG_CONST_STRCMP("_prefix", *input)))
     {
      SKIP_MATCHED_TO(input, "_prefix");

     default_ignore_toggle:
      output_variables_player_load(object);
      
      if (P_IS_AVL(object))
        return (toggle_variables(from, p, info, flags, timestamp,
                                 object->player_ptr->flag_no_prefix,
                                 input, length, out_node));
      else
        return (toggle_variables(from, p, info, flags, timestamp, FALSE,
                                 input, length, out_node));
     }
     else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("_rooms", *input)))
     {
      SKIP_MATCHED_TO(input, "_rooms");

      output_variables_player_load(object);
      
      if (P_IS_AVL(object))
        return (toggle_variables(from, p, info, flags, timestamp,
                                 !object->player_ptr->flag_room_enter_brief,
                                 input, length, out_node));
      else
        return (toggle_variables(from, p, info, flags, timestamp, FALSE,
                                 input, length, out_node));
     }
    
     goto default_ignore_toggle;
    }
    break;
   
  case 'L':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Logged", *input)))
    {
     SKIP_MATCHED_TO(input, "Logged"); /* _on is done inside toggles */
     return (toggle_variables(from, p, info, flags, timestamp, P_IS_ON(object),
                              input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Lsu_list", *input)))
    {
     SKIP_MATCHED_TO(input, "Lsu_list");
     if ((flags & OUTPUT_VARIABLES_NO_CHECKS) || PRIV_STAFF(p->saved))
     {
      output_variables_player_load(object);
      
      if (P_IS_AVL(object))
        return (toggle_variables(from, p, info, flags, timestamp,
                                 !object->player_ptr->flag_tmp_su_channel_off,
                                 input, length, out_node));
     }
     
     return (toggle_variables(from, p, info, flags, timestamp, FALSE,
                              input, length, out_node));
    }
    break;

  case 'N':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("New", *input)))
    { /* anyone can see this, as it's helpfull */
     SKIP_MATCHED_TO(input, "New");
    
     if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("_mail", *input)))
     {
      SKIP_MATCHED_TO(input, "_mail");
     default_new_toggle:
      
      return (toggle_variables(from, p, info, flags, timestamp,
                               mail_check_mail_new(object),
                               input, length, out_node));
     }
     else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("bie", *input)))
     {
      SKIP_MATCHED_TO(input, "bie");
      return (toggle_variables(from, p, info, flags, timestamp,
                               from && from->player_ptr && 
                               (!object->priv_base ||
                                (from->player_ptr->list_newbie_time >
                                 real_total_logon_time(object))),
                               input, length, out_node));
     }
     
     goto default_new_toggle;
    }
    break;
  case 'Q':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Quiet_edit", *input)))
    { /* NOTE: maybe this one should not be world readable as well ? */
     SKIP_MATCHED_TO(input, "Quiet_edit");
     
     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       return (toggle_variables(from, p, info, flags, timestamp,
                                object->player_ptr->flag_quiet_edit,
                                input, length, out_node));
     else
       return (toggle_variables(from, p, info, flags, timestamp, FALSE,
                                input, length, out_node));
    }
    break;
  case 'R':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Room_enter", *input)))
    { /* NOTE: maybe this one should not be world readable as well ? */
     SKIP_MATCHED_TO(input, "Room_enter");

     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       return (toggle_variables(from, p, info, flags, timestamp,
                                object->player_ptr->flag_see_room_events,
                                input, length, out_node));    
     else
       return (toggle_variables(from, p, info, flags, timestamp, FALSE,
                                input, length, out_node));
    }
    break;
  case 'S':
    if ((cmp_sve < 1) &&
        !(cmp_sve = BEG_CONST_STRCMP("Session_in_who", *input)))
    { /* NOTE: maybe this one should not be world readable as well ? */
     SKIP_MATCHED_TO(input, "Session_in_who");

     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       return (toggle_variables(from, p, info, flags, timestamp,
                                object->player_ptr->flag_session_in_who,
                                input, length, out_node));    
     else
       return (toggle_variables(from, p, info, flags, timestamp, FALSE,
                                input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Show_exits", *input)))
    { /* NOTE: maybe this one should not be world readable as well ? */
     SKIP_MATCHED_TO(input, "Show_exits");

     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       return (toggle_variables(from, p, info, flags, timestamp,
                                object->player_ptr->flag_room_exits_show,
                                input, length, out_node));    
     else
       return (toggle_variables(from, p, info, flags, timestamp, FALSE,
                                input, length, out_node));
    }
    break;

  default:
    break;
 }
 
 tmp = "<Bad Formatting (Setting)>";
 info->used_twinkles = TRUE;
   
 return (output_string(from, p, info, flags, timestamp, &tmp, INT_MAX,
                       length, out_node));
}

static output_node *cpu_variables(player_tree_node *from, player *p,
                                  twinkle_info *info,
                                  int flags, time_t timestamp,
                                  const struct timeval *this,
                                  const struct timeval *total,
                                  const char **input,
                                  int *length, output_node *out_node)
{
 long number = 0;
 struct timeval dummy = {0, 0};
 int cmp_sve = -1;
 const char *tmp = "<** ERROR **>";

 if (!this)
   this = &dummy;
 if (!total)
   total = &dummy;
 
 BTRACE("compression_variables");

 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
 
 if (**input == '?')
 {
  ++*input;
  return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                           "Seconds, Total_seconds, Total_useconds, Useconds",
                           input, length, out_node));
 }
 
 if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Seconds", *input)))
 {
  SKIP_MATCHED_TO(input, "Seconds");
  number = this->tv_sec;
 }
 else if ((cmp_sve < 1) &&
          !(cmp_sve = BEG_CONST_STRCMP("Total_seconds", *input)))
 {
  SKIP_MATCHED_TO(input, "Total_seconds");
  number = total->tv_sec;
 }
 else if ((cmp_sve < 1) &&
          !(cmp_sve = BEG_CONST_STRCMP("Total_useconds", *input)))
 {
  SKIP_MATCHED_TO(input, "Total_useconds");
  number = total->tv_usec;
 }
 else if ((cmp_sve < 1) &&
          !(cmp_sve = BEG_CONST_STRCMP("Useconds", *input)))
 {
  SKIP_MATCHED_TO(input, "Useconds");
  number = this->tv_usec;
 }
 else
 {
  tmp = "<Bad Formatting (Cpu)>";
  info->used_twinkles = TRUE;
  
  return (output_string(from, p, info, flags, timestamp, &tmp, INT_MAX,
                        length, out_node));
 }
 
 return (number_variables(from, p, info, flags, timestamp,
                          number, input, length, out_node));
}

static output_node *compression_variables(player_tree_node *from, player *p,
                                          twinkle_info *info,
                                          int flags, time_t timestamp,
                                          player *p_object,
                                          const char **input,
                                          int *length, output_node *out_node)
{
 int cmp_sve = -1;
 const char *tmp = "<** ERROR **>";
#ifdef HAVE_ZLIB_H
 z_stream *comp = p_object ? p_object->output_compress_lib : NULL;
#else
 int comp = 0;
#endif
 
 BTRACE("compression_variables");

 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
 
 switch (**input)
 {
  case '?':
    ++*input;
    return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                             "Cpu, Data_type, In, Out",
                             input, length, out_node));
  case 'C':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Cpu", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Cpu", length, out_node);
     return (cpu_variables(from, p, info, flags, timestamp,
                           p_object ? &p_object->comp_cpu : NULL,
                           p_object ? &p_object->comp_cpu : NULL,
                           input, length, out_node));
    }
    break;

  case 'D':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Data_type", *input)))
    {
     SKIP_MATCHED_TO(input, "Data_type");
     if (comp)
       return (number_variables(from, p, info, flags, timestamp,
                                OUTPUT_COMPRESS_MEM(comp, data_type),
                                input, length, out_node));
     else
       return (number_variables(from, p, info, flags, timestamp, 0,
                                input, length, out_node));
    }
    break;

  case 'I':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("In", *input)))
    {
     SKIP_MATCHED_TO(input, "In");
     if (comp)
       return (number_variables(from, p, info, flags, timestamp,
                                OUTPUT_COMPRESS_MEM(comp, total_in),
                                input, length, out_node));
     else
       return (number_variables(from, p, info, flags, timestamp, 0,
                                input, length, out_node));
    }
    break;
    
  case 'O':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Out", *input)))
    {
     SKIP_MATCHED_TO(input, "Out");
     if (comp)
       return (number_variables(from, p, info, flags, timestamp,
                                OUTPUT_COMPRESS_MEM(comp, total_out),
                                input, length, out_node));
     else
       return (number_variables(from, p, info, flags, timestamp, 0,
                                input, length, out_node));
    }
    break;
    
  default:
    break;
 }

 tmp = "<Bad Formatting (Compression)>";
 info->used_twinkles = TRUE;
 
 return (output_string(from, p, info, flags, timestamp, &tmp, INT_MAX,
                       length, out_node));

}

static output_node *player_edit_variables(player_tree_node *from, player *p,
                                          twinkle_info *info,
                                          int flags, time_t timestamp,
                                          player *p_object,
                                          const char **input,
                                          int *length, output_node *out_node)
{
 int cmp_sve = -1;
 const char *tmp = "<** ERROR **>";
 edit_base *base = NULL;
 
 BTRACE("player_edit_variables");

 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));

 if (p_object && p_object->buffers)
   base = p->buffers->current_edit_base;
 
 switch (**input)
 {
  case '?':
    ++*input;
    return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                             "At_beginning, At_end, Current_line, Lines, Size",
                             input, length, out_node));

  case 'A':
    if ((cmp_sve < 1) &&
        !(cmp_sve = BEG_CONST_STRCMP("At_beginning", *input)))
    {
     SKIP_MATCHED_TO(input, "At_beginning");
     if (base)
       return (toggle_variables(from, p, info, flags, timestamp,
                                !base->current_line, input, length, out_node));
     else
       return (toggle_variables(from, p, info, flags, timestamp, 0,
                                input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("At_end", *input)))
    {
     SKIP_MATCHED_TO(input, "At_end");
     if (base)
       return (toggle_variables(from, p, info, flags, timestamp,
                                (base->current_line + p_object->term_height) >
                                base->lines, input, length, out_node));
     else
       return (toggle_variables(from, p, info, flags, timestamp, 0,
                                input, length, out_node));
    }
    break;

  case 'C':
    if ((cmp_sve < 1) &&
        !(cmp_sve = BEG_CONST_STRCMP("Current_line", *input)))
    {
     SKIP_MATCHED_TO(input, "Current_line");
     if (base)
       return (number_variables(from, p, info, flags, timestamp,
                                base->current_line, input, length, out_node));
     else
       return (number_variables(from, p, info, flags, timestamp, 0,
                                input, length, out_node));
    }
    break;

  case 'L':
    if ((cmp_sve < 1) &&
        !(cmp_sve = BEG_CONST_STRCMP("Lines", *input)))
    {
     SKIP_MATCHED_TO(input, "Lines");
     if (base)
       return (number_variables(from, p, info, flags, timestamp, base->lines,
                                input, length, out_node));
     else
       return (number_variables(from, p, info, flags, timestamp, 0,
                                input, length, out_node));
    }
    break;
    
  case 'S':
    if ((cmp_sve < 1) &&
        !(cmp_sve = BEG_CONST_STRCMP("Size", *input)))
    {
     SKIP_MATCHED_TO(input, "Size");
     if (base)
       return (number_variables(from, p, info, flags, timestamp,
                                base->characters, input, length, out_node));
     else
       return (number_variables(from, p, info, flags, timestamp, 0,
                                input, length, out_node));
    }
    break;
  default:
    break;
 }
 
 tmp = "<Bad Formatting (Edit)>";
 info->used_twinkles = TRUE;
 
 return (output_string(from, p, info, flags, timestamp, &tmp, INT_MAX,
                       length, out_node));
}

static output_node *player_variables(player_tree_node *from, player *p,
				     twinkle_info *info,
                                     int flags, time_t timestamp,
				     player_tree_node *object,
				     const char **input,
                                     int *length, output_node *out_node)
{
 int cmp_sve = -1;
 const char *tmp = "<** ERROR **>";
                
 BTRACE("player_variables");

 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
  
 CHECK_OBJECT(object, p, flags, "<Bad player>", length, out_node);
 
 switch (**input)
 { /* no enter_msg, or room_connect */
  case '?':
    ++*input;
    return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                             "Age, Birthday, Compression, Comment, Conjugate, Connect_message, Cpu, Description, Disconnect_message, Dns_address, Edit, Email, Emotive, Following, Gender, Idle_logon, Idle_time, Idle_total, Ip_address, Karma_value, Logoff_timestamp, Logon_time, Logon_timestamp, Logon_total, Mail_recieved, Mail_sent, Mail_unread, Married_to, Name, Name_full, Name_lower, Nationality, Phone_numbers, Plan, Plural, Possessive, Prefix, Privs, Proposed_to, Ressed_on, Room, Schedule_time, Setting, Spodnumber, Terminal, Time, Title, Title_with_name, Typed_commands, Url",
                             input, length, out_node));
    
  case 'A':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Age", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Age",
                               length, out_node);

     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       return (seconds_variables(from, p, info, flags, timestamp, 
                                 real_age(object->player_ptr),
                                 input, length, out_node));
     else
       return (seconds_variables(from, p, info, flags, timestamp, 0,
                                 input, length, out_node));
    }
    break;
   
  case 'B':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Birthday", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Birthday",
                               length, out_node);

     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       return (time_variables(from, p, info, flags, timestamp, 
                              object->player_ptr->birthday,
                              input, length, out_node));
     else
       return (time_variables(from, p, info, flags, timestamp, now,
                              input, length, out_node));
    }
    break;
   
  case 'C':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Comp", *input)))
    {
     if (!BEG_CONST_STRCMP("Compression", *input))
       SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Compression",
                                 length, out_node);
     else
       SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Comp",
                                 length, out_node);

     if (P_IS_ON(object))
       return (compression_variables(from, p, info, flags, timestamp,
                                     object->player_ptr,
                                     input, length, out_node));
     else
       return (compression_variables(from, p, info, flags, timestamp, NULL,
                                     input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Comment", *input)))
    {
     SKIP_MATCHED_TO(input, "Comment");
     
     if (P_IS_ON(object))
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                object->player_ptr->comment,
                                input, length, out_node));
     else
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                "", input, length, out_node)); 
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Conjugate", *input)))
    {
     const char *the_string = NULL;
     
     SKIP_MATCHED_TO(input, "Conjugate");

     output_variables_player_load(object);
     
     if (!P_IS_AVL(object) ||
         ((object->player_ptr == p) && !info->output_not_me))
       the_string = "";
     else
       if (object->player_ptr->gender == GENDER_PLURAL)
         the_string = "";
       else
         the_string = "s";
    
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              the_string, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             (!(cmp_sve = BEG_CONST_STRCMP("Connect_msg", *input)) ||
              !(cmp_sve = BEG_CONST_STRCMP("Connect_message", *input))))
    {
     if (*(*input + CONST_STRLEN("Connect_m")) == 's')
       SKIP_MATCHED_TO(input, "Connect_msg");
     else
       SKIP_MATCHED_TO(input, "Connect_message");
     
     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                object->player_ptr->connect_msg,
                                input, length, out_node));
     else
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                "", input, length, out_node)); 
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Cpu", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Cpu", length, out_node);

     output_variables_player_load(object);
     
     return (cpu_variables(from, p, info, flags, timestamp,
                           P_IS_ON(object) ? &object->player_ptr->this_cpu :
                           NULL,
                           P_IS_AVL(object) ? &object->player_ptr->total_cpu :
                           NULL,
                           input, length, out_node));
    }
    break;
   
  case 'D':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Description", *input)))
    {
     static unsigned int allow_twinkle_depth = OUTPUT_SUB_TWINKLE_DEPTH;
    
     SKIP_MATCHED_TO(input, "Description");

     output_variables_player_load(object);
     
     if (P_IS_AVL(object) && allow_twinkle_depth)
     {
      --allow_twinkle_depth;
      out_node = string_variables(from, p, info, flags, timestamp, INT_MAX,
                                  object->player_ptr->description, input,
                                  length, out_node);
      ++allow_twinkle_depth;
     }
     else
       out_node =  string_variables(from, p, info, flags, timestamp, INT_MAX,
                                    "", input, length, out_node);
     return (out_node);
    }
    else if ((cmp_sve < 1) &&
             (!(cmp_sve = BEG_CONST_STRCMP("Disconnect_msg", *input)) ||
              !(cmp_sve = BEG_CONST_STRCMP("Disconnect_message", *input))))
    {
     if (*(*input + CONST_STRLEN("Disconnect_m")) == 's')
       SKIP_MATCHED_TO(input, "Disconnect_msg");
     else
       SKIP_MATCHED_TO(input, "Disconnect_message");
     
     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                object->player_ptr->disconnect_msg,
                                input, length, out_node));
     else
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                "", input, length, out_node)); 
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Dns_address", *input)))
    {
     SKIP_MATCHED_TO(input, "Dns_address");
    tag_dns_address:

     output_variables_player_load(object);
     
     if (P_IS_AVL(object) &&
         ((object == p->saved) || p->saved->priv_command_trace ||
         (flags & OUTPUT_VARIABLES_NO_CHECKS)))
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                object->player_ptr->dns_address,
                                input, length, out_node));
     else
       return (string_variables(from, p, info, flags, timestamp, INT_MAX, "",
                                input, length, out_node));
    }
    break;
   
  case 'E':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Edit", *input)))
    {
     SKIP_MATCHED_TO(input, "Edit");
    
     if (P_IS_ON(object) &&
         ((from == object) || (flags & OUTPUT_VARIABLES_NO_CHECKS)))
       return (player_edit_variables(from, p, info, flags, timestamp,
                                     object->player_ptr, input,
                                     length, out_node));
     else
       return (player_edit_variables(from, p, info, flags, timestamp,
                                     NULL, input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Email", *input)))
    {
     SKIP_MATCHED_TO(input, "Email");

     output_variables_player_load(object);
     
     if (P_IS_AVL(object) &&
         ((from == object) || !object->flag_private_email ||
          (flags & OUTPUT_VARIABLES_NO_CHECKS)))
       return (string_variables(from, p, info, RAW_OUTPUT | flags,
                                timestamp, INT_MAX,
                                object->player_ptr->email, input,
                                length, out_node));
     else
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                "", input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Emotive", *input)))
    {
     const char *the_string = NULL;
     char buffer[15];
    
     SKIP_MATCHED_TO(input, "Emotive");

     output_variables_player_load(object);
     
     if (!P_IS_AVL(object))
       the_string = "themself"; /* any would do */
     else
       if ((object->player_ptr == p) && !info->output_not_me)
         if (object->player_ptr->gender == GENDER_PLURAL)
           the_string = "yourselves";
         else
           the_string = "yourself";
       else
       {
        if (object->player_ptr->gender == GENDER_PLURAL)
          the_string = "themselves";
        else
        {
         sprintf(buffer, "%sself",
                 gender_choose_str(object->player_ptr->gender,
                                   "him", "her", "them", "it"));
         the_string = buffer;
        }
       }
    
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              the_string, input, length, out_node));
    }
    break;

  case 'F':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Following", *input)))
    {
     SKIP_MATCHED_TO(input, "Following");

     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                object->player_ptr->follow, input,
                                length, out_node));
     else
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                "", input, length, out_node));
    }
    break;
   
  case 'G':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Gender", *input)))
    {
     const char *tag_list_plural[] = {"p", "pl", "plural", NULL};
     const char *tag_list_female[] = {"f", "female", NULL};
     const char *tag_list_male[] = {"m", "male", NULL};
     const char *tag_list_other[] = {"i", "it", "o", "other",
                                     "v", "void", NULL};
     const char *tag_list_me[] = {"me", NULL};
     const char *tag_list_valid[] = {
      DEFAULT_TAG_STRING, "f", "female", "i", "it", "m", "male", "me",
      "o", "other", "p", "pl", "plural", "v", "void", NULL};
     const char *start_tags = NULL;
     int tag_list_size = 0;
     int name_len = 0;
     
     SKIP_MATCHED_TO(input, "Gender");
    
     if (!(cmp_sve = BEG_CONST_STRCMP("_show", *input)))
     {
      const char *the_string = NULL;
      
      SKIP_MATCHED_TO(input, "_show");

      output_variables_player_load(object);
      
      if (!P_IS_AVL(object) ||
          ((object->player_ptr == p) && !info->output_not_me))
        the_string = "you";
      else
        the_string = gender_choose_str(object->player_ptr->gender,
                                       "he", "she", "they", "it");
     
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               the_string, input, length, out_node));
     }
    
     if ((tag_list_size = check_tag_string(from, p, info, flags, timestamp,
                                           input, &start_tags, tag_list_valid,
                                           "Gender", length, &out_node)) < 0)
       return (out_node);
     else if (!tag_list_size)
       return (string_variables(from, p, info, flags, timestamp, name_len,
                                "", input, length, out_node));
         
     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       if ((object->player_ptr != p) || info->output_not_me ||
           ((name_len = tag_get_name(tag_list_me, &start_tags,
                                     tag_list_size)) < 0))
         /* use value if not person ... or a tag ME isn't specified */
         switch (object->player_ptr->gender)
         {
          case GENDER_MALE:
            name_len = tag_get_name(tag_list_male, &start_tags, tag_list_size);
            break;
          case GENDER_FEMALE:
            name_len = tag_get_name(tag_list_female, &start_tags,
                                    tag_list_size);
            break;
          case GENDER_PLURAL:
            name_len = tag_get_name(tag_list_plural, &start_tags,
                                    tag_list_size);
            break;
            
          default:
            assert(FALSE);
          case GENDER_OTHER:
            name_len = tag_get_name(tag_list_other, &start_tags,
                                    tag_list_size);
            break;          
         }
    
     CHECK_TAG_STRING_END(&start_tags, tag_list_size, name_len);
     
     return (string_variables(from, p, info, flags | OUTPUT_IN_TAG, timestamp,
                              name_len, start_tags, input, length, out_node));
    }
    break;

  case 'H':
    break;
    
  case 'I':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Idle", *input)))
    { /* NOTE: is this possible another way (exam)? */
     int time_idle = 0;
     int tot_ans = -1;
     
     if (P_IS_ON(object))
       time_idle = difftime(now, object->player_ptr->last_command_timestamp);
     
     SKIP_MATCHED_TO(input, "Idle");     
     
     if (!BEG_CONST_STRCMP("_time", *input))
     {
      SKIP_MATCHED_TO(input, "_time");
      NEED_CONNECTOR(input, from, p, flags, "Idle_time", length, out_node);
     }
     else if (!((tot_ans = BEG_CONST_STRCMP("_total", *input)) &&
                BEG_CONST_STRCMP("_login", *input) &&
                BEG_CONST_STRCMP("_logon", *input)))
     {
      if (!tot_ans)
      {
       SKIP_MATCHED_TO(input, "_total");
       NEED_CONNECTOR(input, from, p, flags, "Idle_total", length, out_node);
      }
      else
      {
       SKIP_MATCHED_TO(input, "_logon");
       NEED_CONNECTOR(input, from, p, flags, "Idle_logon", length, out_node);
      }
     
      if (time_idle < IDLE_TIME_PERMITTED)
        time_idle = 0;
      
      if (!tot_ans)
        time_idle += object->total_idle_logon;
     
      if (P_IS_ON(object))
        time_idle += object->player_ptr->idle_logon;
     }
     else
       NEED_CONNECTOR(input, from, p, flags, "Idle_time", length, out_node);
     
     return (seconds_variables(from, p, info, flags, timestamp, 
                               time_idle, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Internet_address", *input)))
    {
     SKIP_MATCHED_TO(input, "Internet_address");
     OUT_VAR_DEPRECIATED(from, p, "Internet_address",
                         ", Use Dns_address instead");
     goto tag_dns_address;
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Ip_address", *input)))
    {
     SKIP_MATCHED_TO(input, "Ip_address");

     output_variables_player_load(object);
     
     if (P_IS_AVL(object) &&
         ((object == p->saved) || p->saved->priv_command_trace ||
          (flags & OUTPUT_VARIABLES_NO_CHECKS)))
     {
      char buffer[sizeof("255.255.255.255")];

      sprintf(buffer, "%d.%d.%d.%d",
              (int)object->player_ptr->ip_address[0],
              (int)object->player_ptr->ip_address[1],
              (int)object->player_ptr->ip_address[2],
              (int)object->player_ptr->ip_address[3]);

      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               buffer,
                               input, length, out_node));
     }
     else
       return (string_variables(from, p, info, flags, timestamp, INT_MAX, "",
                                input, length, out_node));
    }
    break;

  case 'J':
    break;
    
  case 'K':
    if ((cmp_sve < 1) &&
        !(cmp_sve = BEG_CONST_STRCMP("Karma_value", *input)))
    {
     SKIP_MATCHED_TO(input, "Karma_value");
     return (number_variables(from, p, info, flags, timestamp,
                              object->karma_value, input, length, out_node));
    }
    break;
   
  case 'L':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Last_on", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Last_on",
                               length, out_node);
     OUT_VAR_DEPRECIATED(from, p, "Last_on",
                         ", Use Loggoff_timestamp instead");
     return (time_variables(from, p, info, flags, timestamp,
                            object->logoff_timestamp,
                            input, length, out_node));
    }
    else if ((!(cmp_sve = BEG_CONST_STRCMP("Logoff_timestamp", *input)) ||
              !(cmp_sve = BEG_CONST_STRCMP("Logout_timestamp", *input))))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Logoff_timestamp",
                               length, out_node);
     
     return (time_variables(from, p, info, flags, timestamp,
                            object->logoff_timestamp,
                            input, length, out_node));
    }
    else if ((!(cmp_sve = BEG_CONST_STRCMP("Login", *input)) ||
              !(cmp_sve = BEG_CONST_STRCMP("Logon", *input))))
    {
     int the_logon_time = 0;

     SKIP_MATCHED_TO(input, "Logon"); /* login is same length */

     if (!BEG_CONST_STRCMP("_total", *input))
     {
      SKIP_MATCHED_TO(input, "_total");
      NEED_CONNECTOR(input, from, p, flags, "Logon_total", length, out_node);

      the_logon_time = real_total_logon_time(object);
           
      if ((flags & OUTPUT_VARIABLES_NO_CHECKS) || p->saved->priv_admin ||
          (from == object) || !object->flag_hide_logon_time)
        return (seconds_variables(from, p, info, flags, timestamp,
                                  the_logon_time, input, length, out_node));
      else
        return (seconds_variables(from, p, info, flags, timestamp, 0,
                                  input, length, out_node));
     }
     else if (!BEG_CONST_STRCMP("_timestamp", *input))
     {
      NEED_CONNECTOR(input, from, p, flags, "Logon_timestamp",
                     length, out_node);

      return (time_variables(from, p, info, flags, timestamp,
                             P_IS_ON(object) ?
                             object->player_ptr->logon_timestamp :
                             object->logoff_timestamp,
                             input, length, out_node));
     }
     else if (!BEG_CONST_STRCMP("_time", *input))
       SKIP_MATCHED_TO(input, "_time");
     
     NEED_CONNECTOR(input, from, p, flags, "Logon_time", length, out_node);

     if (P_IS_ON(object))
       the_logon_time = difftime(now, object->player_ptr->logon_timestamp);

     return (seconds_variables(from, p, info, flags, timestamp, the_logon_time,
                               input, length, out_node));
    }
    break;

  case 'M':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Mail", *input)))
    {
     unsigned int mail_messages = 0;

     SKIP_MATCHED_TO(input, "Mail");
         
     if (!BEG_CONST_STRCMP("_sent", *input))
     {
      SKIP_MATCHED_TO(input, "_sent");
      if ((flags & OUTPUT_VARIABLES_NO_CHECKS) || (object == from))
        mail_messages = mail_check_mailout_size(object);
     }
     else if (!BEG_CONST_STRCMP("_unread", *input))
     {
      SKIP_MATCHED_TO(input, "_unread");
      if ((flags & OUTPUT_VARIABLES_NO_CHECKS) || (object == from))
        mail_messages = mail_check_mailunread_size(object);
     }
     else
     {
      if (!BEG_CONST_STRCMP("_recieved", *input))
        SKIP_MATCHED_TO(input, "_recieved");
      /* allow anyone to see it... */
      mail_messages = mail_check_mailbox_size(object);
     }
    
     return (number_variables(from, p, info, flags, timestamp,
                              mail_messages, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Married_to", *input)))
    {
     SKIP_MATCHED_TO(input, "Married_to");
     
     output_variables_player_load(object);
     
     if (P_IS_AVL(object) && object->player_ptr->flag_married &&
         ((from != object) && !(flags & OUTPUT_VARIABLES_NO_CHECKS) &&
          !object->player_ptr->flag_marriage_hide))
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                object->player_ptr->married_to,
                                input, length, out_node));
     else
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                "", input, length, out_node)); 
    }
    break;
   
  case 'N':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Name", *input)))
    {
     const char *the_string = object->name;
     
     SKIP_MATCHED_TO(input, "Name");

     if (!BEG_CONST_STRCMP("_low", *input))
     {
      SKIP_MATCHED_TO(input, "_low");
      if (!BEG_CONST_STRCMP("er", *input))
        SKIP_MATCHED_TO(input, "er");
      the_string = object->lower_name;
     }
     else if (!BEG_CONST_STRCMP("_full", *input))
     { /* shortcut for using name and prefix */
      char tmp_name[PLAYER_S_NAME_SZ + PLAYER_S_PREFIX_SZ + 1];
      
      SKIP_MATCHED_TO(input, "_full");

      output_variables_player_load(object);
      
      if (P_IS_AVL(object) && object->player_ptr->prefix[0])
      {
       sprintf(tmp_name, "%s %s", object->player_ptr->prefix, object->name);
       return (string_variables(from, p, info, flags | RAW_OUTPUT,
                                timestamp, INT_MAX, tmp_name, input,
                                length, out_node));
      }
     }
     
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              the_string, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Nationality", *input)))
    {
     const char *tag_list_british[] = {"british", "en", "uk", NULL};
     const char *tag_list_american[] = {"american", "us", "usa", NULL};
     const char *tag_list_canadian[] = {"ca", "can", "canadian", NULL};
     const char *tag_list_australian[] = {"au", "aus", "australian", NULL};
     const char *tag_list_other[] = {"other", NULL};
     const char *tag_list_void[] = {"void", NULL};
     const char *tag_list_valid[] = {
      "american", "au", "aus", "australian", "british",
      "ca", "can", "canadian", DEFAULT_TAG_STRING, "en",
      "other", "uk", "us", "usa", "void", NULL};
     const char *start_tags = NULL;
     int tag_list_size = 0;
     int name_len = 0;
     
     SKIP_MATCHED_TO(input, "Nationality");
    
     if ((tag_list_size = check_tag_string(from, p, info, flags, timestamp,
                                           input, &start_tags, tag_list_valid,
                                           "Nationality", length,
                                           &out_node)) < 0)
       return (out_node);    
     else if (!tag_list_size)
       return (string_variables(from, p, info, flags, timestamp, name_len,
                                "", input, length, out_node));

     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       switch (object->player_ptr->nationality)
       {
        case NATIONALITY_VOID:
          name_len = tag_get_name(tag_list_void, &start_tags, tag_list_size);
          break;
        case NATIONALITY_BRITISH:
          name_len = tag_get_name(tag_list_british, &start_tags,
                                  tag_list_size);
          break;
        case NATIONALITY_AMERICAN:
          name_len = tag_get_name(tag_list_american, &start_tags,
                                  tag_list_size);
          break;
        case NATIONALITY_CANADIAN:
          name_len = tag_get_name(tag_list_canadian, &start_tags,
                                  tag_list_size);
          break;
        case NATIONALITY_AUSTRALIAN:
          name_len = tag_get_name(tag_list_australian, &start_tags,
                                  tag_list_size);
          break;
        case NATIONALITY_OTHER:
          name_len = tag_get_name(tag_list_other, &start_tags, tag_list_size);
          break;
        default:
          assert(FALSE);
       }
     
     CHECK_TAG_STRING_END(&start_tags, tag_list_size, name_len);
     
     return (string_variables(from, p, info, flags | OUTPUT_IN_TAG, timestamp,
                              name_len, start_tags, input, length, out_node));
    }
    break;

  case 'O':
    break;
   
  case 'P':
    if ((cmp_sve < 1) &&
        !(cmp_sve = BEG_CONST_STRCMP("Phone_numbers", *input)))
    {
     SKIP_MATCHED_TO(input, "Phone_numbers");

     output_variables_player_load(object);
     
     if (P_IS_AVL(object) && from)
     {
      LIST_SELF_CHECK_FLAG_START(object->player_ptr, from);
      if ((from == object) || LIST_SELF_CHECK_FLAG_DO(friend))
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                object->player_ptr->phone_numbers, input,
                                length, out_node));
      LIST_SELF_CHECK_FLAG_END();
     }
     
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              "", input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Plan", *input)))
    {
     static unsigned int allow_twinkle_depth = OUTPUT_SUB_TWINKLE_DEPTH;
     SKIP_MATCHED_TO(input, "Plan");

     output_variables_player_load(object);
     
     if (P_IS_AVL(object) && allow_twinkle_depth)
     {
      --allow_twinkle_depth;
      out_node = string_variables(from, p, info, flags, timestamp, INT_MAX,
                                  object->player_ptr->plan, input,
                                  length, out_node);
      ++allow_twinkle_depth;
     }
     else
       out_node = string_variables(from, p, info, flags, timestamp, INT_MAX,
                                   "", input, length, out_node);
     return (out_node);
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Plural", *input)))
    {
     const char *the_string = "";
     
     SKIP_MATCHED_TO(input, "Plural");

     output_variables_player_load(object);
     
     if (P_IS_AVL(object) && (object->player_ptr->gender == GENDER_PLURAL))
       the_string = "s";
    
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              the_string, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Possessive", *input)))
    {
     const char *the_string = NULL;
     
     SKIP_MATCHED_TO(input, "Possessive");

     output_variables_player_load(object);
     
     if (!P_IS_AVL(object) ||
         ((object->player_ptr == p) && !info->output_not_me))
       the_string = "your";
     else
       the_string = gender_choose_str(object->player_ptr->gender,
                               "his", "her", "their", "its");
    
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              the_string, input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Prefix", *input)))
    {
     SKIP_MATCHED_TO(input, "Prefix");

     output_variables_player_load(object);
     
     if (P_IS_AVL(object))
       return (string_variables(from, p, info, flags | RAW_OUTPUT,
                                timestamp, INT_MAX, object->player_ptr->prefix,
                                input, length, out_node));
     else
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                "", input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Privs", *input)))
    {
     const char *tag_list_admin[] = {"ad", "admin", NULL};
     const char *tag_list_lower_admin[] = {"lad", "lower admin", NULL};
     const char *tag_list_senior_su[] = {"senior super user", "ssu", NULL};
     const char *tag_list_su[] = {"su", "super user", NULL};
     const char *tag_list_coder[] = {"c", "coder", NULL};
     const char *tag_list_bsu[] = {"basic super user", "bsu", NULL};
     const char *tag_list_spod[] = {"spod", "s", NULL};
     const char *tag_list_minister[] = {"min", "minister", NULL};
     /* equals groups */
     const char *tag_list_admin_eq[] = {"ad=", "admin=", NULL};
     const char *tag_list_lower_admin_eq[] = {"lad=", "lower admin=", NULL};
     const char *tag_list_senior_su_eq[] = {"senior super user=", "ssu=",NULL};
     const char *tag_list_su_eq[] = {"su=", "super user=", NULL};
     const char *tag_list_bsu_eq[] = {"basic super user=", "bsu=", NULL};
     const char *tag_list_valid[] = {
      /* NOTE: this assumes that '=' is less than all the characters,
         which it is under ASCII */
      "ad", "ad=", "admin", "admin=", "basic super user", "basic super user=",
      "bsu", "bsu=", "coder", "c", DEFAULT_TAG_STRING,
      "lad", "lower admin", "min", "minister", "s", "spod",
      "senior super user", "senior super user=", "ssu", "ssu=",
      "su", "su=", "super user", "super user=", NULL};
     int name_len = 0;
     const char *start_tags = NULL;
     int tag_list_size = 0;
    
     SKIP_MATCHED_TO(input, "Privs");
    
     if ((tag_list_size = check_tag_string(from, p, info, flags, timestamp,
                                           input, &start_tags, tag_list_valid,
                                           "Privs", length, &out_node)) < 0)
       return (out_node);
     else if (!tag_list_size)
       return (string_variables(from, p, info, flags, timestamp, name_len,
                                "", input, length, out_node));

     /* do the equals ones first */
     TAG_PRIV_EQ_START();
     /* can't have ;'s on the PRIV_EQUAL macros */
     TAG_PRIV_EQUAL(object->priv_admin, tag_list_admin_eq)
     TAG_PRIV_EQUAL(object->priv_lower_admin, tag_list_lower_admin_eq)
     TAG_PRIV_EQUAL(object->priv_senior_su, tag_list_senior_su_eq)
     TAG_PRIV_EQUAL(object->priv_normal_su, tag_list_su_eq)
     TAG_PRIV_EQUAL(PRIV_STAFF(object), tag_list_bsu_eq)
     /* spod/minister equal would be added here */
     TAG_PRIV_EQ_END();

     TAG_PRIV(object->priv_admin, tag_list_admin);
     TAG_PRIV(object->priv_lower_admin, tag_list_lower_admin);
     TAG_PRIV(object->priv_senior_su, tag_list_senior_su);
     TAG_PRIV(object->priv_normal_su, tag_list_su);
     TAG_PRIV(object->priv_coder, tag_list_coder);
     TAG_PRIV(object->priv_basic_su, tag_list_bsu);
     TAG_PRIV(object->priv_minister, tag_list_minister);
     TAG_PRIV(object->priv_spod, tag_list_spod);
    
     CHECK_TAG_STRING_END(&start_tags, tag_list_size, name_len);
     
     return (string_variables(from, p, info, flags, timestamp, name_len,
                              start_tags, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Proposed_to", *input)))
    {
     SKIP_MATCHED_TO(input, "Proposed_to");
     
     output_variables_player_load(object);
     
     if (P_IS_AVL(object) && !object->player_ptr->flag_married &&
         !object->player_ptr->flag_marriage_hide)
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                object->player_ptr->married_to,
                                input, length, out_node));
     else
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                "", input, length, out_node)); 
    }
    break;

  case 'Q':
    break;
   
  case 'R':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Ressed_on", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Ressed_on",
                               length, out_node);
     
     if (!(flags & OUTPUT_VARIABLES_NO_CHECKS) &&
         (object != from) && !PRIV_STAFF(p->saved) &&
         !((difftime(now, object->c_timestamp) > MK_DAYS(30)) &&
           (object->total_logon > MK_DAYS(1))))
       return (time_variables(from, p, info, flags, timestamp, now,
                              input, length, out_node));
     else
       return (time_variables(from, p, info, flags, timestamp,
                              object->c_timestamp,
                              input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Room", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Room",
                               length, out_node);
    
     if (!P_IS_ON(object) ||
         (!(flags & OUTPUT_VARIABLES_NO_CHECKS) &&
          !room_can_see_location(p, object->player_ptr)))
     {
      room *tmp_room = room_load_find(NULL, "system.void",
                                      PLAYER_FIND_DEFAULT);

      assert(tmp_room); /* it better or deleting rooms is byebye */
      return (room_variables(from, p, info, flags, timestamp,
                             tmp_room ? tmp_room : p->location,
                             input, length, out_node));
     }
     else
       return (room_variables(from, p, info, flags, timestamp,
                              object->player_ptr->location,
                              input, length, out_node));
    }
    break;
   
  case 'S':
    if ((cmp_sve < 1) &&
        !(cmp_sve = BEG_CONST_STRCMP("Schedule_time", *input)))
    {
     SKIP_MATCHED_TO(input, "Schedule_time");
     return (number_variables(from, p, info, flags, timestamp,
                              P_IS_ON(object) ?
                              object->player_ptr->schedule_time : 0,
                              input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Set", *input)))
    {
     SKIP_MATCHED_TO(input, "Set");
     if (!BEG_CONST_STRCMP("ting", *input))
       SKIP_MATCHED_TO(input, "ting");

     NEED_CONNECTOR(input, from, p, flags, "Setting", length, out_node);
     return (toggle_player_variables(from, p, info, flags, timestamp,
                                     object, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Spodnumber", *input)))
    {
     SKIP_MATCHED_TO(input, "Spodnumber");
    
     if ((flags & OUTPUT_VARIABLES_NO_CHECKS) ||
         (p == object->player_ptr) || !object->flag_kill_logon_time)
     {
      spodlist_check_order(object, SPOD_CHECK_EXTERNAL, SPOD_CHECK_DEF_LEVEL);
      return (number_variables(from, p, info, flags, timestamp,
                               object->spod_number - object->spod_offset,
                               input, length, out_node));
     }
     else
       return (number_variables(from, p, info, flags, timestamp,
                                0, input, length, out_node));
    }
    break;

  case 'T':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Terminal", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Terminal",
                               length, out_node);

     if (**input == '?')
     {
      ++*input;
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               "Ansi_override, Height, Name, Name_automatic, Size_automatic, Width",
                               input, length, out_node));
     }

     if ((cmp_sve < 1) &&
         !(cmp_sve = BEG_CONST_STRCMP("Ansi_override", *input)))
     {
      SKIP_MATCHED_TO(input, "Ansi_override");
      
      output_variables_player_load(object);
      
      if (P_IS_AVL(object))
        return (toggle_variables(from, p, info, flags, timestamp,
                                 object->player_ptr->flag_terminal_ansi_colour_override,
                                 input, length, out_node));
      else
        return (toggle_variables(from, p, info, flags, timestamp,
                                 FALSE, input, length, out_node));
     }
     else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Height", *input)))
     {
      SKIP_MATCHED_TO(input, "Height");

      output_variables_player_load(object);
      
      if (P_IS_AVL(object))
        return (number_variables(from, p, info, flags, timestamp,
                                 object->player_ptr->term_height,
                                 input, length, out_node));
      else
        return (number_variables(from, p, info, flags, timestamp,
                                 0, input, length, out_node));
     }
     else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Name", *input)))
     {
      SKIP_MATCHED_TO(input, "Name");
      
      if ((cmp_sve < 1) &&
          !(cmp_sve = BEG_CONST_STRCMP("_automatic", *input)))
      {
       SKIP_MATCHED_TO(input, "_automatic");
       
       if (P_IS_ON(object))
         return (toggle_variables(from, p, info, flags, timestamp,
                                  object->player_ptr->automatic_term_name_got,
                                  input, length, out_node));
       else
         return (toggle_variables(from, p, info, flags, timestamp,
                                  FALSE, input, length, out_node));
      }
      
      if (P_IS_ON(object) && object->player_ptr->termcap)
        return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                 object->player_ptr->termcap->name,
                                 input, length, out_node));
      else
        return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                 "", input, length, out_node));
     }
     else if ((cmp_sve < 1) &&
              !(cmp_sve = BEG_CONST_STRCMP("Size_automatic", *input)))
     {
      SKIP_MATCHED_TO(input, "Size_automatic");
      
      if (P_IS_ON(object))
        return (toggle_variables(from, p, info, flags, timestamp,
                                 object->player_ptr->automatic_term_size_got,
                                 input, length, out_node));
      else
        return (toggle_variables(from, p, info, flags, timestamp,
                                 FALSE, input, length, out_node));
     }
     else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Width", *input)))
     {
      SKIP_MATCHED_TO(input, "Width");

      output_variables_player_load(object);
      
      if (P_IS_AVL(object))
        return (number_variables(from, p, info, flags, timestamp,
                                 object->player_ptr->term_width,
                                 input, length, out_node));
      else
        return (number_variables(from, p, info, flags, timestamp,
                                 0, input, length, out_node));
     }

     tmp = "<Bad Formatting (Terminal)>";
     info->used_twinkles = TRUE;
     return (output_string(from, p, info, flags, timestamp, &tmp, INT_MAX,
                           length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Time", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Time",
                               length, out_node);

     output_variables_player_load(object);
     
     if (P_IS_AVL(object) &&
         ((flags & OUTPUT_VARIABLES_NO_CHECKS) ||
          !object->player_ptr->flag_gmt_offset_hide ||
          (object->player_ptr == p)))
       return (time_variables(from, p, info, flags, timestamp,
                              disp_time_add(now,
                                            object->player_ptr->gmt_offset),
                              input, length, out_node));
     else
       return (time_variables(from, p, info, flags, timestamp, now,
                              input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Title", *input)))
    {
     static unsigned int allow_twinkle_depth = OUTPUT_SUB_TWINKLE_DEPTH;
    
     SKIP_MATCHED_TO(input, "Title");

     output_variables_player_load(object);
     
     if (P_IS_AVL(object) && allow_twinkle_depth)
     {
      --allow_twinkle_depth;
      if (!BEG_CONST_STRCMP("_with_name", *input))
      {
       char buffer[PLAYER_S_NAME_SZ + PLAYER_S_TITLE_SZ + 2];
       sprintf(buffer, "%s%s%s", object->name,
               isits1(object->player_ptr->title),
               isits2(object->player_ptr->title));
       out_node = string_variables(from, p, info, flags, timestamp, INT_MAX,
                                   buffer, input, length, out_node);       
      }
      else
        out_node = string_variables(from, p, info, flags, timestamp, INT_MAX,
                                    object->player_ptr->title, input,
                                    length, out_node);
      ++allow_twinkle_depth;
     }
     else
       out_node = string_variables(from, p, info, flags, timestamp, INT_MAX,
                                   "", input, length, out_node);
     return (out_node);
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Typed_commands", *input)))
    {
     SKIP_MATCHED_TO(input, "Typed_commands");
     return (number_variables(from, p, info, flags, timestamp,
                              p->typed_commands, input, length, out_node));
    }
    break;
   
  case 'U':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Url", *input)))
    {
     SKIP_MATCHED_TO(input, "Url");

     output_variables_player_load(object);
     
     if (P_IS_AVL(object) && object->player_ptr->url[0])
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                object->player_ptr->url, input,
                                length, out_node));
     else
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                "", input, length, out_node));
    }
    break;

  case 'V':
  case 'W':
  case 'X':
  case 'Y':
  case 'Z':
    break;
     
  default:
    break;
 }
 
 tmp = "<Bad Formatting (Player)>";
 info->used_twinkles = TRUE;
   
 return (output_string(from, p, info, flags, timestamp, &tmp, INT_MAX,
                       length, out_node));
}

static void internal_fix_bug(player *p, va_list ap)
{
 int *count = va_arg(ap, int *);

 ++*count;

 /* we _keep_ needing to do stuff like this :O */
 if (p->max_mails < 50)
 {
  p->max_mails = 50;
  p->saved->flag_tmp_player_needs_saving = TRUE;
 }
 
 if (p->max_aliases < 32)
 {
  p->max_aliases = 32;
  p->saved->flag_tmp_player_needs_saving = TRUE;
 }
 
 if (p->max_rooms < 4)
 {
  p->max_rooms = 4;
  p->saved->flag_tmp_player_needs_saving = TRUE;
 }

 if (p->saved->total_idle_logon > p->saved->total_logon)
 {
  p->saved->total_idle_logon = 0;
  p->saved->flag_tmp_player_needs_saving = TRUE;
 }
 
}

static output_node *talker_variables(player_tree_node *from, player *p,
                                     twinkle_info *info,
                                     int flags, time_t timestamp,
                                     const char **input,
                                     int *length, output_node *out_node)
{
 const char *tmp = NULL;
 int cmp_sve = -1;
 
 BTRACE("talker_variables");
 
 switch (**input)
 {
  case '?':
    ++*input;
    return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                             "Address, Angel_started, Boots, Email_abuse, Email_admin, Email_long, Email_short, Email_sus, Logging_on_now, Name_long, Name_short, Pcre_Port, Stack_grown_size, Stack_max_size, Started, Telnet, Uptime, Version_compiled, Version_packaged, Version_pcre, Web, Www",
                             input, length, out_node));
    
  case 'A':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Address", *input)))
    {
     SKIP_MATCHED_TO(input, "Address");
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              "crazylands.org", input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Angel_started", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Angel_started",
                               length, out_node);
     return (time_variables(from, p, info, flags, timestamp, angel_started,
                            input, length, out_node));
    }
    break;

  case 'B':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Boots", *input)))
    {
     SKIP_MATCHED_TO(input, "Boots");
     return (number_variables(from, p, info, flags, timestamp,
                              total_crashes, input, length, out_node));
    }
    break;

  case 'E':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Email", *input)))
    {
     SKIP_MATCHED_TO(input, "Email");
     
     if (!BEG_CONST_STRCMP("_abuse", *input))
     {
      SKIP_MATCHED_TO(input, "_abuse");
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               configure.email_to_abuse,
                               input, length, out_node));
     }
     else if (!BEG_CONST_STRCMP("_admin", *input))
     {
      SKIP_MATCHED_TO(input, "_admin");
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               configure.email_to_admin,
                               input, length, out_node));
     }
     else if (!BEG_CONST_STRCMP("_sus", *input))
     {
      SKIP_MATCHED_TO(input, "_sus");
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               configure.email_to_sus,
                               input, length, out_node));
     }
     else if (!BEG_CONST_STRCMP("_short", *input))
     {
      SKIP_MATCHED_TO(input, "_short");
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               configure.email_from_short,
                               input, length, out_node));
     }

     if (!BEG_CONST_STRCMP("_long", *input))
       SKIP_MATCHED_TO(input, "_long");
     
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              configure.email_from_long,
                              input, length, out_node));
    }
    break;

  case 'F':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Fix_bug", *input)))
    {
     int count = 0;
     SKIP_MATCHED_TO(input, "Fix_bug");
     if (p->saved->priv_admin)
       /* FIXME: bit of a hack so we don't use a cmd slot --
        * takes wayyy too long*/
       do_inorder_all_load(internal_fix_bug, &count);
     
     return (number_variables(from, p, info, flags, timestamp,
                              count, input, length, out_node));
    }    
    break;
    
  case 'L':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Listen_size", *input)))
    {
     SKIP_MATCHED_TO(input, "Listen_size");
     return (number_variables(from, p, info, flags, timestamp,
                              configure.socket_listen_len,
                              input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Logging_on_now", *input)))
    {
     SKIP_MATCHED_TO(input, "Logging_on_now");
     return (number_variables(from, p, info, flags, timestamp,
                              logging_onto_count, input, length, out_node));
    }
    break;

  case 'M':
    if ((cmp_sve < 1) &&
        !(cmp_sve = BEG_CONST_STRCMP("Max_current_players", *input)))
    {
     SKIP_MATCHED_TO(input, "Max_current_players");
     return (number_variables(from, p, info, flags, timestamp,
                              auth_player_total_max, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Max_logging_on_now", *input)))
    {
     SKIP_MATCHED_TO(input, "Max_logging_on_now");
     return (number_variables(from, p, info, flags, timestamp,
                              auth_player_logging_on_max,
                              input, length, out_node));
    }
    break;
    
  case 'N':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Name", *input)))
    {
     SKIP_MATCHED_TO(input, "Name");

     if (!BEG_CONST_STRCMP("_admin", *input))
     {
      SKIP_MATCHED_TO(input, "_admin");
      
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               configure.player_name_admin,
                               input, length, out_node));
     }
     else if (!BEG_CONST_STRCMP("_ascii_art", *input) &&
              (!from || from == p->saved))
     {
      SKIP_MATCHED_TO(input, "_ascii_art");
      
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               configure.name_ascii_art,
                               input, length, out_node));
     }
     else if (!BEG_CONST_STRCMP("_long", *input))
     {
      SKIP_MATCHED_TO(input, "_long");

     def_talker_name:
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               configure.name_long,
                               input, length, out_node));
     }
     else if (!BEG_CONST_STRCMP("_short", *input))
     {
      SKIP_MATCHED_TO(input, "_short");
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               configure.name_short, input, length, out_node));
     }
     else
       goto def_talker_name;
    }
    break;

  case 'P':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Port", *input)))
    { /* first port == main one and one which intercom will use */
     configure_interface_node *inter = configure.socket_interfaces_start;
     log_assert(inter && (inter->type == CONFIGURE_INTERFACE_TYPE_ANY));
     SKIP_MATCHED_TO(input, "Port");
     return (number_variables(from, p, info, flags, timestamp, inter->port,
                              input, length, out_node));
    }
    break;
    
  case 'S':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Started", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Started",
                               length, out_node);
     return (time_variables(from, p, info, flags, timestamp, talker_started_on,
                            input, length, out_node));
    }
    break;
    
  case 'T':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Telnet", *input)))
    {
     SKIP_MATCHED_TO(input, "Telnet");
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              configure.url_access, input, length, out_node));
    }
    else if (!(cmp_sve = BEG_CONST_STRCMP("Time", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Time",
                               length, out_node);
     return (time_variables(from, p, info, flags, timestamp, now,
                            input, length, out_node));
    }
    break;
    
  case 'U':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Uptime", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Uptime",
                               length, out_node);
     return (seconds_variables(from, p, info, flags, timestamp,
                               difftime(now, talker_started_on),
                               input, length, out_node));
    }
    break;
    
  case 'V':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Version", *input)))
    {
     SKIP_MATCHED_TO(input, "Version");

     if (!BEG_CONST_STRCMP("_pcre", *input))
     {
      SKIP_MATCHED_TO(input, "_pcre");
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               pcre_version(), input, length, out_node));
     }
     else if (!BEG_CONST_STRCMP("_packaged", *input))
     {
      SKIP_MATCHED_TO(input, "_packaged");
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               TALKER_CODE_SNAPSHOT, input, length, out_node));
     }
     else if (!BEG_CONST_STRCMP("_compiled", *input))
       SKIP_MATCHED_TO(input, "_compiled");
     
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              VERSION, input, length, out_node));
    }
    break;
    
  case 'W':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Web", *input)))
    {
     SKIP_MATCHED_TO(input, "Web");
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              configure.url_web, input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Www", *input)))
    {
     SKIP_MATCHED_TO(input, "Www");
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              configure.url_web, input, length, out_node));
    }
    break;
    
  default:
    break;
 } 
 
 /* max_name .. title/diescription/plan/prompt */
 /* max_resis how many can connect at once */
 /* logging on people -- how may are in the process of logging on */
 /* max_multis */
 
 tmp = "<Bad Formatting (Talker)>";
 info->used_twinkles = TRUE;
 
 return (output_string(from, p, info, flags, timestamp, &tmp,
                       INT_MAX, length, out_node));
 
}

static output_node *room_variables(player_tree_node *from, player *p,
				   twinkle_info *info,
                                   int flags, time_t timestamp, room *object,
				   const char **input,
				   int *length, output_node *out_node)
{
 int cmp_sve = -1;
 const char *tmp = "<** ERROR **>";
 
 BTRACE("room_variables");
 
 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
  
 CHECK_OBJECT(object, p, flags, "<Bad room>", length, out_node);
 
 if (islower(**input))
   cmp_sve = 1;

 switch (**input)
 {
  case '?':
    ++*input;
    return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                             "Enter_msg, Gathering, Id, Occupants, Owner, Player_alpha, Player_cron, Player_user, Random_player, Where_description",
                             input, length, out_node));

  case 'E':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Enter_msg", *input)))
    {
     static unsigned int allow_twinkle_depth = OUTPUT_SUB_TWINKLE_DEPTH;
     
     SKIP_MATCHED_TO(input, "Enter_msg");
     
     if (allow_twinkle_depth && object->flag_tmp_in_core)
     {
      --allow_twinkle_depth;
      out_node = string_variables(from, p, info, flags, timestamp, INT_MAX,
                                  object->enter_msg, input,
                                  length, out_node);
      ++allow_twinkle_depth;
     }
     else
       out_node = string_variables(from, p, info, flags, timestamp, INT_MAX,
                                   "", input, length, out_node);
     return (out_node);
    }
    break;

  case 'G':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Gathering", *input)))
    {
     SKIP_MATCHED_TO(input, "Gathering");
     if (!(flags & OUTPUT_VARIABLES_NO_CHECKS) &&
         !room_can_enter(p, object, FALSE))
       return (max_number_tag_variables(from, p, info, flags, timestamp,
                                        0, input,
                                        "Gathering", length, out_node));
     else
       return (max_number_tag_variables(from, p, info, flags, timestamp,
                                        object->players_num, input,
                                        "Gathering", length, out_node));
    }
    break;
    
  case 'I':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Id", *input)))
    {
     SKIP_MATCHED_TO(input, "Id");
     return (string_variables(from, p, info, flags | RAW_OUTPUT_VARIABLES,
                              timestamp, INT_MAX, object->id, input,
                              length, out_node));
    }
    break;
    
  case 'N':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Name", *input)))
    {     
     SKIP_MATCHED_TO(input, "Name");
     OUT_VAR_DEPRECIATED(from, p, "Name", ", Use Where_description instead");

     goto where_description_tag;
    }
    break;
    
  case 'O':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Occupants", *input)))
    {
     SKIP_MATCHED_TO(input, "Occupants");

     if (!(flags & OUTPUT_VARIABLES_NO_CHECKS) &&
         !room_can_enter(p, object, FALSE))
       return (number_variables(from, p, info, flags, timestamp, 0,
                                input, length, out_node));
     else
       return (number_variables(from, p, info, flags, timestamp,
                                object->players_num,
                                input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Owner", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Owner", length,
                               out_node);
     return (player_variables(from, p, info, flags, timestamp,
                              object->owner, input, length, out_node));
    }
    break;
    
  case 'P':
    if (!(cmp_sve = BEG_CONST_STRCMP("Player", *input)))
    {
     static unsigned int player_twinkle_type = PLAYER_TWINKLE_DEFAULT;
     long the_number = 0;
     int name_len = 0;
     player_tree_node *the_player = NULL;
     
     SKIP_MATCHED_TO(input, "Player");
     if (!BEG_CONST_STRCMP("_alpha", *input))
     {
      player_twinkle_type = PLAYER_TWINKLE_ALPHA;
      SKIP_MATCHED_TO(input, "_alpha");
     }
     else if (!BEG_CONST_STRCMP("_cron", *input))
     {
      player_twinkle_type = PLAYER_TWINKLE_CRON;
      SKIP_MATCHED_TO(input, "_cron");
     }
     else if (!BEG_CONST_STRCMP("_user", *input))
     {
      if (p->flag_list_show_inorder)
        player_twinkle_type = PLAYER_TWINKLE_ALPHA;
      else
        player_twinkle_type = PLAYER_TWINKLE_CRON;
      SKIP_MATCHED_TO(input, "_user");
     }
     
     if (!(name_len = tag_get_bracket(*input)))
     {
      info->used_twinkles = TRUE;
      return (error_no_tags(from, p, info, flags, timestamp, "Player",
                            length, out_node));
     }
     else if (!(flags & OUTPUT_VARIABLES_NO_CHECKS) &&
              !room_can_enter(p, object, FALSE))
       goto dont_use_tag;
     else
     {
      const char *the_string = tag_skip_prefix(*input);
      the_number = buffer_toint(ALL_T(from, p, info, flags | OUTPUT_IN_TAG,
                                      timestamp),
                                name_len - ((the_string - *input) + 1),
                                the_string);
     }
     *input += name_len;
     
     if ((the_number < 1) ||
         !(the_player = find_player_room_count(
         (player_twinkle_type == PLAYER_TWINKLE_ALPHA), object, the_number)))
     {
     dont_use_tag:
      the_player = p->saved;
     }
     
     NEED_CONNECTOR(input, from, p, flags, "Player", length, out_node);
     return (player_variables(from, p, info, flags, timestamp, the_player,
                              input, length, out_node));
    }
    break;
    
    
  case 'R':
    if (!(cmp_sve = BEG_CONST_STRCMP("Random_player", *input)))
    {
     static unsigned int command_number_save = 0;
     static player_tree_node *output_player_save[10]; /* '0' - '9' */
     int random_player_offset = 0;
     
     SKIP_MATCHED_TO(input, "Random_player");
     
     if (isdigit((unsigned char) **input))
       random_player_offset = TONUMB(*(*input)++);
     
     if (command_number_save != current_command_number)
     {
      command_number_save = current_command_number;
      memset(output_player_save, 0, sizeof(output_player_save));
     }

     if (!(flags & OUTPUT_VARIABLES_NO_CHECKS) &&
         !room_can_enter(p, object, FALSE))
     {
      NEED_CONNECTOR(input, from, p, flags, "Random_player", length, out_node);
      return (player_variables(from, p, info, flags, timestamp, p->saved,
                               input, length, out_node));
     }

     if (!output_player_save[random_player_offset])
       output_player_save[random_player_offset] = room_random_player(object);
     
     NEED_CONNECTOR(input, from, p, flags, "Random_player", length, out_node);
     return (player_variables(from, p, info, flags, timestamp,
                              output_player_save[random_player_offset],
                              input, length, out_node));
    }
    break;

  case 'W':
    if ((cmp_sve < 1) &&
        !(cmp_sve = BEG_CONST_STRCMP("Where_description", *input)))
    {
     static unsigned int allow_twinkle_depth = OUTPUT_SUB_TWINKLE_DEPTH;
     
     SKIP_MATCHED_TO(input, "Where_description");

    where_description_tag:
     if (allow_twinkle_depth && object->flag_tmp_in_core)
     {
      --allow_twinkle_depth;
      out_node = string_variables(from, p, info, flags, timestamp, INT_MAX,
                                  object->where_description, input,
                                  length, out_node);
      ++allow_twinkle_depth;
     }
     else
       out_node = string_variables(from, p, info, flags, timestamp, INT_MAX,
                                   "", input, length, out_node);
     return (out_node);
    }
    break;
    

  default:
    ;
 }
 
 tmp = "<Bad Formatting (Room)>";
 info->used_twinkles = TRUE;
 out_node = output_string(from, p, info, flags, timestamp, &tmp, INT_MAX,
                          length, out_node);
 
 return (out_node);
}

static output_node *math_tag_variables(player_tree_node *from, player *p,
                                       twinkle_info *info,
                                       int flags, time_t timestamp,
                                       int *length,
                                       const char *op_name,
                                       const char *tag_buffer_operator,
                                       int tag_list_size,
                                       output_node **tag_left_side,
                                       output_node **tag_right_side,
                                       output_node *out_node)
{
 const char *tag_list_operator_left[] = {"1", "l", "left", NULL};
 const char *tag_list_operator_right[] = {"2", "r", "right", NULL};
 tmp_output_list_storage tmp_save;
 int name_len = 0;
 const char *name_str = tag_buffer_operator;
 
 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
 assert(tag_buffer_operator &&
        tag_left_side && !*tag_left_side &&
        tag_right_side && !*tag_right_side);
 
 save_tmp_output_list(p, &tmp_save);
 if ((name_len = tag_get_name(tag_list_operator_left, &name_str,
                              tag_list_size)) >= 0)
 {
  fntell_player(ALL_T(from, p, info, flags | OUTPUT_IN_TAG |
                      OUTPUT_BUFFER_TMP, timestamp),
                name_len, INT_MAX, name_str);
  *tag_left_side = output_list_grab(p);

  name_str = tag_buffer_operator;
  if ((name_len = tag_get_name(tag_list_operator_right, &name_str,
                               tag_list_size)) >= 0)
  {
   fntell_player(ALL_T(from, p, info, flags | OUTPUT_IN_TAG |
                       OUTPUT_BUFFER_TMP, timestamp),
                 name_len, INT_MAX, name_str);
   *tag_right_side = output_list_grab(p);
   load_tmp_output_list(p, &tmp_save);
  }
  else
  {
   char error_buffer[64 + sizeof("%.*s (right condition)")];
   sprintf(error_buffer, "%.*s (right condition)", 63, op_name);
   
   output_list_cleanup(tag_left_side);
   load_tmp_output_list(p, &tmp_save);
   return (error_no_tags(from, p, info, flags, timestamp, error_buffer,
                         length, out_node));
  }
 }
 else
 {
  char error_buffer[64 + sizeof("%.*s (left condition)")];
  sprintf(error_buffer, "%.*s (left condition)", 63, op_name);
  
  load_tmp_output_list(p, &tmp_save);
  return (error_no_tags(from, p, info, flags, timestamp, error_buffer,
                        length, out_node));
 }

 return (out_node);
}

/* return (1) true, (-1) false, (0) error */
static int test_tag_variables(player_tree_node *from, player *p,
                              twinkle_info *info, int flags, time_t timestamp,
                              const char **input, int *length,
                              const char *op_name, const char **tag_out,
                              int *tag_len_out, output_node **out_node)
{
 /* has to be ASCII, yet again */
 const char *tag_list_equal[] = {"=", "==", "e", "equal", NULL};
 const char *tag_list_less[] = {"<", "l", "less", NULL};
 const char *tag_list_greater[] = {">", "g", "greater", NULL};
 const char *tag_list_lessequal[] = {"<=", "=<", "le", "lessequal", NULL};
 const char *tag_list_greaterequal[] = {"=>", ">=", "ge","greaterequal", NULL};
 const char *tag_list_true_false[2][4] =
 {{"0", "f", "false", NULL}, {"1", "t", "true", NULL}};
 const char *tag_list_else_if[] =
 {"2", "2_case", "2_num", "2_number",
  "elif", "elif_case", "elif_num", "elif_number",
  "elseif", "elseif_case", "elseif_num", "elseif_number", NULL};
 const char *tag_list_valid[] = {
  "0", "1", "2",  "2_case", "2_num", "2_number",
  "<", "<=", "=", "=<", "==", "=>", ">", ">=",
  "e", "elif", "elif_case", "elif_num", "elif_number",
  "elseif", "elseif_case", "elseif_num", "elseif_number", "equal",
  "f", "false", "l", "le", "less", "lessequal",
  "g", "ge", "greater", "greaterequal", "t", "true", NULL};
 
 const char *start_tags = NULL;
 const char *tmp = start_tags;
 int name_len = 0;
 /* function for comparing the left and right side of the operator */
 int (*tag_compare_func) (output_node *, output_node *) = output_list_cmp;
 /* output_lists ... for comparing left and right sides */
 int op_num = -1;
 int true_false[5][3] =
 {{FALSE, TRUE, FALSE}, /* = */
  {TRUE, FALSE, FALSE}, /* < */  {FALSE, FALSE, TRUE}, /* > */
  {TRUE, TRUE,  FALSE}, /* <= */ {FALSE, TRUE, TRUE} /* >= */};
 int cmp_save = 0;
 output_node *tag_left_side = NULL;
 output_node *tag_right_side = NULL;
 int tag_list_size = 0;

 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
 
 if (!BEG_CONST_STRCMP("_case", *input))
 {
  tag_compare_func = output_list_case_cmp;
  SKIP_MATCHED_TO(input, "_case");
 }
 else if (!BEG_CONST_STRCMP("_num", *input))
 {
  tag_compare_func = output_list_number_cmp;
  if (!BEG_CONST_STRCMP("_number", *input))
    SKIP_MATCHED_TO(input, "_number");
  else
    SKIP_MATCHED_TO(input, "_num");
 }

 if ((tag_list_size = check_tag_string(from, p, info, flags, timestamp, input,
                                       &start_tags, tag_list_valid,
                                       op_name, length, out_node)) <= 0)
   return (0);

 tmp = start_tags;
 *tag_out = start_tags;
 
 /* get the if operators in this order =, <, >, <=, >= */
 if ((name_len = tag_get_name(tag_list_equal, &tmp, tag_list_size)) >= 0)
   op_num = 0;
 else if ((name_len = tag_get_name(tag_list_less, &tmp, tag_list_size)) >= 0)
   op_num = 1;
 else if ((name_len = tag_get_name(tag_list_greater, &tmp,
                                   tag_list_size)) >= 0)
   op_num = 2;
 else if ((name_len = tag_get_name(tag_list_lessequal, &tmp,
                                   tag_list_size)) >= 0)
   op_num = 3;
 else if ((name_len = tag_get_name(tag_list_greaterequal, &tmp,
                                   tag_list_size)) >= 0)
   op_num = 4;
 else
 {
  char error_buffer[64 + sizeof("%.*s (operator)")];
  sprintf(error_buffer, "%.*s (operator)", 63, op_name);
  
  info->used_twinkles = TRUE;
  *out_node = error_no_tags(from, p, info, flags, timestamp,
                            error_buffer, length, *out_node);
  return (0);
 }
 
 assert(RANGE(op_num, 0, 4));

 *out_node = math_tag_variables(from, p, info, flags | OUTPUT_IN_TAG,
                                timestamp, length, op_name, tmp, name_len,
                                &tag_left_side, &tag_right_side, *out_node);
 if (!(tag_left_side && tag_right_side))
 {
  assert(!(tag_left_side || tag_right_side));
  info->used_twinkles = TRUE;
  return (0);
 }
 
 cmp_save = (*tag_compare_func) (tag_left_side, tag_right_side);
 output_list_cleanup(&tag_left_side);
 output_list_cleanup(&tag_right_side);
 
 if (cmp_save < 0) /* fix value for array index */
   cmp_save = 0;
 else if (!cmp_save)
   cmp_save = 1;
 else
   cmp_save = 2; 

 if (true_false[op_num][cmp_save])
 {
  name_len = tag_get_def_name(tag_list_true_false[TRUE], tag_out,
                              tag_list_size);
 }
 else if ((name_len = tag_get_name(tag_list_true_false[FALSE], tag_out,
                                   tag_list_size)) < 0)
 {
  const char *tag_start = NULL;
  int tag_matched = -1;
  
  if ((name_len = tag_get_name(tag_list_else_if, &start_tags,
                               tag_list_size)) >= 0)
  {
   assert((tag_matched >= 0) && /* next bit is off by one, but so what */
          ((size_t) tag_matched <
           (sizeof(tag_list_else_if) / sizeof(const char *))));
   
   if ((tmp = C_strchr(tag_list_else_if[tag_matched], '_')))
     tag_start = tmp;
   else
     tag_start += strlen(tag_list_else_if[tag_matched]);

   /* doesn't re-pass nae_len ... but that's ok ...
    * because it's already parsed it adn it didn't fail */
   return (test_tag_variables(from, p, info, flags, timestamp,
                              &tag_start, length, op_name,
                              tag_out, tag_len_out, out_node));
  }
  else
  {
   *tag_out = "";
   name_len = 0;
  }
 }
  
 *tag_len_out = name_len;
 return (true_false[op_num][cmp_save] ? (1) : (-1));
}



static output_node *variables_char_output(player_tree_node *from, player *p,
                                          twinkle_info *info,
                                          int flags, time_t timestamp, 
					  const char **input,
                                          int *length, output_node *out_node)
{ /* top of the output_variables functions... */
 int cmp_sve = -1;
 const char *tmp = "<** ERROR **>";

 BTRACE("variables_output");
 
 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
 assert(**input == '$');

 ++*input; /* get rid of that first one */

 switch (**input)
 {
  case '?':
    ++*input;
    return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                             "Addition, Character, Connected_total, Connected_uniq, Count, Current_command_name, Current_help_file_name, Current_players, Current_residents, Current_sub_command_name, Diff_time, Divide, From, Gathering, If, Integer, Line_fill, Logging_onto, Merge, Modulus, Multiply, Newbie_players, Number, Player_alpha, Player_cron, Player_name, Player_user, Power, Probability_diff, Probability_same, Raw, Random_resident, Recv, Return, Room, Seconds, Strlength, Subtract, Super_players, String, Talker, Uptime, While",
                             input, length, out_node));
    
  case 'A':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Add", *input)))
    {
     MATH_TAG_START("Add", "Addition");
     
     left_num = output_list_toint(tag_left_side);
     right_num = output_list_toint(tag_right_side);
     
     if (((left_num >= 0) && (right_num < 0)) ||
         ((left_num < 0) && (right_num >= 0)) ||
         ((left_num >= 0) && 
          (right_num <= (LONG_MAX - left_num))) ||
         ((right_num < 0) && 
          (right_num >= (LONG_MIN - left_num))))
       number = left_num + right_num;
     
     MATH_TAG_END();
     
     return (number_variables(from, p, info, flags, timestamp,
                              number, input, length, out_node));
    }
    break;

  case 'B':
    if (!(cmp_sve = BEG_CONST_STRCMP("Bell", *input)))
    {
     SKIP_MATCHED_TO(input, "Bell");

     if ((!from || (flags & OUTPUT_VARIABLES_NO_CHECKS) ||
          (from == p->saved)) &&
         p->flag_allow_bell)
     {
      if (p->flag_audible_bell)
        terminal_do_audible_bell(p, flags, &out_node);
      else
        terminal_do_visual_bell(p, flags, &out_node);
     }
     return (out_node);
    }
    break;
    
  case 'C':
    if (!(cmp_sve = BEG_CONST_STRCMP("Char", *input)))
    {
     const char *tag_list_string[] = {"s", "str", "string", NULL};
     const char *tag_list_offset[] = {"o", "offset", NULL};
     const char *tag_list_valid[] = {DEFAULT_TAG_STRING, "o", "offset",
                                     "s", "str", "string", NULL};
     int tag_list_size = 0;
     const char *start_tags = NULL;
     int name_len = 0;
     char the_string[] = {0, 0};
     int the_offset = 0;
     
     SKIP_MATCHED_TO(input, "Char");
     if (!BEG_CONST_STRCMP("acter", *input))
       SKIP_MATCHED_TO(input, "acter");

     if ((tag_list_size = check_tag_string(from, p, info, flags, timestamp,
                                           input, &start_tags, tag_list_valid,
                                           "Character", length,
                                           &out_node)) < 0)
       return (out_node);
     else if (!tag_list_size)
       return (string_variables(from, p, info, flags, timestamp, name_len,
                                "", input, length, out_node));

     tmp = start_tags;
     if ((name_len = tag_get_name(tag_list_offset, &tmp, tag_list_size)) >= 0)
       the_offset = atoi(tmp);
     
     if ((name_len = tag_get_name(tag_list_string, &start_tags,
                                  tag_list_size)) >= 0)
     {
      tmp_output_list_storage tmp_save;
      output_node *string_list = NULL;
       
      save_tmp_output_list(p, &tmp_save);
      fntell_player(ALL_T(from, p, info, flags | OUTPUT_IN_TAG |
                          OUTPUT_BUFFER_TMP, timestamp),
                    name_len, the_offset + 1, start_tags);
      string_list = output_list_grab(p);
      the_string[0] = output_list_get_char(string_list, the_offset);
      output_list_cleanup(&string_list);
      load_tmp_output_list(p, &tmp_save);
     }

     return (string_variables(from, p, info, flags, timestamp,
                              INT_MAX, the_string, input, length, out_node));
    }
    else if (!(cmp_sve = BEG_CONST_STRCMP("Command", *input)))
    {
     int name_len = 0;
     size_t cmd_number = 0;
     unsigned int cmd_array = 0;
     const char *cmd_name = "";
      
     SKIP_MATCHED_TO(input, "Command");
      
     if (!(name_len = tag_get_bracket(*input)))
     {
      info->used_twinkles = TRUE;
      return (error_no_tags(from, p, info, flags, timestamp, "Command",
                            length, out_node));
     }
     else
     {
      const char *the_string = tag_skip_prefix(*input);
      
      cmd_number = buffer_toint(ALL_T(from, p, info, flags | OUTPUT_IN_TAG,
                                      timestamp),
                                name_len - ((the_string - *input) + 1),
                                the_string);
     }
     *input += name_len;
     
     if (cmd_number >= 1)
       while (cmd_array < 26)
       {
        if (cmd_number <= cmds_alpha[cmd_array].size)
        {
         if ((*cmds_alpha[cmd_array].ptr[cmd_number]->test_can_run)(p->saved))
           cmd_name = cmds_alpha[cmd_array].ptr[cmd_number - 1]->name;
         break;
        }
        
        cmd_number -= cmds_alpha[cmd_array].size;
        ++cmd_array;
       }

     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              cmd_name, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Connected_new", *input)))
    {
    twinkle_connected_uniq:
     SKIP_MATCHED_TO(input, "Connected_");
     if (**input == 'u')
       SKIP_MATCHED_TO(input, "uniq");
     else
       SKIP_MATCHED_TO(input, "new");
     return (number_variables(from, p, info, flags, timestamp,
                              total_uniq_logons, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Connected_total", *input)))
    {
     SKIP_MATCHED_TO(input, "Connected_total");
     return (number_variables(from, p, info, flags, timestamp,
                              total_logons, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Connected_uniq", *input)))
    {
     goto twinkle_connected_uniq;
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Count", *input)))
    {
     const char *tag_list_valid[] = {"dec", "decrement", "id", "identity",
                                     "inc", "increment", "noprint",
                                     "postdec", "postdecrement",
                                     "postinc", "postincrement",
                                     "set", NULL};
     const char *start_tags = NULL;
     int tag_list_size = 0;
     long *counter = info->counter;
     int offset = 0;
     
     SKIP_MATCHED_TO(input, "Count");
     
     if (isdigit((unsigned char) **input))
     {
      offset = TONUMB(**input);
      ++*input;
     }
     
     if (from)
     {
      output_variables_player_load(from);
      if (P_IS_AVL(from)) /* FIXME: should this be _ON ? */
        counter = from->player_ptr->twinkle_counter;
     }
     
     if ((tag_list_size = check_tag_string(from, p, info, flags, timestamp,
                                           input, &start_tags, tag_list_valid,
                                           "Count", length, &out_node)) > 0)
     {
      const char *tag_list_dec[] = {"dec", "decrement", NULL};
      const char *tag_list_inc[] = {"inc", "increment", NULL};
      const char *tag_list_id[] = {"id", "identity", NULL};
      const char *tag_list_noprint[] = {"noprint", NULL};
      const char *tag_list_set[] = {"set", NULL};     
      const char *tag_list_postdec[] = {"postdec", "postdecrement", NULL};
      const char *tag_list_postinc[] = {"postinc", "postincrement", NULL};
      const char *name_str = start_tags;
      int name_len = 0;
      long number = 0;
      
      if ((name_len = tag_get_name(tag_list_id, &name_str,
                                   tag_list_size)) >= 0)
      {
       int new_offset = buffer_toint(ALL_T(from, p, info,
                                           flags | OUTPUT_IN_TAG, timestamp),
                                     name_len, name_str);
       if ((new_offset > 0) && (new_offset < OUTPUT_VARIABLES_COUNT_SZ))
         offset = new_offset;
      }
      name_str = start_tags;
      
      if ((name_len = tag_get_name(tag_list_set, &name_str,
                                   tag_list_size)) >= 0)
        counter[offset] = buffer_toint(ALL_T(from, p, info,
                                             flags | OUTPUT_IN_TAG,
                                             timestamp),
                                       name_len, name_str);
      name_str = start_tags;
      
      /* FIXME: could optimise ... as we don't need the count */
      if (tag_get_name(tag_list_inc, &name_str, tag_list_size) >= 0)
      {
       if (counter[offset] == LONG_MAX)
         counter[offset] = 0;
       else
         ++counter[offset];
      }
      else if (tag_get_name(tag_list_dec, &name_str, tag_list_size) >= 0)
      {
       if (counter[offset] == LONG_MIN)
         counter[offset] = 0;
       else
         --counter[offset];
      }
      name_str = start_tags;
      
      number = counter[offset];

      if (tag_get_name(tag_list_noprint, &name_str, tag_list_size) >= 0)
        info->used_twinkles = TRUE;
      else
        out_node = number_variables(from, p, info, flags, timestamp,
                                    number, input, length, out_node);
      
      if (tag_get_name(tag_list_postinc, &start_tags, tag_list_size) >= 0)
        ++counter[offset];
      else if (tag_get_name(tag_list_postdec, &start_tags, tag_list_size) >= 0)
        --counter[offset];
     }
     else if (!tag_list_size)
       out_node = number_variables(from, p, info, flags, timestamp,
                                   counter[offset], input, length, out_node);
     
     return (out_node);
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Current_command_name", *input)))
    {
     SKIP_MATCHED_TO(input, "Current_command_name");
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              current_command ? current_command : "",
                              input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Current_players", *input)))
    {
     SKIP_MATCHED_TO(input, "Current_players");
     return (number_variables(from, p, info, flags, timestamp,
                              current_players, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Current_residents", *input)))
    {
     SKIP_MATCHED_TO(input, "Current_residents");
     return (number_variables(from, p, info, flags, timestamp,
                              no_of_resis, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Current_sub_command_name",
                                   *input)))
    {
     SKIP_MATCHED_TO(input, "Current_sub_command_name");
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              current_sub_command ? current_sub_command : "",
                              input, length, out_node));
    }
    break;
    
  case 'D':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Date", *input)))
    {
     const char *the_string = NULL;
     time_t the_date = (time_t)-1;
     int name_len = 0;
     int tmp_count = 0;
     
     SKIP_MATCHED_TO(input, "Date");
     
     if (!(name_len = tag_get_bracket(*input)))
     {
      info->used_twinkles = TRUE;
      return (error_no_tags(from, p, info, flags, timestamp, "Date",
                            length, out_node));
     }

     the_string = tag_skip_prefix(*input);
     tmp_count = (the_string - *input) + 1;
     *input += name_len;
     name_len -= tmp_count;
     
     the_date = buffer_totime(ALL_T(from, p, info, flags | OUTPUT_IN_TAG,
                                    timestamp),
                              name_len, the_string);
     
     NEED_CONNECTOR(input, from, p, flags, "Date", length, out_node);
     
     return (time_variables(from, p, info, flags, timestamp,
                            the_date, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Diff_time", *input)))
    {
     const char *tag_list_valid[] = {"1", "2", "l", "left",
                                     "r", "right", NULL};
     output_node *tag_left_side = NULL;
     output_node *tag_right_side = NULL;
     const char *start_tags = NULL;
     int tag_list_size = 0;
     long seconds_difference = 0;
     time_t left_time = 0;
     time_t right_time = 0;
     
     SKIP_MATCHED_TO(input, "Diff_time");
     
     if ((tag_list_size = check_tag_string(from, p, info, flags, timestamp,
                                           input, &start_tags, tag_list_valid,
                                           "Diff_time", length,
                                           &out_node)) < 0)
       return (out_node);

     if (tag_list_size)
     {
      out_node = math_tag_variables(from, p, info, flags, timestamp,
                                    length, "Diff_time",
                                    start_tags, tag_list_size, &tag_left_side,
                                   &tag_right_side, out_node);
      
      if (!(tag_left_side && tag_right_side))
      {
       assert(!(tag_left_side || tag_right_side));
       info->used_twinkles = TRUE;
       return (out_node);
      }
      
      if ((left_time = output_list_totime(tag_left_side)) != -1)
        if ((right_time = output_list_totime(tag_right_side)) != -1)
          seconds_difference = difftime(left_time, right_time);
      output_list_cleanup(&tag_left_side);
      output_list_cleanup(&tag_right_side);
     }
     
     NEED_CONNECTOR(input, from, p, flags, "Diff_time", length, out_node);
     return (seconds_variables(from, p, info, flags, timestamp,
                               seconds_difference, input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Div", *input)))
    {
     MATH_TAG_START("Div", "Divide");
     
     left_num = output_list_toint(tag_left_side);
     right_num = output_list_toint(tag_right_side);
     
     if (left_num && right_num)
       number = left_num / right_num;
     
     MATH_TAG_END();

     return (number_variables(from, p, info, flags, timestamp,
                              number, input, length, out_node));
    }
    break;
    
  case 'F':
    if (!(cmp_sve = BEG_CONST_STRCMP("F", *input)))
    {
     if (!(cmp_sve = BEG_CONST_STRCMP("From", *input)))
       SKIP_MATCHED_TO(input, "From");
     else
       SKIP_MATCHED_TO(input, "F");
     NEED_CONNECTOR(input, from, p, flags, "From", length, out_node);
     
     if (from)
       return (player_variables(from, p, info, flags, timestamp, from, input,
                                length, out_node));
     else
       return (player_variables(from, p, info, flags, timestamp,
                                p->saved, input, length, out_node));
    }
    break;
    
  case 'G':
    if (!(cmp_sve = BEG_CONST_STRCMP("Gathering", *input)))
    {
     SKIP_MATCHED_TO(input, "Gathering");
     return (max_number_tag_variables(from, p, info, flags, timestamp,
                                      current_players, input,
                                      "Gathering", length, out_node));
    }
    break;

  case 'H':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Help",
                                                    *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Help",
                               length, out_node);

     if (**input == '?')
     {
      ++*input;
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               "Primary_name, Search_name, Shown_by",
                               input, length, out_node));
     }

     assert(!cmp_sve);
     if ((cmp_sve < 1) &&
         !(cmp_sve = BEG_CONST_STRCMP("Primary_name", *input)))
     {
      SKIP_MATCHED_TO(input, "Primary_name");
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               help_primary_name ? help_primary_name : "",
                               input, length, out_node));
     }
     else if ((cmp_sve < 1) &&
              !(cmp_sve = BEG_CONST_STRCMP("Search_name", *input)))
     {
      SKIP_MATCHED_TO(input, "Search_name");
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               help_search_name ? help_search_name : "",
                               input, length, out_node));
     }
     else if ((cmp_sve < 1) &&
              !(cmp_sve = BEG_CONST_STRCMP("Shown_by", *input)))
     {
      SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Shown_by",
                                length, out_node);
      if (help_shown_by)
        return (player_variables(from, p, info, flags, timestamp,
                                 help_shown_by->saved, input,
                                 length, out_node));
      else
        return (player_variables(from, p, info, flags, timestamp,
                                 p->saved, input, length, out_node));
     }     
    }
    break;
    
  case 'I':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("If", *input)))
    {
     const char *name_str = NULL;
     int name_len = 0;
     
     SKIP_MATCHED_TO(input, "If");
     
     if (test_tag_variables(from, p, info, flags, timestamp,
                            input, length, "If", &name_str, &name_len,
                            &out_node))
       out_node = string_variables(from, p, info, flags, timestamp, name_len,
                                   name_str, input, length, out_node);
     
     return (out_node);
    }
    else if (!(cmp_sve = BEG_CONST_STRCMP("Int", *input)))
    {
     long the_number = 0;
     int all_len = 0;
     
     SKIP_MATCHED_TO(input, "Int");
     if (!BEG_CONST_STRCMP("eger", *input))
       SKIP_MATCHED_TO(input, "eger");
     
     if (!(all_len = tag_get_bracket(*input)))
     {
      info->used_twinkles = TRUE;
      return (error_no_tags(from, p, info, flags, timestamp, "Int",
                            length, out_node));
     }
     else
     {
      const char *the_string = tag_skip_prefix(*input);
      the_number = buffer_toint(ALL_T(from, p, info, flags | OUTPUT_IN_TAG,
                                      timestamp),
                                all_len - ((the_string - *input) + 1),
                                the_string);
     }
     *input += all_len;
     
     return (number_variables(from, p, info, flags, timestamp,
                              the_number, input, length, out_node));
    }
    break;
    
  case 'L':
    if (!(cmp_sve = BEG_CONST_STRCMP("Line_fill(", *input)))
    {
     int size = 0;
     const char *start_tags = NULL;
     
     SKIP_MATCHED_TO(input, "Line_fill");
     
     if ((size = tag_get_bracket(*input))) /* FIXME: doesn't work */
     {
      int tmp_count = 0;
      
      start_tags = tag_skip_prefix(*input);
      tmp_count = (start_tags - *input) + 1;
      *input += size;
      size -= tmp_count;
     }
     
     info->used_twinkles = TRUE;
     
     if (info->allow_fills && (size > 0))
     {
      if (flags & OUTPUT_BUFFER_TMP)
      {
       flags &= ~OUTPUT_BUFFER_NON_PRINT;
       
       flags |= OUTPUT_REPEAT;
       out_node = output_raw(p, flags, start_tags, size, out_node);
       out_node->end_repeat_list = TRUE;
       flags &= ~OUTPUT_REPEAT;
      }
      else
      { /* optimisation */
       int chars_left = p->term_width ? (p->term_width - p->column) : 79;
       int count = chars_left / size;
       
       while (count > 0)
       {
        int buffer_index = 0;
        
        while (buffer_index < size)
          out_node = normal_char_output(p, flags,
                                        start_tags[buffer_index++],
                                        length, out_node);
        
        --count;
       }
      }
     }
     return (out_node);
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Logging_onto", *input)))
    {
     SKIP_MATCHED_TO(input, "Logging_onto");
     return (number_variables(from, p, info, flags, timestamp,
                              logging_onto_count, input, length, out_node));
    }
    break;
     
  case 'M':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Merge", *input)))
    {
     output_node *tmp_output = NULL;
     MATH_TAG_START("Merge", "Merge");

     IGNORE_PARAMETER(left_num && right_num && number);

     tmp_output = output_list_merge(p, &tag_left_side, &tag_right_side,
                                    FALSE, INT_MAX);
     
     MATH_TAG_END();

     assert(!(tag_left_side || tag_right_side));

     return (out_node_variables(p, info, flags, tmp_output, input,
                                length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Mod", *input)))
    {
     MATH_TAG_START("Mod", "Modulus");
     
     left_num = output_list_toint(tag_left_side);
     right_num = output_list_toint(tag_right_side);
     
     if (left_num && right_num)
       number = left_num % right_num;
     
     MATH_TAG_END();
     
     return (number_variables(from, p, info, flags, timestamp,
                              number, input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Mul", *input)))
    {
     MATH_TAG_START("Mul", "Multiply");
     
     left_num = output_list_toint(tag_left_side);
     right_num = output_list_toint(tag_right_side);
     
     if (abs(left_num) <= (LONG_MAX / abs(right_num)))
       number = left_num * right_num;
     
     MATH_TAG_END();
     
     return (number_variables(from, p, info, flags, timestamp,
                              number, input, length, out_node));
    }
    break;
     
  case 'N':
    if ((cmp_sve < 1) &&
        !(cmp_sve = BEG_CONST_STRCMP("Newbie_players", *input)))
    {
     SKIP_MATCHED_TO(input, "Newbie_players");
     return (number_variables(from, p, info, flags, timestamp,
                              player_newbie_number(),
                              input, length, out_node));
    }
    else if (!(cmp_sve = BEG_CONST_STRCMP("Num", *input)))
    {
     long the_number = 0;
     int name_len = 0;
     
     SKIP_MATCHED_TO(input, "Num");
     if (!BEG_CONST_STRCMP("ber", *input))
       SKIP_MATCHED_TO(input, "ber");

     if (!(name_len = tag_get_bracket(*input)))
     {
      info->used_twinkles = TRUE;
      return (error_no_tags(from, p, info, flags, timestamp, "Number",
                            length, out_node));
     }
     else
     {
      tmp = tag_skip_prefix(*input);
      the_number = buffer_toint(ALL_T(from, p, info, flags | OUTPUT_IN_TAG,
                                      timestamp),
                                name_len - ((tmp - *input) + 1), tmp);
     }
     *input += name_len;
     
     return (number_variables(from, p, info, flags, timestamp,
                              the_number, input, length, out_node));
    }
    break;
     
  case 'P':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Player", *input)))
    {
     static unsigned int player_twinkle_type = PLAYER_TWINKLE_DEFAULT;
     int name_len = 0;
     player_tree_node *the_player = NULL;
      
     SKIP_MATCHED_TO(input, "Player");
     if (!BEG_CONST_STRCMP("_name", *input))
     {
      player_twinkle_type = PLAYER_TWINKLE_NAME;
      SKIP_MATCHED_TO(input, "_name");
     }
     else
     { /* so we can maybe put some more stuff in here, like su etc.. ? */
      if (!BEG_CONST_STRCMP("_alpha", *input))
      {
       player_twinkle_type = PLAYER_TWINKLE_ALPHA;
       SKIP_MATCHED_TO(input, "_alpha");
      }
      else if (!BEG_CONST_STRCMP("_cron", *input))
      {
       player_twinkle_type = PLAYER_TWINKLE_CRON;
       SKIP_MATCHED_TO(input, "_cron");
      }
      else if (!BEG_CONST_STRCMP("_user", *input))
      {
       if (p->flag_list_show_inorder)
         player_twinkle_type = PLAYER_TWINKLE_ALPHA;
       else
         player_twinkle_type = PLAYER_TWINKLE_CRON;
       SKIP_MATCHED_TO(input, "_user");
      }
     }
      
     if (!(name_len = tag_get_bracket(*input)))
     {
      info->used_twinkles = TRUE;
      return (error_no_tags(from, p, info, flags, timestamp, "Player",
                            length, out_node));
     }
     else
     {
      const char *the_string = tag_skip_prefix(*input);
      int tmp_count = (the_string - *input) + 1;
      
      *input += name_len;
      name_len -= tmp_count;

      switch (player_twinkle_type)
      {
       case PLAYER_TWINKLE_ALPHA:
       case PLAYER_TWINKLE_CRON:
       {
        long the_number = 0;
        
        the_number = buffer_toint(ALL_T(from, p, info, flags | OUTPUT_IN_TAG,
                                        timestamp),
                                  name_len, the_string);
         
        if ((the_number < 1) ||
            !(the_player = find_player_loggedon_count(
            (player_twinkle_type == PLAYER_TWINKLE_ALPHA), the_number)))
          if (!(the_player = from))
            the_player = p->saved; /* might die on title screen :*/
       }
       break;
        
       case PLAYER_TWINKLE_NAME:
       {
        tmp_output_list_storage tmp_save;
        output_node *name_list = NULL;
        char name_buffer[PLAYER_S_NAME_SZ];
        int buf_size = 0;
        
        save_tmp_output_list(p, &tmp_save);
        fntell_player(ALL_T(from, p, info, flags | OUTPUT_IN_TAG |
                            OUTPUT_BUFFER_TMP, timestamp),
                      name_len, PLAYER_S_NAME_SZ, the_string);
        name_list = output_list_grab(p);

        buf_size = output_list_into_buffer(p, name_list, name_buffer,
                                           PLAYER_S_NAME_SZ - 1);         
        name_buffer[buf_size] = 0;
         
        if (!(the_player = player_find_all(p, name_buffer,
                                           PLAYER_FIND_EXPAND |
                                           PLAYER_FIND_NEWBIES |
                                           PLAYER_FIND_PICK_FIRST)))
          the_player = p->saved;
        
        output_list_cleanup(&name_list);
        load_tmp_output_list(p, &tmp_save);
       }
       break;
        
       default:
         assert(FALSE);
      }
     }
      
     NEED_CONNECTOR(input, from, p, flags, "Player", length, out_node);
     return (player_variables(from, p, info, flags, timestamp,
                              the_player, input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Pow", *input)))
    {
     MATH_TAG_START("Pow", "Power");    

     left_num = output_list_toint(tag_left_side);
     right_num = output_list_toint(tag_right_side);

     number = pow(left_num, right_num);
     switch (errno)
     {
      case EDOM: /* this can't happen atm as we are dealing with ints only */
        assert(FALSE);
         
      default:
        break; /* nothing else applies */
     }
      
     MATH_TAG_END();

     return (number_variables(from, p, info, flags, timestamp,
                              number, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Prob_diff", *input)))
    {
     SKIP_MATCHED_TO(input, "Prob_diff");
     return (probability_tag_variables(from, p, info, flags, timestamp, FALSE,
                                       input, "Prob_diff", length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Prob_same", *input)))
    {
     SKIP_MATCHED_TO(input, "Prob_same");
     return (probability_tag_variables(from, p, info, flags, timestamp, TRUE,
                                       input, "Prob_same", length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Probability_diff",*input)))
    {
     SKIP_MATCHED_TO(input, "Probability_diff");
     return (probability_tag_variables(from, p, info, flags, timestamp, FALSE,
                                       input, "Probability_diff", length,
                                       out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Probability_same",*input)))
    {
     SKIP_MATCHED_TO(input, "Probability_same");
     return (probability_tag_variables(from, p, info, flags, timestamp, TRUE,
                                       input, "Probability_same", length,
                                       out_node));
    }
    break;
     
  case 'Q':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Qtime", *input)))
    {
     SKIP_MATCHED_TO(input, "Qtime");
     OUT_VAR_DEPRECIATED(from, p, "Qtime", ", Use Time-Clock instead");
     tmp = disp_time_hour_min(now, 0, p->flag_use_24_clock);
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              tmp, input, length, out_node));
    }
    break;
     
  case 'R':
    if (!(cmp_sve = BEG_CONST_STRCMP("Raw", *input)))
    {
     int raw_type = 0;
     
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Raw",
                               length, out_node);

     if (**input == '?')
     {
      ++*input;
      return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                               "Colours, Specials, String, Twinkles, Wands",
                               input, length, out_node));
     }

     assert(!cmp_sve);
     if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Colors", *input)))
     {
      raw_type = RAW_SPECIALS;
      SKIP_MATCHED_TO(input, "Colors");
     }
     else if ((cmp_sve < 1) &&
              !(cmp_sve = BEG_CONST_STRCMP("Colours", *input)))
     {
      raw_type = RAW_SPECIALS;
      SKIP_MATCHED_TO(input, "Colours");
     }
     else if ((cmp_sve < 1) &&
              !(cmp_sve = BEG_CONST_STRCMP("Specials", *input)))
     {
      raw_type = RAW_SPECIALS;
      SKIP_MATCHED_TO(input, "Specials");
     }
     else if ((cmp_sve < 1) &&
              !(cmp_sve = BEG_CONST_STRCMP("String", *input)))
     {
      raw_type = RAW_SPECIALS | RAW_OUTPUT_VARIABLES;
      SKIP_MATCHED_TO(input, "String");
     }
     else if ((cmp_sve < 1) &&
              !(cmp_sve = BEG_CONST_STRCMP("Twinkles", *input)))
     {
      raw_type = RAW_OUTPUT_VARIABLES;
      SKIP_MATCHED_TO(input, "Twinkles");
     }
     else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Wands", *input)))
     {
      raw_type = RAW_SPECIALS;
      SKIP_MATCHED_TO(input, "Wands");
     }
      
     if (raw_type)
     {
      const char *the_string = NULL;
      int name_len = 0;
      int tmp_count = 0;
      
      if (!(name_len = tag_get_bracket(*input)))
      {
       info->used_twinkles = TRUE;
       return (error_no_tags(from, p, info, flags, timestamp, "Raw",
                             length, out_node));
      }
      the_string = tag_skip_prefix(*input);
      tmp_count = (the_string - *input) + 1;
      *input += name_len;
      name_len -= tmp_count;
      
      return (string_variables(from, p, info, flags | OUTPUT_IN_TAG | raw_type,
                               timestamp, name_len, the_string,
                               input, length, out_node));
     }
     else
     {
      tmp = "<Bad Formatting (Raw)>";
      info->used_twinkles = TRUE;
      return (output_string(from, p, info, flags, timestamp, &tmp, INT_MAX,
                            length, out_node));
     }
    }
    else if (!(cmp_sve = BEG_CONST_STRCMP("Random_resident", *input)))
    {
     static unsigned int command_number_save = 0;
     static player_tree_node *output_player_save[10]; /* '0' - '9' */
     int random_player_offset = 0;
      
     SKIP_MATCHED_TO(input, "Random_resident");
      
     if (isdigit((unsigned char) **input))
       random_player_offset = TONUMB(*(*input)++);
      
     if (command_number_save != current_command_number)
     {
      command_number_save = current_command_number;
      memset(output_player_save, 0, sizeof(output_player_save));
     }
      
     if (!output_player_save[random_player_offset])
       output_player_save[random_player_offset] = player_tree_random();
      
     NEED_CONNECTOR(input, from, p, flags, "Random_resident",
                    length, out_node);
     return (player_variables(from, p, info, flags, timestamp,
                              output_player_save[random_player_offset],
                              input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Recv", *input)))
    {
     if (!cmp_sve)
       SKIP_MATCHED_TO(input, "Recv");
     else
     {
     default_r_twinkle_recv:
      SKIP_MATCHED_TO(input, "R");
     }

     NEED_CONNECTOR(input, from, p, flags, "Recv", length, out_node);
     return (player_variables(from, p, info, flags, timestamp, p->saved, input,
                              length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Return", *input)))
    {
     SKIP_MATCHED_TO(input, "Return");
     if (info->returns_limit)
     {
      --info->returns_limit;
      tmp = "\n";
     }
     else
       tmp = " ";
     return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                              tmp, input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Room", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Room",
                               length, out_node);
     return (room_variables(from, p, info, flags, timestamp, p->location,
                            input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Rot13", *input)))
    {
     const char *the_string = NULL;
     int name_len = 0;
     output_node *rot13_list = NULL;
     tmp_output_list_storage tmp_save;
     
     SKIP_MATCHED_TO(input, "Rot13");

     if (!(name_len = tag_get_bracket(*input)))
     {
      info->used_twinkles = TRUE;
      return (error_no_tags(from, p, info, flags, timestamp, "Rot13",
                            length, out_node));
     }
     the_string = tag_skip_prefix(*input);
     
     save_tmp_output_list(p, &tmp_save);
     fntell_player(ALL_T(from, p, info, flags | OUTPUT_IN_TAG |
                         OUTPUT_BUFFER_TMP, timestamp),
                   name_len - ((the_string - *input) + 1), *length,
                   the_string);
     rot13_list = output_list_grab(p);
     load_tmp_output_list(p, &tmp_save);
     
     output_list_rot13(rot13_list);

     *input += name_len;
     
     return (out_node_variables(p, info, flags, rot13_list,
                                input, length, out_node));
    }
    else
      goto default_r_twinkle_recv;
    break;
     
  case 'S':
    if (!(cmp_sve = BEG_CONST_STRCMP("Seconds", *input)))
    {
     const char *the_string = NULL;
     long the_seconds = 0;
     int name_len = 0;
     int tmp_count = 0;
     
     SKIP_MATCHED_TO(input, "Seconds");
      
     if (!(name_len = tag_get_bracket(*input)))
     {
      info->used_twinkles = TRUE;
      return (error_no_tags(from, p, info, flags, timestamp, "Seconds",
                            length, out_node));
     }
     the_string = tag_skip_prefix(*input);
     tmp_count = (the_string - *input) + 1;
     *input += name_len;
     name_len -= tmp_count;
     
     the_seconds = buffer_toint(ALL_T(from, p, info, flags | OUTPUT_IN_TAG,
                                      timestamp),
                                name_len, the_string);

     NEED_CONNECTOR(input, from, p, flags, "Seconds", length, out_node);

     return (seconds_variables(from, p, info, flags, timestamp, the_seconds,
                               input, length, out_node));
    }
    else if (!(cmp_sve = BEG_CONST_STRCMP("Strlen", *input)))
    {
     int the_number = 0;
     int name_len = 0;
     
     SKIP_MATCHED_TO(input, "Strlen");
     if (!BEG_CONST_STRCMP("gth", *input))
       SKIP_MATCHED_TO(input, "gth");
      
     if (!(name_len = tag_get_bracket(*input)))
     {
      info->used_twinkles = TRUE;
      return (error_no_tags(from, p, info, flags, timestamp, "Strlen",
                            length, out_node));
     }
     else
     {
      const char *the_string = tag_skip_prefix(*input);
      tmp_output_list_storage tmp_save;
      output_node *number_list = NULL;
       
      save_tmp_output_list(p, &tmp_save);
      fntell_player(ALL_T(from, p, info, flags | OUTPUT_IN_TAG |
                          OUTPUT_BUFFER_TMP, timestamp),
                    name_len - ((the_string - *input) + 1), INT_MAX,
                    the_string);
      number_list = output_list_grab(p);
      the_number = output_list_print_length(p, number_list, flags, *length);
      output_list_cleanup(&number_list);
      load_tmp_output_list(p, &tmp_save);
     }
     *input += name_len;
     
     return (number_variables(from, p, info, flags, timestamp,
                              the_number, input, length, out_node));
    }
    else if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Sub", *input)))
    {
     MATH_TAG_START("Sub", "Subtract");
      
     left_num = output_list_toint(tag_left_side);
     right_num = output_list_toint(tag_right_side);

     if (((left_num >= 0) && (right_num >= 0)) ||
         ((left_num < 0) && (right_num < 0)) ||
         ((left_num >= 0) && 
          ((0 - right_num) <= (LONG_MAX - left_num))) ||
         ((right_num >= 0) && 
          ((0 - right_num) >= (LONG_MIN - left_num))))
       number = left_num - right_num;
      
     MATH_TAG_END();

     return (number_variables(from, p, info, flags, timestamp,
                              number, input, length, out_node));
    }
    else if ((cmp_sve < 1) &&
             !(cmp_sve = BEG_CONST_STRCMP("Super_players", *input)))
    {
     SKIP_MATCHED_TO(input, "Super_players");

     if ((flags & OUTPUT_VARIABLES_NO_CHECKS) || PRIV_STAFF(p->saved))
       return (number_variables(from, p, info, flags, timestamp,
                                player_list_logon_staff_number(),
                                input, length, out_node));
     else
       return (number_variables(from, p, info, flags, timestamp, 0,
                                input, length, out_node));
    }
    else if (!(cmp_sve = BEG_CONST_STRCMP("String", *input)))
    {
     const char *the_string = NULL;
     int name_len = 0;
     int tmp_count = 0;
     
     SKIP_MATCHED_TO(input, "String");
      
     if (!(name_len = tag_get_bracket(*input)))
     {
      info->used_twinkles = TRUE;
      return (error_no_tags(from, p, info, flags, timestamp, "String",
                            length, out_node));
     }
     the_string = tag_skip_prefix(*input);
     tmp_count = (the_string - *input) + 1;
     *input += name_len;
     name_len -= tmp_count;
     
     return (string_variables(from, p, info, flags | OUTPUT_IN_TAG, timestamp,
                              name_len, the_string, input, length, out_node));
    }
    break;
     
  case 'T':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Talker", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Talker",
                               length, out_node);
     return (talker_variables(from, p, info, flags, timestamp,
                              input, length, out_node));
    }
    else if (!(cmp_sve = BEG_CONST_STRCMP("Text", *input)))
    {
     long the_number = 0;
     int all_len = 0;
     text_objs_node *text_obj = NULL;
     
     SKIP_MATCHED_TO(input, "Text");

     if (!(all_len = tag_get_bracket(*input)))
     {
      info->used_twinkles = TRUE;
      return (error_no_tags(from, p, info, flags, timestamp, "Text",
                            length, out_node));
     }
     else
     {
      const char *the_string = tag_skip_prefix(*input);
      the_number = buffer_toint(ALL_T(from, p, info, flags | OUTPUT_IN_TAG,
                                      timestamp),
                                all_len - ((the_string - *input) + 1),
                                the_string);
     }
     *input += all_len;

     if ((text_obj = text_objs_user_find_id(from, the_number)))
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                text_obj->str, input, length, out_node));
     else
       return (string_variables(from, p, info, flags, timestamp, INT_MAX,
                                "", input, length, out_node));
    }
    else if (!(cmp_sve = BEG_CONST_STRCMP("Time", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Time",
                               length, out_node);
     return (time_variables(from, p, info, flags, timestamp, now,
                            input, length, out_node));
    }
    break;
     
  case 'U':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("Uptime", *input)))
    {
     SKIP_MATCHED_TO_CONNECTOR(input, from, p, flags, "Uptime",
                               length, out_node);
     return (seconds_variables(from, p, info, flags, timestamp,
                               difftime(now, talker_started_on),
                               input, length, out_node));
    }
    break;
   
  case 'V':
    break;
     
  case 'W':
    if ((cmp_sve < 1) && !(cmp_sve = BEG_CONST_STRCMP("While", *input)))
    {
     static unsigned int allow_twinkle_depth = 250;
     const char *saved_input = NULL;
     const char *name_str = NULL;
     int name_len = 0;
     int starter = FALSE;
      
     SKIP_MATCHED_TO(input, "While");
      
     if (allow_twinkle_depth == 250)
       starter = TRUE;

     saved_input = *input;
     if ((allow_twinkle_depth > 0) &&
         (test_tag_variables(from, p, info, flags, timestamp,
                             input, length, "While", &name_str, &name_len,
                             &out_node) > 0))
     {
      const char *end_input = NULL;
       
      do
      {
       --allow_twinkle_depth;
       out_node = string_variables(from, p, info, flags, timestamp, name_len,
                                   name_str, input, length, out_node);
       end_input = *input;
       *input = saved_input;
      } while ((allow_twinkle_depth > 0) &&
               (test_tag_variables(from, p, info, flags, timestamp,
                                   input, length, "While",
                                   &name_str, &name_len, &out_node) > 0));
      *input = end_input;
     }
     else
       out_node = string_variables(from, p, info, flags, timestamp,
                                   INT_MAX, "", input, length, out_node);

     if (starter)
       allow_twinkle_depth = 250;
     return (out_node);
    }
    break;

  case '$':
    ++*input;
    /* FALLTHROUGH */
  default:
    break;
 }

 out_node = normal_char_output(p, flags, '$', length, out_node);

 return (out_node);
}

/* Configure the output for the player then write it to thier output list,
 NOTE: the input_length variable does not garantee that it will only use that
 ammount of characters ... (0, p, 0, &str, 1, out_node) ...
 may not work as you expect it to */
static output_node *internal_output_string(player_tree_node *from, player *p,
                                           twinkle_info *info,
                                           int flags, time_t timestamp, 
                                           const char **input,
                                           int *input_length,
                                           int *output_length,
                                           output_node *out_node)
{
 if (**input && (*input_length > 0))
 {
  static unsigned int allow_main_twinkle_depth = OUTPUT_MAIN_TWINKLE_DEPTH;
  
  assert(p && input && *input && output_length && (*output_length >= 0));

  if (**input == '\n')
  {
   out_node = output_return(p, flags, 0, output_length, out_node);
   DECREMENT_LENGTH(*input_length, 1);
   ++*input;
  }
  else if ((**input == '^') && !(flags & RAW_SPECIALS) && (*input_length > 1))
  {
   const char *tmp = *input;
   
   out_node = special_char_output(from, p, flags, input,
                                  output_length, out_node);
   DECREMENT_LENGTH(*input_length, (*input - tmp));
  }
  else if ((**input == '$') && (allow_main_twinkle_depth > 0) &&
           !(flags & RAW_OUTPUT_VARIABLES) &&
           ((!configure.output_raw_twinkles && !p->see_raw_twinkles) ||
            (flags & OVERRIDE_RAW_OUTPUT_VARIABLES)) &&
           (*input_length > 1))
  {
   const char *tmp = *input;     
   
   --allow_main_twinkle_depth;
   
   out_node = variables_char_output(from, p, info, flags, timestamp, input,
                                    output_length, out_node);

   ++allow_main_twinkle_depth;
   
   DECREMENT_LENGTH(*input_length, (*input - tmp));
  }
  else if ((flags & OUTPUT_IN_TAG) &&
           (**input == '\\') && (*input_length > 1) &&
           ((*(*input + 1) == '\\') ||
            (*(*input + 1) == '(') || (*(*input + 1) == ')')))
  {
   ++*input;
   out_node = normal_char_output(p, flags, **input, output_length, out_node);
   DECREMENT_LENGTH(*input_length, 2);
   ++*input;
  }
  else
  {
   out_node = normal_char_output(p, flags, **input, output_length, out_node);
   DECREMENT_LENGTH(*input_length, 1);
   ++*input;
  }
 }
 
 return (out_node);
}

static output_node *output_string(player_tree_node *from, player *p,
                                  twinkle_info *info,
                                  int flags, time_t timestamp, 
				  const char **input, int input_length,
                                  int *output_length, output_node *out_node)
{
 twinkle_info the_info = DEFAULT_TWINKLE_SETUP;

 assert(p && input && *input && output_length && (*output_length >= 0));
 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
 assert(!(OUTPUT_BYTES & flags));
 
 if (!info) /* gaurantee that the info struct will have something in it */
   info = &the_info;
 
 while (**input && (input_length > 0))
   out_node = internal_output_string(from, p, info, flags, timestamp,
                                     input, &input_length, output_length,
                                     out_node);
 
 return (out_node);
}

output_node *delete_output_raw(int length,
                               output_node **head, output_node *out_node)
{
 output_node *tmp = NULL;
 
 assert((length > 0) && head && *head && out_node);
 
 if (!out_node)
   return (0);

 if (out_node->flags & (OUTPUT_BYTES|OUTPUT_REPEAT|OUTPUT_BUFFER_NON_PRINT))
 {
  tmp = out_node->prev;
  if (tmp)
  {
   if (out_node->flags & OUTPUT_BUFFER_NON_PRINT)
     destroy_output_node(head, out_node);
   return (delete_output_raw(length, head, tmp));
  }
  else
  {
   tmp->flags = 0;
   tmp->length = tmp->output_bit = 0;
   tmp->full_buffer = tmp->has_return = tmp->output_already =
     tmp->end_repeat_list = FALSE;
   return (out_node); /* it was the first output node */
  }
 }
 
 while ((length > 0) && (out_node->length > 0))
 {
  out_node->length--;
  length--;
 }

 if (length <= 0)
 {
  assert(!length);
  return (out_node);
 }
 
 if (!(tmp = out_node->prev))
   return (out_node);

 destroy_output_node(head, out_node);
 return (delete_output_raw(length, head, tmp));
}

void output_for_player_cleanup(player *p)
{
 output_node *tmp = NULL;

 BTRACE("output_for_player_cleanup");

 assert(p);
 
 tmp = p->output_start;
 while (tmp)
 {
  output_node *tmp_next = tmp->next;
  if (!tmp->length ||
      (tmp->output_already &&
       ((tmp->flags & OUTPUT_BYTES) ||
        (difftime(now, tmp->timestamp) > NORM_LINE_TIMEOUT))))
    destroy_output_node(&p->output_start, tmp);
  else
  {
   assert(tmp->length > tmp->output_bit);
  }
  
  tmp = tmp_next;
 }
}

static int output_for_player_aquire(player *p, unsigned int flags,
                                    output_node **ret_out_node,
                                    struct iovec *iovs,
                                    int *found, int *part_out,
                                    FILE **log_out)
{
 int used = 0;
 int ret_check_point = 0;
 struct iovec real_n_iovs[OUTPUT_WRITE_MAX_NODES];
 struct iovec *n_iovs = real_n_iovs;
 int n_used = 0;
 output_node *out_node = *ret_out_node;
 
 BTRACE("output_for_player_aquire");
 
 *ret_out_node = NULL;

 if (!(flags & OUTPUT_PRIORITY))
   n_iovs = iovs;
 
 while (out_node && (used < OUTPUT_WRITE_MAX_NODES))
 {
  if (*found && !(out_node->flags & OUTPUT_COMPRESS_MARK))
    break;

  if (!OUTPUT_NODE_USE(out_node, now))
  {
   /* do nothing */
  }
  else if ((flags & OUTPUT_PRIORITY) && (out_node->flags & OUTPUT_PRIORITY))
  {
   assert(out_node->flags & OUTPUT_BYTES);

   if (!*ret_out_node)
     *ret_out_node = out_node;

   if (out_node->flags & OUTPUT_COMPRESS_MARK)
     *found = TRUE;

   iovs[used].iov_len = out_node->length - out_node->output_bit;
   iovs[used].iov_base = out_node->start + out_node->output_bit;
   ++used;
  }
  else if ((!p->system_info_only || (out_node->flags & SYSTEM_INFO)) &&
           ((used + n_used) < OUTPUT_WRITE_MAX_NODES))
  {
   assert(!(out_node->flags & OUTPUT_PRIORITY));

   if (!n_used)
   {
    if (p->flag_tmp_scripting)
    {
     assert(timer_q_find_data(&scripting_timer_queue, p));
     assert(*p->script_file);
     *log_out = open_wlog(p->script_file);
    }
   }
   
   if (!*ret_out_node)
     *ret_out_node = out_node;
   
   n_iovs[n_used].iov_len = out_node->length - out_node->output_bit;
   n_iovs[n_used].iov_base = out_node->start + out_node->output_bit;
   ++n_used;
   
   if (out_node->has_return)
     /* because sending out lines without a return at the end
      * then pausing for a bit breaks clients --
      * as they "presume" it's a prompt */
     ret_check_point = n_used;
  }

  out_node = out_node->next;
 }

 assert(used <= OUTPUT_WRITE_MAX_NODES);
 
 if (flags & OUTPUT_PRIORITY)
 {
  if (!*found && (used < OUTPUT_WRITE_MAX_NODES))
  {
   if (n_used > (OUTPUT_WRITE_MAX_NODES - used))
   {
    *part_out = TRUE;
    return (used);
   }
   
   memcpy(iovs + used, n_iovs, n_used * sizeof(struct iovec));   
   used += n_used;
  }
  
  return (used);
 }
 assert(!used);

 if (n_used > OUTPUT_WRITE_MAX_NODES)
 {
  *part_out = TRUE;
  n_used = ret_check_point;
 }
  
 return (n_used);
}

static int output_for_player_flush(output_node *out_node,
                                   const struct iovec *iovs, int io_count,
                                   int num, size_t *count, FILE *log_out)
{
 BTRACE("output_for_player_flush");
 
 if (!*count)
    return (FALSE);
 
 while (out_node && (*count > 0) && (io_count < num))
 {
  if ((out_node->start + out_node->output_bit) == iovs[io_count].iov_base)
  {
   if (*count < iovs[io_count].iov_len)
   {
    assert(out_node);
    
    out_node->output_bit += *count;
    if (log_out)
      fwrite(iovs[io_count].iov_base, sizeof(char), *count, log_out);
    return (0);
   }

   assert(*count >= (size_t)(out_node->length - out_node->output_bit));
   *count -= out_node->length - out_node->output_bit;
   out_node->output_already = TRUE;
   if (log_out)
     fwrite(iovs[io_count].iov_base, sizeof(char),
            iovs[io_count].iov_len, log_out);
   ++io_count;
  }
  
  out_node = out_node->next;
 }
 
 return (io_count);
}

int output_for_player(player *p)
{
 output_node *out_node = p->output_start;
 struct iovec iovs[OUTPUT_WRITE_MAX_NODES + 1];
 int used = 0;
 int found = FALSE;
 unsigned int prompt_len = 0;
 char *prompt_str = NULL;
 unsigned int flags = 0;
 int part_out = FALSE;
 FILE *log_out = NULL;
 
 BTRACE("output_for_player");
 
 if (INVALID_PLAYER_SOCKET(p))
   return (FALSE);

 if (p->output_has_priority)
   flags |= OUTPUT_PRIORITY;

 if (p->flag_tmp_prompt_do_output && (p->event < PLAYER_EVENT_LOGOFF) &&
     !sys_flag.shutdown)
   prompt_str = prompt_do_output(p, &prompt_len);
 
 used = output_for_player_aquire(p, flags, &out_node, iovs, &found, &part_out,
                                 &log_out);

 if (!prompt_len && !used)
   /* just more prompt, Ie. input, to output */
   prompt_str = prompt_next_output(p, &prompt_len);

 if (prompt_len)
 {
  assert(used <= OUTPUT_WRITE_MAX_NODES);
  
  iovs[used].iov_len = prompt_len;
  iovs[used].iov_base = prompt_str;
  ++used;
 }
 
 assert(used >= 0);
 
 if (used)
 {
  ssize_t tmp = 0;
  int io_count = 0;

  if (p->output_compress_do)
    tmp = output_compress_writev_1(p, iovs, used, found);
  else
    tmp = socket_writev(&SOCKET_POLL_INDICATOR(p->io_indicator)->fd,
                        iovs, used);

  if (tmp != -1)
  {
   size_t ret = tmp;
   
   do
   {
    io_count = output_for_player_flush(out_node, iovs, io_count,
                                       used, &ret, log_out);
    
    if (prompt_len && (ret <= prompt_len))
    { /* always the last thing to go out, see above */
     p->prompt_out_length += ret;
     ret = 0;
     ++io_count;
    }
   } while (io_count && (io_count < used));
  }
  
  if (!io_count)
    part_out = TRUE;
  else
  {
   assert(io_count == used);
  }
  
  if (log_out)
    close_wlog(log_out);
 }
 else
 {
  assert(!log_out);
 }

 if (p->output_compress_do)
 {
  if (output_compress_writev_2(p))
    part_out = TRUE;
 }
 
 if (part_out)
   return (TRUE);
 
 if (found)
 {
  if (p->output_compress_do)
    output_compress_stop_2(p);
  else
    output_compress_start_2(p);
  return (TRUE);
 }
 
 p->output_has_priority = FALSE;
 
 return (FALSE);
}




/* onto the higher level stuff now... tell_players, test_receive etc... */





static output_node *start_tell_player(player_tree_node *from, player *p,
				      int flags, time_t timestamp)
{
 output_node *out_node = 0;

 IGNORE_PARAMETER(from); /* take out from sometime I think, see below */
 BTRACE("start_tell_player");
 
 assert(p);
 assert(!(NOT_USED_FLAGS_TELL_PLAYER & flags));
 
 if (!p || INVALID_PLAYER_SOCKET(p))
   return (NULL);
 
 if (sys_flag.panic && !(flags & SYSTEM_INFO))
   return (NULL);
 
 out_node = first_output_node(p, flags, timestamp, NULL);
 
 if (flags & HILIGHT)
   out_node = colour_write(p, flags, BOLD_ON, out_node);

 return (out_node);
}

/* needs to see start_tell_player */
int output_maybe_delete_line(player *p)
{ /* delete the input and prompt, and ask to re-output them */
 unsigned int width = p->prompt_print_length;
 int local_prompt_do_output = p->flag_tmp_prompt_do_output;

 BTRACE("output_maybe_delete_line");

 p->flag_tmp_prompt_do_output = TRUE;

 /* FIXME: if the prompt is still in the output list and _NONE_ of it has gone
  * out, we could just remove the old nodes and not send the new delete
  * stuff out (needs a flag to check for in the nodes)...
  * dealing with multiple prompts ? half sent prompts ?
  *
  * Also need to check stuff in output_aquire to make sure we have valid
  * nodes */
 if (!local_prompt_do_output && p->extra_screen_routines && p->termcap &&
     p->termcap->up && p->termcap->ce)
 {
  output_node *out_node = NULL;
  int flags = OUTPUT_BYTES;
  int lines = 0;

  if (!p->prompt_out_length)
    return (TRUE);  
  
  /* play with am flag */
  if ((width > (p->term_width + 1)) && p->term_width)
    lines = width / (p->term_width + 1);

  if (width > (1024 * 4))
  {
   log_assert(FALSE);
   return (TRUE);
  }
  
  ++lines;

  if (p->prompt_out_length < p->prompt_length)
    /* output the rest of the prompt... */
    fvtell_player(ALL_T(p->saved, p, NULL, flags, 0), "%.*s",
                  (int)(p->prompt_length - p->prompt_out_length),
                  p->prompt_output + p->prompt_out_length);
  fvtell_player(ALL_T(p->saved, p, NULL, flags, 0), "%.*s",
                2, "\r\n"); /* ...and a return */

  p->prompt_out_length = 0;
  
  if (!(out_node = start_tell_player(p->saved, p, flags, 0)))
    return (FALSE);

  /* delete the prompt */
  while (lines-- > 0)
  {
   int ret = 0;

   if (OUTPUT_TYPE(p) && p->termcap->ms)
      /* can't move while colours/wands are on */
     out_node = colour_write(p, flags, EVERYTHING_OFF, out_node);

   ret = terminal_do_cursor_up(p, flags, &out_node);
   assert(ret);
   ret = terminal_do_clear_eol(p, flags, &out_node);
   assert(ret);
  }

  fvtell_player(ALL_T(p->saved, p, NULL, flags, 0), "%n",
                OUTPUT_BYTES_TERMINATE);

  p->prompt_out_length = 0;
  
  return (TRUE);
 }

 return (FALSE);
}

/* tell routines implemented using the above functions */
/* formating version of tell player... wrapping from 1-31 plus some flags
   for raw data, system infomation etc... */

/* input number limited version ... this only works on pre-parsed strings
 * Ie. don't touch unless you know what it really does */
static void fntell_player(player_tree_node *from, player *p, 
                          twinkle_info *info, int output_flags, 
                          time_t timestamp, int input_length, int length,
                          const char *output)
{
 output_node *out_node = 0;

 if (!(out_node = start_tell_player(from, p, output_flags, timestamp)))
   return;

 out_node = output_string(from, p, info, output_flags, timestamp, &output,
                          input_length, &length, out_node);
}


void fvtell_player(player_tree_node *from, player *p, 
                   twinkle_info *info, int flags, 
                   time_t timestamp, const char *fmt, ...)
{
 va_list ap;

 va_start(ap, fmt);

 vfvtell_player(from, p, info, flags, timestamp, fmt, ap);
 
 va_end(ap);
}

/* 
 * Var arg tell player... the root of all evil :) .....
 * Stolen from the linux sources .. and then hacked to death.
 *
 */

void vfvtell_player(player_tree_node *from, player *p,  twinkle_info *info, 
		    int output_flags, time_t timestamp,
                    const char *fmt, va_list args)
{
 output_node *out_node = 0;
 int len = 0;
 unsigned long num = 0;
 twinkle_info the_info = DEFAULT_TWINKLE_SETUP;

 int base = 10;
 int int_type = INT_TYPE;
 int flags = 0;
 int field_width = 0;
 int numb_precision = 0;  /* min number of digits, after decimal point */
 int str_precision = INT_MAX;  /* max number of chars */

 int length = INT_MAX; /* to print out a WHOLE string */

 if (!(output_flags & OUTPUT_BYTES))
   output_maybe_delete_line(p);
 
 if (!(out_node = start_tell_player(from, p, output_flags, timestamp)))
   return;

 if (!(player_list_io_find(p) || (flags & OUTPUT_BUFFER_TMP)))
   player_list_io_add(p);
 
 if (!info) /* gaurantee that the info struct will have something in it */
   info = &the_info;

 /* (vsprintf) starts after here.... */

 /* start of fmt analysis */
 for (; *fmt ; ++fmt)
 {
  if (*fmt != '%')
  {
   int dummy = INT_MAX;
   
   assert(length == INT_MAX);
   out_node = internal_output_string(from, p, info, output_flags, timestamp,
                                     &fmt, &dummy, &length, out_node);
   assert(length == INT_MAX);
   --fmt;
   continue;
  }
  
  /* process flags */
  ++fmt; /* this skips first '%' */
  flags = get_output_flags(&fmt);
  
  /* get field width */
  field_width = 0;
  if (isdigit((unsigned char) *fmt))
    field_width = skip_atoi(&fmt);
  else
    if (*fmt == '*')
    {
     ++fmt;
     field_width = (unsigned int) va_arg(args, int);
    }

  if (field_width < 0)
  {
   field_width = -field_width;
   flags |= LEFT;
  }
  
  /* get the precision */
  numb_precision = 0;
  str_precision = INT_MAX;
  if (*fmt == '.')
  {
   int precision = 0;
   
   if (isdigit((unsigned char) *(++fmt)))
     precision = skip_atoi(&fmt);
   else
     if (*fmt == '*')
     {
      ++fmt;
      precision = (unsigned int) va_arg(args, int);
     }

   if (precision < 0) /* check to make sure things go nicely */
     precision = 0;
   
   numb_precision = str_precision = precision;
  }
  
  switch (*fmt)
  {
   case 'l':
     int_type = LONG_TYPE;
     ++fmt;
     break;
   case 'h':
     int_type = SHORT_TYPE;
     ++fmt;
     /*
       break;
       case 'Z':
       int_type = SIZE_T_TYPE;
       ++fmt;
     */
   default:
     break;
  }

  switch (*fmt)
  {
   case 'c':
   {
    const char c = va_arg(args, int);

    if (output_flags & OUTPUT_BYTES)
      out_node = output_raw(p, output_flags, &c, 1, out_node);
    else
    {
     if (!(flags & LEFT))
       if (field_width > 0)
       {
        assert(length == INT_MAX);
        out_node = output_spaces(p, output_flags, field_width,
                                 &length, out_node);
        assert(length == INT_MAX);
        field_width = 0;
       }
     
     str_precision = INT_MAX;
     if (isprint(c))
       out_node = normal_char_output(p, output_flags,
                                     c, &str_precision, out_node);
     else
       out_node = output_raw(p, output_flags | OUTPUT_BUFFER_NON_PRINT,
                             &c, 1, out_node);
     
     if (field_width > 0)
     {
      assert(length == INT_MAX);
      out_node = output_spaces(p, output_flags, field_width,
                               &length, out_node);
      assert(length == INT_MAX);
      field_width = 0;
     }
    }
   }
   continue;
  
   case 's':
   {
    output_node *buffer = NULL;
    const char *s_orig = va_arg(args, char *);
    const char *s = s_orig;

    assert(s);
    if (!s)
      s = "<NULL>";

    if (output_flags & OUTPUT_BYTES)
    {
     if (str_precision && (s == s_orig))
       /* might not be null terminated */
       out_node = output_raw(p, output_flags, s, str_precision, out_node);
     else
       out_node = output_raw(p, output_flags, s, strlen(s), out_node);
    }
    else
    {
     if (field_width)
     { /* this replaces strnlen, as we have dynamicaly sized strings */
      tmp_output_list_storage tmp_save;
      
      save_tmp_output_list(p, &tmp_save);
      fvtell_player(ALL_T(from, p, info,
                          output_flags | OUTPUT_BUFFER_TMP,
                          out_node->timestamp), "%.*s", str_precision, s);
      buffer = output_list_grab(p);
      load_tmp_output_list(p, &tmp_save);
      len = output_list_print_length(p, buffer, output_flags, str_precision);
     }
     else
       len = 0;
     
     if (!(flags & LEFT))
       if (field_width > len)
       {
        assert(length == INT_MAX);
        assert(len != INT_MAX);
        out_node = output_spaces(p, output_flags, field_width - len,
                                 &length, out_node);
        assert(length == INT_MAX);
        field_width = len;
       }
     
     if (buffer)
     {
      out_node = output_list_linkin_after(p, output_flags, &buffer,
                                          &out_node->next, out_node, len);
     }
     else
     {
      assert(!len);
      out_node = output_string(from, p, info, output_flags, timestamp,
                               &s, INT_MAX, &str_precision, out_node);
     }
     assert(!buffer);
     
     if (field_width > len)
     {
      assert(length == INT_MAX);
      assert(len != INT_MAX);
      out_node = output_spaces(p, output_flags, field_width - len,
                               &length, out_node);
      assert(length == INT_MAX);
     }
    }
   }
   continue;
  
   case 'd':
   case 'i':
     flags |= SIGN;
   case 'u':
     break;
    
   case 'X':
     flags |= LARGE;
   case 'x':
     base = 16;
     break;

   case 'o':
     base = 8;
     break;    

   case 'p':
     /* printing of a pointer */
     if (!field_width) /* default field_width */
     {
      field_width = (sizeof(void *) << 1);
      flags |= ZEROPAD;
     }
     str_precision = INT_MAX;
     assert(!(OUTPUT_BYTES & output_flags));
     out_node = output_number(p, &str_precision, out_node, output_flags,
                              (unsigned long) va_arg(args, void *),
                              field_width, numb_precision, 16 | flags);
     continue;    
    
   case 'n':
     if (output_flags & OUTPUT_BYTES)
     { /* use \n to terminate a normal tell and ...
        * use %n to terminate a BYTES tell (OUTPUT_BYTES_TERMINATE as arg) */
      int *term = va_arg(args, int *);
      assert(!term);
      out_node->full_buffer = TRUE;
      assert(!fmt[1]); /* going to return now. */
     }
     else
     {
      assert(FALSE); /* we aren't going to a buffer so we can't do this */
     }
     continue;

     /* floats ... formatting isn't quite the same as in sprintf, maybe I'll
        make it the same one day.... then again */
   case 'e': /* print like [-]x.xxxexx */
   case 'E': /* use big E instead */
   case 'f': /* print like an int */
   case 'g': /* use the smallest of e and f */
   {
    double float_number = va_arg(args, double);
    char fmt_buffer[11];
    char float_buffer_real[1024];
    char *float_buffer = float_buffer_real;
    const char *float_buffer_output = float_buffer_real;
    int tmp = 1;
    int ret = -1;
    
    fmt_buffer[0] = '%';
    if (flags & LEFT)
      fmt_buffer[tmp++] = '-';
   
    if (flags & PLUS)
      fmt_buffer[tmp++] = '+';
   
    if (flags & SPACE)
      fmt_buffer[tmp++] = ' ';
   
    if (flags & SPECIAL)
      fmt_buffer[tmp++] = '#';
   
    if (flags & ZEROPAD)
      fmt_buffer[tmp++] = '0';
   
    fmt_buffer[tmp++] = '*';
    fmt_buffer[tmp++] = '.';
    fmt_buffer[tmp++] = '*';

    fmt_buffer[tmp++] = *fmt;

    assert(tmp <= 11);
    fmt_buffer[tmp] = 0;

#ifdef HAVE_C9X_SNPRINTF
    ret = snprintf(float_buffer, 1024, fmt_buffer,
                   field_width, numb_precision, float_number);
#else
# ifdef HAVE_ASPRINTF
    ret = asprintf(&float_buffer, fmt_buffer,
                   field_width, numb_precision, float_number);
    COPY_STR(float_buffer_real, float_buffer, 1024);

    free(float_buffer);
    float_buffer = float_buffer_real;
# endif
    if (ret < 0)
    {
     float_buffer[0] = '0';
     float_buffer[1] = 0;
    }
#endif

    assert(length == INT_MAX);
    out_node = output_string(from, p, info, output_flags, timestamp,
                             &float_buffer_output,
                             INT_MAX, &length, out_node);
    assert(length == INT_MAX);
   }
   continue;

   default:
     assert(length == INT_MAX);
     assert(!(OUTPUT_BYTES & output_flags));
     out_node = normal_char_output(p, output_flags, '%', &length, out_node);
     assert(length == INT_MAX);
   
     if (*fmt != '%')
       --fmt;
     continue;
  }
  
  /* got number from above ... flags too */
  switch (int_type)
  {
   case SHORT_TYPE:
     if (flags & SIGN)
       num = (signed short) va_arg(args, int);
     else
       num = (unsigned short) va_arg(args, unsigned int);
     break;
   case INT_TYPE:
     if (flags & SIGN)
       num = va_arg(args, int);
     else
       num = va_arg(args, unsigned int);
     break;
   case LONG_TYPE:
     if (flags & SIGN)
       num = va_arg(args, long);
     else
       num = va_arg(args, unsigned long);
     break;
     /*
       case SIZE_T_TYPE:
       if (flags & SIGN)
       num = va_arg(args, signed size_t);
       else
       num = va_arg(args, unsigned size_t);
       break;
     */
    
   default:
     assert(FALSE);
  }
  assert(length == INT_MAX);
  assert(!(OUTPUT_BYTES & output_flags));
  out_node = output_number(p, &length, out_node, output_flags,
                           num, field_width, numb_precision, base | flags);
  assert(length == INT_MAX);
 }
}
