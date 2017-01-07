#define TERMINAL_C
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

/* globs used for tpus function that we don't use ... good eh ?:) */
static player *terminal_output_p = NULL;
static int terminal_output_flags = 0;
static output_node *terminal_output_out_node = NULL;

static terminal_termcap *start = NULL;

static terminal_termcap *terminal_create_termcap(const char *name)
{
 terminal_termcap *new_node = XMALLOC(sizeof(terminal_termcap),
                                      TERMINAL_TERMCAP);

 if (!new_node)
   return (NULL);

 new_node->next = start;
 start = new_node;

 new_node->ref_count = 0;
 
 COPY_STR(new_node->name, name, TERMINAL_NAME_SZ);

 return (new_node);
}

static terminal_termcap **terminal_find_termcap(const char *name)
{
 terminal_termcap **scan = &start;

 while (*scan)
 {
  if (!strncasecmp(name, (*scan)->name, TERMINAL_NAME_SZ - 1))
    return (scan);
  
  scan = &(*scan)->next;
 }

 return (NULL);
}

static int terminal_get_termcap(char *buffer, const char *name,
                                terminal_termcap **ret_node)
{
 terminal_termcap **tmp = NULL;
 
 BTRACE("terminal_get_termcap");
 
 if ((tmp = terminal_find_termcap(name)))
 {
  if (*tmp != *ret_node)
  {
   if (*ret_node)
     terminal_unsetup(ret_node);

   ++(*tmp)->ref_count;
   *ret_node = *tmp;
  }
  return (TERMINAL_TERMCAP_CACHED);
 }

 BTRACE("terminal_get_termcap");
 
 switch (tgetent(buffer, name))
 {
  case 0:
  case -1:
    return (TERMINAL_TERMCAP_ERROR);

  default:
    break;
 }

 BTRACE("terminal_get_termcap");
 
 if (*ret_node)
   terminal_unsetup(ret_node);
 assert(!*ret_node);

 BTRACE("terminal_get_termcap");
 
 if (!(*ret_node = terminal_create_termcap(name)))
   return (TERMINAL_TERMCAP_ERROR);

 ++(*ret_node)->ref_count;

 BTRACE("terminal_get_termcap");
 
 return (TERMINAL_TERMCAP_CREATED);
}

int terminal_setup(player *p, const char *term_name)
{
 char buffer[TERMINAL_TERMCAP_SZ];
 char *data = NULL;
 const char *tmp = NULL;
 int num_sg = 0;
 int num_ug = 0;

 BTRACE("terminal_setup");

 ospeed = SHRT_MAX; /* this is wrong -- but it can't be right AFAIK*/
 PC = 0; /* we don't use either of these anyway atm. it's all wrong and can't
          * be fixed AFAIK */

 if (!term_name[0])
   return (FALSE);
 
 switch (terminal_get_termcap(buffer, term_name, &p->termcap))
 {
  case TERMINAL_TERMCAP_ERROR:
    return (FALSE);
  case TERMINAL_TERMCAP_CACHED:
    return (TRUE);

  default:
    assert(FALSE);

  case TERMINAL_TERMCAP_CREATED:
    break;
 }
 log_assert(p->termcap);
 
 data = p->termcap->data;
 if ((tmp = tgetstr("pc", &data)))
   p->termcap->pc = *tmp;
 else
   p->termcap->pc = 0;
 
 data = p->termcap->data;
 
 if (tgetflag("gn"))
 { /* FIXME: generic terminal -- do msg */
  
 }
 
 p->termcap->up = tgetstr("up", &data); /* move cursor up */
 p->termcap->ce = tgetstr("ce", &data); /* clear end of line */
 p->termcap->cl = tgetstr("cl", &data); /* clear screen */
 
 p->termcap->am = tgetflag("am");
 p->termcap->xn = tgetflag("xn");
 p->termcap->ms = tgetflag("ms");
 
 if ((num_sg = tgetnum("sg")) == -1)
   num_sg = 0;
 if ((num_ug = tgetnum("ug")) == -1)
   num_ug = 0;

 p->termcap->sg = num_sg;
 p->termcap->ug = num_ug;
 
 p->termcap->xs = tgetflag("xs");
 
 if (!p->termcap->sg && !p->termcap->ug && !p->termcap->xs)
 {
  p->termcap->md = tgetstr("md", &data); /* bold */
  p->termcap->mb = tgetstr("mb", &data); /* flahing */
  p->termcap->mh = tgetstr("mh", &data); /* dim */
  p->termcap->mk = tgetstr("mk", &data); /* invisible */
  p->termcap->mr = tgetstr("mr", &data); /* inverse */
  
  p->termcap->us = tgetstr("us", &data); /* underline */
  p->termcap->ue = tgetstr("ue", &data); /* underline normal */

  p->termcap->me = tgetstr("me", &data); /* "all" normal */

  /* FIXME: should follow some of these */
  /* "Co" = max number of colours available at once */
  /* "pa" = max number of colour pairs on screen at once
   *        a pair is for terminals that cannot have every foreground with
   *        every background (so you set up "pairs" that can work together
   *        ... ignore this ? */
  /* "ut" = screen is erased by using background colour --
   *        can I ignore this ? */
  /* "cc" = If present "Ic" takes a colours number (0 -=> Co - 1) and
   *        3 paramters, which means HLS or RGB depending on if "hl" is set
   *        (if it isn't set then it means RGB). */
  /* "NC" = a bitmask of whether colour collides with specials
   *                   Attribute      Bit   Decimal
   *                   A_STANDOUT     0     1
   *                   A_UNDERLINE    1     2
   *                   A_REVERSE      2     4
   *                   A_BLINK        3     8
   *                   A_DIM          4     16
   *                   A_BOLD         5     32
   *                   A_INVIS        6     64
   *                   A_PROTECT      7     128
   *                   A_ALTCHARSET   8     256
   *
   *  Eg. (bitmask & 2), means you can't use underline and colours.
   */
  
  if (!(p->termcap->Sf = tgetstr("Sf", &data))) /* generic foreground colour */
    p->termcap->Sf = tgetstr("AF", &data); /* ANSI foreground colour */
  if (!(p->termcap->Sb = tgetstr("Sb", &data))) /* generic background colour */
    p->termcap->Sb = tgetstr("AB", &data); /* ANSI background colour */
 }
 else
 {
  p->termcap->mb = NULL;
  p->termcap->md = NULL;
  p->termcap->mh = NULL;
  p->termcap->mk = NULL;
  p->termcap->mr = NULL;
  p->termcap->me = NULL;
  
  p->termcap->us = NULL;
  p->termcap->ue = NULL;
  
  p->termcap->Sf = NULL;
  p->termcap->Sb = NULL;
 }
 
 p->termcap->bl = tgetstr("bl", &data);
 p->termcap->vb = tgetstr("vb", &data);
 
 return (TRUE);
}

void terminal_unsetup(terminal_termcap **node)
{
 terminal_termcap **scan = NULL;
 
 assert(node);
 
 if (!*node)
   return;

 if (!(scan = terminal_find_termcap((*node)->name)))
   return;

 --(*node)->ref_count;
 *node = NULL;
}

#if 0 /* for trying to use a tpus type interface ... */
static int internal_terminal_output_padding(int passed_the_char)
{
 char the_char = passed_the_char;
 
 terminal_output_out_node = output_raw(terminal_output_p,
                                       terminal_output_flags |
                                       OUTPUT_BUFFER_NON_PRINT,
                                       &the_char, 1, terminal_output_out_node);

 return (TRUE);
}
#endif

static output_node *terminal_output_raw(player *p, int flags,
                                        const char *str, int len,
                                        output_node *out_node,
                                        int param1, int param2, int param3,
                                        int param4, int param5, int param6,
                                        int param7, int param8, int param9)
{
 int params[9 + 32] = {0};
 int count = 31;

 params[31 + 0] = param1;
 params[31 + 1] = param2;
 params[31 + 2] = param3;
 params[31 + 3] = param4;
 params[31 + 4] = param5;
 params[31 + 5] = param6;
 params[31 + 6] = param7;
 params[31 + 7] = param8;
 params[31 + 8] = param9;
 
 /* This is a full implementation according to the info doco for termcap. */
 /* with hacked int extentions for ncurses ... can't let us have a decent
  * interface to tputs etc. ... oh no */
 /* some stuff between termcap and ncurses conflict ... these are #define'd out
  * the others are always in */

#ifndef USE_NCURSES_DB
 while (*str && isdigit((unsigned char) *str)) /* ignore padding */
   ++str;
#endif
 
 while (*str)
 {
#ifdef USE_NCURSES_DB
  if (*str == '$')
  { /* ignore padding */
   const char *beg_str = str;
   const char *end = NULL;

   ++str;
   if ((*str != '<') || !(end = strchr(str, '>')))
   { /* in ncurses padding can appear anywhere in the string... */
    --len;
    out_node = output_raw(p, flags | OUTPUT_BUFFER_NON_PRINT,
                          beg_str, 1, out_node);
    continue;
   }
   /* complete ignoreance is bliss */
   len -= (end - beg_str);
   str = end;
  } else 
#endif
  if (*str == '%')
  {
   --len;
   ++str;

   if (!*str || (*str == '%'))
   {
    assert(*str); /* termcap is fucked ... or is this defined ? */
    out_node = output_raw(p, flags | OUTPUT_BUFFER_NON_PRINT,
                          str, 1, out_node);
   }
   else if (*str == 'c') /* terminfo */
   {
    char buffer[1];

    buffer[0] = params[count++];
    
    out_node = output_raw(p, flags | OUTPUT_BUFFER_NON_PRINT,
                          buffer, 1, out_node);
   }
   else if (*str == 's') /* terminfo */
   {
    const char *the_str = (char *)params[count++]; /* Oh yes */
    size_t tmp_len = strlen(the_str);
    
    out_node = output_raw(p, flags | OUTPUT_BUFFER_NON_PRINT,
                          the_str, tmp_len, out_node);
   }
   else if ((*str == 'd') ||
            /* others (%2d etc.) are terminfo */
            ((*str == '2') && (str[1] == 'd')) ||
            ((*str == '3') && (str[1] == 'd')) ||
            ((*str == '0') && (str[1] == '2') && (str[2] == 'd')) ||
            ((*str == '0') && (str[1] == '3') && (str[2] == 'd')))
   {
    char buffer[BUF_NUM_TYPE_SZ(int)];
    char fmt[8] = "%";
    int tmp_len = 1;

    while (*str != 'd')
      fmt[tmp_len++] = *str++;
    len -= tmp_len - 1;
    fmt[tmp_len] = 'd';
    fmt[tmp_len + 1] = 0;
    
    tmp_len = sprintf(buffer, fmt, params[count++]);
    
    out_node = output_raw(p, flags | OUTPUT_BUFFER_NON_PRINT,
                          buffer, tmp_len, out_node);
   }
   else if ((*str == 'x') || /* terminfo */
            ((*str == '2') && (str[1] == 'x')) ||
            ((*str == '3') && (str[1] == 'x')) ||
            ((*str == '0') && (str[1] == '2') && (str[2] == 'x')) ||
            ((*str == '0') && (str[1] == '3') && (str[2] == 'x')))
   {
    char buffer[BUF_NUM_TYPE_SZ(int)];
    char fmt[8] = "%";
    int tmp_len = 1;

    while (*str != 'x')
      fmt[tmp_len++] = *str++;
    len -= tmp_len - 1;
    fmt[tmp_len] = 'x';
    fmt[tmp_len + 1] = 0;
    
    tmp_len = sprintf(buffer, fmt, params[count++]);
    
    out_node = output_raw(p, flags | OUTPUT_BUFFER_NON_PRINT,
                          buffer, tmp_len, out_node);
   }
   else if (*str == '2')
   {
    char buffer[BUF_NUM_TYPE_SZ(int)];
    int tmp_len = sprintf(buffer, "%02d", params[count++]);
    
    out_node = output_raw(p, flags | OUTPUT_BUFFER_NON_PRINT,
                          buffer, tmp_len, out_node);
   }
   else if (*str == '3')
   {
    char buffer[BUF_NUM_TYPE_SZ(int)];
    int tmp_len = sprintf(buffer, "%03d", params[count++]);
    
    out_node = output_raw(p, flags | OUTPUT_BUFFER_NON_PRINT,
                          buffer, tmp_len, out_node);
   }
#ifndef USE_NCURSES_DB
   else if (*str == '+')
   {
    --len;
    ++str;
    params[count] += *str;
    goto termcap_handle_dot;
   }
#endif
   else if (*str == '.')
   {
    goto termcap_handle_dot; /* this is a hack to get rid of warnings about
                                unused labels if USE_NCURSES_DB is defined */
   termcap_handle_dot:
    {
     char buffer[2];
     int tmp_len = sprintf(buffer, "%c", params[count++]);
     
     out_node = output_raw(p, flags | OUTPUT_BUFFER_NON_PRINT,
                           buffer, tmp_len, out_node);
    }
   }
   else if (*str == 'i')
   {
    ++params[count];
    ++params[count + 1];
    }
   else if (*str == 'r')
   {
    int tmp = params[count];
    params[count] = params[count + 1];
    params[count + 1] = tmp;
   }
   else if (*str == 's')
   {
    ++count;
   }
   else if (*str == 'b')
   {
    --count;
   }
   else if (*str == '>')
   {
    --len;
    ++str;
    if (params[count] > *str)
      params[count] += str[1];
    --len;
    ++str;
   }
   else if (*str == 'a')
   {
    int new_val = 0;
    
    --len;
    ++str;
    
    new_val = str[2];
    switch (str[1])
    {
     case 'p':
       new_val = params[count + (new_val - 64)];
       break;
       
     case 'c':
       new_val = (new_val & ~0200);
       break;
       
     default:
       assert(FALSE);
    }
    
    switch (*str)
    {
     case '=':
       params[count] = new_val;
       break;
     case '+':
       params[count] += new_val;
       break;
     case '-':
       params[count] -= new_val;
       break;
     case '*':
       params[count] *= new_val;
       break;
     case '/':
       params[count] /= new_val;
       break;
       
     default:
       assert(FALSE);
    }
    
    --len;
    ++str;
    --len;
    ++str;
   }
   else if (*str == 'n')
   {
    params[count] ^= 0140;
    params[count + 1] ^= 0140;
   }
   else if (*str == 'm')
   {
    params[count] = ~params[count];
    params[count + 1] = ~params[count + 1];
   }
   else if (*str == 'B')
   {
    if (params[count])
      params[count] = (params[count] / 10) * 16 + (params[count] % 10);
   }
   else if (*str == 'D')
   {
    params[count] -= 2 * (params[count] % 16);
   }
#ifdef USE_NCURSES_DB /* just do the whole lot ... as a few conflict */
   else if (*str == 'p') /* terminfo */
   {
    int val = TONUMB(str[1]);
    
    ++str;
    --len;
    --val;
    params[--count] = params[31 + val];
   }
   else if (*str == 'P') /* terminfo */
   { log_assert(FALSE); /* FIXME: not implemented */ }
   else if (*str == 'g') /* terminfo */
   { log_assert(FALSE); /* FIXME: not implemented */ }
   else if (*str == '\'') /* terminfo */
   {
    ++str;
    --len;
    params[--count] = *str;
    ++str;
    --len;
   }
   else if (*str == '{') /* terminfo */
   {
    int tmp_num = 0;
    const char *beg_str = str;

    ++str;
    tmp_num = skip_atoi(&str);
    params[--count] = tmp_num;
    
    len -= (str - beg_str);
   }
   else if ((*str == '+') || (*str == '-') || (*str == '*') ||  /* terminfo */
            (*str == '/') || (*str == 'm') || (*str == '&') ||
            (*str == '|') || (*str == '^') || (*str == '=') ||
            (*str == '>') || (*str == '<') || (*str == 'A') || (*str == 'O'))
   {
    int pop1 = params[count++];
    int pop2 = params[count++];
    int res = 0;
    
    if (*str == '+')
    { res = pop1 + pop2; }
    else if (*str == '-')
    { res = pop1 - pop2; }
    else if (*str == '*')
    { res = pop1 * pop2; }
    else if (*str == '/')
    { res = pop1 / pop2; }
    else if (*str == 'm')
    { res = pop1 % pop2; }
    else if (*str == '&')
    { res = pop1 & pop2; }
    else if (*str == '|')
    { res = pop1 | pop2; }
    else if (*str == '^')
    { res = pop1 ^ pop2; }
    else if (*str == '=')
    { res = pop1 == pop2; }
    else if (*str == '>')
    { res = pop1 > pop2; }
    else if (*str == '<')
    { res = pop1 < pop2; }
    else if (*str == 'A')
    { res = pop1 && pop2; }
    else if (*str == 'O')
    { res = pop1 || pop2; }

    params[--count] = res;
   }
   else if (*str == '!') /* terminfo */
   {
    int pop1 = params[count++];

    params[--count] = !pop1;
   }
   else if (*str == '~') /* terminfo */
   {
    int pop1 = params[count++];

    params[--count] = ~pop1;
   }
   else if (*str == '?') /* terminfo */
   { log_assert(FALSE); /* FIXME: not implemented */ }
#endif
   else
   { log_assert(FALSE); /* see comment at the top of the function */ }
  }
  else
  {
   out_node = output_raw(p, flags | OUTPUT_BUFFER_NON_PRINT,
                         str, 1, out_node);
  }
  
  --len;
  ++str;
 }

 return (out_node);
}

int terminal_set_normal(player *p, int flags, output_node **passed_out_node)
{
 if (p->termcap && p->termcap->me)
 {
  if (p->termcap && p->termcap->ue && (OUTPUT_TYPE(p) == UNDERLINE_ON))
  {
   TERMINAL_DO_STR(p->termcap->ue, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  }
  else if (p->termcap && p->termcap->me)
  {
   TERMINAL_DO_STR(p->termcap->me, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  }
  
  return (TRUE);
 }
 
 return (FALSE);
}

int terminal_set_bold(player *p, int flags, output_node **passed_out_node)
{
 if (p->termcap && p->termcap->md)
 {
  TERMINAL_DO_STR(p->termcap->md, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  return (TRUE);
 }
 
 return (FALSE);
}

int terminal_set_dim(player *p, int flags, output_node **passed_out_node)
{
 if (p->termcap && p->termcap->mh)
 {
  TERMINAL_DO_STR(p->termcap->mh, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  return (TRUE);
 }
 
 return (FALSE);
}

int terminal_set_flash(player *p, int flags, output_node **passed_out_node)
{
 if (p->termcap && p->termcap->mb)
 {
  TERMINAL_DO_STR(p->termcap->mb, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  return (TRUE);
 }
 
 return (FALSE);
}

int terminal_set_underline(player *p, int flags, output_node **passed_out_node)
{
 if (p->termcap && p->termcap->us)
 {
  TERMINAL_DO_STR(p->termcap->us, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  return (TRUE);
 }

 return (FALSE);
}

int terminal_set_inverse(player *p, int flags, output_node **passed_out_node)
{
 if (p->termcap && p->termcap->mr)
 {
  TERMINAL_DO_STR(p->termcap->mr, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  return (TRUE);
 }

 return (FALSE);
}

int terminal_set_foreground_colour(player *p, int num, int flags,
                                   output_node **passed_out_node)
{
 if (p->termcap &&
     (p->termcap->Sf ||
      (!p->termcap->sg && !p->termcap->ug && !p->termcap->xs &&
       p->flag_terminal_ansi_colour_override)))
 {
  const char *str = TERMINAL_OVERRIDE_ANSI_COLOURS_FORE;
  if (p->termcap && p->termcap->Sf)
    str = p->termcap->Sf;
  
  TERMINAL_DO_STR(str, num, 0, 0, 0, 0, 0, 0, 0, 0);
  return (TRUE);
 }

 return (FALSE);
}

int terminal_set_background_colour(player *p, int num, int flags,
                                   output_node **passed_out_node)
{
 if (p->termcap &&
     (p->termcap->Sb ||
      (!p->termcap->sg && !p->termcap->ug && !p->termcap->xs &&
       p->flag_terminal_ansi_colour_override)))
 {
  const char *str = TERMINAL_OVERRIDE_ANSI_COLOURS_BACK;
  if (p->termcap && p->termcap->Sb)
    str = p->termcap->Sb;

  TERMINAL_DO_STR(str, num, 0, 0, 0, 0, 0, 0, 0, 0);
  return (TRUE);
 }

 return (FALSE);
}

int terminal_do_clear_screen(player *p, int flags,
                             output_node **passed_out_node)
{
 if (p->termcap && p->termcap->cl)
 {
  TERMINAL_DO_STR(p->termcap->cl, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  return (TRUE);
 }
 
 return (FALSE);
}

int terminal_do_clear_eol(player *p, int flags,
                          output_node **passed_out_node)
{
 if (p->termcap && p->termcap->ce)
 {
  TERMINAL_DO_STR(p->termcap->ce, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  return (TRUE);
 }
 
 return (FALSE);
}

int terminal_do_cursor_up(player *p, int flags,
                          output_node **passed_out_node)
{
 if (p->termcap && p->termcap->up)
 {
  TERMINAL_DO_STR(p->termcap->up, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  return (TRUE);
 }
 
 return (FALSE);
}

int terminal_do_audible_bell(player *p, int flags,
                             output_node **passed_out_node)
{
 if (p->termcap && p->termcap->bl)
 {
  TERMINAL_DO_STR(p->termcap->bl, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  return (TRUE);
 }
 
 return (FALSE);
}

int terminal_do_visual_bell(player *p, int flags,
                            output_node **passed_out_node)
{
 if (p->termcap && p->termcap->vb)
 {
  TERMINAL_DO_STR(p->termcap->vb, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  return (TRUE);
 }
 
 return (FALSE);
}

/* user functions ... */

static void user_toggle_ansi_override(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_terminal_ansi_colour_override, TRUE,
                       " You have %sset manual overrider for "
                       "ANSI colours, Ie. if your terminal thinks it "
                       "doesn't support them we'll try ANSI colours anyway.\n",
                       " You have not %sset manual overrider for "
                       "ANSI colours, Ie. If your terminal thinks it "
                       "doesn't support them we wont use them.\n",
                       TRUE);
 if (!p->termcap || !terminal_setup(p, p->termcap->name))
 {
  if (p->termcap)
    fvtell_player(SYSTEM_T(p), " You're terminal type "
                  "of -- ^S^B%s^s -- wasn't recognised.\n", p->termcap->name);
  else
    fvtell_player(SYSTEM_T(p), "%s", " You're terminal type wasn't "
                  "recognised.\n");
 }
}

static void user_toggle_visual_bell(player *p, const char *str)
{
 TOGGLE_COMMAND_OFF_ON(p, str, p->flag_audible_bell, TRUE,
                       " You %sget bell warnings audibly, if available.\n",
                       " You %sget bell warnings visually, if available.\n",
                       TRUE);
}

static void user_toggle_bell(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_allow_bell, TRUE,
                       " You will %sget bell warnings.\n", 
                       " You won't %sget bell warnings.\n",
                       TRUE);
}

static void clear_screen(player *p)
{
 if (p->termcap && p->termcap->cl)
   fvtell_player(NORMAL_T(p), "%s", "$Terminal-Clear_screen");
 else
   fvtell_player(SYSTEM_T(p), "%s",
                 " You have to have a termtype set for this command to"
                 " work. Use the command: ^Bterminal <type>^N\n");
}

static void user_set_terminal(player *p, const char *str)
{
 if (!*str)
 {
  if (p->termcap)
    fvtell_player(SYSTEM_T(p), " Your terminal is currently %sset to %s.\n",
                  p->automatic_term_name_got ? "auto " : "", p->termcap->name);
  TELL_FORMAT(p, "<type>");
 }
 
 if (TOGGLE_MATCH_OFF(str))
 {
  terminal_unsetup(&p->termcap);
  fvtell_player(NORMAL_T(p), "%s", " Terminal turned off, although it still "
                "will try to be autodetected everytime you logon.\n");
  return;
 }
 
 if (terminal_setup(p, str))
 {
  if (p->termcap->Sf)
    fvtell_player(NORMAL_T(p), " Terminal set to ^S^B%s^s, which "
                  "has got colour support.\n", p->termcap->name);
  else
    fvtell_player(NORMAL_T(p), " Terminal set to ^S^B%s^s, which hasn't "
                  "got colour support builtin "
                  "(see the ansi_override command, to \"fix\" this).\n",
                  p->termcap->name);
 }
 else
   fvtell_player(NORMAL_T(p), " Terminal -- ^S^B%s^s -- isn't "
                 "supported. Mail $Talker-Email_long, or log a "
                 "suggestion.\n", str);
}

static void user_set_term_width(player *p, const char *str)
{
 unsigned int tmp = 0;
 
 if (TOGGLE_MATCH_OFF(str))
 {
  fvtell_player(NORMAL_T(p), "%s", " Linewrap turned off.\n");
  p->term_width = 0;
  return;
 }
 
 if (!strcmp("?", str))
 {
  fvtell_player(NORMAL_T(p), " Lineset to %d.\n", p->term_width + 1);
  return;
 }
 
 if (!(tmp = atoi(str)))
 {
  fvtell_player(NORMAL_T(p), 
		" Format: %s off/<terminal_width>\n", current_command);
  return;
 }

 if (tmp <= (p->word_wrap << 1))
 {
  fvtell_player(NORMAL_T(p), "%s", " Can't set terminal width that small "
                "compared to word wrap.\n");
  p->word_wrap = (p->term_width) >> 1;
  fvtell_player(NORMAL_T(p), " Altering word wrap to %d.\n", p->word_wrap);
 }
 
 if (tmp < 35)
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You need to set a terminal of at least 35 characters.\n");
  return;
 }

 p->term_width = tmp - 1;
 fvtell_player(NORMAL_T(p), " Linewrap set on, with terminal width %d.\n",
	      p->term_width + 1);
}

static void user_set_word_wrap(player *p, const char *str)
{
 unsigned int tmp = 0;

 if (TOGGLE_MATCH_OFF(str))
 {
  fvtell_player(NORMAL_T(p), "%s", " Wordwrap turned off.\n");
  p->word_wrap = 0;
  return;
 }
 
 if (!(tmp = atoi(str)))
 {
  fvtell_player(NORMAL_T(p), "%s", " Format: wordwrap off/<max_word_size>\n");
  return;
 }

 if (tmp >= (p->term_width >> 1))
 {
  fvtell_player(NORMAL_T(p), "%s", 
                " Can't set max word length that big compared to term "
                "width.\n");
  return;
 }
 
 p->word_wrap = tmp;
 fvtell_player(NORMAL_T(p), 
	       " Wordwrap set on, with max word size set to %d.\n",
	       p->word_wrap);
}

/* This checks your hiltells work (Nexus suggested this) */
void user_check_terminal(player *p, const char *str)
{
 if (!p->termcap)
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You must have set your terminal type (or had it "
                "set automatically by your client) for wands to work.\n");
  return;
 }

 if (!beg_strcasecmp(str, "all"))
   fvtell_player(NORMAL_T(p),
                 " ^1This^2 message^3 is^4 coloured,^5 "
		 "or^6 should^7 be.^N\n"
		 " ^BThis message is ^S^Bhilighted^s, ^S^Fflashing^s,"
		 " ^S^Iinversed^s and ^S^Uunderlined^s, or should be.^N\n");
 else if (!beg_strcasecmp(str, "normal"))
   fvtell_player(NORMAL_FT(HILIGHT, p), "%s",
                 " This message is hilighted, or should be.\n");
 else if (!(beg_strcasecmp(str, "colours") && beg_strcmp(str, "colors")))
   fvtell_player(NORMAL_T(p), "%s", 
                 " ^1This^2 message^3 is^4 coloured,^5 "
                 "or^6 should^7 be.^N\n");
 else if (!beg_strcasecmp(str, "wands"))
   fvtell_player(NORMAL_T(p), "%s", 
                 " ^BThis message is ^S^Bhilighted^s, ^S^Fflashing^s,"
                 " ^S^Iinversed^s and ^S^Uunderlined^s, or should be.^N\n");
 else
   TELL_FORMAT(p, "[normal | colours | wands | all]");
}

void cmds_init_terminal(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("ansi_override", user_toggle_ansi_override, CONST_CHARS, SETTINGS);

 CMDS_ADD("bell", user_toggle_bell, CONST_CHARS, SETTINGS);
 CMDS_ADD("visual_bell", user_toggle_visual_bell, CONST_CHARS, SETTINGS);

 CMDS_ADD("cls", clear_screen, NO_CHARS, SYSTEM);
 CMDS_XTRA_SECTION(MISC);
 
 CMDS_ADD("terminal", user_set_terminal, CONST_CHARS, SETTINGS);

 CMDS_ADD("linewrap", user_set_term_width, CONST_CHARS, SETTINGS);
 CMDS_ADD("wordwrap", user_set_word_wrap, CONST_CHARS, SETTINGS);
}
