#define SPODLIST_C
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

/* Local Variables */
static player_tree_node *number_anchor = NULL;
static player_tree_node *number_footer = NULL;


/* Start of functions */

static void tell_spod_info(player_tree_node *from, player *p,
                           twinkle_info *info, int flags, time_t timestamp,
                           player_tree_node *entry, int show_star)
{
 int the_logon_time = real_total_logon_time(entry);
 char *used_name = entry->lower_name;
 char buf[256];
 const char *logon_stuff = "Hmm, this logon time seems to be missing.";
 
 if (P_IS_ON(entry))
   used_name = entry->name;
 
 if (p && (p->saved->priv_admin || (p == entry->player_ptr) ||
	   !entry->flag_hide_logon_time))
   logon_stuff = word_time_long(buf, sizeof(buf),
                                the_logon_time, WORD_TIME_DEFAULT);
 
 if (entry->priv_spod && p && p->saved->priv_spod)
   fvtell_player(ALL_T(from, p, info, flags, timestamp),
                 "%c%-4d %-22s(S)  %s\n",
                 show_star ? '*' : ' ',
                 (entry->spod_number - entry->spod_offset),
                 used_name, logon_stuff);
 else
   fvtell_player(ALL_T(from, p, info, flags, timestamp),
                 "%c%-4d %-22s     %s\n",
                 show_star ? '*' : ' ',
                 (entry->spod_number - entry->spod_offset),
                 used_name, logon_stuff);
}

static void inorder_add_to_spodlist(player_tree_node *current,
                                    va_list va __attribute__ ((unused)))
{
 if (current->priv_banished || PRIV_SYSTEM_ROOM(current))
   return;

 /* Call addin, put the return in lastone to send next loop */
 spodlist_addin_player(current);
}

void init_spodlist(void)
{ 
 do_inorder_all(inorder_add_to_spodlist);
}

void spodlist_addin_player(player_tree_node *temp)
{
 player_tree_node *sort_num = NULL;

 /* NOTE: logon time is based on sp->total_logon ONLY
  * this might need to be `fixed' as there will be a lurch
  * if there are a lot of people on who are changing position
  * this will only happen once though */
  
 if (!(sort_num = number_anchor))
   /* if no elements then do first seperatly */
 {
  temp->next_num = NULL;
  temp->prev_num = NULL;
  
  temp->spod_number = 1;
  temp->spod_offset = 0;
  
  number_anchor = temp;
  number_footer = temp;
  return;
 }
 
 while ((sort_num->next_num) &&
        ((sort_num->total_logon - sort_num->total_idle_logon)
         >= (temp->total_logon - temp->total_idle_logon)))
   sort_num = sort_num->next_num;
 
 if ((temp->total_logon - temp->total_idle_logon) <=
     (sort_num->total_logon - sort_num->total_idle_logon))
 {
  if ((temp->next_num = sort_num->next_num))
    temp->next_num->prev_num = temp;
  else
    number_footer = temp;
  
  temp->prev_num = sort_num;
  sort_num->next_num = temp;
  
  temp->spod_number = sort_num->spod_number;
  temp->spod_offset = sort_num->spod_offset;  

  sort_num = temp;
 }
 else
 {
  if ((temp->prev_num = sort_num->prev_num))
    temp->prev_num->next_num = temp;
  else
    number_anchor = temp;
  
  temp->next_num = sort_num;
  sort_num->prev_num = temp;

  temp->spod_number = sort_num->spod_number;
  temp->spod_offset = sort_num->spod_offset;
 }
  
 /* Now incrememnt the numbers below it and the offsets if applicable */
 while (sort_num)
 {
  sort_num->spod_number++;
  if (temp->flag_kill_logon_time)
    sort_num->spod_offset++;
/*  number_footer = sort_num; */
  sort_num = sort_num->next_num;
 }
}

static void spodlist_update_offsets(player_tree_node *entry, int number_to_add)
{
 while ((entry = entry->next_num))
   entry->spod_offset += number_to_add;
}       
      
static void user_spodlist_show_top_spods(player *p) /* lsp */
{
 int count = 0;
 player_tree_node *scan = number_anchor;
 
 ptell_mid(NORMAL_T(p), "Top 20 Spods", FALSE);

 /* shows less than you have if you have less than 20 resis */
 while ((count < 20) && scan->next_num)
 {
  if (scan->flag_kill_logon_time)
    count--;
  else
    tell_spod_info(NORMAL_T(p), scan, FALSE);
  
  ++count;
  scan = scan->next_num;
 }
 
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void user_su_spodlist_fix(player *p)
{
 number_anchor = NULL;
 number_footer = NULL;

 do_inorder_all(inorder_add_to_spodlist); 
 fvtell_player(NORMAL_T(p), "%s", "All done.\n");
}

void spodlist_remove_player(const char *name)
{
 char lowered_name[PLAYER_S_NAME_SZ];
 player_tree_node *to_remove, *scan;
 
 COPY_STR(lowered_name, name, PLAYER_S_NAME_SZ);

 lower_case(lowered_name);
 
 if (!(to_remove = player_tree_find_exact(lowered_name))) /* opto */
   return;
 
 if (to_remove->prev_num)
   to_remove->prev_num->next_num = to_remove->next_num;
 else
   number_anchor = to_remove->next_num;
 
 if (to_remove->next_num)
   to_remove->next_num->prev_num = to_remove->prev_num;
 else
   number_footer = to_remove->prev_num;    
 
 scan = to_remove->next_num;
 while (scan)
 {
  scan->spod_number--;
  if (to_remove->flag_kill_logon_time)
    scan->spod_offset--;
  scan = scan->next_num;
 }

 to_remove->next_num = to_remove->prev_num = NULL;
}

static player_tree_node *find_player_spod_number(unsigned int num)
{
 player_tree_node *scan;

 if (num > (unsigned) no_of_resis)
   return (0);
 
 if (num < (unsigned) (no_of_resis >> 1))
 {
  scan = number_anchor;
  while (scan && (scan->spod_number != num))
    scan = scan->next_num;
 } 
 else
 {
  scan = number_footer;
  while (scan && (scan->spod_number != num))
    scan = scan->prev_num;
 }
 
 if (scan && (scan->spod_offset || scan->flag_kill_logon_time))
 {
  int count = scan->spod_offset;

  /* while we havn't reached the true number yet */
  while (scan && (scan->flag_kill_logon_time || (count > 0)))
  {
   if (scan->flag_kill_logon_time)
     ++count;
   
   scan = scan->next_num;
   --count;
  }
 }
 
 return (scan);
}

static int construct_spodlist_name_list_do(player *scan, va_list ap)
{
 unsigned int *count = va_arg(ap, unsigned int *);
 /* params */
 player_tree_node *from = va_arg(ap, player_tree_node *);
 player *to = va_arg(ap, player *);
 twinkle_info *info = va_arg(ap, twinkle_info *);
 int flags = va_arg(ap, int);
 time_t my_now = va_arg(ap, time_t);

 if (scan->saved->flag_kill_logon_time)
 {
  fvtell_player(SYSTEM_T(scan),
                " The player - ^S^B%s^s - has removed themself "
                "from the spod list.\n", scan->saved->name);
  return (TRUE);
 }

 if (!scan->saved->spod_number)
   return (TRUE);
 
 ++*count;
 
 spodlist_check_order(scan->saved, SPOD_CHECK_EXTERNAL, SPOD_CHECK_DEF_LEVEL);
 tell_spod_info(ALL_T(from, to, info, flags, my_now), scan->saved, FALSE);

 return (TRUE);
}

static void user_spodlist_display(player *p, parameter_holder *params)
{
 player_tree_node *spod_to_show = NULL;
 char *start = NULL;
 char *end = NULL;
 int count = 0;
 
 if (params->last_param != 1)
   TELL_FORMAT(p, "<name|number|\"who\">,[name|number|\"who\"]");

 lower_case((end = GET_PARAMETER_STR(params, 1)));
 while (end && (count < 20))
 {
  start = end;
  
  if ((end = next_parameter(start, ',')))
    *end++ = 0;
  
  count++;
      
  if (!*(start + strspn(start, "1234567890")))
  {
   if (!(spod_to_show = find_player_spod_number(atoi(start))))
     fvtell_player(SYSTEM_T(p), " There is no such spodlist number "
                   "-- ^S^B%d^s --.\n", atoi(start));
  }
  else if (!strcasecmp(start, "who"))
  {
   DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
                 (construct_spodlist_name_list_do, &count, NORMAL_T(p)));
  }
  else
  {
   if ((spod_to_show = player_find_all(p, start, PLAYER_FIND_SC_EXTERN)))
     if (spod_to_show->flag_kill_logon_time)
     {
      fvtell_player(SYSTEM_T(p),
                    " The player - ^S^B%s^s - has removed themself "
                    "from the spod list.\n", spod_to_show->name);
      spod_to_show = NULL;
      continue;
     }
  }

  log_assert(!spod_to_show || spod_to_show->spod_number); /* this fails */
  if (spod_to_show && spod_to_show->spod_number)
  {
   spodlist_check_order(spod_to_show, SPOD_CHECK_EXTERNAL,
                        SPOD_CHECK_DEF_LEVEL);
   tell_spod_info(NORMAL_T(p), spod_to_show, FALSE);
  }
 }
}

void spodlist_check_order(player_tree_node *spod_entry, unsigned int recurse,
                          int recurse_level)
{
 player_tree_node *swap = NULL;
 int total_logon_time = real_total_logon_time(spod_entry);

 log_assert(spod_entry);
 log_assert(recurse);
 log_assert(recurse_level >= 0);

 if (!spod_entry)
   return;

 /* try going up the spodlist */
 while ((swap = spod_entry->prev_num))
 {
  int swap_total_logon_time = real_total_logon_time(swap);
  
  if (P_IS_ON(swap))
  {
   if ((recurse != SPOD_CHECK_RECURSE_DOWN) && (recurse_level > 0))
     spodlist_check_order(swap, SPOD_CHECK_RECURSE_UP, recurse_level - 1);
   
   if (swap != spod_entry->prev_num)
     continue;
  }
      
  if (swap_total_logon_time >= total_logon_time)
    break;

  spod_entry->prev_num = swap->prev_num;
  swap->next_num = spod_entry->next_num;
  swap->prev_num = spod_entry;
  spod_entry->next_num = swap;
  
  swap->spod_number++;
  spod_entry->spod_number--;
  
  if (spod_entry->flag_kill_logon_time)
    swap->spod_offset++;
  
  if (swap->flag_kill_logon_time)
    spod_entry->spod_offset--;
  
  if (!spod_entry->prev_num)
    number_anchor = spod_entry;
  else
    spod_entry->prev_num->next_num = spod_entry;
  
  if (!swap->next_num)
    number_footer = swap;
  else
    swap->next_num->prev_num = swap;
 }

 /* try going in the other direction */
 while ((swap = spod_entry->next_num))
 {
  int swap_total_logon_time = real_total_logon_time(swap);
  
  if (P_IS_ON(swap))
  {
   if ((recurse != SPOD_CHECK_RECURSE_UP) && (recurse_level > 0))
     spodlist_check_order(swap, SPOD_CHECK_RECURSE_DOWN, recurse_level - 1);
   
   if (swap != spod_entry->next_num)
     continue;
  }

  if (swap_total_logon_time <= total_logon_time)
    break;
  
  spod_entry->next_num = swap->next_num;
  swap->prev_num = spod_entry->prev_num;
  swap->next_num = spod_entry;
  spod_entry->prev_num = swap;
  
  swap->spod_number--;
  spod_entry->spod_number++;
  
  if (spod_entry->flag_kill_logon_time)
    swap->spod_offset--;
  
  if (swap->flag_kill_logon_time)
    spod_entry->spod_offset++;
  
  if (!spod_entry->next_num)
    number_footer = spod_entry;
  else
    spod_entry->next_num->prev_num = spod_entry;
  
  if (!swap->prev_num)
    number_anchor = swap;
  else
    swap->prev_num->next_num = swap;
 }
}

static void user_su_spodlist_delete_person(player *p, const char *str)
{
 if (!*str)
   TELL_FORMAT(p, "<name>");

 spodlist_remove_player(str);
 fvtell_player(NORMAL_T(p), "%s", " Ok, all done.\n");
}

static void user_su_spodlist_dump_to_file_number(player *p, const char *str)
{
 IGNORE_PARAMETER(p && str);
#if 0 /* FIXME: stack and very broken */
 char *oldstack, *mark;
 int fd, length_file, admin = 0, html = 0, setspod = 0, setadmin = 0;
 int  backing = 0;
 int tmp = 0;
 player_tree_node *search;

 oldstack = stack;
 
 sprintf(stack, "files/spodlist.txt");

 if ((fd = open(stack, O_CREAT | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR)) == -1)
   return;
 
 stack = oldstack;
 
 if (!p)
 {
  backing = 1;
  html = 1;
 }
 else
 {
  raw_wall(FALSE,
           "\n -=> Program pausing shortly for administration purposes.\n\n");
  
  /* output below */  
  if (*str)
    if (!(strcasecmp("admin", str)))
    {
     if (p->saved->priv_admin)
       admin = 1;
    }
    else
      if (!(strcasecmp("html", str)))
	html = 1;
  
  if (!admin)
  {
   if (p->saved->priv_admin)
   {
    /* FIXME: This is _SUCH_ a hack... and we don't really use it anyway */
    p->residency &= ~ADMIN;
    setadmin = 1;
   }
  }
  else
    fvtell_player(NORMAL_T(p), "%s", " Working in admin mode...\n\n");

  if ((p->residency & SPOD) && !(admin))
  {
   p->residency &= ~SPOD;
   setspod = 1;
  }
 }
  
 if (html)
 {
  if (!backing && p)
    fvtell_player(NORMAL_T(p), "%s", " Formatting in html mode...\n\n");
  length_file = lseek(fd, 0, SEEK_END);
  sprintf(stack, "<html><head><title>CrazyLands Spod List</title></head>\n"
	  "<body bgcolor=#FFFFFF text=\"000000\">\n"
	  "<h1><img src=\"cltoucth.gif\" align=bottom alt=\"CrazyLands\">"
	  "\nTop Spods!</h1> updated on %s. <hr><pre>\n\n",
          disp_time_std(now, 0, TRUE, TRUE));
  tmp = strlen(stack);
  if (write(fd, stack, tmp) != tmp)
  {
   close(fd);
   stack = oldstack;
   return;
  }
  stack = oldstack;
 }

 if (!backing && p)
   output_sockets(); /* output here so the person gets the tells */

 search = number_anchor;
 
 while (search->next_num)
   if (!(search->spod_info & SPOD_KILL_LOGON) || admin)
   {
    if ((length_file = lseek(fd, 0, SEEK_END)) == -1)
    {
     close(fd);
     stack = oldstack;
     return;
    }
      
    mark = stack;
    stack_print_spod(search, p);
    stack = end_string(stack);
    lower_case(mark);
        
    tmp = strlen(oldstack);
    if (write(fd, oldstack, tmp) != tmp)
    {
     close(fd);
     stack = oldstack;
     return;
    }
    stack = oldstack;
    search = search->next_num;
   }
   else
     search = search->next_num;
  
 if (html)
 {
  length_file = lseek(fd, 0, SEEK_END);
  sprintf(stack, "\n</pre></body></html>\n");

  tmp = strlen(oldstack);
  if (write(fd, oldstack, tmp) != tmp)
  {
   close(fd);
   stack = oldstack;
   return;
  }
  stack = oldstack;
 }
 
 if (setspod && p)
   p->residency |= SPOD;
  
 if (setadmin && p)
   p->residency |= ADMIN;
 
 if (!backing)
   raw_wall(FALSE, " -=> All done. Thankyou.\n\n");

 stack = oldstack;
 close(fd);
#endif
}

/* Spod area command, displays those 10 above you and 9 bellow you. */
static void user_spodlist_spodarea(player *p, const char *str)
{
 player_tree_node *spod_to_show = NULL;
 player_tree_node *from = NULL;
 player_tree_node *top = NULL;
 int count = 0;

 if (!*str)
   str = p->saved->name;
 
 if (!*(str + strspn(str, "1234567890")))
 {
  if (!(spod_to_show = find_player_spod_number(atoi(str))) )
  {
   fvtell_player(SYSTEM_T(p), " There is no such spodlist number "
                 "-- ^S^B%d^s --.\n", atoi(str));
   return;
  }
  
  while (spod_to_show->flag_kill_logon_time && spod_to_show->next_num)
    spod_to_show = spod_to_show->next_num;

  if (spod_to_show->flag_kill_logon_time)
    while (spod_to_show->flag_kill_logon_time && spod_to_show->prev_num)
      spod_to_show = spod_to_show->prev_num;
 }
 else
 {
  if (!(spod_to_show = player_find_all(p, str, PLAYER_FIND_SC_EXTERN)))
    return;
  
  if (spod_to_show->flag_kill_logon_time)
  {
   fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- has removed "
                 "themself from the spod list.\n", spod_to_show->name);
   return;
  }
 }
 
 if (!spod_to_show)
 { /* This is for an unfound spod_to_show (even after all the checks) */
  fvtell_player(SYSTEM_T(p), "%s", " Sorry, an error has occured...\n");
  log_assert(FALSE);
  return;
 }
 
 spodlist_check_order(spod_to_show, SPOD_CHECK_EXTERNAL, SPOD_CHECK_DEF_LEVEL);
 
 /* find the point to start display of spods from */
 if (!(from = find_player_spod_number((spod_to_show->spod_number -
                                       spod_to_show->spod_offset) - 11)))
   from = number_anchor;
 
 if (!(top = from))
 {
  /* This is for an unfound number */
  fvtell_player(SYSTEM_T(p), "%s", " Sorry, an error has occured...\n");
  log_assert(FALSE);
  vwlog("error", " Tried to find number: %d - that didn't exist. "
        "(spodarea)",
        ((spod_to_show->spod_number -
          spod_to_show->spod_offset) - 11));
  return;
 }
 
 /* Start messing with the output stuff */
 if (spod_to_show != p->saved)
 {
  char title[sizeof("Spods around %s") + PLAYER_S_NAME_SZ];
  
  sprintf(title, "Spods around %s", spod_to_show->name);
  ptell_mid(NORMAL_T(p), title, FALSE);
 }
 else
   ptell_mid(NORMAL_T(p), "Spods around you", FALSE);
 
 /* Whack the rest of the display on the stack */
 count = 0;
 while ((count < 20) && from)
 {
  if (!from->flag_kill_logon_time)
  {
   tell_spod_info(NORMAL_T(p), from, (from == spod_to_show));
   ++count;
  }
  
  from = from->next_num;
 }
 
 /* Finish the display and tell it to them */
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void user_toggle_kill_logon(player *p, const char *str)
{
 int old_flag = p->saved->flag_kill_logon_time;
 
 TOGGLE_COMMAND_ON_OFF(p, str, p->saved->flag_kill_logon_time, TRUE,
                       " You are %snot on the spod list.\n",
                       " You are %son the spod list..\n", TRUE);

 if (old_flag != p->saved->flag_kill_logon_time)
 {
  if (p->saved->flag_kill_logon_time)
    spodlist_update_offsets(p->saved, 1);
  else
    spodlist_update_offsets(p->saved, -1);
 }
}

static void user_su_force_kill_logon(player *p, parameter_holder *params)
{
 player *p2 = NULL;
 int old_flag_kill_logon_time = FALSE;
 
 if (params->last_param != 2)
   TELL_FORMAT(p, "<name> on/off");
  
 if (!(p2 = player_find_load(p, GET_PARAMETER_STR(params, 1),
                             PLAYER_FIND_SC_SU)))
   return;

 old_flag_kill_logon_time = p2->saved->flag_kill_logon_time;
 TOGGLE_COMMAND_OFF_ON(p, GET_PARAMETER_STR(params, 2),
                       p->saved->flag_kill_logon_time,
                       TRUE, "%s", "%s", FALSE);
 
 if (p2->saved->priv_base)
   if (old_flag_kill_logon_time && !p2->saved->flag_kill_logon_time)
   {
    p2->saved->flag_kill_logon_time = TRUE;
    spodlist_update_offsets(p2->saved, 1);
    fvtell_player(NORMAL_T(p), " You've now removed %s from the spod list.\n",
		  p2->saved->name);
    p2->saved->flag_tmp_player_needs_saving = TRUE;
   }
   else
     if (!old_flag_kill_logon_time && p2->saved->flag_kill_logon_time)
     {
      p2->saved->flag_kill_logon_time = FALSE;
      spodlist_update_offsets(p2->saved, -1);
      fvtell_player(NORMAL_T(p), " You have put %s back on the spod list.\n",
		    p2->saved->name);
      p2->saved->flag_tmp_player_needs_saving = TRUE;
     }
     else
       if (!p2->saved->flag_kill_logon_time)
         fvtell_player(NORMAL_T(p), "%s",
                       " That person is already off the spodlist.\n");
       else
         fvtell_player(NORMAL_T(p), "%s",
                       " That person is already on the spodlist.\n");
 else 
   fvtell_player(NORMAL_T(p), "%s",
                 " An error has occured. Either non-resi or not on"
                 " spodlist.\n");
}

void cmds_init_spodlist(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("display_spod", user_spodlist_display, PARSE_PARAMS, INFORMATION);
 
 CMDS_ADD("dump_number", user_su_spodlist_dump_to_file_number,
          CONST_CHARS, ADMIN);
 CMDS_PRIV(admin);

 CMDS_ADD("fix_spodlist", user_su_spodlist_fix, NO_CHARS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(admin);

 CMDS_ADD("lspodlist", user_spodlist_show_top_spods, NO_CHARS, INFORMATION);
 
 CMDS_ADD("remove_spod", user_su_spodlist_delete_person, CONST_CHARS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(admin);

 CMDS_ADD("spodarea", user_spodlist_spodarea, CONST_CHARS, INFORMATION);
 CMDS_PRIV(base);

 CMDS_ADD("kill_logon", user_toggle_kill_logon, CONST_CHARS, SPOD);
 CMDS_PRIV(spod);
 CMDS_ADD("force_kill_logon", user_su_force_kill_logon, PARSE_PARAMS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(admin);
}
