#define CHANNELS_C
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

static channels_base *channels_start = NULL;
static int channels_num = 0;

static Timer_q_func_node channels_timer_node;

static time_t channels_a_timestamp;
static time_t channels_l_timestamp;
static int channels_timed_isync = FALSE; /* sync the index too */
static int channels_timed_sync = FALSE;

const char *chan_map_colour[CHANNELS_COLOUR_SZ] =
{
 USER_COLOUR_CHAN_1,
 USER_COLOUR_CHAN_2,
 USER_COLOUR_CHAN_3,
 USER_COLOUR_CHAN_4,
 USER_COLOUR_CHAN_5,
 USER_COLOUR_CHAN_6,
 USER_COLOUR_CHAN_7,
 USER_COLOUR_CHAN_8,
};

const char *chan_map_sep[CHANNELS_SEPERATOR_SZ][2] =
{
 { "(", ")" }, /* default */
 { "[", "]" },
 { "<", ">" },
 { "{", "}" },
 { "`", "'" },
 { "\"", "\"" },
 { "->", "<-" },
 { "-=>", "<=-" },
 { "-*>", "<*-" },
 { "-> ", " <-" },
 { "-=> ", " <=-" },
 { "-*> ", " <*-" },
};

static channels_node *channels_add_node(channels_base *base,
                                        player_tree_node *sp, int type)
{
 channels_node *node = XMALLOC(sizeof(channels_node), CHAN_NODE);
 channels_node *scan = NULL;
 
 if (!node)
   return (NULL);

 player_link_add(&base->players_start, sp, NULL,
                 PLAYER_LINK_NAME_ORDERED | PLAYER_LINK_DOUBLE, &node->link.s);
 node->base = base;

 ++base->players_num;
 base->flag_tmp_needs_saving = TRUE;
 
 node->name_sep = 0;
 node->colour_type = 0;
 
 if (!sp->channels_start ||
     (strcasecmp(sp->channels_start->base->name, base->name) > 0))
 {
  node->next = sp->channels_start;
  node->prev = NULL;
  sp->channels_start = node;
  return (node);
 }

 scan = sp->channels_start;
 while (scan->next && (strcasecmp(scan->next->base->name, base->name) < 0))
   scan = scan->next;

 if ((node->next = scan->next))
   node->next->prev = node;
 node->prev = scan;
 scan->next = node;

 assert((type == CHANNELS_JOIN_LEAVE_FILE_ON) ||
        (type == CHANNELS_JOIN_LEAVE_SYS_ON) ||
        (type == CHANNELS_JOIN_LEAVE_USR_ON));
 if (base->join_leave_func)
   (*base->join_leave_func)(sp, type);

 return (node);
}

static void channels_del_node(channels_base *base, channels_node *node,
                              int type)
{
 player_tree_node *sp = PLAYER_LINK_SAV_GET(&node->link.s);
 player_link_del(&base->players_start, sp, NULL,
                 PLAYER_LINK_NAME_ORDERED | PLAYER_LINK_DOUBLE, &node->link.s);

 --base->players_num;
 base->flag_tmp_needs_saving = TRUE;
 
 if (node->next)
   node->next->prev = node->prev;

 if (node->prev)
   node->prev->next = node->next;
 else
   PLAYER_LINK_SAV_GET(&node->link.s)->channels_start = node->next;

 XFREE(node, CHAN_NODE);

 assert((type == CHANNELS_JOIN_LEAVE_SYS_OFF) ||
        (type == CHANNELS_JOIN_LEAVE_USR_OFF) ||
        (type == CHANNELS_JOIN_LEAVE_USR_BOOT) ||
        (type == CHANNELS_JOIN_LEAVE_SYS_DIE));
 if (base->join_leave_func)
   (*base->join_leave_func)(sp, type);
}

channels_base *channels_find_base(const char *name)
{
 channels_base *base = channels_start;
 size_t len = strnlen(name, 3);
 int cmp = 1;
 
 switch (len)
 {
  case 0:
    return (NULL);
    
  case 1:
    if (tolower((unsigned char)name[0]) == configure.channels_main_name_1_1)
      name = configure.channels_main_name;
    else if (tolower((unsigned char)name[0]) == 's')
      name = "spod";
    else if (tolower((unsigned char)name[0]) == 'm')
      name = "minister";
#ifdef USE_INTERCOM
    else if (tolower((unsigned char)name[0]) == 'i')
      name = "intercom:channel";
#endif
    else
      return (NULL);
    break;

  case 2:
    if ((tolower((unsigned char)name[0]) == configure.channels_main_name_2_1) &&
        (tolower((unsigned char)name[1]) == configure.channels_main_name_2_2))
      name = configure.channels_main_name;
    else if ((tolower((unsigned char)name[0]) == 's') &&
             (tolower((unsigned char)name[1]) == 'p'))
      name = "spod";
    else if ((tolower((unsigned char)name[0]) == 's') &&
             (tolower((unsigned char)name[1]) == 'u'))
      name = "staff";
#ifdef USE_INTERCOM
    else if ((tolower((unsigned char)name[0]) == 'i') &&
             (tolower((unsigned char)name[0]) == 'c'))
      name = "intercom:channel";
    else if ((tolower((unsigned char)name[0]) == 'i') &&
             (tolower((unsigned char)name[0]) == 'r'))
      name = "intercom:room";
#endif
    else
      return (NULL);
    break;

  default:
    assert(len == 3);
    break;
 }
 
 while (base && (cmp > 0))
 {
  if (!(cmp = strcasecmp(name, base->name)))
    return (base);

  base = base->next;
 }
 
 return (NULL);
}

channels_node *channels_find_node(channels_base *base, player_tree_node *sp)
{
 channels_node *node = sp->channels_start;

 while (node)
 {
  if (base == node->base)
    return (node);
  
  node = node->next;
 }

 return (NULL);
}

static channels_node *channels_user_find_node(player *p, const char *name,
                                              int verb)
{
 channels_node *node = p->saved->channels_start;
 
 if (!*(name + strspn(name, "0123456789")))
 {
  int offset = strtoul(name, NULL, 0);
  
  if (!offset)
  {
   if (verb)
     fvtell_player(SYSTEM_T(p),
                   " The string -- ^S^B%s^s -- doesn't match a "
                   "channel offset, as they start at ^S^Bone^s.\n", name);
   return (NULL);
  }
  
  while (node && (offset > 0))
  {
   if (!--offset)
     return (node);
   
   node = node->next;   
  }

  if (verb)
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- doesn't match a "
                  "channel offset.\n", name);
 }
 else
 {
  size_t len = strnlen(name, 3);
  int cmp = 1;
  
  switch (len)
  {
   case 0:
     if (verb)
       fvtell_player(SYSTEM_T(p), "%s",
                     " The ^S^Bempty^s string doesn't match a "
                     "channel name/offset.\n");
     return (NULL);
     
   case 1:
     if (tolower((unsigned char)name[0]) == configure.channels_main_name_1_1)
       name = configure.channels_main_name;
     else if (tolower((unsigned char)name[0]) == 's')
       name = "spod";
     else if (tolower((unsigned char)name[0]) == 'm')
       name = "minister";
#ifdef USE_INTERCOM
    else if (tolower((unsigned char)name[0]) == 'i')
      name = "intercom:channel";
#endif
     break;
     
   case 2:
     if ((tolower((unsigned char)name[0]) == configure.channels_main_name_2_1) &&
         (tolower((unsigned char)name[1]) == configure.channels_main_name_2_2))
       name = configure.channels_main_name;
     else if ((tolower((unsigned char)name[0]) == 's') &&
              (tolower((unsigned char)name[1]) == 'p'))
       name = "spod";
     else if ((tolower((unsigned char)name[0]) == 's') &&
              (tolower((unsigned char)name[1]) == 'u'))
       name = "staff";
#ifdef USE_INTERCOM
    else if ((tolower((unsigned char)name[0]) == 'i') &&
             (tolower((unsigned char)name[0]) == 'c'))
      name = "intercom:channel";
    else if ((tolower((unsigned char)name[0]) == 'i') &&
             (tolower((unsigned char)name[0]) == 'r'))
      name = "intercom:room";
#endif
     break;
     
   default:
     if (!strcasecmp(name, "talker")) /* so we can use "talker" in aliases */
       name = configure.channels_main_name;
     break;
  }
  
  while (node && (cmp > 0))
  {
   if (!(cmp = beg_strcasecmp(name, node->base->name)))
     return (node);
   
   node = node->next;
  }

  if (verb)
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- doesn't match a "
                  "channel name, that you are on.\n", name);
 }
 
 return (NULL);
}

channels_base *channels_user_find_base(player *p, const char *name, int verb)
{
 channels_node *node = channels_user_find_node(p, name, verb);

 if (!node)
   return (NULL);

 return (node->base);
}

channels_base *channels_user_find_write_base(player *p, const char *name)
{
 channels_base *base = channels_user_find_base(p, name, TRUE);

 if (!base)
   return (NULL);
 
 LIST_CHAN_CHECK_FLAG_START(base, p->saved);
 if (!LIST_CHAN_CHECK_FLAG_DO(write))
 {
  fvtell_player(SYSTEM_T(p),
                " You do not have -- ^S^B%s^s -- privileges on the "
                "channel -- ^S^B%s^s --.\n", "write", base->name);
  
  return (NULL);
 }
 LIST_CHAN_CHECK_FLAG_END();
 
 return (base);
}

channels_base *channels_user_find_grant_base(player *p, const char *name)
{
 channels_base *base = channels_user_find_base(p, name, TRUE);

 if (!base)
   return (NULL);
 
 LIST_CHAN_CHECK_FLAG_START(base, p->saved);
 if (!LIST_CHAN_CHECK_FLAG_DO(grant))
 {
  fvtell_player(SYSTEM_T(p),
                " You do not have -- ^S^B%s^s -- privileges on the "
                "channel -- ^S^B%s^s --.\n", "grant", base->name);
  
  return (NULL);
 }
 LIST_CHAN_CHECK_FLAG_END();
 
 return (base);
}

static channels_base *channels_add_base(const char *name)
{
 channels_base *base = XMALLOC(sizeof(channels_base), CHAN_BASE);
 channels_base *scan = NULL;
 size_t len = strlen(name);
 
 if (!base)
   return (NULL);

 if (len >= CHANNELS_NAME_SZ)
   len = CHANNELS_NAME_SZ - 1;
 
 COPY_STR_LEN(base->name, name, len);

 base->name_sz = len;
 base->list_start = NULL;
 base->players_start = NULL;
 base->players_num = 0;
 base->def_name_sep = 1;
 base->def_colour_type = 1;
 base->join_leave_func = NULL;
 base->flag_no_kill = FALSE;
 base->flag_no_blocks = FALSE;
 base->flag_tmp_needs_saving = TRUE;

 ++channels_num;
 if (!channels_start || (strcasecmp(channels_start->name, base->name) > 0))
 {
  base->next = channels_start;
  base->prev = NULL;
  channels_start = base;
  return (base);
 }

 scan = channels_start;
 while (scan->next && (strcasecmp(scan->next->name, base->name) < 0))
   scan = scan->next;

 if ((base->next = scan->next))
   base->next->prev = base;
 base->prev = scan;
 scan->next = base;
 
 return (base);
}

static void channels_cleanup_base(channels_base *base)
{
 while (base->players_start)
   channels_del_node(base, (channels_node *)base->players_start,
                     CHANNELS_JOIN_LEAVE_SYS_DIE);
 assert(!base->players_num);
 
 list_chan_cleanup(base);
}

static int channels_del_base(channels_base *base)
{
 assert(base);
 
 channels_cleanup_base(base);

 if (base->next)
   base->next->prev = base->prev;

 if (base->prev)
   base->prev->next = base->next;
 else
   channels_start = base->next;

 XFREE(base, CHAN_BASE);
 --channels_num;
 
 return (TRUE);
}

static int channels_count_channels(player_tree_node *sp)
{
 channels_node *node = sp->channels_start;
 int count = 0;

 while (node)
 {
  ++count;
  node = node->next;
 }

 return (count);
}

static void channels_rejoin_func(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", 
               " Re-Entering channels mode. Use ^Bhelp channels^N for help\n"
               " To run ^Bnormal commands^N type /<command>\n"
               " Use '^Bend^N' to ^Bleave^N.\n");
}

static int channels_command(player *p, const char *str, size_t length)
{
 ICTRACE("channels_command");
 
 if (MODE_IN_MODE(p, CHANNELS))
   MODE_HELPER_COMMAND();
 else
   if (!*str)
   {
    cmds_function tmp_cmd;
    cmds_function tmp_rejoin;
    
    CMDS_FUNC_TYPE_RET_CHARS_SIZE_T((&tmp_cmd), channels_command);
    CMDS_FUNC_TYPE_NO_CHARS((&tmp_rejoin), channels_rejoin_func);
    
    if (mode_add(p, "Channels Mode-> ", MODE_ID_CHANNELS, MODE_FLAGS_DEFAULT,
                 &tmp_cmd, &tmp_rejoin, NULL))
      fvtell_player(NORMAL_T(p), "%s", 
                    " Entering channels mode. Use ^Bhelp channels^N for help\n"
                    " To run ^Bnormal commands^N type /<command>\n"
                    " Use '^Bend^N' to ^Bleave^N.\n");
    else
      fvtell_player(SYSTEM_T(p), 
                    " You cannot enter channels mode as you are in too many "
                    "other modes.\n");
    return (TRUE);
   }
 
 return (cmds_sub_match(p, str, length,
                        &cmds_sub[CMDS_SUB_OFFSET(CMDS_SECTION_CHANNELS)]));
}

static void user_channels_view_commands(player *p)
{
 user_cmds_show_section(p, "channels");
}

static void channels_exit_command(player *p)
{
 assert(MODE_IN_MODE(p, CHANNELS));

 fvtell_player(NORMAL_T(p), "%s", " Leaving channel mode.\n");

 mode_del(p);
}

static int internal_channels_inform_join(player_linked_list *passed_scan,
                                         va_list ap)
{
 player *p = va_arg(ap, player *);
 channels_base *base = va_arg(ap, channels_base *);
 player *scan = PLAYER_LINK_GET(passed_scan);
 const char *col = NULL;
 const char *left_sep = NULL;
 const char *right_sep = NULL;

 if (p == scan)
   return (TRUE);
 
 LIST_SELF_CHECK_FLAG_START(scan, p->saved);
 if (!LIST_SELF_CHECK_FLAG_DO(inform)) /* does this want to be ? */
   return (TRUE);
 LIST_SELF_CHECK_FLAG_END();

 col = CHANNELS_COLOUR_TYPE((channels_node *)passed_scan);
 left_sep =  CHANNELS_NAME_SEP((channels_node *)passed_scan)[0];
 right_sep = CHANNELS_NAME_SEP((channels_node *)passed_scan)[1];
 
 fvtell_player(NORMAL_T(scan), "%s%s%s%s ++ %s%s join%s the channel ++%s\n",
               col, left_sep, base->name, right_sep,
               gender_choose_str(p->gender, "", "", "The ", "The "),
               p->saved->name, (p->gender == GENDER_PLURAL) ? "" : "s", "^N");
 
 return (TRUE);
}

static void channels_save_index(void); /* fwd reference */
static void channels_save_all(int); /* fwd reference */

static void timed_channels_sync(int timed_type, void *data)
{
 int do_save = FALSE;
 int do_retime = FALSE;
 
 IGNORE_PARAMETER(data);
 
 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;
 
 if (timed_type == TIMER_Q_TYPE_CALL_RUN_ALL)
   do_save = TRUE;
 else if (difftime(now, channels_a_timestamp) > CHANNELS_TIMEOUT_SYNC)
   do_save = TRUE;
 else if (difftime(now, channels_l_timestamp) > CHANNELS_TIMEOUT_SYNC_ANYWAY)
 {
  do_save = TRUE;
  /* do_retime = TRUE; */
 }
 else
   do_retime = TRUE;

 if (do_save)
 {
  if (channels_timed_isync)
    channels_save_index();
  channels_save_all(FALSE);
  channels_timed_sync = FALSE;
  channels_timed_isync = FALSE;
 }

 if (do_retime)
 {
  struct timeval tv;
  
  gettimeofday(&tv, NULL);
  TIMER_Q_TIMEVAL_ADD_SECS(&tv, CHANNELS_TIMEOUT_REDO, 0);
  timer_q_add_static_node(&channels_timer_node.s, &timer_queue_global,
                          &channels_timer_node, &tv, TIMER_Q_FLAG_NODE_FUNC);
  timer_q_cntl_node(&channels_timer_node.s, TIMER_Q_CNTL_NODE_SET_FUNC,
                    timed_channels_sync);
 }
}

void channels_timed_save(channels_base *base)
{
 if (base)
   base->flag_tmp_needs_saving = TRUE;
 else
   channels_timed_isync = TRUE;

 channels_a_timestamp = now;
 if (channels_timed_sync)
   return;
 channels_l_timestamp = now;
 channels_timed_sync = TRUE;

 {
  struct timeval tv;
  
  gettimeofday(&tv, NULL);
  TIMER_Q_TIMEVAL_ADD_SECS(&tv, CHANNELS_TIMEOUT_SAVE, 0);
  timer_q_add_static_node(&channels_timer_node.s, &timer_queue_global,
                          &channels_timer_node, &tv, TIMER_Q_FLAG_NODE_FUNC);
  timer_q_cntl_node(&channels_timer_node.s, TIMER_Q_CNTL_NODE_SET_FUNC,
                    timed_channels_sync);
 }
}

static void user_channels_join(player *p, parameter_holder *params)
{
 channels_base *base = NULL;
 channels_node *node = NULL;
 const char *str = NULL;
 int col_type = 0;
 int sep = 0;
 
 switch (params->last_param)
 {
  case 3:
    sep = atoi(GET_PARAMETER_STR(params, 3));
  case 2:
    col_type = atoi(GET_PARAMETER_STR(params, 2));
  case 1:
    if ((!col_type || CHANNELS_VALID_COLOUR_TYPE(col_type)) &&
        (!sep || CHANNELS_VALID_SEPERATOR(sep)))
      break;
    
  default:
    TELL_FORMAT(p, "<channel> [colour type] [seperator]");
 }
 
 str = GET_PARAMETER_STR(params, 1);

 if (!strcasecmp("talker", str))
   str = configure.channels_main_name;
 
 if (!(base = channels_find_base(str)))
 {
  char buf[sizeof("%s alter,boot,read,write,nothing on") + PLAYER_S_NAME_SZ];

  CHANNELS_NAME_CHECK(str);

  if (channels_count_channels(p->saved) > configure.channels_players_join)
  {
   fvtell_player(SYSTEM_T(p), " You can only join -- ^S^B%d^s -- channels "
                 "at once.\n", configure.channels_players_join);
   return;
  }
  
  if (!(base = channels_add_base(str)))
    goto malloc_error_base;

  channels_timed_save(NULL);
  base->def_name_sep = sep ? sep : 1;
  base->def_colour_type = col_type ? col_type : 1;
  
  sprintf(buf, "%s boot,config,grant,read,write on", p->saved->lower_name);
  if (!list_system_change(&base->list_start, LIST_TYPE_CHAN, buf))
    goto malloc_error_list;

  sprintf(buf, "%s read,write on", "everyone");
  if (!list_system_change(&base->list_start, LIST_TYPE_CHAN, buf))
    goto malloc_error_list;
 }
 else if (player_link_find(base->players_start, p->saved, NULL,
                           PLAYER_LINK_NAME_ORDERED))
 {
  fvtell_player(NORMAL_T(p),
                " You are already on the channel -- ^S^B%s^s --.\n",
                base->name);
  return;
 }
 else
 {
  LIST_CHAN_CHECK_FLAG_START(base, p->saved);
  if (!LIST_CHAN_CHECK_FLAG_DO(read))
  {
   fvtell_player(SYSTEM_T(p),
                 " You do not have -- ^S^B%s^s -- privileges on the "
                 "channel -- ^S^B%s^s --.\n", "read", base->name);
   
   return;
  }
  LIST_CHAN_CHECK_FLAG_END();
 }
 
 if (!(node = channels_add_node(base, p->saved, CHANNELS_JOIN_LEAVE_USR_ON)))
   goto malloc_error_node;

 node->name_sep = sep;
 node->colour_type = col_type;

 fvtell_player(NORMAL_T(p), " You join the channel '^S^B%s^s'.\n",
               base->name);
 
 do_order_misc_on(internal_channels_inform_join, base->players_start, p, base);

 channels_timed_save(base);
 return;

 malloc_error_list:
 channels_del_base(base);
 malloc_error_base:
 malloc_error_node: /* don't destroy on malloc_error_node as they can
                       just subscribe themselves again ...
                       plus it might be an "in use" channel */
 P_MEM_ERR(p);
}

static int internal_channels_inform_leave(player_linked_list *passed_scan,
                                          va_list ap)
{
 player *p = va_arg(ap, player *);
 channels_base *base = va_arg(ap, channels_base *);
 player *scan = PLAYER_LINK_GET(passed_scan);
 const char *col = NULL;
 const char *left_sep = NULL;
 const char *right_sep = NULL;
 
 if (p == scan)
   return (TRUE);
 
 LIST_SELF_CHECK_FLAG_START(scan, p->saved);
 if (!LIST_SELF_CHECK_FLAG_DO(inform)) /* does this want to be ? */
   return (TRUE);
 LIST_SELF_CHECK_FLAG_END();

 col = CHANNELS_COLOUR_TYPE((channels_node *)passed_scan);
 left_sep =  CHANNELS_NAME_SEP((channels_node *)passed_scan)[0];
 right_sep = CHANNELS_NAME_SEP((channels_node *)passed_scan)[1];
 
 fvtell_player(NORMAL_T(scan), "%s%s%s%s -- %s%s leave%s the channel --%s\n",
               col, left_sep, base->name, right_sep,
               gender_choose_str(p->gender, "", "", "The ", "The "),
               p->saved->name, (p->gender == GENDER_PLURAL) ? "" : "s", "^N");
 
 return (TRUE);
}

static void user_channels_leave(player *p, parameter_holder *params)
{
 channels_base *base = NULL;
 player_linked_list *node = NULL;
 
 if (params->last_param != 1)
   TELL_FORMAT(p, "<channel>");
 
 if (!(base = channels_user_find_base(p, GET_PARAMETER_STR(params, 1), TRUE)))
   return;

 if (!(node = player_link_find(base->players_start, p->saved, NULL,
                               PLAYER_LINK_NAME_ORDERED)))
 {
  fvtell_player(NORMAL_T(p),
                " You are not on the channel -- ^S^B%s^s --.\n",
                base->name);
  return;
 }

 channels_del_node(base, (channels_node *)node, CHANNELS_JOIN_LEAVE_USR_OFF);
 
 fvtell_player(NORMAL_T(p), " You leave the channel '^S^B%s^s'.\n",
               base->name);

 if (base->players_num)
 {
  do_order_misc_on(internal_channels_inform_leave,
                   base->players_start, p, base);
  channels_timed_save(base);
 }
 else if (!base->flag_no_kill)
 {
  channels_del_base(base);
  channels_timed_save(NULL);
 }
 else
 {
  channels_timed_save(base);
 }
}

static void user_channels_list(player *p, const char *str)
{
 channels_base *base = channels_start;
 channels_node *scan = p->saved->channels_start;
 int off_count = 0;
 int on_count = 0;
 int show_me_only = FALSE;
 output_node *chans = NULL;
 tmp_output_list_storage tmp_save;
 
 save_tmp_output_list(p, &tmp_save);

 if (!beg_strcasecmp(str, "me") || !beg_strcasecmp(str, "on"))
   show_me_only = TRUE;
 
 while (base)
 {
  int am_on = (scan && (base == scan->base));
  int can_see = am_on;
  char buf[128];
  
  if (!can_see)
  {
   LIST_CHAN_CHECK_FLAG_START(base, p->saved);
   if (LIST_CHAN_CHECK_FLAG_DO(read))
     can_see = TRUE;
   LIST_CHAN_CHECK_FLAG_END();
  }
  
  if (can_see)
  {
   if (am_on)
   {
    ++on_count;
    fvtell_player(NORMAL_FT(OUTPUT_BUFFER_TMP, p), "% 4d", on_count);
   }
   else if (!show_me_only)
   {
    ++off_count;
    fvtell_player(NORMAL_FT(OUTPUT_BUFFER_TMP, p), "%4s", "-");
   }

#if (CHANNELS_NAME_SZ + 9) > WRAPPING_SPACES
# error "Bad ... change something."
#endif
   if (am_on || !show_me_only)
     fvtell_player(NORMAL_WFT(OUTPUT_BUFFER_TMP, p),
                   " %-*s %s %s player%s.\n",
                   CHANNELS_NAME_SZ, base->name, am_on ? "*" : "-",
                   word_number_base(buf, 128, NULL,
                                    base->players_num, TRUE, word_number_def),
                   (base->players_num == 1) ? "" : "s");
   
   if (am_on)
     scan = scan->next;
  }
  
  base = base->next;
 }

 chans = output_list_grab(p);
 load_tmp_output_list(p, &tmp_save);

 if (on_count || off_count)
 {
  ptell_mid(NORMAL_T(p), "Channels", FALSE);
  output_list_linkin(p, (CHANNELS_NAME_SZ + 8), &chans, INT_MAX);
  fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 }
 else
 {
  output_list_cleanup(&chans);
  if (show_me_only)
    fvtell_player(NORMAL_T(p), "%s", " You are not ^S^Bon^s any channles.\n");
  else
    fvtell_player(NORMAL_T(p), "%s", " There are currently ^S^Bno channels "
                  "available^s to you.\n");
 }
}

static int internal_channels_who(player_linked_list *passed_scan, va_list ap)
{
 player_tree_node *scan = PLAYER_LINK_SAV_GET(passed_scan);
 player *p = va_arg(ap, player *);
 int do_all = va_arg(ap, int);
 int done = FALSE;
 
 fvtell_player(NORMAL_T(p), "%-*s", PLAYER_S_NAME_SZ, scan->name);
 
 if (do_all)
 {
  fvtell_player(NORMAL_T(p), " - %s", TOGGLE_ON_OFF(P_IS_ON(scan)));
  done = TRUE;
 }
 else
 {
  assert(P_IS_ON(scan));
 }

 if (p->saved->priv_spod && scan->priv_spod)
 {
  fvtell_player(NORMAL_T(p), "%s%s", done ? ", " : " - ", "Spod");
  done = TRUE;
 }
 
 if (p->saved->priv_minister && scan->priv_minister)
 {
  fvtell_player(NORMAL_T(p), "%s%s", done ? ", " : " - ", "Minister");
  done = TRUE;
 }
 
 if (PRIV_STAFF(p->saved) && scan->priv_admin)
 {
  fvtell_player(NORMAL_T(p), "%s%s", done ? ", " : " - ", "Admin");
  done = TRUE;
 }
 else if (PRIV_STAFF(p->saved) && PRIV_STAFF(scan))
 {
  fvtell_player(NORMAL_T(p), "%s%s", done ? ", " : " - ", "Staff");
  done = TRUE;
 }
 
 fvtell_player(NORMAL_T(p), "\n");

 return (TRUE);
}

static void user_channels_who(player *p, parameter_holder *params)
{
 channels_base *base = NULL;
 int do_all = TRUE;
 char buf[CONST_STRLEN("Channel %s, %d") + CHANNELS_NAME_SZ +
         BUF_NUM_TYPE_SZ(int)];

 if ((params->last_param != 1) && (params->last_param != 2))
   TELL_FORMAT(p, "<channel>");

 /* FIXME: should you be allowed to do a who on a channel you aren't
  * on (but are allowed to be on) ? */
 if (!(base = channels_user_find_base(p, GET_PARAMETER_STR(params, 1), FALSE)))
 {
  if (!(base = channels_find_base(GET_PARAMETER_STR(params, 1))))
  {
   fvtell_player(SYSTEM_T(p),
                 " The string -- ^S^B%s^s -- doesn't match a "
                 "channel name, that you have access to.\n",
                 GET_PARAMETER_STR(params, 1));
   return;
  }
  LIST_CHAN_CHECK_FLAG_START(base, p->saved);
  if (!LIST_CHAN_CHECK_FLAG_DO(who))
  {
   fvtell_player(SYSTEM_T(p),
                 " You do not have -- ^S^B%s^s -- privileges on the "
                 "channel -- ^S^B%s^s --.\n", "who", base->name);
   
   return;
  }
  LIST_CHAN_CHECK_FLAG_END();
 }
 
 if (!base->players_num)
 {
  fvtell_player(NORMAL_T(p),
                " There is nobody on the channel -- ^S^B%s^s --.\n",
                base->name);
  return;
 }
 else
   if ((base->players_num > configure.channels_players_do_all) ||
       ((params->last_param == 2) &&
        (!beg_strcasecmp(GET_PARAMETER_STR(params, 2), "on") ||
         !strcmp(GET_PARAMETER_STR(params, 2), "-"))))
     do_all = FALSE;
 
 sprintf(buf, "Channel %s, %d", base->name, base->players_num);
 ptell_mid(NORMAL_T(p), buf, FALSE);

 if (do_all)
   do_order_misc_all(internal_channels_who, base->players_start, p, TRUE);
 else
   do_order_misc_on(internal_channels_who, base->players_start, p, FALSE);

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);

 pager(p, PAGER_DEFAULT);
}

static int internal_channels_wall(player_linked_list *passed_scan,
                                  unsigned int flags,
                                  const char *fmt, va_list ap)
{
 const char *col = NULL;
 const char *left_sep = NULL;
 const char *right_sep = NULL;
 player *scan = PLAYER_LINK_GET(passed_scan);
 channels_base *base = ((channels_node *)passed_scan)->base;
 
 col = CHANNELS_COLOUR_TYPE((channels_node *)passed_scan);
 left_sep =  CHANNELS_NAME_SEP((channels_node *)passed_scan)[0];
 right_sep = CHANNELS_NAME_SEP((channels_node *)passed_scan)[1];
 
 fvtell_player(NORMAL_T(scan), "%s%s%s%s",
               col, left_sep, base->name, right_sep);

 vfvtell_player(TALK_FT(flags, scan->saved, scan), fmt, ap);

 fvtell_player(NORMAL_T(scan), "%s\n", "^N");

 return (TRUE);
}

player_linked_list *channels_wall(const char *name, unsigned int flags,
                                  player *avoid, const char *fmt, ...)
{
 channels_base *base = channels_find_base(name);
 DO_BUILD_ORDER_FMT(internal_channels_wall, NULL, fmt, fmt,
                    base ? base->players_start : NULL,
                    (avoid != PLAYER_LINK_GET(scan)), (scan, flags, fmt, ap));
}

static int internal_channels_user_say(player_linked_list *passed_scan,
                                      va_list ap)
{
 channels_base *base = va_arg(ap, channels_base *);
 player *p = va_arg(ap, player *);
 const char *str = va_arg(ap, const char *);
 const char *mid = va_arg(ap, const char *);
 int *count = va_arg(ap, int *);
 player *scan = PLAYER_LINK_GET(passed_scan);
 const char *col = NULL;
 const char *left_sep = NULL;
 const char *right_sep = NULL;

 if (p == scan)
   return (TRUE);

 if (!base->flag_no_blocks)
 {
  LIST_COMS_CHECK_FLAG_START(scan, p->saved);
  if (LIST_COMS_CHECK_FLAG_DO(channels))
    return (TRUE);
  LIST_COMS_CHECK_FLAG_END();
 }
 
 ++*count;
 
 col = CHANNELS_COLOUR_TYPE((channels_node *)passed_scan);
 left_sep =  CHANNELS_NAME_SEP((channels_node *)passed_scan)[0];
 right_sep = CHANNELS_NAME_SEP((channels_node *)passed_scan)[1];

 fvtell_player(TALK_TP(scan), "%s%s%s%s %s %s '%.*s%s'.%s\n",
               col, left_sep, base->name, right_sep,
               "$If( ==(1(N) 2($R-Set-Ign_prefix))"
               " t($F-Name_full) f($F-Name))",
               mid, OUT_LENGTH_COMMUNICATION, str, col, "^N");
 
 return (TRUE);
}

static void user_channels_say(player *p, const char *str, size_t length)
{
 const char *str_orig = str;
 channels_base *base = NULL;
 int count = 0;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (!get_parameter_parse(&params, &str, 1) ||
     !GET_PARAMETER_LENGTH(&params, 1) || !*str)
   TELL_FORMAT(p, "<channel> <msg>");
 length -= (str - str_orig);
 
 if (!(base = channels_user_find_write_base(p, GET_PARAMETER_STR(&params, 1))))
   return;

 do_order_misc_on(internal_channels_user_say, base->players_start,
                  base, p, str, say_ask_exclaim_group(p, str, length), &count);

 if (count)
 {
  fvtell_player(TALK_TP(p), "%s You %s '%.*s%s' on the \"%s\" channel.%s\n", 
                USER_COLOUR_MINE, say_ask_exclaim_me(p, str, length),
                OUT_LENGTH_COMMUNICATION, str, USER_COLOUR_MINE,
                base->name, "^N");

 }
 else
   fvtell_player(SYSTEM_T(p), " No-one is listening to your "
                 "^S^Bsays^s, on the channel ^S^B%s^s.\n", base->name);
}

static int internal_channels_user_emote(player_linked_list *passed_scan,
                                        va_list ap)
{
 channels_base *base = va_arg(ap, channels_base *);
 player *p = va_arg(ap, player *);
 const char *str = va_arg(ap, const char *);
 int *count = va_arg(ap, int *);
 player *scan = PLAYER_LINK_GET(passed_scan);
 const char *col = NULL;
 const char *left_sep = NULL;
 const char *right_sep = NULL;

 if (p == scan)
   return (TRUE);

 if (!base->flag_no_blocks)
 {
  LIST_COMS_CHECK_FLAG_START(scan, p->saved);
  if (LIST_COMS_CHECK_FLAG_DO(channels))
    return (TRUE);
  LIST_COMS_CHECK_FLAG_END();
 }
 
 ++*count;

 col = CHANNELS_COLOUR_TYPE((channels_node *)passed_scan);
 left_sep =  CHANNELS_NAME_SEP((channels_node *)passed_scan)[0];
 right_sep = CHANNELS_NAME_SEP((channels_node *)passed_scan)[1];
 
 fvtell_player(TALK_TP(scan), "%s%s%s%s %s%s%.*s%s\n",
               col, left_sep, base->name, right_sep,
               "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
               " t($F-Name_full) f($F-Name))",
               isits1(str), OUT_LENGTH_COMMUNICATION, isits2(str), "^N");
 
 return (TRUE);
}

static void user_channels_emote(player *p, const char *str)
{
 channels_base *base = NULL;
 int count = 0;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (!get_parameter_parse(&params, &str, 1) ||
     !GET_PARAMETER_LENGTH(&params, 1) || !*str)
   TELL_FORMAT(p, "<channel> <msg>");
 
 if (!(base = channels_user_find_write_base(p, GET_PARAMETER_STR(&params, 1))))
   return;

 do_order_misc_on(internal_channels_user_emote, base->players_start,
                  base, p, str, &count);
 
 if (count)
 {
  twinkle_info info;
  
  setup_twinkle_info(&info);
  
  info.output_not_me = TRUE;

  fvtell_player(TALK_IT(&info, p->saved, p),
                "%s You emote '%s%s%.*s%s' on the \"%s\" channel.%s\n", 
                USER_COLOUR_MINE,
                "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                " t($F-Name_full) f($F-Name))",
                isits1(str), OUT_LENGTH_COMMUNICATION, isits2(str),
                USER_COLOUR_MINE, base->name, "^N");
 }
 else
   fvtell_player(SYSTEM_T(p), " No-one is listening to your "
                 "^S^Bemotes^s, on the channel ^S^B%s^s.\n", base->name);
}

static int internal_channels_inform_boot(player_linked_list *passed_scan,
                                         va_list ap)
{
 player *p = va_arg(ap, player *);
 player_tree_node *sp = va_arg(ap, player_tree_node *);
 channels_base *base = va_arg(ap, channels_base *);
 player *scan = PLAYER_LINK_GET(passed_scan);
 const char *col = NULL;
 const char *left_sep = NULL;
 const char *right_sep = NULL;
 
 if (p == scan)
   return (TRUE);
 
 LIST_SELF_CHECK_FLAG_START(scan, p->saved);
 if (!LIST_SELF_CHECK_FLAG_DO(inform)) /* does this want to be ? */
   return (TRUE);
 LIST_SELF_CHECK_FLAG_END();

 col = CHANNELS_COLOUR_TYPE((channels_node *)passed_scan);
 left_sep =  CHANNELS_NAME_SEP((channels_node *)passed_scan)[0];
 right_sep = CHANNELS_NAME_SEP((channels_node *)passed_scan)[1];
 
 fvtell_player(NORMAL_T(scan), "%s%s%s%s -- %s%s leave%s the channel --%s\n",
               col, left_sep, base->name, right_sep,
               gender_choose_str(sp->player_ptr->gender, "", "",
                                 "The ", "The "),
               sp->name,
               (sp->player_ptr->gender == GENDER_PLURAL) ? "" : "s", "^N");
 
 return (TRUE);
}

static void user_channels_boot(player *p, parameter_holder *params)
{
 channels_base *base = NULL;
 channels_node *node = NULL;
 player_tree_node *sp = NULL;
 
 if ((params->last_param != 2) || !GET_PARAMETER_LENGTH(params, 1) ||
     !GET_PARAMETER_LENGTH(params, 2))
   TELL_FORMAT(p, "<channel> <player>");
 
 if (!(base = channels_user_find_base(p, GET_PARAMETER_STR(params, 1), TRUE)))
   return;

 LIST_CHAN_CHECK_FLAG_START(base, p->saved);
 if (!LIST_CHAN_CHECK_FLAG_DO(boot))
 {
  fvtell_player(SYSTEM_T(p),
                " You do not have -- ^S^B%s^s -- privileges on the "
                "channel -- ^S^B%s^s --.\n", "boot", base->name);
  
  return;
 }
 LIST_CHAN_CHECK_FLAG_END();

 if (!(sp = player_find_all(p, GET_PARAMETER_STR(params, 2),
                            PLAYER_FIND_SC_EXTERN)))
   return;

 if (!(node = (channels_node *)player_link_find(base->players_start, sp, NULL,
                                                PLAYER_LINK_NAME_ORDERED)))
 {
  fvtell_player(NORMAL_T(p),
                " The player -- ^S^B%s^s -- is not on the "
                "channel -- ^S^B%s^s --.\n", sp->name, base->name);
  return;
 }

 if (P_IS_ON(sp))
   fvtell_player(SYSTEM_T(sp->player_ptr),
                 " -=> You have been booted from the channel ^S^B%s^s.\n",
                 base->name);
 
 channels_del_node(base, (channels_node *)node, CHANNELS_JOIN_LEAVE_USR_BOOT);

 log_assert(base->players_num);

 if (P_IS_ON(sp))
   do_order_misc_on(internal_channels_inform_boot,
                    base->players_start, p, sp, base);
 
 fvtell_player(NORMAL_T(sp->player_ptr),
               " You boot '^S^B%s^s' from the channel ^S^B%s^s.\n",
               sp->name, base->name);
}

static void user_channels_config(player *p, parameter_holder *params)
{
 channels_node *node = NULL;

 switch (params->last_param)
 {
  case 1:
  {
   const char *str = "show";
   if (!get_parameter_parse(params, &str, 2))
     TELL_FORMAT(p, "<channel> <option> [value]");
  }
  case 2:
  case 3:
    break;

  default:
    TELL_FORMAT(p, "<channel> <option> [value]");
 }
 
 if (!(node = channels_user_find_node(p, GET_PARAMETER_STR(params, 1), TRUE)))
   return; /* FIXME: maybe don't need channel name for all options */

 lower_case(GET_PARAMETER_STR(params, 2));
 if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "view") ||
     !beg_strcmp(GET_PARAMETER_STR(params, 2), "show"))
 {
  char buf[128];
  
  ptell_mid(NORMAL_T(p), "Channel config", FALSE);
  fvtell_player(NORMAL_T(p), "%-16s %s\n", "Name:", node->base->name);
  fvtell_player(NORMAL_T(p), "%-16s %s%s%s\n", "Colour type:",
                CHANNELS_COLOUR_TYPE(node),
                node->colour_type ?
                word_number_base(buf, sizeof(buf), NULL, node->colour_type,
                                 TRUE, word_number_def) : "Default", "^N");
  fvtell_player(NORMAL_T(p), "%-16s %s\n", "Left Seperator:",
                CHANNELS_NAME_SEP(node)[0]);
  fvtell_player(NORMAL_T(p), "%-16s %s\n", "Right Seperator:",
                CHANNELS_NAME_SEP(node)[1]);
  fvtell_player(NORMAL_T(p), "%-16s %s%s%s%s%s\n", "Example:",
                CHANNELS_COLOUR_TYPE(node),
                CHANNELS_NAME_SEP(node)[0],
                node->base->name,
                CHANNELS_NAME_SEP(node)[1], "^N");  
  fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "colour_type") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "colour type") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "color_type") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "color type"))
 {
  char buf[128];
  
  if (params->last_param != 3)
  {
  chan_colour_parse_err:
   fvtell_player(SYSTEM_T(p), " A channel colour type, is a number between"
                 " -- ^S^Bone^s -- and ^S^B%s^s --, default or list.\n",
                 word_number_base(buf, sizeof(buf), NULL, CHANNELS_COLOUR_SZ,
                                  FALSE, word_number_def));
   return;
  }
  
  lower_case(GET_PARAMETER_STR(params, 3));
  if (!beg_strcmp(GET_PARAMETER_STR(params, 3), "list"))
  {
   int count = 0;

   ptell_mid(NORMAL_T(p), "Channel colour types", FALSE);
   while (count < CHANNELS_COLOUR_SZ)
   {
    const char *col = chan_map_colour[count];
    ++count;
    fvtell_player(NORMAL_T(p), "%sColour %d%s%s%s\n", col, count,
                  (count == node->base->def_colour_type) ? ", default" : "",
                  (count == node->colour_type) ? ", current" : "",
                  "^N");
   }
   fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
   return;
  }
  else if (!beg_strcmp(GET_PARAMETER_STR(params, 3), "default"))
    node->colour_type = 0;
  else if (*(GET_PARAMETER_STR(params, 3) +
             strspn(GET_PARAMETER_STR(params, 3), "0123456789")))
    goto chan_colour_parse_err;
  else
  {
   int tmp = atoi(GET_PARAMETER_STR(params, 3));

   if (!CHANNELS_VALID_COLOUR_TYPE(tmp))
     goto chan_colour_parse_err;
   
   node->colour_type = tmp;
  }
  channels_timed_save(node->base);
  
  fvtell_player(NORMAL_T(p), " You have set your ^S^B%s^s, for "
                "channel ^S^B%s^s, to ^S^B%s^s.\n", "colour type",
                node->base->name,
                node->colour_type ?
                word_number_base(buf, sizeof(buf), NULL, node->colour_type,
                                 TRUE, word_number_def) : "Default");
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "seperator"))
 {
  char buf[128];

  if (params->last_param != 3)
  {
  chan_sep_parse_err:
   fvtell_player(SYSTEM_T(p), " A channel seperator type, is a number between"
                 " -- ^S^Bone^s -- and ^S^B%s^s --, default or list.\n",
                 word_number_base(buf, sizeof(buf), NULL,CHANNELS_SEPERATOR_SZ,
                                  FALSE, word_number_def));
   return;
  }
  
  lower_case(GET_PARAMETER_STR(params, 3));
  if (!beg_strcmp(GET_PARAMETER_STR(params, 3), "list"))
  {
   int count = 0;

   ptell_mid(NORMAL_T(p), "Channel seperators", FALSE);
   while (count < CHANNELS_SEPERATOR_SZ)
   {
    const char *sep_l = chan_map_sep[count][0];
    const char *sep_r = chan_map_sep[count][1];
    ++count;
    fvtell_player(NORMAL_T(p), "%sSeperator %d%s%s%s\n",
                  sep_l, count, sep_r,
                  (count == node->base->def_name_sep) ? ", default" : "",
                  (count == node->name_sep) ? ", current" : "");
   }
   fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
   return;
  }
  else if (!beg_strcmp(GET_PARAMETER_STR(params, 3), "default"))
    node->name_sep = 0;
  else if (*(GET_PARAMETER_STR(params, 3) +
             strspn(GET_PARAMETER_STR(params, 3), "0123456789")))
    goto chan_sep_parse_err;
  else
  {
   int tmp = atoi(GET_PARAMETER_STR(params, 3));

   if (!CHANNELS_VALID_SEPERATOR(tmp))
     goto chan_sep_parse_err;
   
   node->name_sep = tmp;
  }
  channels_timed_save(node->base);
  
  fvtell_player(NORMAL_T(p), " You have set your ^S^B%s^s, for "
                "channel ^S^B%s^s, to ^S^B%s^s.\n", "name seperator",
                node->base->name,
                node->name_sep ?
                word_number_base(buf, sizeof(buf), NULL, node->name_sep,
                                 TRUE, word_number_def) : "Default");
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "default_colour_type") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "default colour type") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "default_color_type") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "default color type") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "def_colour_type") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "def colour type") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "def_color_type") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "def color type"))
          
 {
  char buf[128];
  
  if (params->last_param != 3)
  {
  chan_def_colour_parse_err:
   fvtell_player(SYSTEM_T(p), " A channel colour type, is a number between"
                 " -- ^S^Bone^s -- and ^S^B%s^s --, default or list.\n",
                 word_number_base(buf, sizeof(buf), NULL, CHANNELS_COLOUR_SZ,
                                  FALSE, word_number_def));
   return;
  }

  if (*(GET_PARAMETER_STR(params, 3) +
        strspn(GET_PARAMETER_STR(params, 3), "0123456789")))
    goto chan_def_colour_parse_err;
  else
  {
   int tmp = atoi(GET_PARAMETER_STR(params, 3));
   
   if (!CHANNELS_VALID_COLOUR_TYPE(tmp))
     goto chan_def_colour_parse_err;
   
   node->base->def_colour_type = tmp;
  }
  channels_timed_save(node->base);

  fvtell_player(NORMAL_T(p), " You have set the defualt ^S^B%s^s, for "
                "channel ^S^B%s^s, to ^S^B%s^s.\n", "colour type",
                node->base->name,
                word_number_base(buf, sizeof(buf), NULL,
                                 node->base->def_colour_type,
                                 TRUE, word_number_def));
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "default seperator") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "def seperator") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "default_seperator") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "def_seperator"))
 {
  char buf[128];

  if (params->last_param != 3)
  {
  chan_def_sep_parse_err:
   fvtell_player(SYSTEM_T(p), " A channel seperator type, is a number between"
                 " -- ^S^Bone^s -- and ^S^B%s^s --, default or list.\n",
                 word_number_base(buf, sizeof(buf), NULL,CHANNELS_SEPERATOR_SZ,
                                  FALSE, word_number_def));
   return;
  }

  if (*(GET_PARAMETER_STR(params, 3) +
        strspn(GET_PARAMETER_STR(params, 3), "0123456789")))
    goto chan_def_sep_parse_err;
  else
  {
   int tmp = atoi(GET_PARAMETER_STR(params, 3));
   
   if (!CHANNELS_VALID_SEPERATOR(tmp))
     goto chan_def_sep_parse_err;
   
   node->base->def_name_sep = tmp;
  }
  channels_timed_save(node->base);
  
  fvtell_player(NORMAL_T(p), " You have set the default ^S^B%s^s, for "
                "channel ^S^B%s^s, to ^S^B%s^s.\n", "name seperator",
                node->base->name,
                word_number_base(buf, sizeof(buf), NULL,
                                 node->base->def_name_sep,
                                 TRUE, word_number_def));
 }
 else
   TELL_FORMAT(p, "<channel> <view|colour type|seperator> <value>");
}

static void internal_channels_init_player(player_tree_node *current,
                                          va_list ap)
{
 priv_test_type test_fn = va_arg(ap, priv_test_type);
 const char *chan = va_arg(ap, const char *);
 
 if ((*test_fn)(current))
   channels_add_system(chan, current);
}

channels_node *channels_add_system(const char *name, player_tree_node *current)
{
 channels_base *base = channels_find_base(name);
 channels_node *node = NULL;
 
 if (base)
   if (!(node = (channels_node *)
        player_link_find(base->players_start, current, NULL,
                         PLAYER_LINK_NAME_ORDERED)))
     node = channels_add_node(base, current, CHANNELS_JOIN_LEAVE_USR_ON);

 return (node);
}

void channels_del_system(const char *name, player_tree_node *current)
{
 channels_base *base = channels_find_base(name);
 channels_node *node = NULL;
 
 if (base && (node = channels_find_node(base, current)))
   channels_del_node(base, node, CHANNELS_JOIN_LEAVE_SYS_OFF);
}

static void channels_load(void)
{
 file_io real_io_channels_index;
 file_io *io_channels_index = &real_io_channels_index;
 int number_of_channels = 0;
 int base_count = 0;
 int del_count = 0;
 
 if (!file_read_open("files/sys/channels/index", io_channels_index))
   return;

 file_section_beg("header", io_channels_index);
 
 number_of_channels = file_get_int("number_of_channels", io_channels_index);
 
 file_section_end("header", io_channels_index);


 /* cun to make alphabetising work,
    plus they are called "zchannels" on some talkers */
 file_section_beg("z", io_channels_index);
 
 while (base_count < number_of_channels)
 {
  char buf[8 + BUF_NUM_TYPE_SZ(int)];
  char file_buf[sizeof("files/sys/channels/x/x/") + CHANNELS_NAME_SZ];
  file_io real_io_channels_chan;
  file_io *io_channels_chan = &real_io_channels_chan;
  int flag_no_kill = FALSE;
  int def_colour_type = 1;
  int def_name_sep = 1;
  list_node *list_start = NULL;
  int number_of_players = 0;
  char name[CHANNELS_NAME_SZ];
  size_t name_sz = 0;
  int count_players = 0;
  int del_players = 0;
  channels_base *base = NULL;
  
  sprintf(buf, "%08d", ++base_count);

  name_sz = file_get_string(buf, name, CHANNELS_NAME_SZ, io_channels_index);
  if ((name_sz >= CHANNELS_NAME_SZ) ||
      !isalpha((unsigned char)name[0]) ||
      !isalpha((unsigned char)name[1]))
  {
   ++del_count;
   continue;
  }
  
  sprintf(file_buf, "files/sys/channels/%c/%c/%s", name[0], name[1], name);
  if (!file_read_open(file_buf, io_channels_chan))
  {
   ++del_count;
   continue;
  }
  
  if (!(def_colour_type = file_get_short("def_colour_type", io_channels_chan)))
    def_colour_type = 1;
  if (!(def_name_sep = file_get_short("def_name_sep", io_channels_chan)))
    def_name_sep = 1;
  
  flag_no_kill = file_get_bitflag("flag_no_kill", io_channels_chan);
  
  file_section_beg("list", io_channels_chan);
  list_load(&list_start, LIST_TYPE_CHAN, io_channels_chan);
  file_section_end("list", io_channels_chan);

  if (name_sz !=
      file_get_string("name", name, CHANNELS_NAME_SZ, io_channels_chan))
  {
   list_load_cleanup(LIST_TYPE_CHAN, list_start);
   file_read_close(io_channels_chan);
   ++del_count;
   continue;
  }
  
  if (!(base = channels_add_base(name)))
    SHUTDOWN_MEM_ERR();
  assert(base->name_sz == name_sz); /* file_get_string is wasted */
  base->flag_tmp_needs_saving = FALSE;
  
  base->flag_no_kill = flag_no_kill;
  base->def_colour_type = def_colour_type;
  base->def_name_sep = def_name_sep;
  base->list_start = list_start;
  
  file_section_beg("players", io_channels_chan);
    
  file_section_beg("header", io_channels_chan);
  
  number_of_players = file_get_int("number_of_players", io_channels_chan);
  
  file_section_end("header", io_channels_chan);

  file_section_beg("players", io_channels_chan);
  while (count_players < number_of_players)
  {
   char players_buf[BUF_NUM_TYPE_SZ(int)];
   player_tree_node *current = NULL;
   short int colour_type = 0;
   short int name_sep = 0;
   char player_name[PLAYER_S_NAME_SZ];
   channels_node *node = NULL;
   
   sprintf(players_buf, "%04d", ++count_players);
   
   file_section_beg(players_buf, io_channels_chan);

   colour_type = file_get_short("colour_type", io_channels_chan);
   name_sep = file_get_short("name_sep", io_channels_chan);
   
   file_get_string("player", player_name, PLAYER_S_NAME_SZ, io_channels_chan);
   
   if (!(current = player_tree_find_exact(player_name)))
   {
    file_section_end(players_buf, io_channels_chan);
    ++del_players;
    continue;
   }
   
   if (!(node = channels_add_node(base, current, CHANNELS_JOIN_LEAVE_FILE_ON)))
     SHUTDOWN_MEM_ERR();
   
   node->colour_type = colour_type;
   node->name_sep = name_sep;
   
   file_section_end(players_buf, io_channels_chan);
  }
  assert(((count_players - del_players) == base->players_num) &&
         (count_players == number_of_players));

  if (!base->players_num)
  {
   if (base->flag_no_kill)
   {
    if (!base->list_start)
      list_system_change(&base->list_start, LIST_TYPE_CHAN,
                         "admins all on");
   }
   else
   {
    channels_del_base(base);
    ++del_count;
   }
  }

  file_section_end("players", io_channels_chan);
  
  file_section_end("players", io_channels_chan);
  
  file_read_close(io_channels_chan);
 }
 assert(((base_count - del_count) == channels_num) &&
        (base_count == number_of_channels));

 file_section_end("z", io_channels_index);
  
 file_read_close(io_channels_index);
}

static void channels_save_index(void)
{
 file_io real_io_channels;
 file_io *io_channels = &real_io_channels;
 channels_base *base = channels_start;
 int count = 0;
 
 if (configure.talker_read_only)
   return;

 if (!file_write_open("files/sys/channels/index.tmp", CHANNELS_FILE_VERSION,
                      io_channels))
 {
  log_assert(FALSE);
  return;
 }
 
 file_section_beg("header", io_channels);

 file_put_int("number_of_channels", channels_num, io_channels);
 
 file_section_end("header", io_channels);

 /* cun to make alphabetising work,
    plus they are called "zchannels" on some talkers */
 file_section_beg("z", io_channels);

 while (base && (count < channels_num))
 {
  char buf[8 + BUF_NUM_TYPE_SZ(int)];
  char chan_buf[CHANNELS_NAME_SZ];
  size_t len = strlen(base->name);
  
  sprintf(buf, "%08d", ++count);

  log_assert(len < CHANNELS_NAME_SZ);
  
  COPY_STR_LEN(chan_buf, base->name, len);
  lower_case(chan_buf);
  file_put_string(buf, chan_buf, len, io_channels);

  base = base->next;
 }
 assert((count == channels_num) && !base);
 
 file_section_end("z", io_channels);

 if (file_write_close(io_channels))
   rename("files/sys/channels/index.tmp", "files/sys/channels/index");
}

static int internal_channels_save_chan(player_linked_list *passed_scan,
                                       va_list ap)
{
 file_io *io_channels = va_arg(ap, file_io *);
 int *count = va_arg(ap, int *);
 player_tree_node *scan = PLAYER_LINK_SAV_GET(passed_scan);
 channels_node *node = (channels_node *)passed_scan;
 char buf[BUF_NUM_TYPE_SZ(int)];
 
 sprintf(buf, "%04d", ++*count);
 
 file_section_beg(buf, io_channels);

 file_put_short("colour_type", node->colour_type, io_channels);
 file_put_short("name_sep", node->name_sep, io_channels);

 file_put_string("player", scan->lower_name, 0, io_channels);
 
 file_section_end(buf, io_channels);

 return (TRUE);
}

static void channels_save_chan(channels_base *base)
{
 file_io real_io_channels;
 file_io *io_channels = &real_io_channels;
 char chan_name[CHANNELS_NAME_SZ];
 char channels_file[sizeof("files/sys/channels/a/a/.tmp") + CHANNELS_NAME_SZ];
 char channels_file_ren[sizeof("files/sys/channels/a/a/") + CHANNELS_NAME_SZ];
 int count = 0;

 if (configure.talker_read_only)
   return;

 if (!base->flag_tmp_needs_saving)
   return;

 COPY_STR(chan_name, base->name, CHANNELS_NAME_SZ);
 lower_case(chan_name);
 
 sprintf(channels_file_ren, "%s/%c/%c/%s", "files/sys/channels",
         chan_name[0], chan_name[1], chan_name);
 sprintf(channels_file, "%s.tmp", channels_file_ren);

 if (!file_write_open(channels_file, CHANNELS_FILE_VERSION, io_channels))
 {
  log_assert(FALSE);
  return;
 }
  
 file_put_short("def_colour_type", base->def_colour_type, io_channels);
 file_put_short("def_name_sep", base->def_name_sep, io_channels);
 
 file_put_bitflag("flag_no_kill", base->flag_no_kill, io_channels);
 
 file_section_beg("list", io_channels);
 list_save(&base->list_start, LIST_TYPE_CHAN, io_channels);
 file_section_end("list", io_channels);

 assert(strlen(base->name) == base->name_sz);
 file_put_string("name", base->name, base->name_sz, io_channels);
 
 file_section_beg("players", io_channels);
 
 file_section_beg("header", io_channels);
 
 file_put_int("number_of_players", base->players_num, io_channels);
 
 file_section_end("header", io_channels);
 
 file_section_beg("players", io_channels);
 do_order_misc_all(internal_channels_save_chan, base->players_start,
                   io_channels, &count);
 assert(count == base->players_num);
 file_section_end("players", io_channels);
 
 file_section_end("players", io_channels);
 
 if (file_write_close(io_channels))
   rename(channels_file, channels_file_ren);

 base->flag_tmp_needs_saving = FALSE;
}

static void channels_save_all(int force)
{
 channels_base *base = channels_start;
 int count = 0;
 
 if (configure.talker_read_only)
   return;

 while (base && (count < channels_num))
 {
  if (force || base->flag_tmp_needs_saving)
    channels_save_chan(base);

  base = base->next;
  ++count;
 }
 assert((count == channels_num) && !base);
}

static void internal_channels_staff_join_leave(player_tree_node *sp, int type)
{ /* auto go on/off duty when you join/leave the staff channel */
 if (P_IS_AVL(sp))
   switch (type)
   {
    case CHANNELS_JOIN_LEAVE_FILE_ON:
      sp->player_ptr->flag_tmp_su_channel_block = FALSE;
      break;
      
    case CHANNELS_JOIN_LEAVE_USR_ON:
      sp->player_ptr->flag_tmp_su_channel_block = FALSE;
      if (P_IS_ON(sp))
        stats_log_event(sp->player_ptr, STATS_ON_SU, STATS_NO_EXTRA);
      break;
      
    case CHANNELS_JOIN_LEAVE_SYS_ON:
      sp->player_ptr->flag_tmp_su_channel_block = FALSE;
      if (P_IS_ON(sp))
        stats_log_event(sp->player_ptr, STATS_ON_SU, STATS_SU_FORCED);
      break;
      
    case CHANNELS_JOIN_LEAVE_USR_BOOT:
    case CHANNELS_JOIN_LEAVE_SYS_OFF:
      sp->player_ptr->flag_tmp_su_channel_block = TRUE;
      if (P_IS_ON(sp))
        stats_log_event(sp->player_ptr, STATS_OFF_SU, STATS_SU_FORCED);
      break;
      
    case CHANNELS_JOIN_LEAVE_USR_OFF:
      sp->player_ptr->flag_tmp_su_channel_block = TRUE;
      if (P_IS_ON(sp))
        stats_log_event(sp->player_ptr, STATS_OFF_SU, STATS_NO_EXTRA);
      break;
      
    case CHANNELS_JOIN_LEAVE_SYS_DIE:
      sp->player_ptr->flag_tmp_su_channel_block = TRUE;
      break;
      
    default:
      log_assert(FALSE);
   }
}

void init_channels(void)
{
 channels_base *base = NULL;
 int changed = FALSE;

 BUILD_FILE_ALPHA_HASH("files/sys/channels");
 
 channels_load();
 
 CHANNELS_INTERNAL_START(configure.channels_main_name);
 list_system_change(&base->list_start, LIST_TYPE_CHAN,
                    "sus all on");
 list_system_change(&base->list_start, LIST_TYPE_CHAN,
                    "everyone read,write on");
 CHANNELS_INTERNAL_END(none);

 CHANNELS_INTERNAL_START("Spod");
 list_system_change(&base->list_start, LIST_TYPE_CHAN,
                    "spods read,write on");
 list_system_change(&base->list_start, LIST_TYPE_CHAN,
                    "admin config,read,write on");
 base->def_name_sep = 2;
 base->def_colour_type = 2;
 CHANNELS_INTERNAL_END(spod);
 
 CHANNELS_INTERNAL_START("Minister");
 list_system_change(&base->list_start, LIST_TYPE_CHAN, "admins all on");
 list_system_change(&base->list_start, LIST_TYPE_CHAN,
                    "minsters read,write on");
 CHANNELS_INTERNAL_END(minister);

 CHANNELS_INTERNAL_START("Staff");
 list_system_change(&base->list_start, LIST_TYPE_CHAN, "admins all on");
 list_system_change(&base->list_start, LIST_TYPE_CHAN, "sus read,write on");
 base->def_name_sep = 3;
 base->def_colour_type = 3;
 CHANNELS_INTERNAL_END(pretend_su);
 base->join_leave_func = internal_channels_staff_join_leave;
 base->flag_no_blocks = TRUE; /* FIXME: make this generic ...
                               * allow configure to do bases */

 CHANNELS_INTERNAL_START("Admin");
 list_system_change(&base->list_start, LIST_TYPE_CHAN,
                    "admins config,read,write on");
 CHANNELS_INTERNAL_END(lower_admin);

 CHANNELS_INTERNAL_START("Debug");
 list_system_change(&base->list_start, LIST_TYPE_CHAN, "admin config,read on");
 CHANNELS_INTERNAL_END(coder_and_admin);

 CHANNELS_INTERNAL_START("log:bug");
 list_system_change(&base->list_start, LIST_TYPE_CHAN, "spod read on");
 list_system_change(&base->list_start, LIST_TYPE_CHAN,
                    "admin config,read,who on");
 CHANNELS_INTERNAL_END(coder);
 
 CHANNELS_INTERNAL_START("log:suggest");
 list_system_change(&base->list_start, LIST_TYPE_CHAN, "spod read on");
 list_system_change(&base->list_start, LIST_TYPE_CHAN,
                    "admin config,read,who on");
 CHANNELS_INTERNAL_END(coder);

 if (changed)
 {
  channels_save_index();
  channels_save_all(TRUE);
 }
}

void user_configure_channels_main_name(player *p, parameter_holder *params)
{
 if ((params->last_param != 3) ||
     (GET_PARAMETER_LENGTH(params, 2) != 1) ||
     (GET_PARAMETER_LENGTH(params, 3) != 2))
   TELL_FORMAT(p, "<name> <1 character shortcut> <2 character shortcut>");

 CHANNELS_NAME_CHECK(GET_PARAMETER_STR(params, 1));

 COPY_STR(configure.channels_main_name, GET_PARAMETER_STR(params, 1),
          CHANNELS_NAME_SZ);
 configure.channels_main_name_1_1 = GET_PARAMETER_STR(params, 2)[0];
 configure.channels_main_name_2_1 = GET_PARAMETER_STR(params, 3)[0];
 configure.channels_main_name_2_2 = GET_PARAMETER_STR(params, 3)[1];

 configure_save(FALSE);
}

void user_configure_channels_players_do_all(player *p, const char *str)
{
 USER_CONFIGURE_INT_FUNC(channels_players_do_all,
                         "Channels", "players do all size", 1, INT_MAX);
}

void user_configure_channels_players_join(player *p, const char *str)
{
 USER_CONFIGURE_INT_FUNC(channels_players_join,
                         "Channels", "players join size", 1, INT_MAX);
}

void cmds_init_channels(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("channels", channels_command, RET_CHARS_SIZE_T, COMMUNICATION);
 CMDS_PRIV(base);
 
#define CMDS_SECTION_SUB CMDS_SECTION_CHANNELS
 CMDS_ADD_SUB("end", channels_exit_command, NO_CHARS);
 CMDS_PRIV(mode_channels);
 CMDS_ADD_SUB("commands", user_channels_view_commands, NO_CHARS);

 CMDS_ADD_SUB("join", user_channels_join, PARSE_PARAMS);
 CMDS_ADD_SUB("leave", user_channels_leave, PARSE_PARAMS);
 CMDS_ADD_SUB("channels", user_channels_list, CONST_CHARS);
 CMDS_ADD_SUB("who", user_channels_who, PARSE_PARAMS);
 
 CMDS_ADD_SUB("list", user_list_chan_view, PARSE_PARAMS);
 CMDS_PRIV(command_room_list); /* more privs done in cmd */
 CMDS_ADD_SUB("clist", user_list_chan_del, PARSE_PARAMS);
 CMDS_PRIV(command_room_list); /* more privs done in cmd */
 CMDS_ADD_SUB("list_change", user_list_chan_change, PARSE_PARAMS);
 CMDS_PRIV(command_room_list); /* more privs done in cmd */

 CMDS_ADD_SUB("say", user_channels_say, CHARS_SIZE_T);
 CMDS_ADD_SUB("emote", user_channels_emote, CONST_CHARS);

 CMDS_ADD_SUB("boot", user_channels_boot, PARSE_PARAMS);
 
 CMDS_ADD_SUB("configure", user_channels_config, PARSE_PARAMS);
#undef CMDS_SECTION_SUB
}
