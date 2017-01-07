#define PROMPT_C
/*
 *  Copyright (C) 1999, 2000 James Antill
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

void prompt_update(player *p, const char *str)
{
 tmp_output_list_storage tmp_save;
 input_node *in_node = input_find_current(p);
 output_node *out_node1 = NULL;

 if (INVALID_PLAYER_SOCKET(p))
   return;

 if (!player_list_io_find(p))
 {
  player_list_io_add(p);
  PLAYER_EVENT_UPGRADE(p, OUTPUT);
 }

 output_maybe_delete_line(p);
 
 save_tmp_output_list(p, &tmp_save); /* do allow fill = false */
 /* precision is just for speed */
 fvtell_player(ALL_T(p->saved, p, NULL,
                     OVERRIDE_RAW_OUTPUT_VARIABLES | OUTPUT_BUFFER_TMP, now),
               "%.*s", PROMPT_OUTPUT_SZ - 4, str);

 out_node1 = output_list_grab(p);

 load_tmp_output_list(p, &tmp_save);

 p->prompt_print_length = output_list_print_length(p, out_node1, 0,
                                                   PROMPT_OUTPUT_SZ - 4);
 p->prompt_length = output_list_into_buffer(p, out_node1, p->prompt_output,
                                            PROMPT_OUTPUT_SZ - 4);
 p->prompt_out_length = 0;
 
 output_list_cleanup(&out_node1);
 assert(!out_node1);

 if (p->telnet_option_eor_on)
 {
  p->prompt_output[p->prompt_length++] = (char)IAC;
  p->prompt_output[p->prompt_length++] = (char)EOR;
 }
 
 if (in_node && in_node->length &&
     !in_node->in_sub_mode && !in_node->comp_generated && !in_node->ready &&
     !p->passwd_mode && p->telnet_option_do_echo)
   prompt_add_input(p, in_node->input, in_node->length);
}

void prompt_add_input(player *p, const char *input, size_t len)
{
 if (p->passwd_mode || !p->telnet_option_do_echo ||
     ((p->prompt_length + len) > PROMPT_OUTPUT_SZ))
   return;

 if (!p->flag_tmp_prompt_do_output &&
     (p->prompt_length == p->prompt_out_length))
 {
  if (!player_list_io_find(p))
  {
   player_list_io_add(p);
   PLAYER_EVENT_UPGRADE(p, OUTPUT);
  }
 }

 memcpy(p->prompt_output + p->prompt_length, input, len);
 p->prompt_print_length += len;
 p->prompt_length += len;
 p->prompt_output[p->prompt_length] = 0;
}

void prompt_del_input(player *p, size_t len)
{
 if (p->passwd_mode || !p->telnet_option_do_echo)
   return;

 if ((p->prompt_print_length < len) || !len)
   return;
 
 if (!p->flag_tmp_prompt_do_output)
   output_maybe_delete_line(p);

 /* opto for just sending delete characters ... */
 
 p->prompt_print_length -= len;
 p->prompt_length -= len;
 p->prompt_output[p->prompt_length] = 0;
}

char *prompt_do_output(player *p, unsigned int *prompt_length)
{
 output_maybe_delete_line(p);
 
 p->flag_tmp_prompt_do_output = FALSE;
 p->prompt_last_output = now;
 
 if (p->prompt_length)
 {
  *prompt_length = p->prompt_length;
  
  return (p->prompt_output);
 }

 return (NULL);
}

char *prompt_next_output(player *p, unsigned int *prompt_length)
{
 if (p->prompt_length && (p->prompt_out_length < p->prompt_length))
 {
  *prompt_length = p->prompt_length - p->prompt_out_length;
  
  return (p->prompt_output + p->prompt_out_length);
 }

 return (NULL);
}

static void user_set_prompt(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_NORM(p->prompt, str, "prompt", PROMPT_SZ, 0);
}

#define CHECK_COPY(x) do \
 if ((unsigned)(copy_to_tmp - copy_to) < (PROMPT_SZ - sizeof(x))) \
   copy_to_tmp = qstrcpy(copy_to_tmp, x); while (FALSE)
static void user_prompt_convert_form(player *p, const char *str)
{
 const char *copy_from = p->prompt;
 char copy_to[PROMPT_SZ];
 char *copy_to_tmp = copy_to;
 int show_only = TRUE;
 
 if (!beg_strcasecmp(str, "show"))
   show_only = TRUE;
 else if (!beg_strcasecmp(str, "do"))
   show_only = FALSE;
 else
   TELL_FORMAT(p, "<do | show>");

 while (*copy_from && ((copy_to_tmp - copy_to) < (PROMPT_SZ - 1)))
   if (*copy_from == '%')
     switch (*(copy_from + 1))
     {
     case '#':
       CHECK_COPY("$If( =( l(Y) r($F-Set-Duty) ) t(#) f(>) )");
       copy_from += 2;
       break;
       
     case '-':
       *copy_to_tmp++ = ' ';
       copy_from += 2;
       break;
      
     case 'T':
       CHECK_COPY("$F-Time-24");
       copy_from += 2;
       break;
     case 't':
       CHECK_COPY("$F-Time-12");
       copy_from += 2;
       break;
       
     case 'n':
     case 'N': /* do we want these ? */
       if (*(copy_from + 1) == 'n')
         CHECK_COPY("$Talker-Name_short");
       else
         CHECK_COPY("$Talker-Name_long");
       copy_from += 2;
      break;      
      
    case 's':
      CHECK_COPY("$F-Spodnumber");
      copy_from += 2;
      break;
      
    case '%':
      copy_from++; /* skip the second '%' */
    default:
      copy_from++;
      *copy_to_tmp++ = '%';
    }
  else /* if *copy_from == '%' */
    *copy_to_tmp++ = *copy_from++; 

 *copy_to_tmp++ = 0;
 
 if (show_only)
   fvtell_player(NORMAL_FT(RAW_OUTPUT, p),
                 " Your prompt before conversion is \n%s\n"
                 " after conversion it is\n%s\n", p->prompt, copy_to);
 else
   qstrcpy(p->prompt, copy_to);
}

void cmds_init_prompt(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("convert_prompt", user_prompt_convert_form, CONST_CHARS, MISC);

 CMDS_ADD("prompt", user_set_prompt, CONST_CHARS, SETTINGS);
 CMDS_FLAG(no_beg_space); CMDS_FLAG(no_end_space);
}
