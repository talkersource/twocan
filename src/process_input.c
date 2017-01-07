#define PROCESS_INPUT_C
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

input_node *input_add(player *p, input_node *in_node)
{
 input_node *new_node = XMALLOC(sizeof(input_node), INPUT_NODE);

 if (!new_node)
   return (NULL);
 
 if (++p->input_node_count > INPUT_NODE_QUEUE_HARD_SZ)
   user_logoff(p, NULL);
 
 new_node->in_sub_mode = FALSE;
 new_node->comp_generated = FALSE;
 new_node->ready = FALSE;
 new_node->input[0] = 0;
 new_node->length = 0;
 
 if (!in_node)
 {
  assert(!p->input_start);
  p->input_start = new_node;
  new_node->next = NULL;
  new_node->prev = NULL;
  return (new_node);
 }

 new_node->next = in_node->next;
 new_node->prev = in_node;
 in_node->next = new_node;

 return (new_node);
}

void input_del(player *p, input_node *togo)
{
 if (togo->prev)
   togo->prev->next = togo->next;
 else
   p->input_start = togo->next;

 if (togo->next)
   togo->next->prev = togo->prev;
 
 XFREE(togo, INPUT_NODE);

 assert(p->input_node_count > 0);
 --p->input_node_count;
}

void input_del_all_comp(player *p)
{
 input_node *in_node = p->input_start;

 while (in_node)
 {
  input_node *in_node_next = in_node->next;

  if (in_node->comp_generated)
    input_del(p, in_node);
  
  in_node = in_node_next;
 }
}

input_node *input_find_current(player *p)
{
 input_node *scan = p->input_start;
 input_node *tmp = NULL;
 unsigned int count = p->input_node_count;
 
 while (scan && (count > 1))
 {
  assert(scan->next);

  if (!scan->ready)
    tmp = scan;
  
  scan = scan->next;
  --count;
 }

 if (!scan || scan->ready)
 {
  if (tmp)
    return (tmp);
  else
    return (input_add(p, scan));
 }
 
 return (scan);
}

static void input_backspace(input_node *in_node)
{
 assert(in_node);

 if (in_node->length > 0)
   --in_node->length;
}

void telopt_ask_passwd_mode_on(player *p)
{ /* we say that we will echo localy ... and we don't hence no chars */
 p->passwd_mode = TRUE;

 if (!p->telnet_option_do_echo)
 {
  p->telnet_option_passwd_restop = TRUE;
  telopt_ask_echo_local(p);
 }
}

void telopt_ask_passwd_mode_off(player *p)
{
 log_assert(p->passwd_mode);
 p->passwd_mode = FALSE;

 if (p->telnet_option_passwd_restop)
 {
  p->telnet_option_passwd_restop = FALSE;
  if (IN_TELOPT_CMD(p, echo))
    /* if they pass the name and password before we get confermation,
     * queue an echo local off */
    p->telnet_option_bounce_echo_off = TRUE;
  else
    telopt_ask_echo_remote(p);
 }
}

void telopt_ask_echo_local(player *p)
{ /* we say that we will echo localy ... and we don't hence no chars */
 unsigned char telopts_out[3] = {IAC, WILL, TELOPT_ECHO};
 if (!IN_TELOPT_CMD(p, echo))
 {
  fvtell_player(BYTES_T(p), "%c%c%c%n",
                telopts_out[0], telopts_out[1], telopts_out[2],
                OUTPUT_BYTES_TERMINATE);
  DEBUG_PRINT(SOCKET_DEBUG, " SENT WILL TELOPT_ECHO");
  ON_TELOPT_CMD(p, echo);
 }
}

void telopt_ask_echo_remote(player *p)
{ /* we say that we will echo localy ... and we don't hence no chars */
 unsigned char telopts_out[3] = {IAC, WONT, TELOPT_ECHO};
 if (!IN_TELOPT_CMD(p, echo))
 {
  fvtell_player(BYTES_T(p), "%c%c%c%n",
                telopts_out[0], telopts_out[1], telopts_out[2],
                OUTPUT_BYTES_TERMINATE);
  DEBUG_PRINT(SOCKET_DEBUG, " SENT WONT TELOPT_ECHO");
  ON_TELOPT_CMD(p, echo);
 }
}

void telopt_ask_end_or_record(player *p)
{
 unsigned char telopts_out[3] = {IAC, WILL, TELOPT_EOR};
 if (!IN_TELOPT_CMD(p, eor))
 {
  fvtell_player(BYTES_T(p), "%c%c%c%n",
                telopts_out[0], telopts_out[1], telopts_out[2],
                OUTPUT_BYTES_TERMINATE);
  DEBUG_PRINT(SOCKET_DEBUG, " SENT WILL TELOPT_EOR");
  ON_TELOPT_CMD(p, eor);
 }
}

void telopt_ask_not_end_or_record(player *p)
{
 unsigned char telopts_out[3] = {IAC, WONT, TELOPT_EOR};
 if (!IN_TELOPT_CMD(p, eor))
 {
  fvtell_player(BYTES_T(p), "%c%c%c%n",
                telopts_out[0], telopts_out[1], telopts_out[2],
                OUTPUT_BYTES_TERMINATE);
  DEBUG_PRINT(SOCKET_DEBUG, " SENT WONT TELOPT_EOR");
  ON_TELOPT_CMD(p, eor);
 }
}

void telopt_ask_chars_mode(player *p)
{
 unsigned char telopts_out[3] = {IAC, WILL, TELOPT_SGA};
 if (!IN_TELOPT_CMD(p, sga))
 {
  fvtell_player(BYTES_T(p), "%c%c%c%n",
                telopts_out[0], telopts_out[1], telopts_out[2],
                OUTPUT_BYTES_TERMINATE);
  DEBUG_PRINT(SOCKET_DEBUG, " SENT WILL TELOPT_SGA");
  ON_TELOPT_CMD(p, sga);
 }
}

void telopt_ask_line_mode(player *p)
{
 unsigned char telopts_out[3] = {IAC, WONT, TELOPT_SGA};
 if (!IN_TELOPT_CMD(p, sga))
 {
  fvtell_player(BYTES_T(p), "%c%c%c%n",
                telopts_out[0], telopts_out[1], telopts_out[2],
                OUTPUT_BYTES_TERMINATE);
  DEBUG_PRINT(SOCKET_DEBUG, " SENT WONT TELOPT_SGA");
  ON_TELOPT_CMD(p, sga);
 }
}

void telopt_ask_term_type(player *p)
{ /* ask for the term type */
 unsigned char telopts_out[3] = {IAC, DO, TELOPT_TTYPE};
 fvtell_player(BYTES_T(p), "%c%c%c%n",
               telopts_out[0], telopts_out[1], telopts_out[2],
               OUTPUT_BYTES_TERMINATE);
 DEBUG_PRINT(SOCKET_DEBUG, " SENT DO TELOPT_TTYPE");
}

void telopt_ask_term_sizes(player *p)
{ /* ask for the term width/height */
 if (!IN_TELOPT_CMD(p, naws))
 {
  unsigned char telopts_out[3] = {IAC, DO, TELOPT_NAWS};
  fvtell_player(BYTES_T(p), "%c%c%c%n",
                telopts_out[0], telopts_out[1], telopts_out[2],
                OUTPUT_BYTES_TERMINATE);
  DEBUG_PRINT(SOCKET_DEBUG, " SENT DO TELOPT_NAWS");
  ON_TELOPT_CMD(p, naws);
 }
}

static void telopt_ask_term_sizes_no(player *p)
{
 if (!IN_TELOPT_CMD(p, naws))
 {
  unsigned char telopts_out[3] = {IAC, DONT, TELOPT_NAWS};
  fvtell_player(BYTES_T(p), "%c%c%c%n",
                telopts_out[0], telopts_out[1], telopts_out[2],
                OUTPUT_BYTES_TERMINATE);
  DEBUG_PRINT(SOCKET_DEBUG, " SENT DONT TELOPT_NAWS");
  ON_TELOPT_CMD(p, naws);
 }
}

void telopt_ask_compress_do(player *p)
{ /* ask for compression */
 if (!IN_TELOPT_CMD(p, compress))
 {
  unsigned char telopts_out[3] = {IAC, WILL, TELOPT_COMPRESS};
  fvtell_player(BYTES_T(p), "%c%c%c%n",
                telopts_out[0], telopts_out[1], telopts_out[2],
                OUTPUT_BYTES_TERMINATE);
  DEBUG_PRINT(SOCKET_DEBUG, " SENT WILL TELOPT_COMPRESS");
  ON_TELOPT_CMD(p, compress);
 }
}

void telopt_ask_no_compress(player *p)
{ /* ask for no compression */
 if (!IN_TELOPT_CMD(p, compress))
 {
  unsigned char telopts_out[3] = {IAC, WONT, TELOPT_COMPRESS};
  fvtell_player(BYTES_T(p), "%c%c%c%n",
                telopts_out[0], telopts_out[1], telopts_out[2],
                OUTPUT_BYTES_TERMINATE);
  DEBUG_PRINT(SOCKET_DEBUG, " SENT WONT TELOPT_COMPRESS");
  ON_TELOPT_CMD(p, compress);
 }
}

void telopt_ask_compress_start(player *p)
{ /* start compression */
 if (output_compress_start_1(p))
 { /* this must go out with highest priority */
  unsigned char telopts_out[5] = {IAC, SB, TELOPT_COMPRESS, WILL, SE};
  fvtell_player(PRIO_BYTES_FT(OUTPUT_COMPRESS_MARK, p), "%c%c%c%c%c%n",
                telopts_out[0], telopts_out[1], telopts_out[2],
                telopts_out[3], telopts_out[4],
                OUTPUT_BYTES_TERMINATE);
  DEBUG_PRINT(SOCKET_DEBUG, " SENT SB TELOPT_COMPRESS WILL SE");
 }
}

void telopt_ask_compress_stop(player *p)
{ /* stop compression */
 if (output_compress_stop_1(p))
 {
  unsigned char telopts_out[5] = {IAC, SB, TELOPT_COMPRESS, WONT, SE};
  fvtell_player(BYTES_FT(OUTPUT_COMPRESS_MARK, p), "%c%c%c%c%c%n",
                telopts_out[0], telopts_out[1], telopts_out[2],
                telopts_out[3], telopts_out[4],
                OUTPUT_BYTES_TERMINATE);
  DEBUG_PRINT(SOCKET_DEBUG, " SENT SB TELOPT_COMPRESS WONT SE");
 }
}

static int input_parse_telnet_options(player *p, input_node **in_node,
                                      const char *const passed_input,
                                      size_t length)
{
 const unsigned char *const input = (const unsigned char *)passed_input;
 
 BTRACE("input_parse_telnet_options");
 
 assert(input[0] == IAC);
 INPUT_ASSURE_NEXT(2);
 
 switch (input[1])
 {
  case NOP: /* no operation */
  case IAC: /* ignore char 255 atm. -- is this in the rfc ? */
    BTRACE("input_parse_telnet_options");
    break;
    
  case EC: /* erase character */
    BTRACE("input_parse_telnet_options");
    DEBUG_PRINT(SOCKET_DEBUG, " ERASE CHAR");
    input_backspace(*in_node);
    prompt_del_input(p, 1);
    break;
  case EL: /* erase line */
    BTRACE("input_parse_telnet_options");
    DEBUG_PRINT(SOCKET_DEBUG, " ERASE LINE");
    prompt_del_input(p, (*in_node)->length);
    (*in_node)->length = 0;
    break;

  case AO: /* abort output and quit... since we always output a prompt
            * this can be treated like IP */
    BTRACE("input_parse_telnet_options");
    DEBUG_PRINT(SOCKET_DEBUG, " SYNC");
#if 1
    { /* I'm not sure if this is correct ... */
     unsigned char telopts_out[2] = {IAC, SYNCH};
     fvtell_player(BYTES_T(p), "%c%c%n",
                   telopts_out[0], telopts_out[1],
                   OUTPUT_BYTES_TERMINATE);
    }
#endif
    /* FALLTHROUGH */
  case ABORT:
    BTRACE("input_parse_telnet_options");
    DEBUG_PRINT(SOCKET_DEBUG, " ABORT");
    /* FALLTHROUGH */
  case IP: /* interupt process permanently */
    BTRACE("input_parse_telnet_options");
    user_logoff(p, NULL);
    DEBUG_PRINT(SOCKET_DEBUG, " KILL");
    break;
    
  case AYT: /* are you there... we should print something */
    BTRACE("input_parse_telnet_options");
    fvtell_player(BYTES_T(p), "%c%n", '*', OUTPUT_BYTES_TERMINATE);
    DEBUG_PRINT(SOCKET_DEBUG, " AYT");
    break;
    
    /* DO DONT WILL WONT ... */
  case DO: /* requests for us to do stuff */
    BTRACE("input_parse_telnet_options");
    INPUT_ASSURE_NEXT(3);
    
    switch (input[2])
    {
     case TELOPT_LOGOUT:
     {
      BTRACE("input_parse_telnet_options");
      DEBUG_PRINT(TRUE, " DO TELOPT_LOGOUT");
      user_logoff(p, NULL);
     } 
     break;
       
     case TELOPT_ECHO:
       BTRACE("input_parse_telnet_options");
       DEBUG_PRINT(SOCKET_DEBUG, " DO TELOPT_ECHO");
       p->telnet_option_do_echo = TRUE;

       p->extra_screen_routines = TRUE;
       p->repeat_prompt = TRUE;

       /* telopt_ask_echo_local(p); -- this should be here ...
        * but tf sometimes sends 2 responces to an IAC */
       OFF_TELOPT_CMD(p, echo);
       if (p->telnet_option_bounce_echo_off)
         /* hope this is legal, as we possibly have 2 options on the wire atm*/
         telopt_ask_echo_remote(p);
       break;
     case TELOPT_SGA:
       BTRACE("input_parse_telnet_options");
       DEBUG_PRINT(SOCKET_DEBUG, " DO TELOPT_SGA");

       telopt_ask_chars_mode(p);
       OFF_TELOPT_CMD(p, sga);
       break;
     case TELOPT_EOR: /* end or record */
       BTRACE("input_parse_telnet_options");
       DEBUG_PRINT(SOCKET_DEBUG, " DO TELOPT_EOR");
       p->telnet_option_eor_on = TRUE;

       telopt_ask_end_or_record(p);
       OFF_TELOPT_CMD(p, eor);
       break;

     case TELOPT_COMPRESS:
       BTRACE("input_parse_telnet_options");
       DEBUG_PRINT(SOCKET_DEBUG, " DO TELOPT_COMPRESS");

       telopt_ask_compress_do(p);
       OFF_TELOPT_CMD(p, compress);
       if (!p->output_compress_do)
         telopt_ask_compress_start(p);
       break;
       
     default:
     {
      unsigned char telopts_out[3] = {IAC, WONT, 0};
      BTRACE("input_parse_telnet_options");
      DEBUG_START(SOCKET_DEBUG);
      fprintf(stderr, " DO not delat with... %d %c", (int)input[2], input[2]);
      DEBUG_END();
      telopts_out[2] = input[2];
      fvtell_player(BYTES_T(p), "%c%c%c%n",
                    telopts_out[0], telopts_out[1], telopts_out[2],
                    OUTPUT_BYTES_TERMINATE);
     }
    }
    BTRACE("input_parse_telnet_options");
    return (3);
    
  case DONT: /* request for us NOT to do stuff */
    BTRACE("input_parse_telnet_options");
    INPUT_ASSURE_NEXT(3);
    
    switch (input[2])
    {
     case TELOPT_ECHO:
       BTRACE("input_parse_telnet_options");
       DEBUG_PRINT(SOCKET_DEBUG, " DONT TELOPT_ECHO");
       p->telnet_option_do_echo = FALSE;
       p->telnet_option_bounce_echo_off = FALSE;

       p->extra_screen_routines = FALSE;
       p->repeat_prompt = FALSE;

       /* telopt_ask_echo_remote(p); -- this should be here ...
        * but tf sometimes sends 2 responces to an IAC */
       OFF_TELOPT_CMD(p, echo);       
       break;
     case TELOPT_SGA:
       BTRACE("input_parse_telnet_options");
       DEBUG_PRINT(SOCKET_DEBUG, " DONT TELOPT_SGA");

       telopt_ask_line_mode(p);
       OFF_TELOPT_CMD(p, sga);
       break;
     case TELOPT_EOR: /* end or record */
       BTRACE("input_parse_telnet_options");
       DEBUG_PRINT(SOCKET_DEBUG, " DONT TELOPT_EOR");
       p->telnet_option_eor_on = FALSE;
       
       telopt_ask_not_end_or_record(p);
       OFF_TELOPT_CMD(p, eor);
       break;

     case TELOPT_COMPRESS:
       BTRACE("input_parse_telnet_options");
       DEBUG_PRINT(SOCKET_DEBUG, " DONT TELOPT_COMPRESS");

       telopt_ask_no_compress(p);
       OFF_TELOPT_CMD(p, compress);
       if (p->output_compress_do)
         telopt_ask_compress_stop(p);
       break;

     default:
     {
      unsigned char telopts_out[3] = {IAC, WONT, 0};
      BTRACE("input_parse_telnet_options");
      DEBUG_START(SOCKET_DEBUG);
      fprintf(stderr, " DONT not delat with... %d %c",
              (int)input[2], input[2]);
      DEBUG_END();
      telopts_out[2] = input[2];
      fvtell_player(BYTES_T(p), "%c%c%c%n",
                    telopts_out[0], telopts_out[1], telopts_out[2],
                    OUTPUT_BYTES_TERMINATE);
     }
    }
    BTRACE("input_parse_telnet_options");
    return (3);
    
  case WILL: /* otherside wants to enable an option */
    BTRACE("input_parse_telnet_options");
    INPUT_ASSURE_NEXT(3);
    
    switch (input[2])
    {
     case TELOPT_NAWS:
     { /* tf sends this... negotiate window size */
      BTRACE("input_parse_telnet_options");
      DEBUG_PRINT(SOCKET_DEBUG, " SEND SB TELOPT_NAWS");
      telopt_ask_term_sizes(p);
      OFF_TELOPT_CMD(p, naws);
     }
     break;

     case TELOPT_LINEMODE:
       BTRACE("input_parse_telnet_options");
       DEBUG_PRINT(SOCKET_DEBUG, " WILL TELOPT_LINEMODE");
       break;

     case TELOPT_TTYPE: /* telopt terminal type */
     {
      unsigned char telopts_out[6] = {IAC, SB, TELOPT_TTYPE,
                                      TELQUAL_SEND, IAC, SE};
      BTRACE("input_parse_telnet_options");
      DEBUG_PRINT(SOCKET_DEBUG, " WILL TELOPT_TTYPE");
      fvtell_player(BYTES_T(p), "%c%c%c%c%c%c%n",
                    telopts_out[0], telopts_out[1], telopts_out[2],
                    telopts_out[3], telopts_out[4], telopts_out[5],
                    OUTPUT_BYTES_TERMINATE);
      DEBUG_PRINT(SOCKET_DEBUG, " SENT SB TTYPE");

      telopt_ask_echo_local(p);

      p->extra_screen_routines = TRUE;
      p->repeat_prompt = TRUE;
     }
     break;
     
     default:
     {
      unsigned char telopts_out[3] = {IAC, DONT, 0};
      BTRACE("input_parse_telnet_options");
      DEBUG_START(SOCKET_DEBUG);
      fprintf(stderr, " WILL not delat with... %d %c",
              (int)input[2], input[2]);
      DEBUG_END();
      telopts_out[2] = input[2];
      fvtell_player(BYTES_T(p), "%c%c%c%n",
                    telopts_out[0], telopts_out[1], telopts_out[2],
                    OUTPUT_BYTES_TERMINATE);
     }
    }
    BTRACE("input_parse_telnet_options");
    return (3);
    
 case WONT: /* otherside wants to disable an option */
   BTRACE("input_parse_telnet_options");
   INPUT_ASSURE_NEXT(3);
   
   switch (input[2])
   {
    case TELOPT_NAWS:
      /* tush doesn't give this out */
      BTRACE("input_parse_telnet_options");
      DEBUG_PRINT(SOCKET_DEBUG, " WONT TELOPT_NAWS");
      telopt_ask_term_sizes_no(p);
      OFF_TELOPT_CMD(p, naws);
      break;
      
    case TELOPT_LINEMODE:
      /* they are going to send stuff in chars now... Woo woo */
      BTRACE("input_parse_telnet_options");
      DEBUG_PRINT(SOCKET_DEBUG, " WONT TELOPT_LINEMODE");
      break;
      
    case TELOPT_TTYPE: /* telopt terminal type */
      /* TF doesn't like giving this out.. :( */
      BTRACE("input_parse_telnet_options");
      DEBUG_PRINT(SOCKET_DEBUG, " WONT TELOPT_TTYPE");
      break;
      
    default:
    {
     unsigned char telopts_out[3] = {IAC, DONT, 0};
     BTRACE("input_parse_telnet_options");
     DEBUG_START(SOCKET_DEBUG);
     fprintf(stderr, " WONT not delat with... %d %c",
             (int)input[2], input[2]);
     DEBUG_END();
     telopts_out[2] = input[2];
     fvtell_player(BYTES_T(p), "%c%c%c%n",
                   telopts_out[0], telopts_out[1], telopts_out[2],
                   OUTPUT_BYTES_TERMINATE);
    }
   }
   BTRACE("input_parse_telnet_options");
   return (3);
   
  case SB:
    BTRACE("input_parse_telnet_options");
    DEBUG_PRINT(SOCKET_DEBUG, " SB");
    if (!(*in_node = input_add(p, *in_node)))
    {
     log_assert(FALSE);
     user_logoff(p, NULL);
     BTRACE("input_parse_telnet_options");
     return (0);
    }
     
    (*in_node)->in_sub_mode = TRUE;
    break;
   
  case SE:
    BTRACE("input_parse_telnet_options");
  {
   unsigned char *tmp = (unsigned char *)(*in_node)->input;
   
   switch (*tmp)
   {
    case TELOPT_LINEMODE:
      BTRACE("input_parse_telnet_options");
      break;
      
    case TELOPT_NAWS:
    {
     int width = 0;
     int height = 0;
     
     BTRACE("input_parse_telnet_options");
     
     ++tmp;

     if ((*in_node)->length < 5)
       break;
     
     width = *tmp;
     width <<= 8;
     ++tmp;
     width |= *tmp;
     ++tmp;
     
     height = *tmp;
     height <<= 8;
     ++tmp;
     height |= *tmp;
     ++tmp;
     
     log_assert(!*tmp);

     DEBUG_START(SOCKET_DEBUG);
     fprintf(stderr, " height = %d, width = %d\n", height, width);
     DEBUG_END();
     
     if ((width > 35) && (width < (5 * 1024)))
       p->term_width = width - 1;
     else if (!width)
       p->term_width = 0;
     
     if ((height > 4) && (height < 256))
       p->term_height = height;
     p->automatic_term_size_got = TRUE;
    }
    break;
    
    case TELOPT_TTYPE:
    {
     char term_name[TERMINAL_NAME_SZ];

     BTRACE("input_parse_telnet_options");
     
     if ((*in_node)->length < 3)
       break;
     
     ++tmp;

     DEBUG_START(SOCKET_DEBUG && (*tmp != TELQUAL_IS));
     fprintf(stderr, " telopt_termtype second is %d %c", (int)*tmp, *tmp);
     DEBUG_END();
     
     if (*tmp != TELQUAL_IS) /* remote says terminal type is */
     {
      BTRACE("input_parse_telnet_options");
      return (2);
     }
     
     ++tmp;

     INPUT_TERMINATE((*in_node));

     COPY_STR(term_name, (const char *)tmp, TERMINAL_NAME_SZ);
     lower_case(term_name);

     DEBUG_START(SOCKET_DEBUG);
     fprintf(stderr, " Autofound term type... (%s)", term_name);
     DEBUG_END();

     if (terminal_setup(p, term_name))
       p->automatic_term_name_got = TRUE;
     logon_shortcut_logon_start(p);
    }
    break;
    
    default:
      BTRACE("input_parse_telnet_options");
      DEBUG_START(SOCKET_DEBUG);
      fprintf(stderr, " SE not delat with... %d %c", (int)*tmp, *tmp);
      DEBUG_END();
   }

   input_del(p, *in_node);
   *in_node = input_find_current(p);
   if (!*in_node)
   {
    log_assert(FALSE);
    user_logoff(p, NULL);
    BTRACE("input_parse_telnet_options");
    return (0);
   }
   /* remove the [IAC] [SE] [it] */
   BTRACE("input_parse_telnet_options");
   return (2);
  }
  break;
    
  default:
    BTRACE("input_parse_telnet_options");
    DEBUG_START(SOCKET_DEBUG);
    fprintf(stderr, " IAC not delat with... %d %c",
            (int)input[1], input[1]);
    DEBUG_END();
 }

 BTRACE("input_parse_telnet_options");
 return (2);
}

void input_process(player *p, input_node *in_node,
                   const char *input, size_t length)
{
 size_t count = 0;
 
 while (count < length)
 {
  if (in_node->in_sub_mode)
  {
   if ((unsigned char)input[count] == IAC)
   {
    int tmp = 0;
    INPUT_TERMINATE(in_node);
    if (!(tmp = input_parse_telnet_options(p, &in_node, input + count,
                                           length - count)))
      return;
    
    count += tmp - 1;
   }
   else
     INPUT_ADD(in_node, input[count]);
  }
  else
  {
   switch ((unsigned char)input[count])
   {
    case IAC:
    {
     int tmp = 0;
     
     p->input_last_eol = 0;
     if (!(tmp = input_parse_telnet_options(p, &in_node, input + count,
                                            length - count)))
       return;
     count += tmp - 1;
    }
    break;
    
    case '\n':
    case '\r':
    case 0: /* don't ask... needed for compatability */
      if (p->input_last_eol && (p->input_last_eol != input[count]))
        p->input_last_eol = 0;
      else
      {
       p->input_last_eol = input[count];
       assert(in_node == input_find_current(p));
       output_maybe_delete_line(p);
       prompt_del_input(p, in_node->length);
       if (!player_list_io_find(p))
         player_list_io_add(p); /* so commands get run asap */
       INPUT_TERMINATE(in_node);
       in_node->ready = TRUE;
       
       if (!(in_node = input_add(p, in_node)))
       {
        log_assert(FALSE);
        user_logoff(p, NULL);
        return;
       }
       
       ++p->typed_commands;
      }
      break;
      
    case 8:
    case 127:
      p->input_last_eol = 0;
      if (in_node->length)
      {
       input_backspace(in_node);
       prompt_del_input(p, 1);
      }
      break;
      
    default:
      p->input_last_eol = 0;
      
      /* FIXME: if doing multi lingual support this needs to change */
      if (strchr(ALPHABET_LOWER ALPHABET_UPPER
                 "1!2\"3£4$5%6^7&8*9(0)-_=+[]{};:'@#~,<.>/?\\|` ",
                 ((unsigned char) input[count])))
      {
       INPUT_ADD(in_node, input[count]);
       prompt_add_input(p, input + count, 1);
      }
      break;
   }
  }
  
  ++count;
 }
 assert(count == length);
}

static void user_input_formating_flags(player *p, const char *str)
{
 unsigned int saved_extra_screen_routines = p->extra_screen_routines;

 if (IN_TELOPT_CMD(p, echo) || IN_TELOPT_CMD(p, sga))
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " Sorry but we are delaing with your last formatting change\n"
                "  request at the moment please wait then try again.\n");
  return;
 }
 
 TOGGLE_COMMAND_ON_OFF(p, str, p->extra_screen_routines, TRUE,
                       " You are %susing talker enabled extensions.\n"
                       " This information is not saved.\n",
                       " You are not %susing talker enabled extensions.\n"
                       " This information is not saved.\n",
                       TRUE);
 
 if (((unsigned)(p->repeat_prompt = p->extra_screen_routines)) !=
     saved_extra_screen_routines)
 {
  if (p->extra_screen_routines)
  {
   telopt_ask_echo_local(p);
   telopt_ask_chars_mode(p);
  }
  else
  {
   telopt_ask_echo_remote(p);
   telopt_ask_line_mode(p);
  }
 }
}

void cmds_init_process_input(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("extra_formating", user_input_formating_flags, CONST_CHARS, SYSTEM);
}
