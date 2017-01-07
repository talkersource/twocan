#define EDITOR_C
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



size_t edit_dump(player *p, char *buffer, size_t len)
{
 edit_base *base = EDIT_BASE(p);
 edit_line_node *scan = base->lines_start;
 char *passed_buffer = buffer;
 
 while (scan && ((size_t)((buffer - passed_buffer) + 1) < len))
 {
  size_t length = scan->length;

  if (length)
  {
   if ((length + (buffer - passed_buffer)) > len)
     length = len - (buffer - passed_buffer);
   
   memcpy(buffer, scan->line, length);
   buffer += length;
  }
  
  if (!base->is_raw && ((size_t)((buffer - passed_buffer) + 1) < len))
    *buffer++ = '\n';

  scan = scan->next;
 }
 if ((size_t)(buffer - passed_buffer) < len)
   *buffer = 0;

 return (buffer - passed_buffer);
}

char *edit_malloc_dump(player *p, size_t *ret_len)
{
 edit_base *base = EDIT_BASE(p);
 size_t len = EDIT_SIZE(base) + 1;
 char *buff = MALLOC(len);
 size_t dummy;
 
 if (!buff)
   return (NULL);

 if (!ret_len)
   ret_len = &dummy;
 
 *ret_len = edit_dump(p, buff, len + 1);
 assert(!buff[len - 1]);
 assert(*ret_len == EDIT_SIZE(base));
 
 return (buff);
}

static unsigned int internal_edit_count_words(const char *str)
{
 unsigned int words = 0;
 
 while (*str)
 {
  str += strspn(str, " \t");

  if (!*str)
    break;
  
  str += strcspn(str, " \t");
  
  ++words;
 }

 return (words);
}

static edit_line_node *edit_find_line(edit_base *base, unsigned int line)
{
 edit_line_node *scan = base->lines_start;
 unsigned int count = 0;
 
 assert(line <= base->lines);
 
 if (!line)
   return (scan);
 else if (line == base->lines)
   return (base->lines_end);
 
 if ((base->lines / 2) < line)
 {
  scan = base->lines_end;
  count = base->lines + 1;
  
  while (scan && (--count > line))
    scan = scan->prev;

  return (scan);
 }

 while (scan && (++count < line))
   scan = scan->next;

 return (scan);
}

edit_line_node *edit_find_point(edit_base *base)
{
 if (base->cached_current_line)
   return (base->cached_current_line);
 
 return ((base->cached_current_line =
          edit_find_line(base, base->current_line)));
}

static void internal_edit_new_line_node(edit_base *base, edit_line_node *node)
{
 edit_line_node *after = NULL;

 if (!base->lines)
 {
  node->next = NULL;
  node->prev = NULL;
  
  base->lines_end = node;
  base->lines_start = node;
 }
 else if (!base->current_line)
 {
  if ((node->next = base->lines_start))
    node->next->prev = node;
  
  node->prev = NULL;

  assert(base->lines_end);
  
  base->lines_start = node;
 }
 else if (!(after = edit_find_point(base)))
 {
  node->next = NULL;
  node->prev = base->lines_end;

  assert(base->lines_start);

  base->lines_end = node;
 }
 else
 {
  if ((node->next = after->next))
    node->next->prev = node;
  else
    base->lines_end = node;
  
  after->next = node;
  node->prev = after;
 }

 ++base->current_line;
 if (base->cached_current_line)
   base->cached_current_line = base->cached_current_line->next;

 ++base->lines;
 if (!base->is_raw)
   base->words += internal_edit_count_words(node->line);
 else
 {
  assert(!base->words);
 }
 
 base->characters += node->length;
}

static int edit_new_line_node(edit_base *base, edit_line_node *node,
                              const char *str, size_t len, int indent)
{
 if (indent)
   len += 2;
 
 if (!(node->line = MALLOC(len + 1)))
   return (FALSE);

 if (indent)
   memcpy(node->line, "> ", 2);
 
 if (indent)
 {
  if (len > 2)
    memcpy(node->line + 2, str, len - 2);
 }
 else
 {
  if (len)
    memcpy(node->line, str, len);
 }
 
 node->line[len] = 0;
 node->length = len; 

 internal_edit_new_line_node(base, node);
 
 return (TRUE);
}

static void edit_delete_line_node(edit_base *base, edit_line_node *node)
{
 if (node->line)
 {
  if (node->prev)
    node->prev->next = node->next;
  else
    base->lines_start = node->next;
  
  if (node->next)
    node->next->prev = node->prev;
  else
    base->lines_end = node->prev;
  
  --base->lines;
  if (!base->is_raw)
    base->words -= internal_edit_count_words(node->line);
  else
  {
   assert(!base->words);
  }

  base->characters -= node->length;
 }
 
 FREE(node->line);
 XFREE(node, EDIT_LINE_NODE);
}

static int edit_insert_line(player *p, const char *str, size_t len)
{
 edit_base *base = EDIT_BASE(p);
 edit_line_node *node = NULL;

 if (base->max_lines && (base->lines >= base->max_lines))
 {
  fvtell_player(SYSTEM_T(p), " You have reached the -- ^S^B%s^s -- limit.\n",
                "line");
  return (FALSE);
 }

 if (base->max_characters &&
     ((EDIT_SIZE(base) + len + 3) > base->max_characters))
 {
  fvtell_player(SYSTEM_T(p), " You have reached the -- ^S^B%s^s -- limit.\n",
                "character");
  return (FALSE);
 }
 
 if (!(node = XMALLOC(sizeof(edit_line_node), EDIT_LINE_NODE)))
   goto mem_node_fail;

 if (!edit_new_line_node(base, node, str, len, FALSE))
   goto mem_line_fail;

 return (TRUE);
 
 mem_line_fail:
 edit_delete_line_node(base, node);
 mem_node_fail:
 P_MEM_ERR(p);
 return (FALSE);
}

int edit_pager_insert_line(player *p, struct iovec *iovs, int num)
{
 edit_base *base = EDIT_BASE(p);
 edit_line_node *node = NULL;
 int count = 0;
 int total_size = 0;
 char *tmp = NULL;
 
 if (base->max_lines && (base->lines >= base->max_lines))
 {
  fvtell_player(SYSTEM_T(p), "%s", " There are too many -- ^S^Blines^s -- "
                "to add mode to the pager.\n");
  return (FALSE);
 }

 while (count < num)
 {
  total_size += iovs[count].iov_len;
  ++count;
 }
 
 if (base->max_characters &&
     ((EDIT_SIZE(base) + total_size + 1) > base->max_characters))
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " There are too many -- ^S^Bcharacters^s -- "
                "to add mode to the pager.\n");
  return (FALSE);
 }

 if (!(node = XMALLOC(sizeof(edit_line_node), EDIT_LINE_NODE)))
   goto mem_node_fail;

 if (!(node->line = MALLOC(total_size + 1)))
   goto mem_line_fail;

 node->length = total_size;
 
 count = 0;
 tmp = node->line;
 while (count < num)
 {
  memcpy(tmp, iovs[count].iov_base, iovs[count].iov_len);
  tmp += iovs[count].iov_len;
  ++count;
 }
 assert((tmp - node->line) == total_size);
 *tmp = 0;

 internal_edit_new_line_node(base, node);
 
 return (TRUE);

 mem_line_fail:
 XFREE(base, EDIT_BASE);
 mem_node_fail:
 P_MEM_ERR(p);
 return (FALSE);
}

static void internal_edit_wipe(edit_base *base)
{
 while (base->lines_start)
   edit_delete_line_node(base, base->lines_start);
 
 base->current_line = 0;
 base->cached_current_line = NULL;
}

static void edit_view_line(player *p)
{
 edit_base *base = EDIT_BASE(p);
 edit_line_node *curr = NULL;

 if (!base->current_line)
 {
  fvtell_player(NORMAL_FT(SYSTEM_INFO, p),
                " You are at the top of the buffer.\n");
  return;
 }
 
 curr = edit_find_point(base);

 if (base->is_raw)
   fvtell_player(BYTES_T(p), "%.*s%n",
                 (int)curr->length, curr->line, OUTPUT_BYTES_TERMINATE);
 else
   fvtell_player(NORMAL_FT(SYSTEM_INFO, p), "%s\n", curr->line);
}

static void edit_input(player *p, const char *str, size_t length)
{
 if (edit_insert_line(p, str, length))
   edit_view_line(p);
}

static int edit_main(player *p, const char *str, size_t length)
{
 ICTRACE("edit_main");
 
 if (!p->buffers->current_edit_base)
   log_assert(FALSE);
 else
 {
  assert(MODE_IN_MODE(p, EDIT));

  MODE_HELPER_COMMAND();

  if (*str == '.')
    return (cmds_sub_match(p, str + 1, length - 1,
                           &cmds_sub[CMDS_SUB_OFFSET(CMDS_SECTION_EDITOR)]));
  else
    edit_input(p, str, length);
 }
 
 return (TRUE);
}

void edit_limit_lines(player *p, unsigned int max_lines)
{
 edit_base *base = EDIT_BASE(p);

 if (max_lines)
 {
  while (max_lines < base->lines)
    edit_delete_line_node(base, base->lines_end);
  
  if (base->current_line > base->lines)
  {
   base->current_line = base->lines;
   base->cached_current_line = NULL;
  }
 }
 
 base->max_lines = max_lines;
}

void edit_limit_characters(player *p, unsigned int max_characters)
{
 edit_base *base = EDIT_BASE(p);

 if (max_characters)
 {
  while (max_characters < base->characters)
    edit_delete_line_node(base, base->lines_end);

  if (base->current_line > base->lines)
  {
   base->current_line = base->lines;
   base->cached_current_line = NULL;
  }
 }
 
 base->max_characters = max_characters;
}

void edit_stats(player *p)
{
 edit_base *base = EDIT_BASE(p);

 fvtell_player(NORMAL_FT(SYSTEM_INFO, p), " Used %u", base->characters);

 if (base->max_characters)
   fvtell_player(NORMAL_FT(SYSTEM_INFO, p), ", out of %u,",
                 base->max_characters);

 fvtell_player(NORMAL_FT(SYSTEM_INFO, p), " bytes; in %u", base->lines);

 if (base->max_lines)
   fvtell_player(NORMAL_FT(SYSTEM_INFO, p), ", out of %u,",
                 base->max_lines);

 fvtell_player(NORMAL_FT(SYSTEM_INFO, p), "%s", " lines");

 if (!base->is_raw)
   fvtell_player(NORMAL_FT(SYSTEM_INFO, p), "; with %u words", base->words);

 fvtell_player(NORMAL_FT(SYSTEM_INFO, p), ".\n"); 
}

static void edit_view(player *p)
{
 edit_line_node *scan = EDIT_BASE(p)->lines_start;

 if (!scan)
   fvtell_player(NORMAL_FT(SYSTEM_INFO, p),
                 " The editor is currently ^S^Bempty^s.\n");
 
 while (scan)
 {
  fvtell_player(NORMAL_FT(SYSTEM_INFO, p), "%s\n", scan->line);
  scan = scan->next;
 }
}

void edit_pager_view(player *p)
{
 edit_line_node *scan = edit_find_point(EDIT_BASE(p));
 int count = p->term_height;

 output_maybe_delete_line(p);
 
 while (scan && (count-- > 0))
 {
  fvtell_player(BYTES_T(p), "%.*s", (int)scan->length, scan->line);
  scan = scan->next;
 }
 fvtell_player(BYTES_T(p), "%n", OUTPUT_BYTES_TERMINATE);
}

static void edit_raw_view(player *p)
{
 edit_line_node *scan = EDIT_BASE(p)->lines_start;

 if (!scan)
   fvtell_player(NORMAL_FT(SYSTEM_INFO, p),
                 " The editor is currently ^S^Bempty^s.\n");
 
 while (scan)
 {
  fvtell_player(NORMAL_FT(SYSTEM_INFO | RAW_OUTPUT, p), "%s\n", scan->line);
  scan = scan->next;
 }
}

static void edit_wipe(player *p)
{
 internal_edit_wipe(p->buffers->current_edit_base);
 
 fvtell_player(NORMAL_FT(SYSTEM_INFO, p), "%s",
               " Edit buffer wiped.\n");
 edit_stats(p);
}

static void edit_rejoin_func(player *p)
{
 fvtell_player(NORMAL_FT(SYSTEM_INFO, p), "%s",
               " Re-Entering editor. Use ^B/help editor^N for help.\n"
               " To run ^Bnormal^N commands type /<command>\n"
               " To run ^Beditor^N commands type .<command>\n"
               " Use '^B.end^N' to ^Bfinish and keep^N edit.\n"
               " Use '^B.quit^N' to ^Bfinish and scrap^N edit.\n");
}

static void internal_edit_cleanup_func(edit_base *base)
{
 internal_edit_wipe(base);
 
 XFREE(base, EDIT_BASE);
}

void edit_cleanup_func(player *p)
{
 edit_base *base = NULL;

 base = p->buffers->current_edit_base;
 cmds_run_func(&p->buffers->current_edit_base->cmd_quit, p, NULL, 0);
 assert(!p->buffers || (base != p->buffers->current_edit_base));

 internal_edit_cleanup_func(base);
}

static void edit_quit(player *p)
{
 assert(MODE_IN_MODE(p, EDIT));
 
 mode_del(p);
}

static void edit_end(player *p)
{ 
 edit_base *base = NULL;
 
 assert(MODE_IN_MODE(p, EDIT));
 
 base = p->buffers->current_edit_base;
 cmds_run_func(&p->buffers->current_edit_base->cmd_end, p, NULL, 0);

 internal_edit_cleanup_func(base);

 CMDS_FUNC_TYPE_NOTHING(&MODE_CURRENT(p).cleanup_func, NULL);
 mode_del(p);
}

static int internal_edit_start(player *p, const char *header,
                               const char *indent)
{
 edit_base *base = NULL;
 unsigned int flags = MODE_FLAGS_DEFAULT;
 cmds_function tmp_cmd;
 cmds_function tmp_rejoin;
 cmds_function tmp_cleanup;
 const char *current = header;
 
 if (p->buffers && p->buffers->current_edit_base)
   return (FALSE);
 
 if (!(base = XMALLOC(sizeof(edit_base), EDIT_BASE)))
 {
  fvtell_player(SYSTEM_T(p), 
                " There is a system problem that prevents you from going into"
                " edit mode at this time.\n");
  return (FALSE);
 }

 if (p->flag_quiet_edit)
   flags = MODE_FLAGS_SYSTEM_INFO_ONLY;

 CMDS_FUNC_TYPE_RET_CHARS_SIZE_T(&tmp_cmd, edit_main);
 CMDS_FUNC_TYPE_NO_CHARS(&tmp_rejoin, edit_rejoin_func);
 CMDS_FUNC_TYPE_NO_CHARS(&tmp_cleanup, edit_cleanup_func);
 
 if (!mode_add(p, "+", MODE_ID_EDIT, flags,
               &tmp_cmd, &tmp_rejoin, &tmp_cleanup))
 {
  XFREE(base, EDIT_BASE);
  fvtell_player(SYSTEM_T(p),
                " You cannot enter edit mode as you are in too many "
                "other modes.\n");
  return (FALSE);
 }

 CMDS_FUNC_TYPE_NOTHING(&base->cmd_quit, NULL);
 CMDS_FUNC_TYPE_NOTHING(&base->cmd_end, NULL);

 base->lines_start = NULL;
 base->lines_end = NULL;
 
 base->max_lines = 0;
 base->max_characters = 0;

 base->lines = 0;
 base->words = 0;
 base->characters = 0;

 base->current_line = 0;
 base->cached_current_line = NULL;
 
 base->is_raw = FALSE;
 
 while (header || indent)
 {
  int do_indent = FALSE;
  
  if (header)
  {
   current = header;
   header = NULL;
  }
  else
  {
   current = indent;
   indent = NULL;
   do_indent = TRUE;
  }
  
  while (current)
  {
   const char *end = NULL;
   edit_line_node *node = XMALLOC(sizeof(edit_line_node), EDIT_LINE_NODE);
   size_t len = 0;
   
   if (!node)
     break;
   
   if ((end = C_strchr(current, '\n')))
   {
    len = end - current;
    ++end;
   }
   else
     len = strlen(current);
   
   if (!edit_new_line_node(base, node, current, len, do_indent))
   {
    XFREE(node, EDIT_LINE_NODE);
    break;
   }
   
   current = end;
  }
 }
 
 fvtell_player(NORMAL_FT(SYSTEM_INFO, p), "%s",
               " Entering editor. Use ^B/help editor^N for help.\n"
               " To run ^Bnormal^N commands type /<command>\n"
               " To run ^Beditor^N commands type .<command>\n"
               " Use '^B.end^N' to ^Bfinish and keep^N edit.\n"
               " Use '^B.quit^N' to ^Bfinish and scrap^N edit.\n");
 
 if (p->flag_quiet_edit)
   fvtell_player(NORMAL_FT(SYSTEM_INFO, p), "%s", 
                 " Blocking shouts and tells whilst in editor.\n");

 p->buffers->current_edit_base = base;
 
 if (base->lines_start)
   edit_view(p);
 
 edit_stats(p);
 
 return (TRUE);
}

int edit_start(player *p, const char *current)
{
 return (internal_edit_start(p, current, NULL));
}

int edit_indent_start(player *p, const char *header, const char *indent)
{
 return (internal_edit_start(p, header, indent));
}

int edit_pager_start(player *p)
{
 edit_base *base = NULL;
 unsigned int flags = MODE_FLAGS_DEFAULT;
 
 if (p->buffers && p->buffers->current_edit_base)
   return (FALSE);
 
 if (!(base = XMALLOC(sizeof(edit_base), EDIT_BASE)))
 {
  fvtell_player(SYSTEM_T(p), 
                " There is a system problem that prevents you from going into"
                " edit mode at this time.\n");
  return (FALSE);
 }

 if (p->flag_quiet_edit)
   flags = MODE_FLAGS_SYSTEM_INFO_ONLY;

 CMDS_FUNC_TYPE_NOTHING(&base->cmd_quit, NULL);
 CMDS_FUNC_TYPE_NOTHING(&base->cmd_end, NULL);

 base->lines_start = NULL;
 base->lines_end = NULL;
 
 base->max_lines = 0;
 base->max_characters = 0;

 base->lines = 0;
 base->words = 0;
 base->characters = 0;

 base->current_line = 0;
 base->cached_current_line = NULL;
 
 base->is_raw = TRUE;
 
 if (p->flag_quiet_edit)
   fvtell_player(NORMAL_FT(SYSTEM_INFO, p), "%s", 
                 " Blocking shouts and tells whilst in pager.\n");

 p->buffers->current_edit_base = base;
 
 return (TRUE);
}

static void edit_backward_line(player *p, const char *str)
{
 edit_base *base = EDIT_BASE(p);
 int togo = 1;
 int before_togo = 1;
 char buf[256];

 if (*(str + strspn(str, "0123456789")))
   TELL_FORMAT(p, "[number of lines]");

 if (!(before_togo = togo = atoi(str)))
   before_togo = togo = 1;
 
 while (togo > 0)
 {
  --togo;
  if (!base->current_line)
  {
   fvtell_player(NORMAL_FT(SYSTEM_INFO, p), "%s",
                 " Reached the top of buffer.\n");
   break;
  }
  else
  {
   --base->current_line;
   if (base->cached_current_line)
     base->cached_current_line = base->cached_current_line->prev;
  }
 }

 switch (before_togo - togo)
 {
  case 1:
    break;

  default:
    fvtell_player(NORMAL_FT(SYSTEM_INFO, p), " Moved up '^S^B%s^s' lines.\n",
                  word_number_base(buf, sizeof(buf), NULL,
                                   (before_togo - togo),
                                   FALSE, word_number_def));
 }

 edit_view_line(p);
}

void edit_pager_backward_line(player *p)
{
 edit_base *base = EDIT_BASE(p);

 if (base->current_line)
 {
  if (base->cached_current_line)
    base->cached_current_line = base->cached_current_line->prev;
  --base->current_line;
  edit_pager_view(p);
 }
}

static void edit_forward_line(player *p, const char *str)
{
 edit_base *base = EDIT_BASE(p);
 int togo = 1;
 int before_togo = 1;
 char buf[256];

 if (*(str + strspn(str, "0123456789")))
   TELL_FORMAT(p, "[number of lines]");

 if (!(before_togo = togo = atoi(str)))
   before_togo = togo = 1;
 
 while (togo > 0)
 {
  --togo;
  if (base->current_line >= base->lines)
  {
   assert(base->current_line == base->lines);
   fvtell_player(NORMAL_FT(SYSTEM_INFO, p), "%s",
                 " Reached the bottom of buffer.\n");
   break;
  }
  else
  {
   ++base->current_line;
   if (base->cached_current_line)
     base->cached_current_line = base->cached_current_line->next;
  }
 }

 switch (before_togo - togo)
 {
  case 1:
    break;

  default:
    fvtell_player(NORMAL_FT(SYSTEM_INFO, p), " Moved down '^S^B%s^s' lines.\n",
                  word_number_base(buf, sizeof(buf), NULL,
                                   (before_togo - togo),
                                   FALSE, word_number_def));
 }

 edit_view_line(p);
}

void edit_pager_forward_line(player *p)
{
 edit_base *base = EDIT_BASE(p);

 if ((base->current_line + p->term_height) <= base->lines)
 {
  edit_line_node *curr = NULL;
  
  ++base->current_line;
  if (base->cached_current_line)
    base->cached_current_line = base->cached_current_line->next;

  output_maybe_delete_line(p);

  curr = edit_find_line(base, base->current_line + p->term_height - 1);
  fvtell_player(BYTES_T(p), "%.*s%n", (int)curr->length, curr->line,
                OUTPUT_BYTES_TERMINATE);
 }
}

void edit_pager_backward_page(player *p)
{
 edit_base *base = EDIT_BASE(p);

 if (base->current_line > p->term_height)
 {
  base->current_line -= p->term_height;
  base->cached_current_line = NULL;
 }
 else
 {
  base->current_line = 1;
  base->cached_current_line = base->lines_start;
 }
 
 edit_pager_view(p);
}

void edit_pager_forward_page(player *p)
{
 edit_base *base = EDIT_BASE(p);
 unsigned int count = 0;

 while ((count < p->term_height) &&
        ((base->current_line + p->term_height) <= base->lines))
 {
  edit_pager_forward_line(p);
  ++count;
 }
}

static void edit_view_commands(player *p)
{ /* FIXME: system info */
 user_cmds_show_section(p, "editor");
}

static void edit_goto_top(player *p)
{
 edit_base *base = EDIT_BASE(p);

 base->current_line = 0;
 base->cached_current_line = NULL;

 fvtell_player(NORMAL_FT(SYSTEM_INFO, p), "%s", " Top of buffer.\n");
}

void edit_pager_goto_top(player *p)
{
 edit_base *base = EDIT_BASE(p);

 base->current_line = 1;
 base->cached_current_line = base->lines_start;
}

static void edit_goto_bottom(player *p)
{
 edit_base *base = EDIT_BASE(p);

 base->current_line = base->lines;
 base->cached_current_line = base->lines_end;

 fvtell_player(NORMAL_FT(SYSTEM_INFO, p), "%s", " Bottom of buffer.\n");
}

void edit_pager_goto_bottom(player *p)
{
 edit_base *base = EDIT_BASE(p);

 if (p->term_height >= base->lines)
   base->current_line = 1;
 else
   base->current_line = base->lines - p->term_height;

 base->cached_current_line = NULL;
}

static void edit_delete_line(player *p, const char *str)
{
 edit_base *base = EDIT_BASE(p);
 edit_line_node *curr = edit_find_point(base);
 int before_togo = 1;
 int togo = 1;
 char buf[256];
 
 if (!curr)
 {
  fvtell_player(SYSTEM_T(p), "%s", " The editor is currently ^S^Bempty^s.\n");
  return;
 }
 
 if (*(str + strspn(str, "0123456789")))
   TELL_FORMAT(p, "[number of lines]");

 if (!(before_togo = togo = atoi(str)))
   before_togo = togo = 1;
 
 while (togo > 0)
 {
  edit_line_node *cache = curr->next;
  
  --togo;
  edit_delete_line_node(base, curr);
  
  if (!(base->cached_current_line = curr = cache))
  {
   base->current_line = base->lines;
   break;
  }
  assert(base->current_line <= base->lines);
 }
 
 switch (before_togo - togo)
 {
  case 1:
    fvtell_player(NORMAL_FT(SYSTEM_INFO, p), " Deleted '^S^B%s^s' Line.\n",
                  "one");
    break;

  default:
    fvtell_player(NORMAL_FT(SYSTEM_INFO, p), " Deleted '^S^B%s^s' lines.\n",
                  word_number_base(buf, sizeof(buf), NULL,
                                   (before_togo - togo),
                                   FALSE, word_number_def));
 }
}

void edit_goto_line(player *p, const char *str)
{
 edit_base *base = EDIT_BASE(p);
 unsigned int lines = strtoul(str, NULL, 0);

 if (lines > base->lines)
 {
  fvtell_player(SYSTEM_T(p), " The current edit buffer only "
                "contains -- ^S^B%d^s -- lines setting position to the end.\n",
                base->lines);
  lines = base->lines;
 }

 base->current_line = lines;
 base->cached_current_line = NULL;

 edit_view(p);
}

void user_toggle_edit_quiet(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_quiet_edit, TRUE,
                       " You will %sblock tells and shouts when editing.\n",
                       " You will %snot block shouts and tells when "
                       "editing.\n",
                       TRUE);

 if (p->flag_quiet_edit && MODE_IN_MODE(p, EDIT))
 {
  MODE_CURRENT(p).flags |= MODE_FLAGS_SYSTEM_INFO_ONLY;
  p->system_info_only = TRUE;
 }
}

void cmds_init_editor(void)
{
 CMDS_BEGIN_DECLS();

#define CMDS_SECTION_SUB CMDS_SECTION_EDITOR
 
 CMDS_ADD_SUB("+line", edit_forward_line, CONST_CHARS);
 CMDS_FLAG(no_clever_expand);
 CMDS_ADD_SUB("-line", edit_backward_line, CONST_CHARS);
 CMDS_FLAG(no_clever_expand);

 CMDS_ADD_SUB("stats", edit_stats, NO_CHARS);

 CMDS_ADD_SUB("bottom", edit_goto_bottom, NO_CHARS);
 CMDS_ADD_SUB("top", edit_goto_top, NO_CHARS);
 CMDS_ADD_SUB("line", edit_goto_line, CONST_CHARS);
 
 CMDS_ADD_SUB("commands", edit_view_commands, NO_CHARS);
 
 CMDS_ADD_SUB("delete", edit_delete_line, CONST_CHARS);
 
 CMDS_ADD_SUB("end", edit_end, NO_CHARS);
 CMDS_FLAG(no_expand);
 CMDS_ADD_SUB("quit", edit_quit, NO_CHARS);
 CMDS_FLAG(no_expand);
 
 CMDS_ADD_SUB("view", edit_view, NO_CHARS);
 CMDS_ADD_SUB("raw_view", edit_raw_view, NO_CHARS);
 
 CMDS_ADD_SUB("input", edit_input, CHARS_SIZE_T);
 CMDS_FLAG(no_beg_space); CMDS_FLAG(no_end_space);
 CMDS_ADD_SUB("insert", edit_input, CHARS_SIZE_T); /* FIXME: aliases */
 CMDS_FLAG(no_beg_space); CMDS_FLAG(no_end_space);
 CMDS_ADD_SUB("wipe", edit_wipe, NO_CHARS);
 
 CMDS_ADD_SUB("quiet", user_toggle_edit_quiet, CONST_CHARS);
 
#undef CMDS_SECTION_SUB

 CMDS_ADD("edit_quiet", user_toggle_edit_quiet, CONST_CHARS, SETTINGS);
}
