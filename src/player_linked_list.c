#define PLAYER_LINKED_LIST_C
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


/* for the logon linked list */
static player_linked_list *alpha_start = NULL;
static player_linked_list *cron_start = NULL;

/* how many ppl are loged on who have privs */
static player_linked_list *logon_staff_start = NULL;
static int prived_logged_on = 0;

/* staff list -- saved players */
static player_linked_list *perm_staff_start = NULL;

/* output list */
static player_linked_list *io_start = NULL;

/* list of spods with spod priv */
static player_linked_list *spod_start = NULL;


/* functions..... */
static player_linked_list **internal_player_l_find(player_linked_list **scan,
                                                   player_tree_node *saved,
                                                   player *on,
                                                   unsigned int flags)
{
 const char *name = NULL;
 
 assert((saved && !(flags & PLAYER_LINK_LOGGEDON)) ||
        (on && (flags & PLAYER_LINK_LOGGEDON)));
 assert(!(flags & PLAYER_LINK_BAD_FLAGS));
 
 if (!scan)
   return (NULL);
 
 if (flags & PLAYER_LINK_NAME_ORDERED)
 { /* has to be able to access name */
  assert(saved || on->saved);
  
  if (flags & PLAYER_LINK_LOGGEDON)
    name = on->saved->lower_name;
  else
    name = saved->lower_name;   
  
  while (*scan && (strcmp(name, PLAYER_LINK_SAV_GET(*scan)->lower_name) > 0))
    scan = &(*scan)->next;
 }
 else
   if (flags & PLAYER_LINK_LOGGEDON)
     while (*scan && !(on == PLAYER_LINK_GET(*scan)))
       scan = &(*scan)->next;
   else
     while (*scan && !(saved == PLAYER_LINK_SAV_GET(*scan)))
       scan = &(*scan)->next;

 if (*scan && (!(flags & PLAYER_LINK_NAME_ORDERED) ||
               !strcmp(name, PLAYER_LINK_SAV_GET(*scan)->lower_name)))
   return (scan);
 else
   return (NULL);
}

player_linked_list *player_link_find(player_linked_list *scan,
                                     player_tree_node *current_saved,
                                     player *current_on, unsigned int flags)
{
 player_linked_list **ret = &scan;

 BTRACE("player_link_find");

 if (!(ret = internal_player_l_find(ret, current_saved, current_on, flags)))
   return (NULL);

 return (*ret);
}

/* magic functions... */
player_linked_list *player_link_add(player_linked_list **start,
                                    player_tree_node *current_saved,
                                    player *current_on,
                                    unsigned int flags,
                                    player_linked_list *passed_add)
{
 player_linked_list *add = NULL;
 const char *name = NULL; 

 BTRACE("player_link_add");
 assert((current_saved && !(flags & PLAYER_LINK_LOGGEDON)) ||
        (current_on && (flags & PLAYER_LINK_LOGGEDON)));
 assert(!player_link_find(*start, current_saved, current_on, flags));
 assert(!(flags & PLAYER_LINK_BAD_FLAGS));

 if (passed_add)
   add = passed_add;
 else
 {
  if (flags & PLAYER_LINK_DOUBLE)
    add = XMALLOC(sizeof(player_linked_list), PLAYER_LINKED_DOUBLE_LIST);
  else
    add = XMALLOC(sizeof(player_linked_list), PLAYER_LINKED_LIST);
  
  if (!add)
  {
   if (flags & PLAYER_LINK_LOGGEDON)  /* couldn't malloc */
     user_logoff(current_on, NULL);
   else
     user_logoff(current_saved->player_ptr, NULL);
   
   return (NULL);
  }
 }

 if (flags & PLAYER_LINK_DOUBLE)
   add->has_prev = TRUE;
 else
   add->has_prev = FALSE;
 
 if (flags & PLAYER_LINK_LOGGEDON)
 {
  add->loggedon = TRUE;
  add->this.loggedon = current_on;
  name = current_on->saved->lower_name;
 }
 else
 {
  add->loggedon = FALSE;
  add->this.saved = current_saved;
  name = current_saved->lower_name;
 }
 
 if (!*start)
 {
  *start = add;
  add->next = NULL;
  if (flags & PLAYER_LINK_DOUBLE)
    ((player_linked_double_list *) add)->prev = NULL;
 }
 else
   if (flags & PLAYER_LINK_NAME_ORDERED)
   {
    player_linked_list *scan = *start;
    
    if (strcmp(PLAYER_LINK_SAV_GET(scan)->lower_name, name) > 0)
    {
     if (flags & PLAYER_LINK_DOUBLE)
       ((player_linked_double_list *) add)->prev = NULL;
     *start = add;
     
     add->next = scan;
     if (scan->has_prev)
     {
      assert(!((player_linked_double_list *) scan)->prev);
      ((player_linked_double_list *) scan)->prev = add;
     }
     
     return (add);
    }

    while (scan->next &&
           (strcmp(PLAYER_LINK_SAV_GET(scan->next)->lower_name, name) < 0))
      scan = scan->next;
    
    assert(!scan->next ||
           (strcmp(PLAYER_LINK_SAV_GET(scan->next)->lower_name, name) > 0));
    assert(strcmp(PLAYER_LINK_SAV_GET(scan)->lower_name, name) < 0);
     
    if (flags & PLAYER_LINK_DOUBLE)
      ((player_linked_double_list *) add)->prev = scan;
    
    add->next = scan->next;
    if (add->next && add->next->has_prev)
      ((player_linked_double_list *) add->next)->prev = add;
    
    scan->next = add;
   }
   else
   {
    assert(*start);
    assert(!(*start)->has_prev ||
           !((player_linked_double_list *) *start)->prev);
    
    if (flags & PLAYER_LINK_DOUBLE)
      ((player_linked_double_list *) add)->prev = NULL;

    if ((*start)->has_prev)
      ((player_linked_double_list *) *start)->prev = add;
    
    add->next = *start;
    *start = add;
   }  
 
 return (add);
}

int player_link_del(player_linked_list **start,
                    player_tree_node *current_saved,
                    player *current_on, unsigned int flags,
                    player_linked_list *passed_rem)
{
 player_linked_list **rem = NULL;
 player_linked_list *to_free = NULL;
 
 BTRACE("player_link_remove");
 assert((current_saved && !(flags & PLAYER_LINK_LOGGEDON)) ||
        (current_on && (flags & PLAYER_LINK_LOGGEDON)));
 assert(!(flags & PLAYER_LINK_BAD_FLAGS));
 assert(start && *start);
 
 if (*start == passed_rem)
 {
  rem = start;
  
  if ((*rem)->next && (*rem)->next->has_prev)
    ((player_linked_double_list *) (*rem)->next)->prev = NULL;
 }
 else
 {
  if (passed_rem && passed_rem->has_prev)
  {
   /* start would have caught the other one */
   assert(((player_linked_double_list *) passed_rem)->prev);
   
   rem = &((player_linked_double_list *) passed_rem)->prev->next;
  }
  else if (!(rem = internal_player_l_find(start, current_saved,
                                          current_on, flags)))
    return (FALSE);
 
  if ((*rem)->next && (*rem)->next->has_prev)
  {
   player_linked_list *prev = NULL;

   /* ANSI magic */
   prev = (player_linked_list *)
     (((char *)rem) - offsetof(player_linked_list, next));
   
   ((player_linked_double_list *) (*rem)->next)->prev = prev;
  }
 }

 to_free = *rem;
 *rem = (*rem)->next;

 if (!passed_rem)
 {
  if (to_free->has_prev)
    XFREE(to_free, PLAYER_LINKED_DOUBLE_LIST);
  else
    XFREE(to_free, PLAYER_LINKED_LIST);
 }
 
 return (TRUE);
}




/* ******************************************************************8
 * Wrapper functions below ... *
 */

 
 /* functions for normal list.... */
void player_list_alpha_add(player_tree_node *current)
{
 assert(current && current->player_ptr);

 player_link_add(&alpha_start, current, NULL,
                 PLAYER_LINK_NAME_ORDERED | PLAYER_LINK_DOUBLE,
                 &current->player_ptr->logon_list_alpha.s);
}

void player_list_alpha_del(player_tree_node *current)
{
 player_link_del(&alpha_start, current, NULL,
                 PLAYER_LINK_NAME_ORDERED | PLAYER_LINK_DOUBLE,
                 &current->player_ptr->logon_list_alpha.s);
}

player_linked_list *player_list_alpha_start(void)
{
 return (alpha_start);
}

void player_list_cron_add(player *current)
{
 assert(current);

 player_link_add(&cron_start, NULL, current,
                 PLAYER_LINK_LOGGEDON | PLAYER_LINK_DOUBLE,
                 &current->logon_list_cron.s);
}

void player_list_cron_del(player *current)
{
 player_link_del(&cron_start, NULL, current,
                 PLAYER_LINK_LOGGEDON | PLAYER_LINK_DOUBLE,
                 &current->logon_list_cron.s);
}

player_linked_list *player_list_cron_start(void)
{
 return (cron_start);
}


 /* functions for prived list.... */
void player_list_logon_staff_add(player_tree_node *current)
{
 if (player_link_add(&logon_staff_start, current, NULL,
                     PLAYER_LINK_NAME_ORDERED, NULL))
   ++prived_logged_on;
}

void player_list_logon_staff_del(player_tree_node *current)
{
 if (player_link_del(&logon_staff_start, current, NULL,
                     PLAYER_LINK_NAME_ORDERED, NULL))
   --prived_logged_on;
}

player_linked_list *player_list_logon_staff_start(void)
{
 return (logon_staff_start);
}

int player_list_logon_staff_number(void)
{
 return (prived_logged_on);
}

void player_list_io_add(player *current)
{
 assert(current);
 
 if (current)
 {
  player_link_add(&io_start, NULL, current,
                  PLAYER_LINK_LOGGEDON | PLAYER_LINK_DOUBLE,
                  &current->logon_list_io.s);
  assert(current->io_indicator);
  if (current->io_indicator)
    SOCKET_POLL_INDICATOR(current->io_indicator)->events |= POLLOUT;
 }
}

void player_list_io_del(player *current)
{
 assert(current);
 
 if (current)
 {
  player_link_del(&io_start, NULL, current,
                  PLAYER_LINK_LOGGEDON | PLAYER_LINK_DOUBLE,
                  &current->logon_list_io.s);
  if (current->io_indicator)
  {
   SOCKET_POLL_INDICATOR(current->io_indicator)->events &= ~POLLOUT;
   SOCKET_POLL_INDICATOR(current->io_indicator)->revents &= ~POLLOUT;
  }
 }
}

player_linked_list *player_list_io_start(void)
{
 return (io_start);
}

player_linked_list *player_list_io_find(player *current)
{
 return (player_link_find(io_start, NULL,
                          current, PLAYER_LINK_LOGGEDON));
}

void player_list_perm_staff_add(player_tree_node *sp)
{
 player_link_add(&perm_staff_start, sp, NULL, PLAYER_LINK_NAME_ORDERED, NULL);
}

void player_list_perm_staff_del(player_tree_node *sp)
{
 player_link_del(&perm_staff_start, sp, NULL,
                 PLAYER_LINK_NAME_ORDERED, NULL);
}

player_linked_list *player_list_perm_staff_start(void)
{
 return (perm_staff_start);
}

void player_list_spod_add(player_tree_node *sp)
{
 player_link_add(&spod_start, sp, NULL,
                 PLAYER_LINK_NAME_ORDERED, NULL);
}

void player_list_spod_del(player_tree_node *sp)
{
 player_link_del(&spod_start, sp, NULL,
                 PLAYER_LINK_NAME_ORDERED, NULL);
}

player_linked_list *player_list_spod_start(void)
{
 return (spod_start);
}


/*
 * This is part of the flatlist thats created in logon.c and logoff.c
 * The flatlist needs converting to a linked list within this file.
 */

static int internal_cronlist_find(player_linked_list *passed_scan, va_list ap)
{
 const char *name = va_arg(ap, const char *);
 player *scan = PLAYER_LINK_GET(passed_scan);
 
 if (scan->saved && !strcmp(name, scan->saved->lower_name))
   return (FALSE);

 return (TRUE);
}

/* need the right name... lower etc... */
player *find_player_cronlist_exact(const char *name)
{
 player_linked_list *tmp = NULL;

 tmp = do_order_misc_on_all(internal_cronlist_find,
                            player_list_cron_start(), name);

 if (tmp)
   return (PLAYER_LINK_GET(tmp));

 return (NULL);
}
