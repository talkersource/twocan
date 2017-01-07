#define PAGER_C
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


static void pager_quit(player *p)
{
 mode_del(p);
}

static void pager_quit_func(player *p)
{
 buffer_pager_destroy(p);
 fvtell_player(ALL_T(NULL, p, NULL, SYSTEM_INFO, 0), "%s", "^N\n");
}

static void pager_prompt(player *p)
{
 edit_base *base = EDIT_BASE(p);
 char prompt_tmp[128];
 
 if ((base->current_line + p->term_height) <= base->lines)
   sprintf(prompt_tmp,  "^N[Pager: line (^4%d/%d^0) %s] ",
           base->current_line, base->lines, "^BType h for help^b"); 
 else
   if (p->flag_pager_auto_quit)
   {
    pager_quit(p);
    return; /* don't do a prompt... just quit */
   }
   else
     sprintf(prompt_tmp, "^N[Pager: %s %s] ",
	     "(^4END^0)", "^BType h for help^b");
  
 prompt_update(p, prompt_tmp);
}

static void pager_rejoin_func(player *p)
{ /* not used... as you can't leave the pager atm. */
 log_assert(FALSE);
 pager_prompt(p);
}

static void pager_command(player *p, const char *str, size_t length)
{
 edit_base *base = EDIT_BASE(p);
 
 ICTRACE("pager");
 
 output_maybe_delete_line(p); /* FIXME: */

 before_switch_command:
 /* switch (tolower((unsigned char) *str)) */
 switch (*str)
 {
  case 0:/* pressed return */
    if (p->flag_page_on_return)
      str = "n";
    else
      str = "f";
    goto before_switch_command;
    
  case ' ':
  case '.':
    if (!p->flag_page_on_return)
      str = "n";
    else
      str = "f";
    goto before_switch_command;
    
  case 'G':
  case 'e':
    edit_pager_goto_bottom(p);
    edit_pager_view(p);
    break;
    
  case 'a':
  {
   unsigned int line = 0;
   
   do 
   {
    line = base->current_line;
    edit_pager_forward_page(p);
   } while (line != base->current_line);
  }
  break;
  
  case 'r':
    edit_pager_view(p);
    break;
    
  case 'f':
    edit_pager_forward_line(p);
    break;
    
  case 'b':
  case 'p':
    edit_pager_backward_page(p);
    break;
    
  case 'n':
    edit_pager_forward_page(p);
    break;
    
  case 'g':
  case 't':
    edit_pager_goto_top(p);
    edit_pager_view(p);
    break;
    
  case '?':
  case 'h':
    fvtell_player(SYSTEM_T(p), "\n"
                  "a -- all the rest of the file\n"
                  "b -- previous page\n"
                  "e -- end page\n"
                  "f -- forward one line\n"
                  "h -- this text\n"
                  "n -- next page\n"
                  "p -- previous page\n"
                  "q -- quit the pager\n"
                  "r -- refresh page\n"
                  "s -- search forward\n"
                  "t -- top page\n"
                  "? -- this text\n"
                  "/ -- search forward\n"
                  "%s"
                  "Anything else does nothing.\n\n",
                  p->flag_page_on_return ?
                  (". -- forward line\n"
                   "Return -- forward page\n"
                   "Space -- forward line\n"
                   ) :
                  (". -- forward page\n"
                   "Return -- forward line\n"
                   "Space -- forward page\n"		
                   )
                  );
    break;
    
  case '/':
  case 's':
  {
   edit_line_node *scan = edit_find_point(base);
   char *search = p->buffers->pager_buff->searching;
   int count = 0;
   
   ++str;
   if (!*str)
   {
    if (!search)
    {
     fvtell_player(ALL_T(NULL, p, NULL, SYSTEM_INFO, 0), "%s",
                   " You need to have a string to search for.\n");
     break;
    }
    
    ++count;
    scan = scan->next;
   }
   else
   {
    if (search)
      FREE(p->buffers->pager_buff->searching);
    
    /* length must be at least 1 too big (the '/' char)...
     * so the terminator is in there*/
    if (!(p->buffers->pager_buff->searching = MALLOC(sizeof(char) * length)))
    {
     P_MEM_ERR(p);
     break;
    }
    
    search = p->buffers->pager_buff->searching;
    strcpy(search, str);
   }
   
   while (scan)
   {
    if (strstr(scan->line, search))
      break;
    ++count;
    scan = scan->next;
   }
   
   if (scan)
   {
    base->current_line += count;
    edit_pager_view(p);
   }
   else
     fvtell_player(SYSTEM_T(p),
                   " The string -- ^S^B%s^s -- was not found.\n", search);
  }
  break;
  
  case 'q':
    pager_quit(p);
    return;
    
  default:
    fvtell_player(SYSTEM_T(p), "%s",
                  " That is not a pager command.\n");
 }
 
 pager_prompt(p);   
}

void pager(player *p, int flags)
{
 output_node *out_node = 0;
 int created = 0;
 unsigned int lines = 0;
 cmds_function tmp_cmd;
 cmds_function tmp_rejoin;
 cmds_function tmp_cleanup;
    
 CMDS_FUNC_TYPE_CHARS_SIZE_T(&tmp_cmd, pager_command);
 CMDS_FUNC_TYPE_NO_CHARS(&tmp_rejoin, pager_rejoin_func);
 CMDS_FUNC_TYPE_NO_CHARS(&tmp_cleanup, edit_cleanup_func);

 if (p->flag_pager_off && !(flags & PAGER_USE_FORCE))
   return;
 
 if (INVALID_PLAYER_SOCKET(p))
   return;
 
 output_maybe_delete_line(p);

 output_for_player_cleanup(p);
 
 out_node = p->output_start; 
 while (out_node && (lines <= p->term_height))
 { /* NOTE: don't care if it has already gone out... but we DON'T want
      BYTE stufff, Ie. prompt strings, telnet options */
  if (out_node->timestamp && OUTPUT_NODE_USE(out_node, now) &&
      !(out_node->flags & (OUTPUT_REPEAT | OUTPUT_BYTES)))
  {
   if (out_node->has_return)
     lines++;
  }
  
  out_node = out_node->next;
 }
 
 if (lines <= p->term_height)
   return;

 if (!(created = buffer_pager_create(p)))
 {
  struct iovec iovs[OUTPUT_WRITE_MAX_NODES];
  int used = 0;
  
  if (!edit_pager_start(p))
    goto malloc_edit_fail;

  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_quit, pager_quit_func);
  
  if (!mode_add(p, NULL, MODE_ID_PAGER, MODE_FLAGS_SYSTEM_INFO_ONLY,
                &tmp_cmd, &tmp_rejoin, &tmp_cleanup))
  {
   fvtell_player(SYSTEM_T(p), 
                 " You cannot enter the pager as you are in too many "
                 "other modes.\n");
   edit_cleanup_func(p); /* no goto -- pretend it worked and quit */
   return;
  }
  
  out_node = p->output_start; 
  while (out_node)
  {
   while (out_node)
   {
    if (out_node->timestamp && OUTPUT_NODE_USE(out_node, now) &&
        !(out_node->flags & (OUTPUT_REPEAT | OUTPUT_BYTES)))
    {
     iovs[used].iov_len = out_node->length - out_node->output_already;
     iovs[used].iov_base = out_node->start + out_node->output_already;
     ++used;
     
     out_node->output_already = TRUE;

     if (out_node->has_return)
       break;
     if (used >= OUTPUT_WRITE_MAX_NODES)
       break;
    }
    
    out_node = out_node->next;
   }

   assert(used <= OUTPUT_WRITE_MAX_NODES);
   if (!edit_pager_insert_line(p, iovs, used))
   { /* FIXME: lose some output due to output_already being marked on
      * all pager nodes so far */
    edit_cleanup_func(p); /* no goto -- pretend it worked and quit */
    return;
   }
   used = 0;

   if (out_node)
     out_node = out_node->next;
  }
 }
 else if (created > 0)
   goto malloc_pager_buffer_fail;
 else
 { /* FIXME: can be altered fairly easily to work now... do we want to ? */
  fvtell_player(NORMAL_T(p), "%s",
                " You cannot use a command that enters the pager whilst "
                "already in the pager.\n");
  return;
 }

 edit_pager_goto_top(p);
 edit_pager_view(p);
 
 pager_prompt(p);

 return;
 
 malloc_edit_fail:
 buffer_pager_destroy(p);
 
 malloc_pager_buffer_fail:
 P_MEM_ERR(p);
}

static void user_toggle_pager_off(player *p, const char *str)
{
 TOGGLE_COMMAND_OFF_ON(p, str, p->flag_pager_off, TRUE,
                       " You will %snot get paged output.\n",
                       " You will %sget paged output.\n", TRUE);
}

static void user_toggle_pager_return(player *p, const char *str)
{
 const char *now_str = "^S^Bnow^s "; /* not quite correct */

 if (*str) /* speed hack */
 {
  if (beg_strcasecmp("page", str))
    if (beg_strcasecmp("line", str))
      if (!TOGGLE_MATCH_ON(str))
        if (!TOGGLE_MATCH_OFF(str))
          if (beg_strcasecmp("autoquit", str) &&
              beg_strcasecmp("auto_quit", str) &&
              beg_strcasecmp("auto quit", str))
            if (beg_strcasecmp("manualquit", str) &&
                beg_strcasecmp("manual_quit", str) &&
                beg_strcasecmp("manual quit", str))
            {
             now_str = "";
            }
            else
              p->flag_pager_auto_quit = FALSE;
          else
            p->flag_pager_auto_quit = TRUE;
        else
          p->flag_pager_off = TRUE;
      else
        p->flag_pager_off = FALSE;
    else
      p->flag_page_on_return = FALSE;
  else
    p->flag_page_on_return = TRUE;
 }
 else
   now_str = "";
 
 /* tell them what they have chosen */
 if (p->flag_pager_off)
   fvtell_player(NORMAL_T(p), " You will not %sget paged output.\n", now_str);
 else
 {
  fvtell_player(NORMAL_T(p),
                " You will %sget paged output with these options...\n",
                now_str);
  
  if (p->flag_page_on_return)
    fvtell_player(NORMAL_T(p), "%s",
                  "  1) When you press return in the pager you "
                  "will get a page of text.\n");
  else
    fvtell_player(NORMAL_T(p), "%s",
                  "  1) When you press return in the pager you "
                  "will get a line of text.\n");
  
  if (p->flag_pager_auto_quit)
    fvtell_player(NORMAL_T(p), "%s",
                  "  2) You will automaticaly quit the pager when "
                  "you reach the bottom.\n");
  else
    fvtell_player(NORMAL_T(p), "%s",
                  "  2) You will have to manualy quit the pager.\n");
 }
}

void cmds_init_pager(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("nopager", user_toggle_pager_off, CONST_CHARS, HIDDEN);
 CMDS_FLAG(no_expand);

 CMDS_ADD("pager", user_toggle_pager_return, CONST_CHARS, SETTINGS);
}
