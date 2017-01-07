#define AUTH_PLAYER_C
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

int auth_player_total_max = TALKER_MAX_PLAYERS_ON;
int auth_player_logging_on_max = TALKER_MAX_PLAYERS_LOGGING_ON;

static Timer_q_base timer_queue_auth_player_name;
static Timer_q_base timer_queue_auth_player_net;

static auth_player_name *start_name = NULL;
static auth_player_net *start_net = NULL;

static const unsigned int auth_player_ip_masks[9] =
{ /* magic numbers ... well not really, think binary */
 0x00,
 0x01, 0x03, 0x07, 0x0F,
 0x1F, 0x3F, 0x7F, 0xFF,
};

static void user_su_banish_player(player *p, const char *str)
{
 player_tree_node *sp = NULL;

 if (!*str)
   TELL_FORMAT(p, "<player>");

 CHECK_DUTY(p);
 
 if (!(sp = player_find_all(p, str, PLAYER_FIND_SC_SU)))
   return;
 
 if (!priv_test_user_check(p->saved, sp, "banish", 1))
   return;

 player_load(sp);
 sp->flag_tmp_player_needs_saving = TRUE;
 sp->priv_banished = TRUE;
 
 if (P_IS_ON(sp))
 {
  logoff_all(sp->lower_name);
  stats_log_event(sp->player_ptr, STATS_RESI_OFF, STATS_RESI_OFF_FORCED);
  
  fvtell_player(SYSTEM_T(sp->player_ptr), "%s",
                "\n\n -=> You have been banished !!!.\n\n"
                "     This means you can no longer log on with this name!\n"
                "\n\n");
 }

 channels_wall("staff", 3, NULL, " -=> %s banish%s %s.", p->saved->name,
               (p->gender != GENDER_PLURAL) ? "es" : "", str);

 vwlog("banish", "%s banish%s %s.\n",
       p->saved->name, (p->gender != GENDER_PLURAL) ? "es" : "", str);
}

static void user_su_unbanish_player(player *p, const char *str)
{
 player_tree_node *sp = NULL;

 CHECK_DUTY(p);

 if (!(sp = player_find_all(p, str, PLAYER_FIND_SC_SU)))
   return;

 if (!sp->priv_banished)
 {
  fvtell_player(NORMAL_T(p), " The player -- ^S^B%s^s -- isn't banished!\n",
                sp->name);
  return;
 }

 if (timer_q_find_data(&nuke_timer_queue, sp))
 {
  fvtell_player(NORMAL_T(p), " The player -- ^S^B%s^s -- is system banished, "
                "which means you can't unbanish them.\n",
                sp->name);
  return;
 }
 
 player_load(sp);
 
 sp->priv_banished = FALSE;
 sp->flag_tmp_player_needs_saving = TRUE;
 
 channels_wall("staff", 3, NULL, " -=> %s unbanish%s %s.", 
               p->saved->name, (p->gender != GENDER_PLURAL) ? "es" : "", str);
 
 vwlog("banish", "%s banish%s %s.\n",
       p->saved->name, (p->gender != GENDER_PLURAL) ? "es" : "", str);
}

static unsigned int auth_player_name_number(void)
{
 auth_player_name *scan = start_name;
 unsigned int count = 0;
 
 while (scan)
 {
  if (!scan->flag_tmp_tmp)
    ++count;
  scan = scan->next;
 }

 return (count);
}

static unsigned int auth_player_net_number(void)
{
 auth_player_net *scan = start_net;
 unsigned int count = 0;
 
 while (scan)
 {
  if (!scan->flag_tmp_tmp)
    ++count;
  scan = scan->next;
 }

 return (count);
}

static void auth_player_save(void)
{
 file_io fs;

 if (file_write_open("files/sys/auth_player.tmp",
                     AUTH_PLAYER_FILE_VERSION, &fs))
 {
  int count = 0;
  auth_player_name *scan_names = start_name;
  auth_player_net *scan_nets = start_net;

  file_section_beg("name", &fs);

  file_section_beg("header", &fs);
  file_put_unsigned_int("number", auth_player_name_number(), &fs);
  file_section_end("header", &fs);
  
  file_section_beg("names", &fs);
  
  while (scan_names)
  {
   if (!scan_names->flag_tmp_tmp)
   {
    char buffer[BUF_NUM_TYPE_SZ(int)];

    sprintf(buffer, "%05d", ++count);
    
    file_section_beg(buffer, &fs);
    
    file_put_bitflag("block", scan_names->block, &fs);
    file_put_string("code", scan_names->code, 0, &fs);
    
    file_section_end(buffer, &fs);
   }
   
   scan_names = scan_names->next;
  }
  
  file_section_end("names", &fs);
  
  file_section_end("name", &fs);

  file_section_beg("net", &fs);

  file_section_beg("header", &fs);
  file_put_unsigned_int("number", auth_player_net_number(), &fs);
  file_section_end("header", &fs);

  file_section_beg("nets", &fs);

  count = 0;
  while (scan_nets)
  {
   if (!scan_nets->flag_tmp_tmp)
   {
    char buffer[BUF_NUM_TYPE_SZ(int)];
    
    sprintf(buffer, "%05d", ++count);
    
    file_section_beg(buffer, &fs);
    
    file_put_short("auth_type", scan_nets->auth_type, &fs);
    
    file_section_beg("ip_addr", &fs);
    file_put_short("1", scan_nets->ips[0], &fs);
    file_put_short("2", scan_nets->ips[1], &fs);
    file_put_short("3", scan_nets->ips[2], &fs);
    file_put_short("4", scan_nets->ips[3], &fs);
    file_section_end("ip_addr", &fs);
    
    file_put_short("ip_mask", scan_nets->ip_mask, &fs);
    
    file_section_end(buffer, &fs);
   }
   
   scan_nets = scan_nets->next;
  }
  
  file_section_end("nets", &fs);
  
  file_section_end("net", &fs);

  if (file_write_close(&fs))
    rename("files/sys/auth_player.tmp", "files/sys/auth_player");
 }
}

static auth_player_name *auth_player_name_add(player *p, long order,
                                              char *code,
                                              int block, int do_save)
{
 auth_player_name *add = XMALLOC(sizeof(auth_player_name), AUTH_PLAYER_NAME);
 auth_player_name *scan = start_name;
 const char *errptr = NULL;
 int erroffset = 0;
 
 if (!add)
 {
  if (p)
    P_MEM_ERR(p);
  LOG_MEM_ERR();
  return (NULL);
 }

 add->flag_tmp_tmp = FALSE;
 add->code = code;
 if (!(add->compiled_code = pcre_compile(code, 0, &errptr, &erroffset, NULL)))
 {
  if (p)
    fvtell_player(SYSTEM_T(p), " Pcre err: at char %d (%s).\n",
                  erroffset, errptr);
  return (NULL);
 }
 
 add->studied_code = pcre_study(add->compiled_code, 0, &errptr);
 add->block = block;

 if (!scan || (order == 1))
 {
  if ((add->next = scan))
    scan->prev = add;
  add->prev = NULL;
  start_name = add;
 }
 else
 {
  while (scan && (--order > 0))
    scan = scan->next;

  if ((add->next = scan->next))
    add->next->prev = add;
  
  scan->next = add;
  add->prev = scan;
 }
 
 if (do_save)
   auth_player_save();

 if (p)
 {
  fvtell_player(NORMAL_T(p), "%s", " Added regexp '^S^B");
  fvtell_player(NORMAL_FT(RAW_OUTPUT, p), "%s", code);
  fvtell_player(NORMAL_T(p), "^s', with blocking %s.\n", TOGGLE_ON_OFF(block));
 }
 
 return (add);
}

static void internal_auth_player_name_delete(auth_player_name *scan)
{
 if (scan->next)
   scan->next->prev = scan->prev;
 
 if (scan->prev)
   scan->prev->next = scan->next;
 else
   start_name = scan->next;

 if (scan->studied_code)
   (*pcre_free)(scan->studied_code);
 (*pcre_free)(scan->compiled_code);
 
 FREE(scan->code);
 XFREE(scan, AUTH_PLAYER_NAME);
}

static void auth_player_name_delete(player *p, long order)
{
 auth_player_name *scan = start_name;
 long orig = order;
 
 while (scan && (--order > 0))
   scan = scan->next;
 
 if (scan)
 {
  if (p)
  {
   fvtell_player(NORMAL_T(p), " Deleted regexp (%ld) '^S^B", orig);
   fvtell_player(NORMAL_FT(RAW_OUTPUT, p), "%s", scan->code);
   fvtell_player(NORMAL_T(p), "^s', with blocking %s.\n",
                 TOGGLE_ON_OFF(scan->block));
  }
  
  if (scan->flag_tmp_tmp)
    timer_q_del_data(&timer_queue_auth_player_name, scan);
  internal_auth_player_name_delete(scan);

  auth_player_save();
 }
 else
   if (p)
     fvtell_player(NORMAL_T(p), " Regexp -- ^S^B%ld^s -- not found.\n", orig);
}

static void timed_name_delete(int timed_type, void *passed_name_del)
{
 auth_player_name *name_del = passed_name_del;

 TCTRACE("timed_name_delete");

 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;

 internal_auth_player_name_delete(name_del);
}

static void auth_player_name_list(player *p)
{
 auth_player_name *scan = start_name;
 int count = 1;

 ptell_mid(NORMAL_T(p), "Name block list", FALSE);
 while (scan)
 {
  fvtell_player(NORMAL_T(p), "(%d)%s ", count,
                scan->flag_tmp_tmp ? "^S^BT^s" : " ");
  fvtell_player(NORMAL_FT(RAW_OUTPUT, p), "%s", scan->code);
  fvtell_player(NORMAL_T(p), " ^S^B%s^s\n", scan->block ? "block" : "allow");
  
  scan = scan->next;
  ++count;
 }
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static int auth_player_name_do(const char *name)
{
 auth_player_name *scan = start_name;
 size_t len = strlen(name);
 
 while (scan)
 {
  if (pcre_exec(scan->compiled_code, scan->studied_code, name, len,
                0, 0, NULL, 0) >= 0)
    return (!scan->block);
  
  scan = scan->next;
 }
 
 return (TRUE);
}

static void internal_auth_player_tell_net_block(player *p,
                                                auth_player_net *scan)
{
 const char *type = NULL;
 
 switch (scan->auth_type)
 {
  case AUTH_PLAYER_NET_OPEN:
    type = "open";
    break;
  case AUTH_PLAYER_NET_BLOCK_NEWBIES:
    type = "block newbies";
    break;
    
  case AUTH_PLAYER_NET_BLOCK_RESIS:
    type = "block residents and lower";
    break;
    
  case AUTH_PLAYER_NET_BLOCK_SUS:
    type = "block sus and lower";
    break;
    
  case AUTH_PLAYER_NET_BLOCK_ALL:
    type = "block all";
    break;
    
  default:
    assert(FALSE);
 }
 
 fvtell_player(NORMAL_T(p), "%d.%d.%d.%d/%hd, ^S^B%s^s.",
               (int)scan->ips[0], (int)scan->ips[1],
               (int)scan->ips[2], (int)scan->ips[3],
               scan->ip_mask, type);
}

static auth_player_net *auth_player_net_add(player *p, unsigned char *ips,
                                            int ip_mask,
                                            short auth_type, int do_save)
{
 auth_player_net *add = XMALLOC(sizeof(auth_player_net), AUTH_PLAYER_NET);
 auth_player_net *scan = start_net;
 
 if (!add)
 {
  if (p)
    P_MEM_ERR(p);
  LOG_MEM_ERR();
  return (NULL);
 }
 
 add->ips[0] = AUTH_PLAYER_NET_MASK(ips, ip_mask, 0);
 add->ips[1] = AUTH_PLAYER_NET_MASK(ips, ip_mask, 1);
 add->ips[2] = AUTH_PLAYER_NET_MASK(ips, ip_mask, 2);
 add->ips[3] = AUTH_PLAYER_NET_MASK(ips, ip_mask, 3);

 add->ip_mask = ip_mask;
 add->auth_type = auth_type;
 add->flag_tmp_tmp = FALSE;
  
 if (!scan)
 {
  add->next = NULL;
  add->prev = NULL;
  start_net = add;

  if (do_save)
    auth_player_save();
  
  if (p)
  {
   fvtell_player(NORMAL_T(p), " Added network block for ");
   internal_auth_player_tell_net_block(p, add);
  }

  return (add);
 }
 
 while (scan->next && (ip_mask < scan->ip_mask))
   scan = scan->next;

 if (ip_mask < scan->ip_mask)
 {
  if ((add->next = scan->next))
    add->next->prev = add;
  
  add->prev = scan;
  scan->next = add;
 }
 else
 {
  if ((add->prev = scan->prev))
    add->prev->next = add;
  else
    start_net = add;

  add->next = scan;
  scan->prev = add;
 }
 
 if (do_save)
   auth_player_save();

 if (p)
 {
  fvtell_player(NORMAL_T(p), " Added network block for ");
  internal_auth_player_tell_net_block(p, add);
 }
 
 return (add);
}

static void internal_auth_player_net_delete(auth_player_net *scan)
{ 
 if (scan->next)
   scan->next->prev = scan->prev;
 
 if (scan->prev)
   scan->prev->next = scan->next;
 else
   start_net = scan->next;
 
 XFREE(scan, AUTH_PLAYER_NET);
}

static void auth_player_net_delete(player *p, long order)
{
 auth_player_net *scan = start_net;
 long orig = order;
 
 while (scan && (--order > 0))
   scan = scan->next;

 if (scan)
 {
  if (p)
    fvtell_player(NORMAL_T(p), " CIDR netblock -- ^S^B%ld^s -- deleted.\n",
                  orig);

  if (scan->flag_tmp_tmp)
    timer_q_del_data(&timer_queue_auth_player_net, scan);
  internal_auth_player_net_delete(scan);

  auth_player_save();
 }
 else
   if (p)
     fvtell_player(NORMAL_T(p), " CIDR netblock -- ^S^B%ld^s -- not found.\n",
                   orig);
}

static void timed_net_delete(int timed_type, void *passed_net_del)
{
 auth_player_net *net_del = passed_net_del;

 TCTRACE("timed_net_delete");

 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;

 internal_auth_player_net_delete(net_del);
}

static void auth_player_net_list(player *p)
{
 auth_player_net *scan = start_net;
 int count = 1;

 ptell_mid(NORMAL_T(p), "Network block list", FALSE);
 while (scan)
 {
  fvtell_player(NORMAL_T(p), " (%d)%s ",
                count, scan->flag_tmp_tmp ? "^S^BT^s" : " ");
  internal_auth_player_tell_net_block(p, scan);
  
  scan = scan->next;
  ++count;
 }
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static int auth_player_net_do(unsigned char *ips)
{
 auth_player_net *scan = start_net;
 
 while (scan)
 {
  if (AUTH_PLAYER_NET_MASK_EQ(ips, scan->ips, scan->ip_mask))
    return (scan->auth_type);
  
  scan = scan->next;
 }
 
 return (AUTH_PLAYER_NET_OPEN);
}

static void auth_player_load(void)
{
 file_io fs;

 if (file_read_open("files/sys/auth_player", &fs))
 {
  int count = 0;
  int number_of = 0;

  file_section_beg("name", &fs);

  file_section_beg("header", &fs);
  number_of = file_get_unsigned_int("number", &fs);
  file_section_end("header", &fs);
  
  file_section_beg("names", &fs);
  
  while (count < number_of)
  {
   char buffer[BUF_NUM_TYPE_SZ(int)];
   char *code = NULL;
   int block = FALSE;
   
   sprintf(buffer, "%05d", ++count);
   
   file_section_beg(buffer, &fs);
   
   block = file_get_bitflag("block", &fs);
   if (!(code = file_get_malloc("code", NULL, &fs)))
     SHUTDOWN_MEM_ERR();
   
   file_section_end(buffer, &fs);

   if (!auth_player_name_add(NULL, 1, code, block, FALSE))
     shutdown_error("loading banished list: %s, %d.\n", __FILE__, __LINE__);
  }  
  file_section_end("names", &fs);
  
  file_section_end("name", &fs);

  file_section_beg("net", &fs);

  file_section_beg("header", &fs);
  number_of = file_get_unsigned_int("number", &fs);
  file_section_end("header", &fs);

  file_section_beg("nets", &fs);

  count = 0;
  while (count < number_of)
  {
   char buffer[BUF_NUM_TYPE_SZ(int)];
   int ip_mask = 0;
   unsigned char ips[4];
   short auth_type = 0;
   
   sprintf(buffer, "%05d", ++count);

   file_section_beg(buffer, &fs);

   auth_type = file_get_short("auth_type", &fs);

   file_section_beg("ip_addr", &fs);
   ips[0] = file_get_short("1", &fs);
   ips[1] = file_get_short("2", &fs);
   ips[2] = file_get_short("3", &fs);
   ips[3] = file_get_short("4", &fs);
   file_section_end("ip_addr", &fs);
   
   ip_mask = file_get_short("ip_mask", &fs);
   
   file_section_end(buffer, &fs);
   
   auth_player_net_add(NULL, ips, ip_mask, auth_type, FALSE);
  }
  file_section_end("nets", &fs);
  
  file_section_end("net", &fs);

  file_read_close(&fs);
 }
}

int auth_parse_ip_addr(const char *str, unsigned char *ips, int allow_cidr)
{
 long tmp = 0;

 ips[0] = 0;
 ips[1] = 0;
 ips[2] = 0;
 ips[3] = 0;
 
 tmp = skip_atoi(&str);
 if ((tmp > 256) || (tmp < 0) || ((*str != '.') && *str))
   return (FALSE);
 *ips++ = tmp;

 if (!*str) return (TRUE);
 ++str;
 tmp = skip_atoi(&str);
 if ((tmp > 256) || (tmp < 0) || ((*str != '.') && *str))
   return (FALSE);
 *ips++ = tmp;

 if (!*str) return (TRUE);
 ++str;
 tmp = skip_atoi(&str);
 if ((tmp > 256) || (tmp < 0) || ((*str != '.') && *str))
   return (FALSE);
 *ips++ = tmp;

 if (!*str) return (TRUE);
 ++str;
 tmp = skip_atoi(&str);
 if ((tmp > 256) || (tmp < 0) || (*str && allow_cidr ? (*str != '/') : FALSE))
   return (FALSE);
 *ips = tmp;

 return (TRUE);
}

/* Classless Inter-Domain Routing ... so no you know :) */
int auth_parse_cidr(const char *str, unsigned char *ips, short *ip_mask)
{
 const char *end_ip = C_strchr(str, '/');
 long tmp = 0;
 
 if (!end_ip)
   return (FALSE);
 ++end_ip;

 tmp = skip_atoi((const char **)&end_ip);
 if ((tmp > 32) || (tmp < 0) || *end_ip)
   return (FALSE);
 *ip_mask = tmp;

 return (auth_parse_ip_addr(str, ips, TRUE));
}

static void auth_player_internal_tmp_net_add(player *p,
                                             unsigned char ips[4],
                                             short ip_mask,
                                             int auth_type,
                                             unsigned int parsed_time)
{
 auth_player_net *tmp = NULL;
 struct timeval tv;

 gettimeofday(&tv, NULL);
 TIMER_Q_TIMEVAL_ADD_SECS(&tv, parsed_time, 0);
 
 tmp = auth_player_net_add(p, ips, ip_mask, auth_type, TRUE);
 if (!tmp || !timer_q_add_node(&timer_queue_auth_player_net,
                               tmp, &tv, TIMER_Q_FLAG_NODE_DEFAULT))
 {
  if (tmp)
    fvtell_player(SYSTEM_T(p), " Couldn't do a timed delete, so they will "
                  "be blocked forever.\n");
 }
 else
 {
  char buf[256];
  
  tmp->flag_tmp_tmp = TRUE;
  fvtell_player(NORMAL_T(p), " Network block will be removed in ^S^B%s^s.\n",
                word_time_long(buf, sizeof(buf),
                               parsed_time, WORD_TIME_DEFAULT));
 }
}
                                             
                                             

static void user_su_auth_player(player *p, parameter_holder *params)
{
 if (params->last_param < 2)
   TELL_FORMAT(p, "<add | delete | list> <name | net> [options]");

 lower_case(GET_PARAMETER_STR(params, 1));
 lower_case(GET_PARAMETER_STR(params, 2));
 
 if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "add"))
 {
  if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "name"))
  {
   int block = FALSE;
   char *code = NULL;
   
   if ((params->last_param != 5) && (params->last_param != 6))
     TELL_FORMAT(p, "add name <order> <regexp> <block> [period]");
   lower_case(GET_PARAMETER_STR(params, 4));

   if (*(GET_PARAMETER_STR(params, 3) +
         strspn(GET_PARAMETER_STR(params, 3), "0123456789")))
     TELL_FORMAT(p, "add name <order> <regexp> <block> [period]");

   if (!(code = MALLOC(GET_PARAMETER_LENGTH(params, 4) + 1)))
   {
    P_MEM_ERR(p);
    return;
   }
   memcpy(code, GET_PARAMETER_STR(params, 4),
          GET_PARAMETER_LENGTH(params, 4) + 1);
   
   if (TOGGLE_MATCH_ON(GET_PARAMETER_STR(params, 5)))
     block = TRUE;
   else if (TOGGLE_MATCH_OFF(GET_PARAMETER_STR(params, 5)))
     block = FALSE;
   else
   {
    FREE(code);
    TELL_FORMAT(p, "add name <order> <regexp> <on | off> [period]");
   }

   if (params->last_param == 6)
   {
    int err;
    unsigned int parsed_time = 0;

    parsed_time = word_time_parse(GET_PARAMETER_STR(params, 6),
                                  WORD_TIME_PARSE_ERRORS, &err);
    if (!err)
    {
     auth_player_name *tmp = auth_player_name_add(p,
                               strtol(GET_PARAMETER_STR(params, 3), NULL, 10),
                                                  code, block, TRUE);
       
     if (tmp)
     {
      struct timeval tv;

      gettimeofday(&tv, NULL);
      TIMER_Q_TIMEVAL_ADD_SECS(&tv, parsed_time, 0);

      if (timer_q_add_node(&timer_queue_auth_player_name,
                           tmp, &tv, TIMER_Q_FLAG_NODE_DEFAULT))
      {
       char buf[256];
       
       tmp->flag_tmp_tmp = TRUE;
       fvtell_player(NORMAL_T(p), " Name will be removed in ^S^B%s^s.\n",
                     word_time_long(buf, sizeof(buf),
                                    parsed_time, WORD_TIME_DEFAULT));
      }
     }
     else
       FREE(code);
    }
   }
   else
     if (!auth_player_name_add(p,
                               strtol(GET_PARAMETER_STR(params, 3), NULL, 10),
                               code, block, TRUE))
       FREE(code);
  }
  else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "net"))
  {
   short int auth_type = AUTH_PLAYER_NET_OPEN;
   unsigned char ips[4];
   short int ip_mask;

   if ((params->last_param != 5) && (params->last_param != 4))
     TELL_FORMAT(p, "add net <cidr> <auth_type> [period]");
   lower_case(GET_PARAMETER_STR(params, 4));
   
   if (!auth_parse_cidr(GET_PARAMETER_STR(params, 3), ips, &ip_mask))
     TELL_FORMAT(p, "add net <cidr> <auth_type> [period]");
   
   if (!beg_strcmp(GET_PARAMETER_STR(params, 4), "open"))
     auth_type = AUTH_PLAYER_NET_OPEN;
   else if (!beg_strcmp(GET_PARAMETER_STR(params, 4), "newbies"))
     auth_type = AUTH_PLAYER_NET_BLOCK_NEWBIES;
   else if (!beg_strcmp(GET_PARAMETER_STR(params, 4), "residents"))
     auth_type = AUTH_PLAYER_NET_BLOCK_RESIS;
   else if (!beg_strcmp(GET_PARAMETER_STR(params, 4), "sus"))
     auth_type = AUTH_PLAYER_NET_BLOCK_SUS;
   else if (!beg_strcmp(GET_PARAMETER_STR(params, 4), "all"))
     auth_type = AUTH_PLAYER_NET_BLOCK_ALL;
   else
     TELL_FORMAT(p, "net add <cidr> <open | newbies | residents | sus | all>"
                 " [period]");

   if (params->last_param == 5)
   {
    int err;
    unsigned int parsed_time = 0;
    
    parsed_time = word_time_parse(GET_PARAMETER_STR(params, 5),
                                  WORD_TIME_PARSE_ERRORS, &err);
    if (!err && parsed_time)
      auth_player_internal_tmp_net_add(p, ips, ip_mask, auth_type,
                                       parsed_time);
    else
      TELL_FORMAT(p, "net add <cidr> <auth_type> [period]");
   }
   else
   {
    if (auth_player_net_add(p, ips, ip_mask, auth_type, TRUE))
      fvtell_player(NORMAL_T(p), "\n");
   }
  }
  else
    TELL_FORMAT(p, "add <name | net | tmp_net> <options>");
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "delete"))
 {
  if (params->last_param != 3)
    TELL_FORMAT(p, "delete <name | net> <order>");

  if (*(GET_PARAMETER_STR(params, 3) +
        strspn(GET_PARAMETER_STR(params, 3), "0123456789")))
    TELL_FORMAT(p, "delete <name | net> <order>");

  if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "name"))
    auth_player_name_delete(p, strtol(GET_PARAMETER_STR(params, 3), NULL, 10));
  else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "net"))
    auth_player_net_delete(p, strtol(GET_PARAMETER_STR(params, 3), NULL, 10));
  else
    TELL_FORMAT(p, "delete <name | net> <order>");
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "list"))
 {
  if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "name"))
    auth_player_name_list(p);
  else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "net"))
    auth_player_net_list(p);
  else
    TELL_FORMAT(p, "list <name | net>");
 }
 else
   TELL_FORMAT(p, "<add | delete | list> <name | net> [options]");
}

static int construct_same_cidr_list_do(player *scan, va_list ap)
{
 /* params */
 player_tree_node *from = va_arg(ap, player_tree_node *);
 player *to = va_arg(ap, player *);
 twinkle_info *info = va_arg(ap, twinkle_info *);
 int flags = va_arg(ap, int);
 time_t my_now = va_arg(ap, time_t);

 unsigned char *ips = va_arg(ap, unsigned char *);
 short ip_mask = va_arg(ap, short);

 if (AUTH_PLAYER_NET_MASK_EQ(scan->ip_address, ips, ip_mask))
 {
  fvtell_player(ALL_T(from, to, info, flags, my_now),
                "%s: %d.%d.%d.%d ^S^B%s^s\n", scan->saved->name,
                scan->ip_address[0], scan->ip_address[1],
                scan->ip_address[2], scan->ip_address[3],
                scan->dns_address);
 }
 
 return (TRUE);
}

static void user_su_site_view(player *p, const char *str)
{
 unsigned char ips[4];
 short ip_mask = -1;
 char buffer[sizeof("people from %d.%d.%d.%d/%hd") + 3*5];

 if (!*str)
   TELL_FORMAT(p, "<player | CIDR>");
 
 if (isalpha((unsigned char) *str))
 {
  player *scan = NULL; 
  
  if (!(scan = player_find_on(p, str, PLAYER_FIND_SC_SU_ALL |
                              PLAYER_FIND_PICK_FIRST)))
    return;

  memcpy(ips, scan->ip_address, 4);
  ip_mask = 32;
 }
 else if (!auth_parse_cidr(str, ips, &ip_mask))
 {
  fvtell_player(SYSTEM_T(p), " The string -- ^S^B%s^s -- isn't a valid"
                " player or CIDR address.\n", str);
  TELL_FORMAT(p, "<player | CIDR>");
  return;
 }

 sprintf(buffer, "people from %d.%d.%d.%d/%hd",
         (int)ips[0], (int)ips[1], (int)ips[2], (int)ips[3], ip_mask);
 ptell_mid(NORMAL_T(p), buffer, FALSE);

 DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
               (construct_same_cidr_list_do, NORMAL_T(p), ips, ip_mask));

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static int construct_logoff_cidr_list_do(player *scan, va_list ap)
{
 unsigned char *ips = va_arg(ap, unsigned char *);
 short ip_mask = va_arg(ap, short);

 if (PRIV_STAFF(scan->saved))
   return (TRUE);
 
 if (AUTH_PLAYER_NET_MASK_EQ(scan->ip_address, ips, ip_mask))
 {
  fvtell_player(SYSTEM_T(scan), "%s",
                " You have been dragged from the talker, because you are "
                "from the site site as an abusive player.\n");
    
  user_logoff(scan, NULL);
 }
 
 return (TRUE);
}

static void user_su_site_drag(player *p, parameter_holder *params)
{
 unsigned char ips[4];
 short ip_mask = -1;
 unsigned int parsed_time = MK_MINUTES(1);
 
 switch (params->last_param)
 {
  case 2:
  {
   int err;
   parsed_time = word_time_parse(GET_PARAMETER_STR(params, 2),
                                 WORD_TIME_PARSE_ERRORS, &err);

   if (err)
     TELL_FORMAT(p, "<player | CIDR> [time]");
  }
  break;
  
  case 1:
    break;

  default:
    TELL_FORMAT(p, "<player | CIDR> [time]");
 }
 
 if (isalpha((unsigned char) *GET_PARAMETER_STR(params, 1)))
 {
  player *scan = NULL; 
  
  if (!(scan = player_find_on(p, GET_PARAMETER_STR(params, 1),
                              PLAYER_FIND_SC_SU_ALL | PLAYER_FIND_PICK_FIRST)))
    return;

  memcpy(ips, scan->ip_address, 4);
  ip_mask = 32;
 }
 else if (!auth_parse_cidr(GET_PARAMETER_STR(params, 1), ips, &ip_mask))
 {
  fvtell_player(SYSTEM_T(p), " The string -- ^S^B%s^s -- isn't a valid"
                " player or CIDR address.\n", GET_PARAMETER_STR(params, 1));
  TELL_FORMAT(p, "<player | CIDR>");
  return;
 }

 do_inorder_logged_on(construct_logoff_cidr_list_do, ips, ip_mask);
  
 fvtell_player(NORMAL_T(p), " Residents from %d.%d.%d.%d/%hd, "
               "have been dragged from the program.\n",
               (int)ips[0], (int)ips[1], (int)ips[2], (int)ips[3], ip_mask);
 channels_wall("staff", 3, p,
               " -=> %s%s site dragged residents from %d.%d.%d.%d/%hd.", 
               gender_choose_str(p->gender, "", "", "The ", "The "),
               p->saved->name,
               (int)ips[0], (int)ips[1], (int)ips[2], (int)ips[3], ip_mask);

 if (parsed_time)
   auth_player_internal_tmp_net_add(p, ips, ip_mask,
                                    AUTH_PLAYER_NET_BLOCK_RESIS, parsed_time);
}

static void user_su_trace(player *p, const char *str)
{
 player *p2 = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<person>");

 if (!(p2 = player_find_load(p, str, PLAYER_FIND_SC_SU_ALL)))
   return;

 ptell_mid(NORMAL_T(p), "Trace", FALSE);
 
 if (p2->email[0])
 {
  assert(p2->saved->priv_base);
  if (!p2->saved->flag_private_email || p->saved->priv_admin)
    fvtell_player(NORMAL_T(p),
                  " %s %s%s\n",
                  p2->saved->name, p2->email,
                  (p2->saved->flag_private_email ? " (private)" : ""));
 }

 if (p2->is_fully_on)
   fvtell_player(NORMAL_T(p), " %s is connected from %s.\n",
                 p2->saved->name, p2->dns_address);
 else
 {
  fvtell_player(NORMAL_T(p),
                " %-*s last connected from %d.%d.%d.%d:%s\n"
                " %-*s and disconnected at ",
                PLAYER_S_NAME_SZ, p2->saved->name,
                p2->saved->last_ip_address[0],
                p2->saved->last_ip_address[1],
                p2->saved->last_ip_address[2],
                p2->saved->last_ip_address[3],
                p2->saved->last_dns_address,
                PLAYER_S_NAME_SZ, "");
  
  fvtell_player(NORMAL_T(p),
                "%s\n", DISP_TIME_P_STD(p2->saved->logoff_timestamp, p));
 }

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

void user_configure_talker_closed_to_newbies(player *p, const char *str)
{ /* command added in user_configure.c */
 const char *arg = "";
 
 CHECK_DUTY(p);
 
 if (TOGGLE_MATCH_ON(str) || !beg_strcasecmp(str, "open"))
 {
  if (configure.talker_closed_to_newbies)
  {
   configure.talker_closed_to_newbies = FALSE;
   
   channels_wall("staff", 3, NULL,
                 " -=> %s%s open%s the program to newbies.", 
                 gender_choose_str(p->gender, "", "", "The ", "The "),
                 p->saved->name, (p->gender == GENDER_PLURAL) ? "" : "s");
   vwlog("newbies", "Program opened to newbies by %s", p->saved->name);
   stats_log_event(p, STATS_OPEN_NEW, STATS_NO_EXTRA);
   return;
  }
  arg = "already ";
 }
 else if (TOGGLE_MATCH_OFF(str) || !beg_strcasecmp(str, "closed"))
 {
  if (!configure.talker_closed_to_newbies)
  {
   configure.talker_closed_to_newbies = TRUE;
   
   channels_wall("staff", 3, NULL,
                 " -=> %s%s close%s the program to newbies.", 
                 gender_choose_str(p->gender, "", "", "The ", "The "),
                 p->saved->name, (p->gender == GENDER_PLURAL) ? "" : "s");
   
   vwlog("newbies", "Program closed to newbies by %s",p->saved->name);
   stats_log_event(p, STATS_CLOSE_NEW, STATS_NO_EXTRA);
   return;
  }
  arg = "already ";
 }
 else if (*str)
   TELL_FORMAT(p, "[on|off]");

 if (configure.talker_closed_to_newbies)
   fvtell_player(NORMAL_T(p),
                 " Program is ^S^B%sclosed^s to newbies.^N\n", arg);
 else
   fvtell_player(NORMAL_T(p),
                 " Program is ^S^B%sopen^s to newbies.^N\n", arg);
}

void user_configure_talker_closed_to_resis(player *p, const char *str)
{ /* command added in user_configure.c */
 const char *arg = "";
 
 CHECK_DUTY(p);
 
 if (TOGGLE_MATCH_ON(str) || !beg_strcasecmp(str, "open"))
 {
  if (configure.talker_closed_to_resis)
  {
   configure.talker_closed_to_resis = FALSE;
    
   vwlog("resis", "Program opened to resis by %s",p->saved->name);
   
   channels_wall("staff", 3, NULL,
                 " -=> %s%s open%s the program to resisidents.",
                 gender_choose_str(p->gender, "", "", "The ", "The "),
                 p->saved->name, (p->gender == GENDER_PLURAL) ? "" : "s");
   
   return;
  }
  arg = "already ";
 }
 else if (TOGGLE_MATCH_OFF(str) || !beg_strcasecmp(str, "closed"))
 {
  if (!configure.talker_closed_to_resis)
  {
   configure.talker_closed_to_resis = TRUE;
   
   channels_wall("staff", 3, NULL,
                 " -=> %s%s close%s the program to residents.", 
                 gender_choose_str(p->gender, "", "", "The ", "The "),
                 p->saved->name,
                 (p->gender == GENDER_PLURAL) ? "" : "s");
   
   vwlog("resis", "Program closed to resis by %s",p->saved->name);
   return;
  }
  arg = "already ";
 }
 else if (*str)
   TELL_FORMAT(p, "[on|off]");

 if (configure.talker_closed_to_resis)
   fvtell_player(NORMAL_T(p),
                 " Program is ^S^B%sclosed^s to residents.^N\n", arg);
 else
   fvtell_player(NORMAL_T(p),
                 " Program is ^S^B%sopen^s to residents.^N\n", arg);
}

int auth_check_logon(player *p)
{
 if (current_players >= (auth_player_total_max + 4))
 {
  fvtell_player(ALL_T(0, p, NULL, 3 | RAW_SPECIALS | SYSTEM_INFO, 0), "%s",
                msg_full.text);
  return (FALSE);
 }

 if (logging_onto_count > auth_player_logging_on_max)
 {
  fvtell_player(ALL_T(0, p, NULL, 3 | RAW_SPECIALS | SYSTEM_INFO, 0), "%s",
                msg_full_logon.text);
  return (FALSE);
 }

 if (auth_player_net_do(p->ip_address) == AUTH_PLAYER_NET_BLOCK_ALL)
 {
  fvtell_player(ALL_T(p->saved, p, NULL, 3, 0), "%s",
                msg_auth_blocked_net.text);
  return (FALSE);
 }
 
 return (TRUE);
}

int auth_check_player(player *p)
{
 if (PRIV_SYSTEM_ROOM(p->saved))
 {
  fvtell_player(ALL_T(p->saved, p, NULL, 3, 0), "%s", 
                "\n Sorry but that name is reserved.\n"
                " Choose a different name ...\n\n");
  return (FALSE);
 }
 
 if (!p->saved->priv_admin && (current_players >= auth_player_total_max))
 {
  fvtell_player(ALL_T(0, p, NULL, 3 | RAW_SPECIALS | SYSTEM_INFO, 0), "%s",
                msg_full.text);
  return (FALSE);
 }

 if (configure.talker_closed_to_resis && !PRIV_STAFF(p->saved))
 {
  fvtell_player(ALL_T(p->saved, p, NULL, 3, 0), "%s",
                msg_auth_no_residents.text);
  return (FALSE);
 }

 if (!p->saved->priv_base)
 {
  if (configure.talker_closed_to_newbies)
  {
   fvtell_player(ALL_T(p->saved, p, NULL, 3, 0), "%s",
                 msg_auth_no_newbies.text);
   return (FALSE);
  }

  if (strnlen(p->saved->lower_name, 3) != 3)
  { /* 2 letter names are bad, but some people have them on talkers
     * already :( */
   fvtell_player(LOGON_T(p), "%s", 
                 " Thats a bit short, try something longer.\n"
                 " Although if you really want this name then an admin "
                 "might let you have it if you logon (with a different name) "
                 "and ask.\n\n");
   return (FALSE);
  }

  if (!auth_player_name_do(p->saved->lower_name))
  {
   fvtell_player(ALL_T(p->saved, p, NULL, 3, 0), "%s",
                 msg_auth_blocked_name.text);
   return (FALSE);
  }
 }
 else
 {
  if (p->saved->priv_banished ||
      timer_q_find_data(&timer_queue_player_no_logon, p->saved))
  { /* FIXME: **** real message */
   fvtell_player(LOGON_T(p), "%s", " You are not allowed to connect atm.\n");
   return (FALSE);
  }
 }
 
 switch (auth_player_net_do(p->ip_address))
 {
  case AUTH_PLAYER_NET_OPEN:
    break;
    
  case AUTH_PLAYER_NET_BLOCK_NEWBIES:
    if (p->saved->priv_base)
      break;
    fvtell_player(ALL_T(p->saved, p, NULL, 3, 0), "%s",
                  msg_auth_blocked_net.text);
    return (FALSE);
    
  case AUTH_PLAYER_NET_BLOCK_RESIS:
    if (PRIV_STAFF(p->saved))
      break;
    fvtell_player(ALL_T(p->saved, p, NULL, 3, 0), "%s",
                  msg_auth_blocked_net.text);
    return (FALSE);
    
  case AUTH_PLAYER_NET_BLOCK_SUS:
    if (p->saved->priv_admin)
      break;
    fvtell_player(ALL_T(p->saved, p, NULL, 3, 0), "%s",
                  msg_auth_blocked_net.text);
    return (FALSE);
    
  case AUTH_PLAYER_NET_BLOCK_ALL:
    fvtell_player(ALL_T(p->saved, p, NULL, 3, 0), "%s",
                  msg_auth_blocked_net.text);
    return (FALSE);
 }

 return (TRUE);
}

static void user_su_auth_player_max_users(player *p, parameter_holder *params)
{

 switch (params->last_param)
 {
  default:
    TELL_FORMAT(p, "<total | logon> <number>");
  case 1:
  case 2:
    break;
 }
 lower_case(GET_PARAMETER_STR(params, 1));

 if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "total"))
 {
  if (params->last_param == 2)
    auth_player_total_max = strtol(GET_PARAMETER_STR(params, 2), NULL, 10);
  fvtell_player(NORMAL_T(p),
                " Total ammount of players able to logon is %s%d.\n",
                TOGGLE_CHANGED(params->last_param, 1), auth_player_total_max);
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "logon"))
 {
  if (params->last_param == 2)
    auth_player_logging_on_max = strtol(GET_PARAMETER_STR(params, 2),NULL, 10);
   
  fvtell_player(NORMAL_T(p),
                " Total ammount of players able to be in the "
                "logon stage at once is %s%d.\n",
                TOGGLE_CHANGED(params->last_param, 1),
                auth_player_logging_on_max);
 }
 else
   TELL_FORMAT(p, "<total | logon> <number>");   
}

static void *internal_auth_player_pcre_malloc_wrapper(size_t len)
{
 return (XMALLOC(len, PCRE));
}

static void internal_auth_player_pcre_free_wrapper(void *ptr)
{
 XFREE(ptr, PCRE);
}

void init_auth_player(void)
{
 timer_q_add_static_base(&timer_queue_auth_player_name,
                         timed_name_delete, TIMER_Q_FLAG_BASE_DEFAULT);
 timer_q_add_static_base(&timer_queue_auth_player_net,
                         timed_net_delete, TIMER_Q_FLAG_BASE_DEFAULT);

 pcre_malloc = internal_auth_player_pcre_malloc_wrapper;
 pcre_free = internal_auth_player_pcre_free_wrapper;
 
 auth_player_load();
}

void cmds_init_auth_player(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("auth_player", user_su_auth_player, PARSE_PARAMS, SU);
 CMDS_PRIV(normal_su);

 CMDS_ADD("site", user_su_site_view, CONST_CHARS, SU);
 CMDS_PRIV(command_trace);
 CMDS_ADD("site_drag", user_su_site_drag, PARSE_PARAMS, SU);
 CMDS_FLAG(no_expand); CMDS_PRIV(normal_su);
 
 CMDS_ADD("trace", user_su_trace, CONST_CHARS, SU);
 CMDS_PRIV(command_trace);

 CMDS_ADD("max_users", user_su_auth_player_max_users, PARSE_PARAMS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(coder_admin);

 CMDS_ADD("banish", user_su_banish_player, CONST_CHARS, SU);
 CMDS_FLAG(no_expand); CMDS_PRIV(normal_su);
 CMDS_ADD("unbanish", user_su_unbanish_player, CONST_CHARS, SU);
 CMDS_PRIV(normal_su);

 CMDS_ADD("newbies", user_configure_talker_closed_to_newbies, CONST_CHARS, SU);
 CMDS_FLAG(no_expand); CMDS_PRIV(pretend_su);
}
