#define MULTI_BASE_C
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

/* 
 * Base code for grouping ppl from a user list.
 *
 * Deficiencies include:
 *   o Admin group not supported.
 *   o Channels not supported (can do chan:name/chan name ...
 *      and even chan:admin) ?
 */

/* internal variables... */
static multi_base *multis_start = NULL;
static multi_base *multis_end = NULL;





static void insert_base_entry(multi_base *tmp, multi_base *new_base)
{
 BTRACE("insert_base_entry");
 
 if (tmp)
   if (new_base->number > tmp->number)
   {
    if ((new_base->next = tmp->next))
      new_base->next->prev = new_base;
    else
      multis_end = new_base;
    
    tmp->next = new_base;
    new_base->prev = tmp;
   }
   else
   {
    if ((new_base->prev = tmp->prev))
      new_base->prev->next = new_base;
    else
      multis_start = new_base;
    
    tmp->prev = new_base;
    new_base->next = tmp;
   }
 else
 {
  multis_start = new_base;
  multis_end = new_base;
  new_base->next = NULL;
  new_base->prev = NULL;
 }
}

/* forward referance destroy_multi_list */
static void multi_destroy_list(unsigned int multi_number);


static multi_base *do_cleanup_on_bases(multi_base *tmp)
{
 BTRACE("do_cleanup_on_bases");

 while (tmp)
 {
  multi_base *tmp_next = tmp->next;

  assert(MALLOC_VALID(tmp, sizeof(multi_base), MULTI_BASE));
  
  if ((difftime(now, tmp->last_used) > MULTI_TIMEOUT_SECONDS) &&
      !(tmp->flags & MULTI_KEEP_ALIVE))
    multi_destroy_list(tmp->number);
  tmp = tmp_next;
 }

 return (multis_start);
}

static multi_base *new_base_entry(multi_return *values)
{
 multi_base *new_base = XMALLOC(sizeof(multi_base), MULTI_BASE);
 unsigned int count = 1;
 multi_base *tmp = do_cleanup_on_bases(multis_start);

 IGNORE_PARAMETER(values);

 BTRACE("new_base_entry");
 if (!new_base)
   return (NULL);
  
 new_base->flags = 0;
 new_base->last_used = now;
 new_base->first_node = NULL;
 new_base->number = 1;
 new_base->total_players = 1; /* whoever starts the multi is on it */
 
 if (tmp)
 {
  while (tmp->next && (tmp->number == count))
  {
   assert(tmp->first_node && !tmp->first_node->prev_node);
   
   count++;
   tmp = tmp->next;
  }
  
  if (tmp->number == count)
    ++count; /* tmp->next = NULL */
  
  new_base->number = count;
 }

 insert_base_entry(tmp, new_base);
 
 return (new_base); /* this is NULL if malloc failed */
}

static void remove_base_entry(multi_base *base)
{
 assert(MALLOC_VALID(base, sizeof(multi_base), MULTI_BASE));

 BTRACE("remove_base_entry");
 if (base->prev)
   base->prev->next = base->next;
 else
   multis_start = base->next;

 if (base->next)
   base->next->prev = base->prev;
 else
   multis_end = base->prev; 
}

static multi_base *find_base_entry(unsigned int multi_number)
{
 multi_base *tmp = multis_start;

 BTRACE("find_base_entry");
 
 if (!VALID_MULTI(multi_number))
   return (NULL);
 
 if (abs(multi_number - multis_start->number) >
     abs(multi_number - multis_end->number))
 {
  tmp = multis_end;
  
  while (tmp && (tmp->number > multi_number))
    tmp = tmp->prev;
 }
 else
 {
  while (tmp && (tmp->number < multi_number))
    tmp = tmp->next;
 }
 
 if (tmp && (tmp->number == multi_number))
   return (tmp);
 else
   return (NULL);
}

static multi_node *find_first_node(unsigned int multi_number)
{
 multi_base *tmp = find_base_entry(multi_number);
  
 if (tmp)
   return (tmp->first_node);
 else
   return (NULL);
}

static int multi_should_see(multi_node *entry,
                            multi_base *base, multi_return *values)
{
 IGNORE_PARAMETER(values); /* may be needed l8r */

 assert(MALLOC_VALID(entry, sizeof(multi_node), MULTI_NODE));
 assert(MALLOC_VALID(base, sizeof(multi_base), MULTI_BASE));

 if ((base->flags & MULTI_COMPLETE_IGNORE) ||
     (entry->flags & (MULTI_BLOCKED | MULTI_TMP_BLOCK)) ||
     ((base->flags & MULTI_IGNORE_BUT_START) && !(entry->flags & MULTI_OWNER)))
   return (FALSE);
 else
   return (TRUE);
}

static void multi_add_entry(multi_node *entry)
{
 multi_node *tmp = entry->parent->multis_start;

 assert(MALLOC_VALID(entry, sizeof(multi_node), MULTI_NODE));
 BTRACE("multi_add_entry");
 
 if (tmp)
 { 
  assert(MALLOC_VALID(tmp, sizeof(multi_node), MULTI_NODE));
  
  while (tmp->next_multi && (tmp->number < entry->number))
    tmp = tmp->next_multi;

  assert(tmp->number != entry->number);

  if (tmp->number < entry->number)
  {
   if ((entry->next_multi = tmp->next_multi))
     entry->next_multi->prev_multi = entry;
   
   tmp->next_multi = entry;
   entry->prev_multi = tmp;
  }
  else
  {
   if ((entry->prev_multi = tmp->prev_multi))
     entry->prev_multi->next_multi = entry;
   else
     entry->parent->multis_start = entry;
   
   tmp->prev_multi = entry;
   entry->next_multi = tmp;
  }  
 }
 else
 {
  entry->parent->multis_start = entry;
  entry->next_multi = NULL;
  entry->prev_multi = NULL;
 }
}

static multi_node *multi_new_entry(player_tree_node *current,
                                   unsigned int multi_number)
{
 multi_node *new_node = XMALLOC(sizeof(multi_node), MULTI_NODE);

 if (!new_node)
   return (NULL);

 assert(!multi_find_entry(current, multi_number));
 
 new_node->parent = current;
 new_node->number = multi_number;
 new_node->flags = 0;
 new_node->next_node = NULL;
 new_node->prev_node = NULL;

 multi_add_entry(new_node);
 
 return (new_node);
}

multi_node *multi_find_entry(const player_tree_node *current,
                             unsigned int multi_number)
{
 multi_node *tmp = NULL;

 BTRACE("multi_find_entry");
 assert(MALLOC_VALID(current, sizeof(player_tree_node), PLAYER_TREE_NODE));
  
 if (!(VALID_MULTI(multi_number) &&
       MALLOC_VALID(current, sizeof(player_tree_node), PLAYER_TREE_NODE)))
   return (NULL);

 tmp = current->multis_start;
 
 while (tmp && (tmp->number < multi_number))
   tmp = tmp->next_multi;
 
 if (tmp && (tmp->number == multi_number))
   return (tmp);
 else
   return (NULL);
}

static void multi_del_entry(multi_node *entry)
{
 multi_node *before = NULL;
 multi_node *after = NULL;
      
 assert(MALLOC_VALID(entry, sizeof(multi_node), MULTI_NODE));
 assert(MALLOC_VALID(entry->parent, sizeof(player_tree_node),
                     PLAYER_TREE_NODE));
 assert(MALLOC_VALID(entry->parent->multis_start, sizeof(multi_node),
                     MULTI_NODE));
 BTRACE("multi_del_entry");
  
 if ((after = entry->next_multi))
   after->prev_multi = entry->prev_multi;
 
 if ((before = entry->prev_multi))
   before->next_multi = after;
 else
   entry->parent->multis_start = after; 
}

static int inorder_multi_flag_reset(multi_node *entry, va_list va)
{
 unsigned int flags = va_arg(va, unsigned int);
 
 entry->flags &= ~flags;

 return (TRUE);
}

static int multi_flag_reset(multi_base *base, int all, unsigned int flags)
{
 assert(MALLOC_VALID(base, sizeof(multi_base), MULTI_BASE));
 
 if (all)
 {
  base->flags &= ~MULTI_GROUP_FLAGS_ALL;
  do_inorder_multi(inorder_multi_flag_reset, base->number,
                   MULTI_SHOW_ALL, MULTI_GROUP_FLAGS_ALL);
 }
 else
   if (flags & base->flags)
   {
    base->flags &= ~flags;
    do_inorder_multi(inorder_multi_flag_reset, base->number,
			 MULTI_SHOW_ALL, flags);
   }
   else
     return (FALSE);

 return (TRUE);
}

static int multi_flag(multi_node *entry, multi_base *base,
                      unsigned int flag)
{
 assert(base && entry);
 
 entry->flags |= flag;
 base->flags |= flag;
 
 return (TRUE);
}

static int multi_remove_from(player_tree_node *current,
                             unsigned int multi_number,
                             multi_node *entry)
{
 multi_node *before = NULL;
 multi_node *after = NULL;

 BTRACE("multi_remove_from");
 
 if (entry)
 {
  multi_number = entry->number;

  if (!VALID_MULTI(multi_number))
    return (FALSE); 
 }
 else
 {
  if (!VALID_MULTI(multi_number))
    return (FALSE);
  
  entry = multi_find_entry(current, multi_number);
 }

 if (entry)
 {  /* remove them from the multi */
  multi_base *base = find_base_entry(multi_number);

  assert(base);
  --(base->total_players); 
 
  multi_flag_reset(base, TRUE, 0);
  /* reset_flag_for_multi(base, FALSE, entry->flags); */
  
  if ((after = entry->next_node))
    after->prev_node = entry->prev_node;
      
  if ((before = entry->prev_node))
    before->next_node = after;
  else
    if (!(base->first_node = after))
    {
     remove_base_entry(base);
     XFREE(base, MULTI_BASE);
    }
  
  multi_del_entry(entry);
  XFREE(entry, MULTI_NODE); 

  return (TRUE);
 }
 else
   return (FALSE);
}

static int inorder_multi_destroy_list(multi_node *entry,
                                      va_list va __attribute__ ((unused)))
{
 multi_del_entry(entry);
 XFREE(entry, MULTI_NODE);

 return (TRUE);
}

static void multi_destroy_list(unsigned int multi_number)
{
 multi_base *base = find_base_entry(multi_number);

 BTRACE("multi_destroy_list");
 if (!base)
   return;
 
 do_inorder_multi(inorder_multi_destroy_list, multi_number,
                  MULTI_SHOW_ALL);
 base->first_node = NULL;
 remove_base_entry(base);
 XFREE(base, MULTI_BASE);
}

static int multi_start(multi_node *entry, unsigned int multi_number)
{
 if (entry->flags & MULTI_OWNER)
 {
  multi_base *tmp = find_base_entry(multi_number);

  assert(tmp);
  tmp->flags &= ~MULTI_STOPPED;
  entry->flags &= ~MULTI_OWNER;
  
  return (TRUE);
 }
 else
   return (FALSE);
}

static int multi_stop(multi_node *entry, unsigned int multi_number)
{
 multi_base *tmp = NULL;
 
 if (entry->flags & MULTI_STOPPED)
   return (FALSE);

 tmp = find_base_entry(multi_number);
 
 entry->flags |= MULTI_OWNER;
 tmp->flags |= MULTI_STOPPED;
 
 return (TRUE);
}

static multi_node *multi_add_player(multi_node *adder,
                                    player_tree_node *addie,
                                    multi_base *base, multi_return *values)
{
 multi_node *to_add = NULL;
 
 assert(adder && addie && base && values);
 BTRACE("multi_add_player");
 
 if (adder->parent == addie)
 { /* they are adding themself */
  --values->players_added;
  adder->flags |= (MULTI_THIS_TIME);
  multi_flag(adder, base, MULTI_TO_SELF);
  return (adder);
 }
 
 if (!(to_add = multi_find_entry(addie, values->multi_number)))
   if ((to_add = multi_new_entry(addie, values->multi_number)))
   {
    multi_node *tmp = base->first_node;

    assert(tmp);

    if (values->codes & MULTI_STAY_LOCAL)
      if (P_IS_ON(addie) &&
          (addie->player_ptr->location != adder->parent->player_ptr->location))
      {
       multi_del_entry(to_add);
       XFREE(to_add, MULTI_NODE); 
       return (NULL);
      }
    
    while (tmp->next_node &&
           (strcmp(tmp->parent->lower_name, addie->lower_name) < 0))
      tmp = tmp->next_node;

    if (strcmp(tmp->parent->lower_name, addie->lower_name) < 0)
    {
     if ((to_add->next_node = tmp->next_node))
       to_add->next_node->prev_node = to_add;

     tmp->next_node = to_add;
     to_add->prev_node = tmp;
    }
    else
    {
     if ((to_add->prev_node = tmp->prev_node))
       to_add->prev_node->next_node = to_add;
     else
       base->first_node = to_add;
     
     tmp->prev_node = to_add;
     to_add->next_node = tmp;
    }

    /* do they want to be a part of the multi? afterwards ... */
    LIST_COMS_2CHECK_FLAG_START(addie, adder->parent,
                                !(values->codes & MULTI_NO_COMS_CHECKS));
    if (LIST_COMS_2CHECK_FLAG_DO(multis))
      to_add->flags |= MULTI_BLOCKED;
    LIST_COMS_2CHECK_FLAG_END();
    
    /* do flags and stuff to say what happened */
    ++base->total_players;
    multi_flag(to_add, base, MULTI_ADDED_THIS_TIME);
    to_add->flags |= (MULTI_THIS_TIME);
    return (to_add);
   }
   else
   {
    LOG_MEM_ERR();
    return (NULL);
   }
 else
 {
  /* they are already in it... but have been specified again */
  --(values->players_added);
  to_add->flags |= MULTI_THIS_TIME;
  return (to_add);
 }
 
 return (NULL);
}

static int internal_multi_add_room(player *scan, va_list ap)
{
 multi_node *current = va_arg(ap, multi_node *);
 multi_base *base = va_arg(ap, multi_base *);
 multi_return *values = va_arg(ap, multi_return *);
 int flag = va_arg(ap, int);
 int *count = va_arg(ap, int *);
 multi_node *tmp = NULL;

 if (current->parent == scan->saved)
   return (TRUE);
 
 LIST_COMS_2CHECK_FLAG_START(scan->saved, current->parent,
                             !(values->codes & MULTI_NO_COMS_CHECKS));
 if (LIST_COMS_2CHECK_FLAG_DO(says))
   return (TRUE);
 LIST_COMS_2CHECK_FLAG_END();
 
 if ((tmp = multi_add_player(current, scan->saved, base, values)))
 {
  ++*count;
  multi_flag(tmp, base, flag);
 }
 
 return (TRUE);
}

static int internal_multi_add_shout(player *scan, va_list ap)
{
 multi_node *current = va_arg(ap, multi_node *);
 multi_base *base = va_arg(ap, multi_base *);
 multi_return *values = va_arg(ap, multi_return *);
 int flag = va_arg(ap, int);
 int *count = va_arg(ap, int *);
 multi_node *tmp = NULL;

 if (current->parent == scan->saved)
   return (TRUE);

 LIST_COMS_2CHECK_FLAG_START(scan->saved, current->parent,
                             !(values->codes & MULTI_NO_COMS_CHECKS));
 if (LIST_COMS_2CHECK_FLAG_DO(shouts))
   return (TRUE);
 LIST_COMS_2CHECK_FLAG_END();
 
 if ((tmp = multi_add_player(current, scan->saved, base, values)))
 {
  ++*count;
  multi_flag(tmp, base, flag);
 }
 
 return (TRUE);
}

static int multi_add_newbies(multi_node *current, multi_base *base,
                             multi_return *values)
{
 int count = 0;
 player_tree_node *tmp = player_newbie_start();
 multi_node *tmp_entry = NULL;
 
 assert(current && base && values);
 BTRACE("multi_add_newbies");
 
 if ((values->codes & MULTI_NO_NEWBIES) ||
     (base->flags & MULTI_TO_NEWBIES))
   return (0);

 multi_flag(current, base, MULTI_TO_NEWBIES);
     
 for (; tmp; tmp = tmp->next)
   if ((tmp != current->parent) &&
       (tmp_entry = multi_add_player(current, tmp, base, values)))
   {
    multi_flag(tmp_entry, base, MULTI_AM_NEWBIE);
    count++;
   }

 return (count);
}

static int multi_add_everyone(multi_node *current, multi_base *base,
                              multi_return *values)
{
 int count = 0;
   
 assert(current && base && values);
 BTRACE("multi_add_everyone");
 
 if ((values->codes & MULTI_NO_EVERYONE) ||
     (base->flags & MULTI_TO_EVERYONE))
   return (0);

 multi_flag(current, base, MULTI_TO_EVERYONE);

 do_inorder_logged_on(internal_multi_add_shout, current, base, values,
                      MULTI_AM_EVERYONE, &count);
 
 return (count); 
}

static int inorder_multi_add_multi(multi_node *entry, va_list va)
{ /* could be optomised by moving the declarations */
 multi_node *current = va_arg(va, multi_node *);
 multi_base *base = va_arg(va, multi_base *);
 multi_return *values = va_arg(va, multi_return *);
 int *count = va_arg(va, int *);

 if (entry == current)
   return (TRUE);

 LIST_COMS_2CHECK_FLAG_START(entry->parent, current->parent,
                             !(values->codes & MULTI_NO_COMS_CHECKS));
 if (LIST_COMS_2CHECK_FLAG_DO(multis))
   return (TRUE);
 LIST_COMS_2CHECK_FLAG_END();
 
 if ((entry = multi_add_player(current, entry->parent, base, values)))
 {
  ++*count;
  multi_flag(entry, base, MULTI_AM_PERSON);
 }

 return (TRUE);
}

static int multi_add_multi(multi_node *current, unsigned int multi_number,
                           multi_base *base, multi_return *values)
{
 int count = 0;
   
 assert(current && base && values);
 BTRACE("multi_add_multi");
 
 if (values->codes & MULTI_NO_MULTIS)
   return (0);

 multi_flag(current, base, MULTI_TO_PERSON);

 do_inorder_multi(inorder_multi_add_multi, multi_number,
                  MULTI_SHOW_ALL, current, base, values, &count);
 
 return (count);
}

static int internal_multi_add_minister(player *scan, va_list ap)
{
 multi_node *current = va_arg(ap, multi_node *);
 multi_base *base = va_arg(ap, multi_base *);
 multi_return *values = va_arg(ap, multi_return *);
 int *count = va_arg(ap, int *);
 multi_node *tmp = NULL;

 if ((current->parent != scan->saved) && scan->saved->priv_minister &&
     (tmp = multi_add_player(current, scan->saved, base, values)))
 {
  ++*count;
  multi_flag(tmp, base, MULTI_AM_MINISTER);
 }
 
 return (TRUE);
}

static int multi_add_ministers(multi_node  *current, multi_base *base,
                               multi_return *values)
{
 int count = 0;
   
 assert(current && base && values);
 BTRACE("multi_add_ministers");
 
 if ((values->codes & MULTI_NO_MINISTERS) ||
     (base->flags & MULTI_TO_MINISTERS))
   return (0);

 multi_flag(current, base, MULTI_TO_MINISTERS);

 do_inorder_logged_on(internal_multi_add_minister,
                      current, base, values, &count);
 
 return (count); 
}

static int multi_add_room(multi_node *current, multi_base *base,
                          multi_return *values)
{
 int count = 0;
 
 /* anything to do with current will have died above */
 assert(current && current->parent && P_IS_ON(current->parent) &&
        base && values);
 BTRACE("multi_add_room");
 
 if ((values->codes & MULTI_NO_ROOM) ||
     (base->flags & MULTI_TO_ROOM))
   return (0);

 multi_flag(current, base, MULTI_TO_ROOM);

 do_inorder_room(internal_multi_add_room,
                 current->parent->player_ptr->location,
                 current, base, values, MULTI_AM_ROOM, &count);
 
 return (count);
}

static int multi_add_saved_supers(player_linked_list *passed_scan, va_list va)
{
 int *count = va_arg(va, int *);
 multi_node *current = va_arg(va, multi_node *);
 multi_base *base = va_arg(va, multi_base *);
 multi_return *values = va_arg(va, multi_return *);
 multi_node *tmp_entry = NULL;
 player_tree_node *scan = PLAYER_LINK_SAV_GET(passed_scan);
 
 if ((current->parent != scan) &&
     (tmp_entry = multi_add_player(current, scan, base, values)))
 {
  multi_flag(tmp_entry, base, MULTI_AM_SUPER);
  ++*count;
 }
 
 return (TRUE);
}

static int multi_add_supers(multi_node *current, multi_base *base,
                            multi_return *values)
{
 int count = 0;
 
 assert(current && current->parent && P_IS_ON(current->parent));
 BTRACE("multi_add_supers");
 
 if ((values->codes & MULTI_NO_SUS) ||
     (base->flags & MULTI_TO_SUPERS))
   return (0);

 multi_flag(current, base, MULTI_TO_SUPERS);

 if (values->codes & MULTI_USE_NOT_ON)
   do_order_misc_all(multi_add_saved_supers, player_list_perm_staff_start(),
                     &count, current, base, values);
 else
   do_order_misc_on(multi_add_saved_supers, player_list_logon_staff_start(),
                    &count, current, base, values);
 
 return (count);
}

static void multi_add_saved_friends(player_tree_node *current, va_list va)
{
 multi_node *friends_of = va_arg(va, multi_node *);
 int *count = va_arg(va, int *);
 multi_node *adder = va_arg(va, multi_node *);
 int block_my_friends = va_arg(va, int);
 multi_base *base = va_arg(va, multi_base *);
 multi_return *values = va_arg(va, multi_return *);
 multi_node *tmp_entry = NULL;
 int do_friend = FALSE;
 
 if (current->priv_banished || PRIV_SYSTEM_ROOM(current))
   return; /* mainly for sus */

 LIST_SELF_CHECK_FLAG_START(friends_of->parent->player_ptr, current);
 if (LIST_SELF_CHECK_FLAG_DO(friend) &&
     !LIST_SELF_CHECK_FLAG_GROUP("everyone"))
   do_friend = TRUE;
 LIST_SELF_CHECK_FLAG_END();

 if (!do_friend)
   return;

 LIST_COMS_2CHECK_FLAG_START(current, adder->parent,
                             !(values->codes & MULTI_NO_COMS_CHECKS));
 if (LIST_COMS_2CHECK_FLAG_DO(tfsof))
   return;
 LIST_COMS_2CHECK_FLAG_END();
 
 LIST_COMS_2CHECK_FLAG_START(current, friends_of->parent,
                             !(values->codes & MULTI_NO_COMS_CHECKS));
 if (LIST_COMS_2CHECK_FLAG_DO(tfs))
   return;
 LIST_COMS_2CHECK_FLAG_END();
 
 if ((tmp_entry = multi_add_player(adder, current, base, values)))
 {
  if (block_my_friends)
    multi_flag(tmp_entry, base, MULTI_AM_FRIEND);
  else
    multi_flag(tmp_entry, base, MULTI_AM_FRIEND_OF);
  ++*count;
 }
}

/* should be converted to static eventualy */
static int multi_add_friends(multi_node *friends_of, multi_node *current,
                             multi_base *base, multi_return *values)
{
 int count = 0;
 int block_my_friends = FALSE;
 multi_node *tmp_entry = NULL;

 assert(current);
 BTRACE("multi_add_friends");

 if (!player_load(friends_of->parent))
   return (0);
 
 if (friends_of == current)
 {
  if ((values->codes & MULTI_NO_FRIENDS) ||
      (base->flags & MULTI_TO_FRIENDS))
    return (0);
  block_my_friends = TRUE;
  multi_flag(current, base, MULTI_TO_FRIENDS | MULTI_AM_FRIEND);
 }
 else
   if (values->codes & MULTI_NO_FRIENDS_OF)
     return (0);
   else
   {
    multi_flag_reset(base, FALSE, MULTI_TO_FRIENDS_OF| MULTI_AM_FRIEND_OF);
    multi_flag(friends_of, base,
		   MULTI_TO_FRIENDS_OF | MULTI_AM_FRIEND_OF);
   }
 
 if (values->codes & MULTI_USE_NOT_ON)
 {
  /* makes a list of all your friends, even if they are logged off */
  do_inorder_all(multi_add_saved_friends,
                 friends_of, &count, current, block_my_friends, base, values);
 }
 else
 {
  player_linked_list *tmp = player_list_alpha_start();
  
  /* makes a list of friends who are logged on */
  for (; tmp; tmp = PLAYER_LINK_NEXT(tmp))
  {
   int do_friend = FALSE;
   
   if (PLAYER_LINK_SAV_GET(tmp) != friends_of->parent)
   {
    LIST_SELF_CHECK_FLAG_START(friends_of->parent->player_ptr,
                               PLAYER_LINK_SAV_GET(tmp));
    if (LIST_SELF_CHECK_FLAG_DO(friend) &&
        !LIST_SELF_CHECK_FLAG_GROUP("everyone"))
      do_friend = TRUE;
    LIST_SELF_CHECK_FLAG_END();
   }
   
   LIST_COMS_2CHECK_FLAG_START(PLAYER_LINK_SAV_GET(tmp), current->parent,
                               do_friend && !block_my_friends &&
                               !(values->codes & MULTI_NO_COMS_CHECKS));
   if (LIST_COMS_2CHECK_FLAG_DO(tfsof))
     do_friend = FALSE;
   LIST_COMS_2CHECK_FLAG_END();

   LIST_COMS_2CHECK_FLAG_START(PLAYER_LINK_SAV_GET(tmp), friends_of->parent,
                               do_friend &&
                               !(values->codes & MULTI_NO_COMS_CHECKS));
   if (LIST_COMS_2CHECK_FLAG_DO(tfs))
     do_friend = FALSE;
   LIST_COMS_2CHECK_FLAG_END();

   if (do_friend &&
       (tmp_entry = multi_add_player(current, PLAYER_LINK_SAV_GET(tmp),
                                     base, values)))
   {
    if (block_my_friends)
      multi_flag(tmp_entry, base, MULTI_AM_FRIEND);
    else
      multi_flag(tmp_entry, base, MULTI_AM_FRIEND_OF);
    count++;
   }
  }
 }
 
 return (count);
}

static multi_base *multi_get_new_base(player_tree_node *current,
                                      multi_return *values)
{
 multi_base *base = new_base_entry(values);
 
 assert(current && P_IS_ON(current) && values);
 BTRACE("multi_get_new_base");
   
 if (base)
 {
  multi_node *tmp = multi_new_entry(current, base->number);
  
  if (tmp)
  {
   base->first_node = tmp;
   values->multi_number = base->number;

   ++(values->players_added);
   tmp->flags |= MULTI_THIS_TIME;
   
   if (values->codes & MULTI_COMPLETE_IGNORE)
   {
    base->flags |= (MULTI_COMPLETE_IGNORE | MULTI_DESTROY_CLEANUP);
    values->codes |= MULTI_NO_DO_MATCH;
   }

   if (values->codes & MULTI_IGNORE_BUT_START)
   {
    tmp->flags |= MULTI_OWNER;
    base->flags |= MULTI_IGNORE_BUT_START;
    values->codes |= MULTI_NO_DO_MATCH;
   }

   if (values->codes & MULTI_KEEP_ALIVE)
   {
    base->flags |= MULTI_KEEP_ALIVE;
    values->codes |= MULTI_NO_DO_MATCH;
   }  
   values->error_number = MULTI_CREATED;
  }
  else
  {
   multi_destroy_list(base->number);
   base = NULL;
   values->multi_number = 0; /* failed */
   values->error_name = current->lower_name;
   values->error_number = MULTI_NO_ENTRY_CREATED;
  }
 }
 else
 {
  values->multi_number = 0; /* failed */
  values->error_number = MULTI_NO_BASE_CREATED;
 }
 
 return (base);
}

static int inorder_multi_refresh(multi_node *tmp,
                                 va_list va __attribute__ ((unused)))
{
 tmp->flags |= MULTI_THIS_TIME;

 return (TRUE);
}

static void multi_refresh(unsigned int multi_number)
{
 do_inorder_multi(inorder_multi_refresh, multi_number, 0);
}

static multi_base *get_working_base(player_tree_node *current, char **temp,
                                    multi_node **entry, multi_return *values)
{
 multi_base *base = NULL;
 
 BTRACE("get_working_base");

 if (!(values->codes & MULTI_MUST_CREATE))
 {
  char *tmp = *temp;
  values->multi_number = skip_atol((const char **) temp);
  if (!**temp || (**temp == ','))
  {
   if (!*(*temp = skip_chars(*temp, ',')))
     *temp = NULL;
  }
  else
  {
   values->multi_number = 0;
   *temp = tmp;
  }
 }

 if (!P_IS_ON(current))
   values->codes &= ~MULTI_VERBOSE;
 
 if (values->multi_number)
 {
  /* add ppl to an existing list */
  if ((*entry = multi_find_entry(current, values->multi_number)))
  {
   base = find_base_entry(values->multi_number);
    
   if (!multi_should_see(*entry, base, values))
   {
    if (values->codes & MULTI_VERBOSE)
      fvtell_player(NORMAL_T(current->player_ptr), "%s", 
                    " You are not on that multi.\n");
    values->multi_number = 0;
    values->error_number = MULTI_BAD_IGNORE;
    return (NULL);
   }
   
   if (values->codes & MULTI_REFRESH)
     multi_refresh(base->number);
   
   if (!(values->codes & MULTI_KEEP_GROUP_FLAGS))
     multi_flag_reset(base, TRUE, 0);
   
   if (!*temp) /* they are not trying to add ... just access */
     return (base);
   
   if (base->flags & MULTI_STOPPED)
   {
    /* the multi is stopped and so can not be added to */
    if (values->codes & MULTI_VERBOSE)
      fvtell_player(SYSTEM_T(current->player_ptr),
                    " Multi -- ^S^B%d^s -- has already been stopped.\n",
                    base->number);
    
    if (values->codes & MULTI_DIE_STOPPED)     
    {
     values->error_number = MULTI_STOPPED_ALREADY;
     values->error_multi = values->multi_number;
     values->multi_number = 0;
     return (NULL);
    }
    
    *temp = NULL; /* don't allow ppl to add */
   }
   
   /* this is what to return */
   return (base);
  }
  else
  {
   /* they put a multi in that they arn't on */
   if (values->codes & MULTI_VERBOSE)
     fvtell_player(NORMAL_T(current->player_ptr),
                   " You are not on the -- ^S^B%u^s -- multi.\n",
                   values->multi_number);
   
   if (values->codes & MULTI_DIE_MATCH_MULTI)
   {
    values->error_number = MULTI_BAD_MULTI_SELECTION;
    values->error_multi = values->multi_number;
    values->multi_number = 0;
    return (NULL);
   }
  }
 }
 
 if ((base = multi_get_new_base(current, values)))
 {
  *entry = base->first_node;
  return (base);
 }
 else
   return (NULL);
}

static int inorder_multi_find_match(multi_node *current, va_list va)
{
 unsigned int *length = va_arg(va, unsigned int *);
 multi_node **smallest = va_arg(va, multi_node **);
 multi_node *tmp = current->parent->multis_start;
 unsigned int count = 0;
 
 while (tmp && (count < *length))
 {
  ++count;
  tmp = tmp->next_multi;
 }

 if (!tmp)
 {
  *length = count;
  *smallest = current;
  
  if (count == 1)
    return (FALSE);
 }
 
 return (TRUE);
}

static int internal_multi_compare(multi_base *one, multi_base *two)
{
 multi_node *tmp_one = one->first_node;
 multi_node *tmp_two = two->first_node;

 BTRACE("internal_multi_compare");
 if (one->total_players != two->total_players)
   return (FALSE);
 
 while (tmp_one && tmp_two && (tmp_one->parent == tmp_two->parent))
 {
  if (((tmp_one->flags & MULTI_THIS_TIME) !=
       (tmp_two->flags & MULTI_THIS_TIME)))
    return (FALSE);
  
  tmp_one = tmp_one->next_node;
  tmp_two = tmp_two->next_node;
 }

 if (tmp_one || tmp_two)
   return (FALSE);
 else
   return (TRUE);
}

int multi_compare(unsigned int one, unsigned int two)
{
 multi_base *base_one = find_base_entry(one);
 multi_base *base_two = find_base_entry(two);

 if (one == two)
   return (TRUE);
 
 return (internal_multi_compare(base_one, base_two));
}

static int inorder_multi_change_number(multi_node *tmp, va_list va)
{
 unsigned int new_numb = va_arg(va, unsigned int);
 
 multi_del_entry(tmp);
 tmp->number = new_numb;
 multi_add_entry(tmp);

 return (TRUE);
}


static void multi_change_number(multi_base *base, multi_return *values,
                                unsigned int new_numb)
{
 multi_base *tmp = NULL;

 BTRACE("multi_change_number");
 do_inorder_multi(inorder_multi_change_number, base->number,
                      MULTI_SHOW_ALL, new_numb);
 
 remove_base_entry(base);

 base->number = new_numb; /* change the numbers in the base and values */
 values->multi_number = new_numb;
 
 if ((tmp = multis_start)) /* have to find where to put it ourself */
   while (tmp->next && (tmp->number < base->number))
     tmp = tmp->next;

 insert_base_entry(tmp, base); /* to insert into the linked list */
}

static multi_return *multi_find_match(multi_base *base, multi_return *values)
{
 multi_node *smallest = NULL;
 unsigned int length = INT_MAX;
 multi_node *tmp = NULL;

 BTRACE("multi_find_match");
 do_inorder_multi(inorder_multi_find_match, base->number, 0,
                      &length, &smallest);

 if (length == 1)
   return (values);

 assert(smallest && smallest->parent && smallest->parent->multis_start);
 
 tmp = smallest->parent->multis_start;
 while (tmp)
 {
  multi_base *cmp_base = find_base_entry(tmp->number);
  if ((base != cmp_base) && internal_multi_compare(base, cmp_base))
  { /* we have match */
   unsigned int new_numb = cmp_base->number;

   multi_destroy_list(new_numb);
   multi_change_number(base, values, new_numb);
   return (values);
  }
  tmp = tmp->next_multi;
 }

 return (values);
}

static void multi_init(multi_return *values)
{
 values->multi_number = 0;
 values->players_added = 0;
 values->single_player = NULL;
 values->error_number = 0;
 values->error_multi = 0;
 values->error_name = NULL;
 values->codes = 0;
 values->find_flags = 0;
}

multi_return *multi_add(player_tree_node *current, char *str,
                        unsigned int start_codes,
                        unsigned int start_find_flags)
{
 static multi_return the_values;
 multi_return *values = &the_values;
 char *temp = str;             /* these are both used to get the    */
 multi_node *entry = NULL;
 multi_base *base = NULL;
 int padded = 0; /* for adding players, in groups */
 
 BTRACE("multi_add");

 multi_init(values);
 values->codes = start_codes;
 values->find_flags = start_find_flags;
 
 if ((base = get_working_base(current, &temp, &entry, values)))
 {
  /* sorted out what multi we are going to... add people now */
  base->last_used = now;
  
  while (temp) 
  {
   char *temp_end = NULL;/* player names/groups/multi numbers */
   
   if ((temp_end = next_parameter(temp, ',')))
     *temp_end++ = 0;
   
   lower_case(temp);
   
   if (isdigit((unsigned char) *temp))
   {
    unsigned int new_multi_number = atol(temp);
    
    /* it's a multi */
    if (!(padded = multi_add_multi(entry, new_multi_number,
                                   base, values)))
    { /* noone was added */
     if (values->codes & MULTI_VERBOSE)
       fvtell_player(NORMAL_T(current->player_ptr),
                     " None of the people in the multi -- ^S^B%u^s -- have "
                     "been added.\n", new_multi_number);
     if (values->codes & MULTI_DIE_EMPTY_GROUP)
       values->error_multi = new_multi_number;
     MULTI_DIE_IF_EMPTY(MULTI_FIND); 
    }
   }
   else
   { /* not a multi */
    if (!strcmp(temp, "friends"))
    {
     if (!(padded = multi_add_friends(entry, entry, base, values)))
     { /* noone was added */
      if (values->codes & MULTI_VERBOSE)
        fvtell_player(NORMAL_T(current->player_ptr), "%s",
                      " None of ^S^Byour friends^s have been added.\n");
      MULTI_DIE_IF_EMPTY(FRIENDS_FIND); 
     }
    }      
    else
    { /* not friends */
     if (!(beg_strcmp("friendsof:", temp) && beg_strcmp("friendsof ", temp) &&
           beg_strcmp("friends of ", temp)))
     { /* not blocking friends of */
      player *person = NULL;
      player_tree_node *friends_of = NULL;
      multi_node *friends_of_node = NULL;
      int bad_friends = FALSE;
      
      temp += CONST_STRLEN("friendsof ");
      temp += strspn(temp, " ");
      if ((person = player_find_on(current->player_ptr, temp,
                                   values->find_flags)))
        friends_of = person->saved;
      else
        if (values->codes & MULTI_USE_NOT_ON)
        {
         friends_of = player_find_all(current->player_ptr, temp,
                                      values->find_flags);
         if (!player_load(friends_of))
           bad_friends = TRUE;
        }
        else
          bad_friends = TRUE;
      
      if (!bad_friends)
      {
       LIST_COMS_CHECK_FLAG_START(friends_of->player_ptr, current);
       if (LIST_COMS_CHECK_FLAG_DO(tells))
         bad_friends = TRUE;
       LIST_COMS_CHECK_FLAG_END();
      }
      
      if (!bad_friends &&
          (friends_of_node =
           multi_add_player(entry, friends_of, base, values)))
      { /* ***************************************************
         * note that the friends_of HAS to be in the room if your blocking
         * tells (or have the noisy flag), if they aren't then it will not
         * go to ANY of there friends
         */
       
       ++(values->players_added);
       
       if (!(padded = multi_add_friends(friends_of_node, entry,
                                        base, values)))
       { /* nonoe was added */
        if (values->codes & MULTI_VERBOSE)
          fvtell_player(NORMAL_T(current->player_ptr),
                        " None of the friends of -- ^S^B%s^s -- have "
                        "been added.\n",
                        friends_of->name);
        MULTI_DIE_IF_EMPTY(FRIENDS_OF_FIND);
       }
      }
      else
      {
       if (values->codes & MULTI_VERBOSE)
       {
        if (friends_of && P_IS_AVL(friends_of))
          fvtell_player(NORMAL_T(current->player_ptr),
                        " The player -- ^SB%s^s -- doesn't have you on %s "
                        "friends list.\n", friends_of->name,
                        gender_choose_str(friends_of->player_ptr->gender,
                                          "his", "her", "their", "its"));
        /* else player_find gave out the error meessage */
       }
       if (friends_of && P_IS_AVL(friends_of))
         MULTI_DIE_IF_MATCH(FRIENDS_OF_SELECTION);
       else
         MULTI_DIE_IF_MATCH(FRIENDS_OF_NAME_FIND);
      }
     }
     else
     { /* not friends(of) */
      if (!strcmp(temp, "room"))
      {
       if (!(padded = multi_add_room(entry, base, values)))
       { /* noone was added */
        if (values->codes & MULTI_VERBOSE)
          fvtell_player(NORMAL_T(current->player_ptr), "%s",
                        " No-one in the ^S^Broom^s has been added.\n");
        MULTI_DIE_IF_EMPTY(ROOM_FIND);
       }
      }
      else
      { /* not friends(of) or room */
       if (!(strcmp(temp, "sus") && strcmp(temp, "supers")))
       { /* FIXME: lsu and telling the names works, so tell sus
          * should do a tell to the people on duty */
        if (!((values->codes & MULTI_NO_PRIVS_NEEDED) ||
              PRIV_STAFF(current)))
        {
         if (values->codes & MULTI_VERBOSE)
           fvtell_player(NORMAL_T(current->player_ptr), "%s",
                          " None of the ^S^Bsus^s have been added.\n");
         MULTI_DIE_IF_MATCH(SU_SELECTION);
        }
        else
        {
         if (!(padded = multi_add_supers(entry, base, values)))
         { /* noone was added */
          if (values->codes & MULTI_VERBOSE)
            fvtell_player(NORMAL_T(current->player_ptr), "%s",
                          " None of the ^S^Bsus^s have been added.\n");
          MULTI_DIE_IF_EMPTY(SU_FIND);
         } /* not added any */
        } /* have the privs */
       }
       else
       { /* not supers, room, or friends(of) */
        if (!strcmp(temp, "ministers"))
        {
         if (!((values->codes & MULTI_NO_PRIVS_NEEDED) ||
               current->priv_minister || current->priv_admin))
         {/* they don't have the privs */
          if (values->codes & MULTI_VERBOSE)
            fvtell_player(NORMAL_T(current->player_ptr), "%s",
                          " None of the ^S^Bministers^s have been added.\n");
          MULTI_DIE_IF_MATCH(MINISTER_SELECTION);
         }
         else
         {
          if (!(padded = multi_add_ministers(entry, base, values)))
          { /* noone was added */
           if (values->codes & MULTI_VERBOSE)
             fvtell_player(NORMAL_T(current->player_ptr), "%s",
                           " None of the ^S^Bministers^s have been added.\n");
           MULTI_DIE_IF_EMPTY(MINISTER_FIND);
          } /* not added any */
         } /* allowed to add ministers */
        }
        else
        { /* not friends(of) or room or supers or ministers */
         if (!strcmp(temp, "newbies"))
         {
          if (!((values->codes & MULTI_NO_PRIVS_NEEDED) ||
                PRIV_STAFF(current)))
          {
           if (values->codes & MULTI_VERBOSE)
             fvtell_player(NORMAL_T(current->player_ptr), "%s",
                           " None of the ^S^Bnewbies^s have been added.\n");
           MULTI_DIE_IF_MATCH(NEWBIE_SELECTION);
          }
          else
            if (!(padded = multi_add_newbies(entry, base, values)))
            { /* noone was added */
             if (values->codes & MULTI_VERBOSE)
               fvtell_player(NORMAL_T(current->player_ptr), "%s",
                             " None of the ^S^Bnewbies^s have been added.\n");
             MULTI_DIE_IF_EMPTY(NEWBIE_FIND);
            }
         }
         else
         { /* not friends(of) or room or supers or ministers */
          if (!(strcmp(temp, "everyone") && strcmp(temp, "all")))
          {
           if (!(padded = multi_add_everyone(entry, base, values)))
           { /* noone was added */
            if (values->codes & MULTI_VERBOSE)
              fvtell_player(NORMAL_T(current->player_ptr), "%s",
                            " No-one has been added.\n");
            MULTI_DIE_IF_EMPTY(EVERYONE_FIND); 
           }
          }
          else /* last try is try a real name... */
          {
           if (values->codes & MULTI_NO_NAMES)
           { /* names not allowed */
            if (values->codes & MULTI_VERBOSE)
              fvtell_player(NORMAL_T(current->player_ptr), "%s",
                            " ^S^BPlayer names^s are not allowed.\n");
            
            if (values->codes & MULTI_DIE_MATCH_NAME)
            {
             if (IS_NEW_MULTI(values))
               multi_destroy_list(base->number);
             values->error_name = temp;
             values->error_number = MULTI_BAD_NAME_SELECTION;
             values->multi_number = 0;
             return (values);
            }
           }
           else
           {
            player *person = NULL;
            player_tree_node *saved = NULL;
            multi_node *added = NULL;
            int do_add = TRUE;
            
            if (values->codes & MULTI_STAY_LOCAL)
            {
             if ((person = player_find_local(current->player_ptr, temp,
                                             values->find_flags)))
               saved = person->saved;
            }
            else
              if (values->codes & MULTI_USE_NOT_ON)
                saved = player_find_all(current->player_ptr, temp,
                                        values->find_flags);
              else
                if ((person = player_find_on(current->player_ptr, temp,
                                             values->find_flags)))
                  saved = person->saved;
            
            if (saved && (saved != entry->parent))
            {
             LIST_COMS_2CHECK_FLAG_START(saved, entry->parent,
                                      !(values->codes & MULTI_NO_COMS_CHECKS));
             if (LIST_COMS_2CHECK_FLAG_DO(tells))
               do_add = FALSE;
             LIST_COMS_2CHECK_FLAG_END();
            }
            
            if (saved && do_add &&
                (added = multi_add_player(entry, saved, base, values)))
            {
             multi_flag(entry, base, MULTI_TO_PERSON);
             multi_flag(added, base, MULTI_AM_PERSON);
             ++(values->players_added);
            }
            else
            { /* person wasn't added */
             if (values->codes & MULTI_VERBOSE)
             {
              if (saved)
                fvtell_player(SYSTEM_T(current->player_ptr),
                              " The player -- ^S^B%s^s -- was not added.\n",
                              temp);
              /* else player_find gave out the error message */
             }
             
             if (values->codes & MULTI_DIE_MATCH_NAME)
             {
              if (IS_NEW_MULTI(values))
                multi_destroy_list(base->number);
              values->error_name = temp;
              values->error_number = MULTI_BAD_NAME_FIND;
              values->multi_number = 0;
              return (values);
             }
            }
           } /* they added another person, or themself */
          } /* not everyone */
         } /* not newbies */
        } /* not ministers */
       } /* not super users */
      } /* not room */
     } /* not friends of */
    } /* not friends */
   } /* not multi */
    
   values->players_added += padded;
   padded = 0; /* so the players_added doesn't go up */
    
   temp = temp_end;
  }
 }
 else
   return (values);

 if (IS_NEW_MULTI(values) && (values->players_added < 3) &&
     !(values->codes & MULTI_LIVE_ON_SMALL))
 {
  if (values->codes & MULTI_VERBOSE)
    fvtell_player(NORMAL_T(current->player_ptr), "%s",
                  " You need at least ^S^Btwo other people^s in a group.\n");
  
  if (values->players_added == 1)
    values->error_number = MULTI_NO_PEOPLE_ADDED;
  else
  {
   assert(values->players_added == 2);
   assert(entry->next_node || entry->prev_node);
   
   values->error_number = MULTI_NOT_ENOUGH_PEOPLE;
   if (entry->next_node)
     values->single_player = entry->next_node->parent;
   else
     if (entry->prev_node)
       values->single_player = entry->prev_node->parent;
  }
  
  multi_destroy_list(base->number);
  values->multi_number = 0;

  return (values);
 }

 if (IS_NEW_MULTI(values) &&
     !(values->codes & MULTI_NO_DO_MATCH))
   return (multi_find_match(base, values));
 else
   return (values);
}

multi_node *do_inorder_multi(int (*func) (multi_node *, va_list),
                             unsigned int multi_number,
                             unsigned int flags, ... )
{
 multi_node *tmp = find_first_node(multi_number);
 VA_R_DECL(r_ap);
 
 VA_R_START(r_ap, flags);

 BTRACE("do_inorder_multi");
 
 while (tmp)
 {
  multi_node *tmp_next = tmp->next_node; /* so they can be removed etc... */
  
  assert(MALLOC_VALID(tmp, sizeof(multi_base), MULTI_NODE));
  
  if (((P_IS_ON(tmp->parent) || (flags & MULTI_USE_NOT_ON)) &&
       (tmp->flags & MULTI_THIS_TIME)) ||
      (flags & MULTI_SHOW_ALL))
  {
   int carry_on = TRUE;
   VA_C_DECL(ap);
   
   VA_C_START(ap, flags);
   
   VA_C_COPY(ap, r_ap);
   
   carry_on = (*func) (tmp, ap);

   VA_C_END(ap);

   if (!carry_on) { VA_R_END(r_ap); return (tmp); }
  }
  
  tmp = tmp_next;
 }
 VA_R_END(r_ap);
 
 return (NULL);
}

static int inorder_cleanup_multi(multi_node *entry, va_list va)
{
 multi_base *base = va_arg(va, multi_base *);

 if (multi_should_see(entry, base, 0))
   entry->flags &= ~MULTI_ADDED_THIS_TIME;
 else
   entry->flags &= ~(MULTI_THIS_TIME | MULTI_ADDED_THIS_TIME);
 
 return (TRUE);
}

void multi_cleanup(unsigned int multi_number, unsigned int flags)
{
 multi_base *base = find_base_entry(multi_number);

 assert(base); /* you gave an invalid multi_number */
 BTRACE("multi_cleanup");
 
 if (base->flags & MULTI_DESTROY_CLEANUP)
   multi_destroy_list(multi_number);
 else
 {
  do_inorder_multi(inorder_cleanup_multi, multi_number, flags, base);
  base->flags &= ~MULTI_ADDED_THIS_TIME;
 }
}

const char *multi_get_number(multi_node *entry)
{
 static char prefix_array[BUF_NUM_TYPE_SZ(unsigned int) + 3];
 multi_base *base = find_base_entry(entry->number);
  
 if (multi_should_see(entry, base, 0))
 {
  if (P_IS_ON(entry->parent))
    entry->parent->player_ptr->multi_last_used = entry->number;
  sprintf(prefix_array, "(%u) ", entry->number);
 }
 else
   return("");
 
 return (prefix_array);
}

static int inorder_multi_get_node_with_flag(multi_node *tmp, va_list va)
{
 unsigned int flag = va_arg(va, unsigned int);
 multi_node **found = va_arg(va, multi_node **);

 if (flag == (tmp->flags & flag))
 {
  *found = tmp;
  return (FALSE);
 }
 else
   return (TRUE);
}

/* finds the first node with the flag, if there is one */
multi_node *multi_get_node_with_flag(unsigned int multi_number,
                                     unsigned int flag)
{
 multi_node *found = NULL;
 multi_base *base = find_base_entry(multi_number);
 
 if (base->flags & flag)
   do_inorder_multi(inorder_multi_get_node_with_flag, multi_number,
                    MULTI_SHOW_ALL, flag, &found);

 return (found);
}

unsigned int multi_check_for_flag(unsigned int multi_number, unsigned int flag)
{
 multi_base *base = find_base_entry(multi_number);

 assert(MALLOC_VALID(base, sizeof(multi_base), MULTI_BASE));
 
 return (base->flags & flag);
}

unsigned int multi_check_node_for_flag(multi_node *entry, unsigned int flag)
{
 assert(MALLOC_VALID(entry, sizeof(multi_node), MULTI_NODE));

 return (entry->flags & flag);
}

static const char *get_your_possessive(multi_node *entry, multi_node *tmp)
{ /* his her it's etc... */
 if (tmp && P_IS_AVL(tmp->parent))
 {
  if (tmp == entry)
    return ("your");
  else
    return (gender_choose_str(tmp->parent->player_ptr->gender,
                              "him", "her", "them", "it"));
 }
 else
   return ("");
}

static const char *get_friends_possessive(multi_node *entry, multi_node *tmp)
{ /* his her it's etc... */
 if (tmp && P_IS_AVL(tmp->parent))
 {
  if (tmp == entry)
    return ("your");
  else
    return (gender_choose_str(tmp->parent->player_ptr->gender,
                              "his", "her", "their", "its"));
 }
 else
   return ("");
}

static int get_names_groups(char *name_array, int size,
                            unsigned int multi_number,
			    multi_node *entry, multi_base *base, int flags,
                            multi_node *friends, multi_node *friends_of,
                            char *buffer)
{
 assert(buffer && !*buffer);
 BTRACE("get_names_groups");
  
 if ((base->flags & MULTI_TO_SELF) && (flags & MULTI_TO_SELF))
 {
  multi_node *tmp = multi_get_node_with_flag(multi_number, MULTI_AM_SELF);

  assert(tmp);
  if (!(tmp->flags & (MULTI_GROUP_FLAGS_AM &
		      ~(MULTI_AM_PERSON | MULTI_AM_SELF))))
    /* treat it like a name and don't show it if they are
       in one of the other groups */
    sprintf(buffer, "%sself", get_your_possessive(entry, tmp));
 }
 
 if ((base->flags & MULTI_TO_FRIENDS) && !(flags & MULTI_NO_FRIENDS) &&
     ((base->flags & MULTI_AM_FRIEND) || (flags & MULTI_USE_EMPTY_GROUPS)))
 {
  if (*buffer)
    size += sprintf(name_array + size, ADD_COMMA_FRONT(size, "%s"),
                    buffer);

  if (!friends)
    friends = multi_get_node_with_flag(multi_number, MULTI_TO_FRIENDS);

  if (entry == friends_of)
    CONST_COPY_STR_LEN(buffer, "your friends");
  else
    sprintf(buffer, "%s friends%s",
            get_friends_possessive(entry, friends),
            USE_STRING((flags & MULTI_POSSESSIVE_OUTPUT), "'"));
 }
 
 if ((base->flags & MULTI_TO_FRIENDS_OF) && !(flags & MULTI_NO_FRIENDS_OF) &&
     ((base->flags & MULTI_AM_FRIEND_OF) || (flags & MULTI_USE_EMPTY_GROUPS)))
 {
  if (*buffer)
    size += sprintf(name_array + size, ADD_COMMA_FRONT(size, "%s"), buffer);

  if (!friends_of)
    friends_of = multi_get_node_with_flag(multi_number, MULTI_TO_FRIENDS_OF);

  if (entry == friends_of)
    sprintf(buffer, "your friends%s",
            USE_STRING((flags & MULTI_POSSESSIVE_OUTPUT), "'"));
  else
    sprintf(buffer, "the friends%s of %s%s",
            USE_STRING((flags & MULTI_POSSESSIVE_OUTPUT), "'"),
            (P_IS_AVL(friends_of->parent) &&
             (friends_of->parent->player_ptr->gender == GENDER_PLURAL)) ?
            "the " : "",
            friends_of->parent->name);
 }
 
 if ((base->flags & MULTI_TO_ROOM) && !(flags & MULTI_NO_ROOM) &&
     ((base->flags & MULTI_AM_ROOM) || (flags & MULTI_USE_EMPTY_GROUPS)))
 {
  if (*buffer)
    size += sprintf(name_array + size, ADD_COMMA_FRONT(size, "%s"), buffer);
  
  sprintf(buffer, "everyone in the room%s",
          USE_STRING((flags & MULTI_POSSESSIVE_OUTPUT), "'s"));
 }
 
 if ((base->flags & MULTI_TO_SUPERS) && !(flags & MULTI_NO_SUS) &&
     ((base->flags & MULTI_AM_SUPER) || (flags & MULTI_USE_EMPTY_GROUPS)))
 {
  if (*buffer)
    size += sprintf(name_array + size, ADD_COMMA_FRONT(size, "%s"), buffer);

  sprintf(buffer, "the super users%s",
          USE_STRING((flags & MULTI_POSSESSIVE_OUTPUT), "'"));
 }
 
 if ((base->flags & MULTI_TO_MINISTERS) && !(flags & MULTI_NO_MINISTERS) &&
     ((base->flags & MULTI_AM_MINISTER) || (flags & MULTI_USE_EMPTY_GROUPS)))
 {
  if (*buffer)
    size += sprintf(name_array + size, ADD_COMMA_FRONT(size, "%s"), buffer);

  sprintf(buffer, "the ministers%s",
          USE_STRING((flags & MULTI_POSSESSIVE_OUTPUT), "'"));
 }
 
 if ((base->flags & MULTI_TO_NEWBIES) && !(flags & MULTI_NO_NEWBIES) &&
     ((base->flags & MULTI_AM_NEWBIE) || (flags & MULTI_USE_EMPTY_GROUPS)))
 {
  if (*buffer)
    size += sprintf(name_array + size, ADD_COMMA_FRONT(size, "%s"), buffer);

  sprintf(buffer, "the newbies%s",
          USE_STRING((flags & MULTI_POSSESSIVE_OUTPUT), "'"));
 }
 
 return (size);
}

const char *multi_get_names(unsigned int multi_number, multi_node *entry,
			    player_tree_node *avoid,
                            multi_node *friends, multi_node *friends_of,
                            int flags)
{
 static char name_array[MAX_NAMES_IN_MULTI_STRING];
 int array_size = 0;
 multi_base *base = NULL; 

 BTRACE("multi_get_names");
 base = find_base_entry(multi_number);

 assert(MALLOC_VALID(base, sizeof(multi_base), MULTI_BASE));
 if (!base)
   return ("** NO BASE NODE **");

 if (!entry || (flags & MULTI_NO_MULTIS) ||
     !multi_should_see(entry, base, NULL))
 {
  /* sees the names... */
  if ((base->flags & MULTI_TO_EVERYONE) && !(flags & MULTI_NO_EVERYONE))
    CONST_COPY_STR_LEN(name_array, "everyone");
  else
  {
   multi_node *tmp = find_first_node(multi_number);
   char the_last_name[PLAYER_S_NAME_SZ + 20] = "";
   const char *last_name = NULL;
   int mask_names = 0;
   
   array_size = get_names_groups(name_array, array_size, multi_number,
                                 entry, base, flags,
                                 friends, friends_of, the_last_name);
   BTRACE("get_names_groups (finnished)");
   
   if (*the_last_name)
     last_name = the_last_name;
   
   if (flags & MULTI_MASK_NAMES)
   {
    if (flags & MULTI_NO_NAMES) mask_names |= MULTI_AM_PERSON;
    if (flags & MULTI_NO_MINISTERS) mask_names |= MULTI_AM_MINISTER;
    if (flags & MULTI_NO_SUS) mask_names |= MULTI_AM_SUPER;
    if (flags & MULTI_NO_NEWBIES) mask_names |= MULTI_AM_NEWBIE;
    if (flags & MULTI_NO_FRIENDS) mask_names |= MULTI_AM_FRIEND;
    if (flags & MULTI_NO_FRIENDS_OF) mask_names |= MULTI_AM_FRIEND_OF;
    if (flags & MULTI_NO_ROOM) mask_names |= MULTI_AM_ROOM;
    if (flags & MULTI_NO_EVERYONE) mask_names |= MULTI_AM_EVERYONE;
    
    mask_names = ~mask_names;
   }
   
   while (tmp)
   {
    int personal_flags = (tmp->flags & MULTI_GROUP_FLAGS_AM);
    if ((tmp->flags & MULTI_THIS_TIME) && (tmp->parent != avoid) &&
        (P_IS_ON(tmp->parent) || (flags & MULTI_SHOW_ALL)) &&
        /* trust in the flags luke */
        ((flags & MULTI_ALL_NAMES) ? TRUE :
         (flags & (MULTI_MASK_NAMES | MULTI_SPEC_NAMES)) ?
         ((flags & MULTI_MASK_NAMES) &&
          !(personal_flags & mask_names)) ||
         ((flags & MULTI_SPEC_NAMES) &&
          (personal_flags & MULTI_AM_PERSON)) :
         !(personal_flags & ~MULTI_AM_PERSON)))
    {
     if (last_name)
     {
      if ((array_size + PLAYER_S_NAME_SZ + 4) < MAX_NAMES_IN_MULTI_STRING)
        array_size += sprintf(name_array + array_size,
                              ADD_COMMA_FRONT(array_size, "%s"),
                              last_name);
      else
        if (flags & MULTI_POSSESSIVE_OUTPUT)
          return ("some people's");
        else
          return ("some people");
     }
     
     if (tmp == entry)
     {
      if (flags & MULTI_POSSESSIVE_OUTPUT)
        last_name = "your";
      else
        last_name = "you";
     }
     else
       if (P_IS_AVL(tmp->parent) &&
           (tmp->parent->player_ptr->gender == GENDER_PLURAL))
       {
        sprintf(the_last_name, "the %s%s", tmp->parent->name,
                USE_STRING((flags & MULTI_POSSESSIVE_OUTPUT), "'"));
        last_name = the_last_name;
       }
       else
         if (flags & MULTI_POSSESSIVE_OUTPUT)
         {
          sprintf(the_last_name, "%s's", tmp->parent->name);
          last_name = the_last_name;
         }
         else
           last_name = tmp->parent->name;
    }
    
    tmp = tmp->next_node;
   }
   
   /* make the end look nice */
   if (last_name)
     if ((array_size + PLAYER_S_NAME_SZ + 7) < MAX_NAMES_IN_MULTI_STRING)
       array_size += sprintf(name_array + array_size,
                             ((array_size) ? " and %s%s" : "%s%s"),
                             last_name,
                             USE_STRING((flags & MULTI_POSSESSIVE_OUTPUT),
                                        "'s"));

     else
        return ("some people");
   else
     return ("????no-one????");
  }
 }
 else /* they should get the multi number */
   return ("the multi");
 
 return (name_array);
}

static void multis_flag_all(player_tree_node *current, int flags, int addit)
{
 multi_node *tmp = current->multis_start; 

 while (tmp)
 {
  if (addit)
    tmp->flags |= flags;
  else
    tmp->flags &= ~flags;

  tmp = tmp->next_multi;
 }
}

void init_multis(void)
{
 return;
}

void multis_init_for_player(player_tree_node *logging_on)
{
 assert(logging_on && P_IS_ON(logging_on));
 
 logging_on->player_ptr->multi_last_used = 0; 

 multis_flag_all(logging_on, MULTI_BLOCKED, TRUE);
}

void multis_destroy_for_player(player_tree_node *current)
{
 multi_node *tmp = current->multis_start;

 while (tmp)
 {
  multi_node *tmp_next = tmp->next_multi;
  multi_remove_from(current, 0, tmp);
  tmp = tmp_next;
 }

 assert(!current->multis_start);
}

unsigned int multi_count_players(unsigned int multi_number)
{
 multi_base *base = find_base_entry(multi_number);
 
 if (base)
   return (base->total_players);
 else 
   return (0); 
}





/* user commands */







static void user_unblock_a_multi(player *p, const char *str)
{
 unsigned int multi_number = atol(str);
 multi_node *entry = NULL;

 if (!*str)
   TELL_FORMAT(p, "<number>");
 
 if ((entry = multi_find_entry(p->saved, multi_number) ))
 {
  entry->flags &= ~MULTI_TMP_BLOCK;
  
  fvtell_player(NORMAL_T(p), " You have unblocked multi %d.\n", multi_number);
 }
 else
   fvtell_player(NORMAL_T(p), " You are not on multi %d.\n", multi_number);
}

static void user_block_a_multi(player *p, const char *str)
{
 unsigned int multi_number = atol(str);
 multi_node *entry = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<number>");

 if ((entry = multi_find_entry(p->saved, multi_number) ))
  {
   entry->flags |= MULTI_TMP_BLOCK;

   fvtell_player(NORMAL_T(p), " You have blocked multi %d.\n", multi_number);
  }
  else
   fvtell_player(NORMAL_T(p), " You are not on multi %d.\n", multi_number);  
}

static void user_start_the_multi(player *p, const char *str)
{
 multi_node *entry = NULL;
 unsigned int multi_number = atol(str);

 if (!*str)
   TELL_FORMAT(p, "<number>");

 if ((entry = multi_find_entry(p->saved, multi_number)))
   if (multi_start(entry, multi_number))
     fvtell_player(NORMAL_T(p), " You have started multi %u.\n", multi_number);
   else
     fvtell_player(NORMAL_T(p), 
                   " You can not start multi %u.\n", multi_number);
 else
   fvtell_player(NORMAL_T(p), " You are not on that multi\n");
}

static void user_stop_the_multi(player *p, const char *str)
{
 multi_node *entry = NULL;
 unsigned int multi_number = atol(str);
 
 if (!*str)
   TELL_FORMAT(p, "<number>");
 
 if ((entry = multi_find_entry(p->saved, multi_number)))
   if (multi_stop(entry, multi_number))
     fvtell_player(NORMAL_T(p), " You have stopped multi %u.\n", multi_number);
   else
     fvtell_player(NORMAL_T(p), " You can not stop multi %u.\n", multi_number);
 else
   fvtell_player(NORMAL_T(p), " You are not on that multi.\n");
}

static void multi_tell_identifiers(player *p,
                                   multi_node *entry, multi_base *base)
{
 int done = FALSE;
 
 if (base && (base->flags & MULTI_STOPPED))
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"), "Stopped");
  done = TRUE;
 }
   
 if (entry->flags & MULTI_OWNER)
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"), "Owner");
  done = TRUE;
 }
 
 if (entry->flags & MULTI_BLOCKED)
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"), "Blocked");
  done = TRUE;
 }
 
 if (entry->flags & MULTI_TMP_BLOCK)
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"), "Tmp Blocked");
  done = TRUE;
 }
 
 if (entry->flags & MULTI_AM_SELF)
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"), "Self");
  done = TRUE;
 }
 
 if (entry->flags & MULTI_TO_FRIENDS)
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"),
                "has ^S^BFriends^s on");
  done = TRUE;
 }
 
 if (entry->flags & MULTI_AM_FRIEND)
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"), "Friend");
  done = TRUE;
 }
 
 if (entry->flags & MULTI_TO_FRIENDS_OF)
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"),
                "has ^S^BFriends^s on");
  done = TRUE;
 }
 
 if (entry->flags & MULTI_AM_FRIEND_OF)
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"), "Friend of");
  done = TRUE;
 }
 
 if ((entry->flags & MULTI_AM_NEWBIE) && PRIV_STAFF(p->saved))
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"), "Newbie");
  done = TRUE;
 }
 
 if (entry->flags & MULTI_AM_EVERYONE)
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"), "Everyone");
  done = TRUE;
 }
 
 if (entry->flags & MULTI_AM_SUPER)
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"), "Super");
  done = TRUE;
 }
 
 if (entry->flags & MULTI_AM_MINISTER)
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"), "Minister");
  done = TRUE;
 }
 
 if (entry->flags & MULTI_AM_ROOM)
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"), "Room");
  done = TRUE;
 }
}

static int inorder_list_multi_for_player(multi_node *entry, va_list va)
{
 int *on_multi = va_arg(va, int *);
 multi_node *avoid = va_arg(va, multi_node *);
 player *p = avoid->parent->player_ptr;
 
 fvtell_player(NORMAL_T(p), " %-*s - ", PLAYER_S_NAME_SZ, entry->parent->name);
 multi_tell_identifiers(p, entry, NULL);
 fvtell_player(NORMAL_T(p), "%s", "\n");
    
 ++*on_multi;
 
 return (TRUE);
}

static void user_list_multis_for_player(player *p, const char *str)
{
 unsigned int multi_number = 0;
 multi_node *avoid = p->saved->multis_start;
 int on_multi = 0;

 if (*str)
 {
  multi_number = atol(str);
  if ((avoid = multi_find_entry(p->saved, multi_number)) &&
      multi_should_see(avoid, find_base_entry(multi_number), 0))
  {
   char buffer[sizeof("People on multi (%u)") +
              BUF_NUM_TYPE_SZ(unsigned int)];
   sprintf(buffer, "People on multi (%u)", multi_number);
   ptell_mid(NORMAL_T(p), buffer, FALSE);
   
   do_inorder_multi(inorder_list_multi_for_player, multi_number,
                    (p->saved->priv_spod ? MULTI_SHOW_ALL : 0 ),
                    &on_multi, avoid);
   
   fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
  }
  else
    fvtell_player(NORMAL_T(p), "%s", " You are not on that multi.\n");
 }
 else
 { /* list multi numbers */
  while (avoid)
  {
   multi_base *base = find_base_entry(avoid->number); /* must work */
   
   if (multi_should_see(avoid, base, NULL))  
   { /* make sure there are other ppl on */
    if (!on_multi)
      ptell_mid(NORMAL_T(p), "on multis", FALSE);
    
    fvtell_player(NORMAL_T(p), "% 4d ", avoid->number);
    multi_tell_identifiers(p, avoid, base);
    fvtell_player(NORMAL_T(p), "%s", "\n");
    
    ++on_multi;
   }

   avoid = avoid->next_multi;
  }
  
  if (on_multi)
    fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
  else
    fvtell_player(NORMAL_T(p), "%s", " You are not on any multis.\n");
 }
}

static void user_remove_yourself_from_multi(player *p, const char *str)
{
 unsigned int multi_number = atol(str);

 if (!*str)
   TELL_FORMAT(p, "<number>");
 
 if (multi_remove_from(p->saved, multi_number, 0))
   fvtell_player(NORMAL_T(p), 
                 " You are no longer on multi %d.\n", multi_number);
 else
   fvtell_player(NORMAL_T(p), " You are not on multi %d.\n", multi_number);
}

void cmds_init_multi_base(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("kill_multi", user_block_a_multi, CONST_CHARS, MULTI);
 CMDS_ADD("list_multis", user_list_multis_for_player, CONST_CHARS, MULTI);
 CMDS_ADD("lock_multi", user_stop_the_multi, CONST_CHARS, MULTI);
 CMDS_ADD("remove_multi", user_remove_yourself_from_multi, CONST_CHARS, MULTI);

 CMDS_ADD("unkill_multi", user_unblock_a_multi, CONST_CHARS, MULTI);
 CMDS_ADD("unlock_multi", user_start_the_multi, CONST_CHARS, MULTI);
}
