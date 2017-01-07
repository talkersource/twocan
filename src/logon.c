#define LOGON_C
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

int logging_onto_count = 0;

Timer_q_base logon_timer_queue;

/* last timestamp someone entered or left the program */
struct timeval last_entered_left = {0, 0};

int total_logons = 0;
int total_uniq_logons = 0;

int current_players = 0;

/* functions... */
static void logon_program(player *);

#define PRINT_FLAG(x, y) ((x) ? (", " y) : (" You are " y))

static void logon_player_flags(player *p)
{
 int done_already = FALSE;
 unsigned int unread_mail = 0;
 
 log_assert(p->saved->priv_base);
 
 fvtell_player(LOGON_T(p), 
               "\n --\n Last logged in at %s from %d.%d.%d.%d:%s.\n",
               DISP_TIME_P_STD(p->saved->logoff_timestamp, p),
               (int)p->saved->last_ip_address[0],
               (int)p->saved->last_ip_address[1],
               (int)p->saved->last_ip_address[2],
               (int)p->saved->last_ip_address[3],
               p->saved->last_dns_address);
 
 if (p->flag_location_hide)
 {
  fvtell_player(LOGON_FT(HILIGHT | RAW_OUTPUT, p), "%s",
                PRINT_FLAG(done_already, "in hiding"));
  done_already = TRUE;
 }
 
 if (p->flag_no_prefix)
 {
  fvtell_player(LOGON_FT(HILIGHT | RAW_OUTPUT, p), "%s",
                PRINT_FLAG(done_already, "ignoring prefixes"));
  done_already = TRUE;
 }
 
 if (p->flag_no_emote_prefix)
 {
  fvtell_player(LOGON_FT(HILIGHT | RAW_OUTPUT, p), "%s",
                PRINT_FLAG(done_already, "ignoring emote prefixes"));
  done_already = TRUE;
 }
 
 if (p->flag_room_enter_brief)
 {
  fvtell_player(LOGON_FT(HILIGHT | RAW_OUTPUT, p), "%s",
                PRINT_FLAG(done_already, "ignoring room descriptions"));
  done_already = TRUE;
 }
 
 if (p->flag_follow_block)
 {
  fvtell_player(LOGON_FT(HILIGHT | RAW_OUTPUT, p), "%s",
                PRINT_FLAG(done_already, "blocking follows"));
  done_already = TRUE;
 }
 
 if (done_already)
   fvtell_player(LOGON_FT(HILIGHT | RAW_OUTPUT, p), "%s", ".\n");
 
 if ((unread_mail = mail_check_mailunread_size(p->saved)))
   fvtell_player(LOGON_T(p), " You have unread mail ^S^B(%d/%d)^s.\n",
                 unread_mail, mail_check_mailbox_size(p->saved));
 
 news_check_new_arrived(LOGON_T(p));
 
 if (PRIV_STAFF(p->saved))
 {
  if (configure.talker_closed_to_newbies)
  {
   if (configure.talker_closed_to_resis)
     fvtell_player(LOGON_FT(HILIGHT, p), "%s",
                   " $Talker-Name is closed to newbies."
                   " $Talker-Name is closed to RESIDENTS.\n");
   else
     fvtell_player(LOGON_FT(HILIGHT, p), "%s",
                   " $Talker-Name is closed to newbies.\n");
  }
  else
    if (configure.talker_closed_to_resis)
      fvtell_player(LOGON_FT(HILIGHT, p), "%s",
                    " $Talker-Name is close to RESIDENTS.\n");
 }
 
 /* no need to turn off the hiligting as it is done above */
 fvtell_player(LOGON_T(p), "%s", " --\n");
}

static void logon_player_newbie(player *p)
{
 BTRACE("logon_player_newbie");
 
 assert(MALLOC_VALID(p->saved, sizeof(player_tree_node), PLAYER_TREE_NODE));
 assert(!player_tree_find_exact(p->saved->lower_name));

 player_list_alpha_add(p->saved);
 player_newbie_add(p->saved);
 
 if (gettimeofday(&last_entered_left, NULL))
 {
  assert(FALSE);
 } 

 multis_init_for_player(p->saved);
}

static void logon_player_resident(player *p)
{
 player_tree_node *current = p->saved;
  
 BTRACE("logon_player_resident");
 
 assert(MALLOC_VALID(current, sizeof(player_tree_node), PLAYER_TREE_NODE));
 assert(player_tree_find_exact(p->saved->lower_name));

 if ((current = p->saved))
 {  
  player_list_alpha_add(current);

  if (gettimeofday(&last_entered_left, NULL))
  {
   assert(FALSE);
  }

  if (PRIV_STAFF(current))
    player_list_logon_staff_add(current);
  
  multis_init_for_player(current);
 }
}

void logon_player_make(player *p, const char *lowered_name,
                       const char *title)
{
 player_tree_node *newbie_node = XMALLOC(sizeof(player_tree_node),
                                         PLAYER_TREE_NODE);

 if (!newbie_node)
   SHUTDOWN_MEM_ERR();
 
 memset(newbie_node, 0, sizeof(player_tree_node));
 
 /* set up saved struct */
 qstrcpy(newbie_node->lower_name, lowered_name);
 qstrcpy(newbie_node->name, lowered_name);
 *newbie_node->name = toupper((unsigned char) *newbie_node->name);
 
 newbie_node->flag_private_email = TRUE;
 p->flag_show_echo = TRUE;
 p->flag_see_echo = TRUE;

 p->flag_no_emote_prefix = TRUE;
 
 p->flag_session_in_who = TRUE;
 
 p->loaded_player = TRUE; /* so P_IS_AVL() works */
 
 p->saved = newbie_node;
 newbie_node->player_ptr = p;

 p->saved->flag_tmp_player_needs_saving = TRUE;
 p->saved->flag_tmp_mail_needs_saving = TRUE;
 p->saved->flag_tmp_room_needs_saving = TRUE;
 
 CONST_COPY_STR(p->prompt,
                "$Room-Owner-Name.$Room-Id-$F-Time-Clock"
                "-" /* note this is a normal - character */
                "$If(=(l(Y)r($F-Set-Duty))t(#)f(>)) ",
                PROMPT_SZ);
 
 CONST_COPY_STR_LEN(p->converse_prompt, "(^BConverse^b)-> ");
 
 CONST_COPY_STR_LEN(p->enter_msg,
                    "enter$F-Conjugate, in a standard kind of way.");
 
 p->mask_coms_hit_timestamp = p->su_motdlr = p->motdlr = now;
 
 p->gender = GENDER_OTHER;
 
 CONST_COPY_STR_LEN(p->title, "$F-Gender(pl(are) def(the)) newbie$F-Plural, "
                    "so treat $F-Gender(pl(them) m(him) f(her) v(it)) "
                    "nicely.");
 p->description[0] = 0;
 p->email[0] = 0;
 p->plan[0] = 0;
 
 p->max_rooms = 4;
 p->max_exits = 5;
 p->max_autos = 5;
 p->max_list_entries = 25;
 p->max_mails = 50;
 p->max_aliases = 32;
 p->max_nicknames = 16;
 
 p->flag_social_auto_name = TRUE;
 p->saved->flag_tmp_list_room_glob_in_core = TRUE;
 p->flag_pager_auto_quit = TRUE;
 p->flag_page_on_return = TRUE;
 p->flag_list_show_inorder = TRUE;
 p->flag_use_birthday_as_age = TRUE;
 p->flag_raw_passwd = TRUE;
 p->birthday = now;

 p->list_newbie_time = MK_DAYS(1);

 p->flag_audible_bell = TRUE;
 p->flag_allow_bell = TRUE;
 p->flag_use_24_clock = TRUE;
 
 if (title && *title)
   COPY_STR(p->title, title, PLAYER_S_TITLE_SZ);
}

/* load and do linking -- NOTE if you change, then make needs changing too */
static int logon_player_load_make(player *p, const char *name,
                                  const char *title)
{
 int load_ret = 0;
 char lowered_name[PLAYER_S_NAME_SZ];
 player_tree_node *tmp = NULL;
 
 COPY_STR(lowered_name, name, PLAYER_S_NAME_SZ);
 
 lower_case(lowered_name);
 
 if ((tmp = player_find_all(p, lowered_name, PLAYER_FIND_DEFAULT)) &&
     (load_ret = player_load(tmp)))
 { /* stuff to do for your resis */
  p->saved = tmp;
  
  p = tmp->player_ptr;

  if (p->term_width && (p->term_width > 200 || (p->term_width < 35)))
  {
   p->saved->flag_tmp_player_needs_saving = TRUE;
   p->term_width = 79;
  }
 }
 else
 {
  log_assert(!tmp); /* This failing means that player_load() failed
                       which is _bad_ */
  logon_player_make(p, lowered_name, title);
 }
 
 if (gettimeofday(&last_entered_left, NULL))
 {
  assert(FALSE);
 }

 return (load_ret);
}

player *player_create(void)
{
 player *p = XMALLOC(sizeof(player), PLAYER);

 BTRACE("player_create");
 
 if (!p)
   return (NULL);
 
 memset(p, 0, sizeof(player)); /* FIXME: needs init func */
 
 colourise_set_defaults(p);
 
 p->term_width = TERMINAL_DEFAULT_WIDTH - 1;
 p->term_height = TERMINAL_DEFAULT_HEIGHT;
 p->word_wrap = 15;
 p->flag_list_show_inorder = TRUE; /* for who at the logon prompt */
 p->flag_use_24_clock = TRUE; /* for uptime at the logon prompt */

 p->last_command_timestamp = now;
 p->logon_timestamp = now;
 
 if (!input_add(p, NULL))
 {
  log_assert(FALSE);
  FREE(p);
  return (NULL);
 }
 
 return (p);
}

static int internal_logon_inform(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 const char *msg = va_arg(ap, const char *);
 const char *su_newbie = va_arg(ap, const char *);
 int do_inform = FALSE;
 int do_friend = FALSE;
 int do_beep = FALSE;
 
 if (scan == p)
   return (TRUE);

 LIST_SELF_CHECK_FLAG_START(scan, p->saved);
 if (LIST_SELF_CHECK_FLAG_DO(inform))
   do_inform = TRUE;
 if (LIST_SELF_CHECK_FLAG_DO(inform_beep))
   do_beep = TRUE;
 if (LIST_SELF_CHECK_FLAG_DO(friend))
   do_friend = TRUE;
 LIST_SELF_CHECK_FLAG_END();

 if (!do_inform)
   return (TRUE);
 
 if (do_friend)
   fvtell_player(INFO_T(p->saved, scan), "%s", "^B");
 
 fvtell_player(INFO_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p->saved, scan),
               "%s", msg);
 
 if (do_friend)
   fvtell_player(INFO_T(p->saved, scan), "%s", " [Friend]");
 else
   if (PRIV_STAFF(scan->saved))
     fvtell_player(INFO_T(p->saved, scan), "%s", su_newbie);
 
 if (do_friend)
   fvtell_player(INFO_T(p->saved, scan), "%s", "^N");
 
 if (do_beep)
   fvtell_player(INFO_T(scan->saved, scan), "%s", "$Bell");
 
 fvtell_player(INFO_T(p->saved, scan), "%s", "\n");

 return (TRUE);
}

void logon_inform(player *p, const char *msg)
{
 const char *su_newbie = "";

 BTRACE("logon_inform");

 log_assert(P_IS_ON_P(p->saved, p));

 if (PRIV_STAFF(p->saved))
   su_newbie = " [Su]";
 else if (!p->saved->priv_base)
   su_newbie = " [Newbie]";
 
 do_inorder_logged_on(internal_logon_inform, p, msg, su_newbie);
}

static void timed_logout(int timed_type, void *passed_player)
{
 player *p = passed_player;

 TCTRACE("timed_logout");

 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;
 
 fvtell_player(LOGON_T(p), "%s", "\n\n Connection timed out.\n\n");
 
 user_logoff(p, NULL);
}

static player *logon_program_resident(player *p)
{
 Timer_q_node *current_timer = NULL;
 player *p_new = p->saved->player_ptr;
 
 socket_player_output(p);
 
 if (p_new->output_start)
   output_list_cleanup(&p_new->output_start);
 if (p_new->output_buffer_tmp)
   output_list_cleanup(&p_new->output_buffer_tmp);

 current_timer = timer_q_find_data(&logon_timer_queue, p);
 assert(current_timer);
 assert(!timer_q_find_data(&logon_timer_queue, p_new));

 current_timer->data = p_new;
 
 assert(!timer_q_find_data(&logon_timer_queue, p));
 
 socket_copy_fd(p_new, p);
 
 if (p->io_indicator)
 {
  int fd = SOCKET_POLL_INDICATOR(p->io_indicator)->fd;
  socket_poll_del(p->io_indicator);
  close(fd);
 }
 p->io_indicator = 0;
 
 if (!p->input_start && !input_add(p, NULL))
   log_assert(FALSE);

 PLAYER_EVENT_UPGRADE(p, RECONNECTION);

 user_logoff(p, NULL);

 p->saved = NULL;
 
 return (p_new);
}

static void logon_program_reconnection(player *p)
{
 log_assert(p == p->saved->player_ptr);
 
 fvtell_player(LOGON_T(p), "%s", "\n Closing ^S^Bother^s connection.\n\n");
 
 output_maybe_delete_line(p);
 
 stats_log_event(p, STATS_RESI_ON, STATS_RESI_ON_RECONNECT);
 
 logon_inform(p, "[$F-Name reconnects] $F-Dns_address");
 
 vtell_room_says(p->location, p,
                 " %s's image shimmers for a moment as %s re-connects.\n",
                 p->saved->name,
                 gender_choose_str(p->gender, "he", "she", "they", "it"));
 fvtell_player(NORMAL_T(p), "%s", " You have now ^S^Breconnected^s.\n");

 assert(p->is_fully_on);
 p->saved->flag_tmp_player_needs_saving = TRUE;
 
 last_command_add(p, "reconnection");
 
 assert(timer_q_find_data(&logon_timer_queue, p));
 timer_q_del_data(&logon_timer_queue, p);
 assert(!timer_q_find_data(&logon_timer_queue, p));
 
 assert(P_IS_ON_P(p->saved, p));

 if (MODE_CURRENT(p).prompt)
   prompt_update(p, MODE_CURRENT(p).prompt);
 else
   prompt_update(p, "Reconnected: "); /* FIXME: -- pager etc. */
}

static void logon_program_new(player *p)
{
 player_list_cron_add(p);
 
 if (gettimeofday(&last_entered_left, NULL))
 {
  assert(FALSE);
 }
}

static void logon_program_command_motd(player *p, const char *str)
{
 ICTRACE("logon_program_command_motd");

 assert(MODE_IN_MODE(p, LOGON_MOTD));

 IGNORE_PARAMETER(str);
 
 mode_del(p);

 p->saved->player_ptr->motdlr = now;
 p->saved->flag_tmp_player_needs_saving = TRUE;

 logon_program(p);
}

static void logon_program_command_sumotd(player *p, const char *str)
{
 ICTRACE("logon_program_command_sumotd");

 assert(MODE_IN_MODE(p, LOGON_SUMOTD));

 IGNORE_PARAMETER(str);
 
 mode_del(p);

 p->saved->player_ptr->su_motdlr = now;
 p->saved->flag_tmp_player_needs_saving = TRUE;

 logon_program(p);
}

static void logon_command_finish(player *p, const char *str)
{ /* finish the initial logon commands */
 ICTRACE("logon_command_finish");

 assert(MODE_IN_MODE(p, LOGON_FINISH));

 IGNORE_PARAMETER(str);
 
 mode_del(p);
 
 logon_program(p);
}

static void logon_command_disclaimer(player *p, const char *str)
{
 ICTRACE("logon_command_disclaimer");

 assert(MODE_IN_MODE(p, LOGON_DISCLAIMER));
 
 if (!strcasecmp(str, "continue"))
 {
  cmds_function tmp_cmd;
  
  p->saved->flag_agreed_disclaimer = TRUE;
  mode_del(p);
  
  CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, logon_command_finish);
  
  fvtell_player(LOGON_T(p), "%s", msg_newbie_finish.text);
  
  mode_add(p, " Hit return to contine: ",
           MODE_ID_LOGON_FINISH, 0, &tmp_cmd, NULL, NULL);
 }
 else if (!strcasecmp(str, "end"))
 {
  fvtell_player(NORMAL_T(p), "%s",
                "\n\n ^S^BDisconnecting^s from program.\n\n");
  user_logoff(p, NULL);
 }
 else
 {
  fvtell_player(NORMAL_T(p), "%s", "\n\n Try again.\n\n");
 }
}

void logon_program(player *p)
{
 assert(p->saved);
 assert(MODE_INVALID(p));
 
 if (p->saved->priv_base && (p != p->saved->player_ptr))
 { /* residents who haven't already been through this function */
  player_load(p->saved);
  
  if (P_IS_ON(p->saved))
  {
   p = logon_program_resident(p);
   logon_program_reconnection(p);
   return;
  }
  else
  {
   if (difftime(p->saved->player_ptr->motdlr, system_data.motd) < 0)
   {
    cmds_function tmp_cmd;
    
    CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, logon_program_command_motd);
    
    fvtell_player(LOGON_T(p), "%s", msg_motd.text);
    
    mode_add(p, " Hit return to contine: ",
             MODE_ID_LOGON_MOTD, 0, &tmp_cmd, NULL, NULL);
    return;
   }

   if (PRIV_STAFF(p->saved) &&
       (difftime(p->saved->player_ptr->su_motdlr, system_data.su_motd) < 0))
   {
    cmds_function tmp_cmd;
    
    CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, logon_program_command_sumotd);
    
    fvtell_player(LOGON_T(p), "%s", msg_sumotd.text);
    
    mode_add(p, " Hit return to contine: ",
             MODE_ID_LOGON_SUMOTD, 0, &tmp_cmd, NULL, NULL);
    return;
   }
   
   p = logon_program_resident(p);
   logon_program_new(p);
  }
 }
 assert(p == p->saved->player_ptr);
 
 if (!p->saved->flag_agreed_disclaimer)
 { /* made characters... also */
  cmds_function tmp_cmd;
  
  CMDS_FUNC_TYPE_CONST_CHARS((&tmp_cmd), logon_command_disclaimer);

  fvtell_player(LOGON_T(p), "%s", msg_disclaimer.text);

  mode_add(p, "Enter 'continue' or 'end': ",
           MODE_ID_LOGON_DISCLAIMER, 0, &tmp_cmd, NULL, NULL);
  return;
 }
 
 if (p->saved->priv_base)
 {
  marriage_update_spouce_name(p);
  
  if (current_players > system_data.max_people)
    system_data.max_people = current_players;
  
  ++system_data.total_ever_logons;

  if (++total_logons > system_data.max_logons)
  {
   system_data.max_logons = total_logons;
   system_data.max_l_timeup = difftime(now, talker_started_on);
  }
  
  if (difftime(p->saved->logoff_timestamp, talker_started_on) < 0)
  { /* player only logged on after before boot */
   if (++total_uniq_logons > system_data.max_uniq_logons)
   {
    system_data.max_uniq_logons = total_uniq_logons;
    system_data.max_nl_timeup = difftime(now, talker_started_on);
   }
  }
 }
 else
 {
  log_assert(logging_onto_count > 0);
  
  if (logging_onto_count > 0)
    --logging_onto_count; /* they have now logged on properly --
                           * residents logoff. */
 }
 
 stats_log_event(p, STATS_RESI_ON, STATS_NO_EXTRA);

 DNS_RESOLVE(); /* need to do it before is_fully_on */

 /* we can tell that they have actualy gone through here */
 p->is_fully_on = TRUE;
 ++current_players;
 
 /* reset tmp flags... soon find out if I've missed any :) */
 last_command_clear(p);
 last_command_add(p, "logon");
 p->idle_logon = 0;
 p->typed_commands = 0;
 p->multi_last_used = 0;
 p->this_cpu.tv_sec = 0;
 p->this_cpu.tv_usec = 0;
 p->flag_tmp_su_channel_off = FALSE;
 p->flag_tmp_su_channel_block = FALSE;
 p->flag_tmp_minister_channel_block = FALSE;
 p->flag_tmp_scripting = FALSE;
 p->flag_tmp_dont_do_normal_msgs = FALSE;
 p->saved->flag_tmp_player_needs_saving = TRUE;
 
 alias_lib_ldconfig(p);
 idle_timer_start(p);
 
 assert(timer_q_find_data(&logon_timer_queue, p));
 timer_q_del_data(&logon_timer_queue, p);
 assert(!timer_q_find_data(&logon_timer_queue, p));

 assert(P_IS_ON_P(p->saved, p));
 
 if (p->saved->priv_base)
 {
  logon_player_flags(p);
  /* setup multis etc... */
  logon_player_resident(p);
  tip_logon_show(p);
 }
 else
   logon_player_newbie(p);

 mode_init_base(p); /* needs to be done before jail'ing */

 log_assert(P_IS_ON_P(p->saved, p));
 logon_inform(p, "[$F-Name $F-Gender(plural(are) def(is)) running riot "
              "in $Talker-Name] $F-Dns_address");

 if (timer_q_find_data(&timer_queue_player_jail, p->saved))
 {
  room_enter_jail(p, ROOM_TRANS_LOGON);
 }
 else if (p->flag_trans_to_home)
 {
  room *r = room_find_home(p->saved);
  
  if (r)
    room_player_transfer(p, r, ROOM_TRANS_LOGON);
  else
  {
   p->flag_trans_to_home = FALSE;
   fvtell_player(NORMAL_T(p), "%s", " -=> Tried to connect your home, "
                 "but you don't have one.\n\n");
  }
 }
 else if (*p->room_connect)
 { /* FIXME: have a better than default error messages */
  if (!room_user_player_transfer(p, p->room_connect, ROOM_TRANS_LOGON))
    *p->room_connect = 0;
 }
 
 if (!p->location)
 {
  if (!room_player_rand_main_transfer(p, ROOM_TRANS_LOGON))
    shutdown_error(" Couldn't find system.void.\n");
  
  assert(p->location);
 }
 
 if (MODE_CURRENT(p).prompt)
   prompt_update(p, MODE_CURRENT(p).prompt);
 
 fvtell_player(LOGON_FT(HILIGHT, p), "\n ~Welcome to %s %s!!~\n^N\n",
               configure.name_long, p->saved->name);
 
 if (TIMER_IS_ACTIVE(&glob_timer, shutdown))
 {
  char buf[256];
  
  fvtell_player(SYSTEM_FT(HILIGHT, p),
                " -=> $Talker-Name is currently shutting down in %s.\n", 
                word_time_long(buf, sizeof(buf),
                               TIMER_TOGO(&glob_timer, shutdown),
                               WORD_TIME_DEFAULT));
  
  fvtell_player(SYSTEM_FT(RAW_OUTPUT | HILIGHT, p), " -=> Started by: %s\n",
                shutdown_player);
  fvtell_player(SYSTEM_FT(RAW_OUTPUT | HILIGHT, p), " -=> Reason: %s\n",
                shutdown_msg);
 }
 
 if (p->flag_married && !*p->married_to)
 {
  fvtell_player(NORMAL_FT(HILIGHT, p), "%s", 
                " -=> Since your last logon, you have been divorced.\n"
                " This maybe due to your partners character no longer being "
                "on the program or due to an official divorce.\n"
                " Please consult a net minister for more information.\n");
  p->flag_married = FALSE;
 }
 else if (!strcmp(p->married_to, "1decline"))
 {
  fvtell_player(NORMAL_T(p), "%s", 
                " -=> Since your last logon, the last proposal of marriage "
                "you made has been declined.\n");
  *p->married_to = 0;
 }
  
 autoexec_commands(p);
}

static void logon_newbie_command_6(player *p, const char *str)
{
 ICTRACE("logon_newbie_command_6");

 assert(MODE_IN_MODE(p, LOGON_NEWBIE_6)); 
 assert(p->termcap);
 
 if (!beg_strcasecmp(str, "yes"))
 {
  p->flag_terminal_ansi_colour_override = TRUE;
  terminal_setup(p, p->termcap->name);
 }
 else if (!beg_strcasecmp(str, "no"))
 {
  p->flag_terminal_ansi_colour_override = FALSE;
 }
 else
   return; /* follow instructions */
   
 mode_del(p);

 logon_program(p);
}

static void logon_newbie_command_5(player *p, const char *str)
{
 cmds_function tmp_cmd;
 
 CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, logon_newbie_command_6);

 ICTRACE("logon_newbie_command_5");

 assert(MODE_IN_MODE(p, LOGON_NEWBIE_5)); 

 if (!*str)
   str = "ansi";
 
 mode_del(p);

 terminal_setup(p, str);

 if (p->termcap && (!p->termcap->Sf || !p->termcap->Sb))
 {
  CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, logon_newbie_command_6);
  
  fvtell_player(SYSTEM_WFT(0, p),
                "\n\n Your chosen terminal type of '^S^B%s^s' "
                "doesn't have colour information associated with it.\n"
               " Because many terminals don't provide this information we "
                "provide the ability to ^S^Boverride^s this with ^S^BANSI^s "
                "colours.\n"
                " If you which to set this just type ^S^Byes^s.\n\n",
                p->termcap->name);
  
  mode_add(p, "Ansi override, Y(es) or N(o): ",
           MODE_ID_LOGON_NEWBIE_6, 0, &tmp_cmd, NULL, NULL);
 }
 else
   logon_program(p);
}

static void logon_newbie_command_4(player *p, const char *str)
{
 cmds_function tmp_cmd;
 
 ICTRACE("logon_newbie_command_4");

 assert(MODE_IN_MODE(p, LOGON_NEWBIE_4));
 
 if (!beg_strcasecmp(str, "male"))
 {
  p->gender = GENDER_MALE;
  fvtell_player(LOGON_T(p), "%s", " Gender set to Male.\n");
 }
 else if (!beg_strcasecmp(str, "female"))
 {
  p->gender = GENDER_FEMALE;
  fvtell_player(LOGON_T(p), "%s", " Gender set to Female.\n");
 }
 else if (!beg_strcasecmp(str, "plural"))
 {
  p->gender = GENDER_PLURAL;
  fvtell_player(LOGON_T(p), "%s", " Gender set to Plural.\n");
  
  CONST_COPY_STR(p->title, "the newbies, so treat us nicely.",
                 PLAYER_S_TITLE_SZ);
  CONST_COPY_STR(p->description,
                 "Isn't it time we wrote our own descriptions ?",
                 PLAYER_S_DESCRIPTION_SZ);
  CONST_COPY_STR(p->plan, "We must write ourselves a proper plan sometime ...",
                 PLAYER_S_PLAN_SZ);
  CONST_COPY_STR(p->enter_msg, "enter in a standard kind of way.",
                 PLAYER_S_ENTER_MSG_SZ);
 }
 else if (!beg_strcasecmp(str, "none") || !beg_strcasecmp(str, "void") ||
          !beg_strcasecmp(str, "other"))
 {
  p->gender = GENDER_OTHER;
  fvtell_player(LOGON_T(p), "%s",
                " Gender set to well, erm, something.\n");
 }
 else
   return;

 mode_del(p);

 fvtell_player(SYSTEM_T(p), "%s", "\n");

 if (!p->automatic_term_name_got)
 {
  CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, logon_newbie_command_5);

  /* use ansi because vt100 barfs with -Ttelnet on tf */
  fvtell_player(SYSTEM_WFT(0, p), "%s",
                " The talker couldn't automaticaly detect what "
                "terminal type you have so we are guessing at '^S^Bansi^s', "
                "if that isn't the case then just enter your terminal type "
                "at the prompt. If it is or you aren't sure ... just press "
                "return.\n\n");

  mode_add(p, "Please choose a terminal type [ansi]: ",
           MODE_ID_LOGON_NEWBIE_5, 0, &tmp_cmd, NULL, NULL);
 }
 else if (p->termcap && (!p->termcap->Sf || !p->termcap->Sb))
 {
  CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, logon_newbie_command_6);

  fvtell_player(SYSTEM_WFT(0, p), " The talker has automaticaly detected that "
                "you have a terminal of type '^S^B%s^s', but that terminal "
                "doesn't have colour information associated with it.\n"
                " Because many terminals don't provide this information we "
                "provide the ability to ^S^Boverride^s this with ^S^BANSI^s "
                "colours.\n If you which to set this just type ^S^Byes^s.\n\n",
                p->termcap->name);
  
  mode_add(p, "Ansi colour override, Y(es) or N(o): ",
           MODE_ID_LOGON_NEWBIE_6, 0, &tmp_cmd, NULL, NULL);
 }
 else
   logon_program(p);
}

static void logon_newbie_command_3(player *p, const char *str)
{
 ICTRACE("logon_newbie_command_3");
 
 assert(MODE_IN_MODE(p, LOGON_NEWBIE_3));

 if (!beg_strcasecmp(str, "yes"))
 {
  cmds_function tmp_cmd;
  
  CMDS_FUNC_TYPE_CONST_CHARS((&tmp_cmd), logon_newbie_command_4);
  
  mode_del(p);
  
  fvtell_player(LOGON_T(p), "%s",
                "\n\n The program requires that you enter your gender.\n"
                " This is used solely for the purposes "
                "of correct english and grammer.\n"
                " If you object to this, then simply "
                "type 'n' for none or not applicable.\n\n");
  
  mode_add(p, "Please choose M(ale), F(emale), P(lural) or N(one): ",
           MODE_ID_LOGON_NEWBIE_4, 0, &tmp_cmd, NULL, NULL);
 }
 else if (!beg_strcasecmp(str, "no"))
 {
  /* this is changed because to do it any other way is pretty hard.
   * you'll have to virtualy quit them and then send them back to
   * the begining  */
  user_logoff(p, NULL);
  fvtell_player(LOGON_T(p), "%s", 
                " Ok, then, please logon again and enter a different name.\n");
 }
}

#if 0 /* skipped ... */
static void logon_newbie_command_2(player *p, const char *str)
{
 cmds_function tmp_cmd;
 
 CMDS_FUNC_TYPE_CONST_CHARS((&tmp_cmd), logon_newbie_command_3);

 ICTRACE("logon_newbie_command_2");
 
 assert(p->saved);
 assert(MODE_IN_MODE(p, LOGON_NEWBIE_2) || MODE_IN_MODE(p, LOGON_NEWBIE_3));
 
 IGNORE_PARAMETER(str);

 mode_del(p);
 
 fvtell_player(LOGON_T(p),
               "\n\n You entered the name '%s' when you first logged in.\n"
               " Is this the name that you wish to be known as on "
               "the program ?\n\n", p->saved->name);

 mode_add(p, "Use name, Y(es) or N(o): ",
          MODE_ID_LOGON_NEWBIE_3, 0, &tmp_cmd, NULL, NULL);
}
#endif

static void logon_newbie_command_1(player *p, const char *str)
{
 cmds_function tmp_cmd;
 
 CMDS_FUNC_TYPE_CONST_CHARS((&tmp_cmd), logon_newbie_command_3);
 
 ICTRACE("logon_newbie_command_1");

 assert(MODE_IN_MODE(p, LOGON_NEWBIE_1));
 
 IGNORE_PARAMETER(str);

 mode_del(p);

 fvtell_player(LOGON_T(p),
               "\n\n You entered the name '%s' when you first logged in.\n"
               " Is this the name that you wish to be known as on "
               "the program ?\n\n", p->saved->name);

 mode_add(p, "Use name, Y(es) or N(o): ",
          MODE_ID_LOGON_NEWBIE_3, 0, &tmp_cmd, NULL, NULL);
}

static void internal_end_got_password(player *p, const char *str)
{
 telopt_ask_passwd_mode_off(p);

 if (!p->flag_raw_passwd)
 {
  COPY_STR(p->saved->player_ptr->passwd, str, PLAYER_S_PASSWD_SZ);
  
  p->saved->player_ptr->flag_raw_passwd = TRUE;
 }
 
 logon_program(p);
}

static int internal_logon_extern_email_passwd(FILE *fp, void *passed_p)
{
 player *p = passed_p;
 int ret = 0;
 
 p->emailed_passwd_timestamp = now;
 p->saved->flag_tmp_player_needs_saving = TRUE;
 
 ret += fprintf(fp, " Your username is: %s\n", p->saved->name);
 ret += fprintf(fp, " Your password is: %s\n", p->passwd);
 ret += fprintf(fp, " You last logged in at: %s\n",
                DISP_TIME_P_STD(p->saved->logoff_timestamp, p));
 ret += fprintf(fp, "              from  ip: %d.%d.%d.%d\n",
                (int)p->saved->last_ip_address[0],
                (int)p->saved->last_ip_address[1],
                (int)p->saved->last_ip_address[2],
                (int)p->saved->last_ip_address[3]);
 ret += fprintf(fp, "              from dns: %s\n",
                p->saved->last_dns_address);

 ret += fprintf(fp, "\n\n");

 ret += fprintf(fp, "%s\n", "-- ");
 ret += fprintf(fp, "%s\n", configure.email_from_long);
 ret += fprintf(fp, "%s\n", configure.url_access);
 ret += fprintf(fp, "%s\n", configure.url_web);

 return (ret);
}

static void logon_password_command_4(player *p, const char *str)
{
 ICTRACE("logon_password_command_4");

 assert(MODE_IN_MODE(p, LOGON_PASSWD_4));

 if (!beg_strcasecmp(str, "yes"))
 {
  mode_del(p);

  fvtell_player(LOGON_T(p), "%s", " Ok, emailing your password to your "
                "email address now.\n");


  {
   char subj[sizeof("Your password for %s on %s") +
            CONFIGURE_NAME_SZ + PLAYER_S_NAME_SZ];
   email_info ei = EMAIL_INFO_INIT();
   
   sprintf(subj, "Your password for %s on %s",
           configure.name_long, p->saved->name);
   
   ei.to = p->saved->player_ptr->email;
   ei.subject = subj;
   ei.func = internal_logon_extern_email_passwd;
   ei.param = p->saved->player_ptr;
   
   email_generic(&ei);
  }
  
  p->saved = NULL;
  user_logoff(p, NULL);
 }
 else if (!beg_strcasecmp(str, "no"))
 {
  mode_del(p);

  fvtell_player(LOGON_T(p), "%s", " Ok, see you again.\n");

  p->saved = NULL;
  user_logoff(p, NULL);
 }
 else
  fvtell_player(LOGON_T(p), "%s", " Please answer 'yes' or 'no'.\n");
}

static void logon_password_command_3(player *p, const char *str)
{
 ICTRACE("logon_password_command_3");

 assert(MODE_IN_MODE(p, LOGON_PASSWD_3));
 mode_del(p);

 if (!passwd_check(p, str))
 {
  if (PRIV_STAFF(p->saved))
    vwlog("su_connection", "Password fail: %s %d.%d.%d.%d:%s", p->saved->name,
          (int)p->ip_address[0], (int)p->ip_address[1],
          (int)p->ip_address[2], (int)p->ip_address[3], p->dns_address);
  else
    vwlog("connection", "Password fail: %s %d.%d.%d.%d:%s", p->saved->name,
          (int)p->ip_address[0], (int)p->ip_address[1],
          (int)p->ip_address[2], (int)p->ip_address[3], p->dns_address);  
  
  fvtell_player(LOGON_T(p), "%s", "\n\n"
                " Wrong password, you've failed to logon.\n\n");
  
  if (p->saved->player_ptr->flag_raw_passwd && p->saved->player_ptr->email[0])
  {
   if (difftime(now, p->saved->player_ptr->emailed_passwd_timestamp) >
       MK_DAYS(1))
   {
    cmds_function tmp_cmd;
    
    CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, logon_password_command_4);

    telopt_ask_passwd_mode_off(p);
    
    fvtell_player(LOGON_T(p), "%s",
                  " Would you like us to send your password "
                 "to your email address?\n\n");
    mode_add(p, "Send password: ", MODE_ID_LOGON_PASSWD_4, 0,
             &tmp_cmd, NULL, NULL);
    return;
   }
   else
    fvtell_player(LOGON_T(p),
                  " We would normally allow you to send your password to "
                  "your email address, but we have already done that within "
                  "the last ^S^B%s^s so you'll have to wait a bit.\n",
                  "24 hours");
  }

  p->saved = NULL;
  user_logoff(p, NULL);
  return;
 }

 internal_end_got_password(p, str);
}

static void logon_password_command_2(player *p, const char *str)
{
 ICTRACE("logon_password_command_2"); 

 assert(MODE_IN_MODE(p, LOGON_PASSWD_2));
 mode_del(p);

 if (!passwd_check(p, str))
 {
  cmds_function tmp_cmd;
  
  CMDS_FUNC_TYPE_CONST_CHARS((&tmp_cmd), logon_password_command_3);

  fvtell_player(LOGON_T(p), "%s",
                "\n Wrong password ... try for the last time.\n\n");
  mode_add(p, "Please enter your password: ", MODE_ID_LOGON_PASSWD_3, 0,
           &tmp_cmd, NULL, NULL);
  return;
 }
 
 internal_end_got_password(p, str);
}

static void logon_password_command_1(player *p, const char *str)
{
 ICTRACE("logon_password_command_1");
 
 assert(MODE_IN_MODE(p, LOGON_PASSWD_1));
 mode_del(p);
 
 if (!passwd_check(p, str))
 {
  cmds_function tmp_cmd;
  
  CMDS_FUNC_TYPE_CONST_CHARS((&tmp_cmd), logon_password_command_2);

  fvtell_player(LOGON_T(p), "%s",
                "\n Wrong password ... try again.\n\n");
  mode_add(p, "Please enter your password: ", MODE_ID_LOGON_PASSWD_2, 0,
           &tmp_cmd, NULL, NULL);
  return;
 }

 internal_end_got_password(p, str);
}

static void logon_name_command(player *p, const char *str, size_t length)
{
 player_tree_node *sp = NULL;
 char players_name[PLAYER_S_NAME_SZ];
 const char *to = C_strchr(str, ' ');
 cmds_function tmp_cmd;
   
 ICTRACE("logon_name_command");

 if (to)
   length = to - str;
 
 if (length >= (PLAYER_S_NAME_SZ - 1))
 {
  fvtell_player(LOGON_T(p), "%s",
                "\n Sorry, that name is too long, please enter something "
                "shorter.\n\n");
  return;
 }
 else if (length < 2)
 {
  fvtell_player(LOGON_T(p), "%s", 
                " Thats a bit short, try something longer.\n\n");
  return;
 }

 COPY_STR_LEN(players_name, str, length);

 lower_case(players_name);
 
 if (!BEG_CONST_STRCMP("who", players_name))
 {
  user_short_who(p, NULL);
  return;
 }
 else if (!BEG_CONST_STRCMP("quit", players_name))
 {
  user_logoff(p, NULL);
  return;
 }
 else if (!BEG_CONST_STRCMP("uptime", players_name))
 {
  user_uptime(p, NULL);
  return;
 }
 
 if (to)
   to += strspn(to, " "); /* most people don't want it */

 if (*(players_name + strspn(players_name, ALPHABET_LOWER)))
 {
  fvtell_player(LOGON_T(p), "%s", 
                "\n Sorry you can only have alphabetic characters "
                "in a name.\n");
  return;
 }
 
 if ((!(sp = player_find_all(NULL, players_name, PLAYER_FIND_DEFAULT)) &&
      find_player_cronlist_exact(players_name)) ||
     (sp && PRIV_SYSTEM_ROOM(sp)))
 {
  fvtell_player(LOGON_T(p), "%s", 
                "\n Sorry there is already someone on the program "
                "with that name.\nPlease try again, but use a "
                "different name.\n\n");
  return;
 }

 if (logon_player_load_make(p, players_name, to))
 { /* resident */
  if (!auth_check_player(p))
  {
   user_logoff(p, NULL);
   return;
  }
  
  mode_del(p);
  assert(MODE_INVALID(p));

  if (p->saved->player_ptr->passwd[0])
  {
   CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, logon_password_command_1);

   fvtell_player(LOGON_FT(RAW_OUTPUT, p), 
                 "\n Password needed for '%s'\n\n", p->saved->name);
  
   telopt_ask_passwd_mode_on(p);

   assert(MODE_INVALID(p));
   mode_add(p, "Please enter your password: ", MODE_ID_LOGON_PASSWD_1, 0,
            &tmp_cmd, NULL, NULL);
  }
  else
  { /* FIMXE: enforce passwd setting */
   fvtell_player(LOGON_T(p), "%s",
                 "\n You have no password !!!\n"
                 " Please set one as soon as possible with the "
                 "'^S^Bpassword^s' command.\n");
   logon_program(p);
  }
 }
 else
 { /* newbie */
  struct timeval tv;
  
  if (!auth_check_player(p))
  {
   user_logoff(p, NULL);
   return;
  }

  mode_del(p);
  assert(MODE_INVALID(p));
  
  CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, logon_newbie_command_1);
  
  fvtell_player(LOGON_T(p), "%s", msg_newbie_start.text);
  
  mode_add(p, "Hit return to continue: ",
           MODE_ID_LOGON_NEWBIE_1, 0, &tmp_cmd, NULL, NULL);
  
  timer_q_del_data(&logon_timer_queue, p);

  gettimeofday(&tv, NULL);

  TIMER_Q_TIMEVAL_ADD_MINS(&tv, 10, 0);

  timer_q_add_node(&logon_timer_queue, p, &tv, TIMER_Q_FLAG_NODE_DEFAULT);
 }
}

static void logon_start_real(player *p)
{
 cmds_function tmp_cmd;
 struct timeval tv;
 
 CMDS_FUNC_TYPE_CHARS_SIZE_T(&tmp_cmd, logon_name_command);

 p->allow_run_commands = TRUE;
 
 fvtell_player(LOGON_FT(SYSTEM_INFO, p), "%s", msg_connect.text);
 
 assert(MODE_INVALID(p));
 
 mode_add(p, "Please enter your name: ", MODE_ID_LOGON_NAME, 0,
          &tmp_cmd, NULL, NULL);
 /* alas we need to run a command before prompts auto work */
 prompt_update(p, "Please enter your name: ");
 
 gettimeofday(&tv, NULL);
 
 TIMER_Q_TIMEVAL_ADD_MINS(&tv, 4, 0);
 
 timer_q_add_node(&logon_timer_queue, p, &tv, TIMER_Q_FLAG_NODE_DEFAULT);
}

static void timed_logon(int timed_type, void *passed_player)
{
 player *p = passed_player;

 TCTRACE("timed_logon");

 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;
 
 logon_start_real(p);
}

void logon_shortcut_logon_start(player *p)
{
 if (!p->allow_run_commands &&
     p->output_compress_do && p->automatic_term_name_got &&
     *(p->dns_address + strspn(p->dns_address, "0123456789.")))
 {
  timer_q_del_data(&logon_timer_queue, p);
  logon_start_real(p);
 }
}

void logon_start(player *p)
{
 Timer_q_node *current_timer = NULL;
 struct timeval tv;
 
 telopt_ask_end_or_record(p);
 telopt_ask_term_sizes(p);
 telopt_ask_term_type(p);
 telopt_ask_chars_mode(p);
 telopt_ask_compress_do(p);
 
 assert(MODE_INVALID(p));
 
 gettimeofday(&tv, NULL);
 
 TIMER_Q_TIMEVAL_ADD_SECS(&tv, LOGON_INIT_TIMEOUT, 0);
 
 if ((current_timer = timer_q_add_node(&logon_timer_queue, p, &tv,
                                       TIMER_Q_FLAG_NODE_DOUBLE |
                                       TIMER_Q_FLAG_NODE_FUNC)))
   timer_q_cntl_node(current_timer, TIMER_Q_CNTL_NODE_SET_FUNC, timed_logon);
 else
   logon_start_real(p);
}

void init_logon(void)
{
 timer_q_add_static_base(&logon_timer_queue, timed_logout,
                         TIMER_Q_FLAG_BASE_INSERT_FROM_END |
                         TIMER_Q_FLAG_BASE_MALLOC_DOUBLE |
                         TIMER_Q_FLAG_BASE_MOVE_WHEN_EMPTY);
}
