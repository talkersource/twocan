#define PLAYER_FIND_C
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

static int (*player_find_msg_func[PLAYER_FIND_MSG_ARR_SZ])(int, player *,
                                                           const char *,
                                                           const char *);
static unsigned int player_find_msg_count = 0;

static int player_find_msg_default(int type, player *p,
                                   const char *name, const char *player_name)
{
 switch (type)
 {
  case PLAYER_FIND_MSG_TYPE_STR_TOO_LONG:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- is too long for a "
                  "players name.\n", name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_SELF:
    fvtell_player(SYSTEM_T(p),
                  " You typed in ^S^Byour name^s and you can't "
                  "find yourself.\n");
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_NICKNAME_SELF:
    fvtell_player(SYSTEM_T(p),
                  " You have the nickname -- ^S^B%s^s -- "
                  "and you can't find yourself.\n",
                  name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_LOCAL_NICKNAME:
    fvtell_player(SYSTEM_T(p),
                  " The player -- ^S^B%s^s -- with the nickname"
                  " -- ^S^B%s^s -- is not in this room.\n",
                  player_name, name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_ON_NICKNAME:
    fvtell_player(SYSTEM_T(p),
                  " The player -- ^S^B%s^s -- with the nickname"
                  " -- ^S^B%s^s -- is not logged on.\n",
                  player_name, name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_LOCAL_MATCH_START:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match the"
                  " start of any other players name who is in this room.\n",
                  name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_ON_MATCH_START:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match the"
                  " start of any players' name who is logged on.\n",
                  name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_ON_RESIDENT_MATCH_START:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match the"
                  " start of any residents' name who is logged on.\n",
                  name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_ON_OTHER_MATCH_START:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match"
                  " another players' name who is logged on.\n",
                  name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_ON_OTHER_RESIDENT_MATCH_START:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match the"
                  " start of any other residents' name who is logged on.\n",
                  name);
  case PLAYER_FIND_MSG_TYPE_NO_ON_MATCH:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match"
                  " any players' name who is logged on.\n",
                  name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_ON_RESIDENT_MATCH:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match"
                  " any residents' name who is logged on.\n",
                  name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_MATCH_START:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match the"
                  " start of any players' name.\n",
                  name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_RESIDENT_MATCH_START:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match the"
                  " start of any residents' name.\n",
                  name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_OTHER_MATCH_START:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match the"
                  " start of any other players' name.\n",
                  name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_OTHER_RESIDENT_MATCH_START:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match the"
                  " start of any other residents' name.\n",
                  name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_MATCH:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match"
                  " any players' name.\n",
                  name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_NO_RESIDENT_MATCH:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match"
                  " any residents' name.\n",
                  name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_BANISHED:
    if (p->saved->priv_pretend_su)
      fvtell_player(SYSTEM_T(p),
                    " The player -- ^S^B%s^s -- is banished.\n", name);
    else
      fvtell_player(SYSTEM_T(p),
                    " The player name -- ^S^B%s^s -- is not valid.\n", name);
    return (TRUE);
  case PLAYER_FIND_MSG_TYPE_SYSTEM_CHARACTER:
    if (p->saved->priv_pretend_su)
      fvtell_player(SYSTEM_T(p),
                    " The player name -- ^S^B%s^s -- is a system"
                    " character.\n", name);
    else
      fvtell_player(SYSTEM_T(p),
                    " The player name -- ^S^B%s^s -- is not valid.\n", name);
    return (TRUE);
    
  default:
    break;
 }

 return (FALSE);
}

unsigned int player_find_msg_add(int (*func)(int, player *,
                                             const char *, const char *))
{
 if (player_find_msg_count >= PLAYER_FIND_MSG_ARR_SZ)
   return (0);
 ++player_find_msg_count;

 player_find_msg_func[player_find_msg_count - 1] = func;

 return (player_find_msg_count);
}

void player_find_msg_del(unsigned int offset)
{
 if (player_find_msg_count < offset)
   return;

 if (!offset)
 {
  assert(FALSE);
  return;
 }
 
 player_find_msg_count = offset - 1;
}

static void player_find_msg_run(int type, player *p,
                                const char *name, const char *player_name)
{
 unsigned int tmp = player_find_msg_count;

 if (!tmp)
 {
  assert(FALSE); /* should always have the default player_find_msg_func */

  init_player_find();
 }

 do
 {
  --tmp;

  assert(type && p && name);
  if (player_find_msg_func[tmp](type, p, name, player_name))
    return;
  
 } while (tmp);
}

static int construct_find_local_name_list_do(player *scan, va_list ap)
{
 const char *name = va_arg(ap, const char *);
 int *count = va_arg(ap, int *);
 player **tmp = va_arg(ap, player **);
 player *requester = va_arg(ap, player *);
 int flags = va_arg(ap, int);

 if (!(flags & PLAYER_FIND_EXPAND))
 {
  if (!strcmp(name, scan->saved->lower_name))
  {
   *tmp = scan;
   *count = 1;
   return (FALSE);
  }
  return (TRUE);
 }
 
 if (!beg_strcmp(name, scan->saved->lower_name))
 {
  if (!(flags & PLAYER_FIND_SELF) && (scan == requester))
    return (TRUE);

  switch (*count)
  {
   case 0:
     assert(!*tmp);
     
     if (!strcmp(name, scan->saved->lower_name))
     {
      assert(requester->saved != scan->saved);
      *tmp = scan;
      *count = 1;
      return (FALSE);
     }
     
     if (flags & PLAYER_FIND_PICK_FIRST)
     {
      *tmp = scan;
      *count = 1;
      
      return (FALSE);
     }
     
     if (!requester)
       return (FALSE);
     break;

     /* because it's in order it will only be a complete match on case 0 */
   case 1:
     if (requester && (flags & PLAYER_FIND_VERBOSE))
       fvtell_player(SYSTEM_T(requester),
                     " ^S^BMultiple matches^s: %s", (*tmp)->saved->name);
     break;
     
   default:
     if (requester && (flags & PLAYER_FIND_VERBOSE))
       fvtell_player(SYSTEM_T(requester), ", %s", scan->saved->name);
  }
  
  *tmp = scan;
  ++*count;
 }
 
 return (TRUE);
}

player *player_find_local(player *requester, const char *name, int flags)
{
 char lowered_name[PLAYER_S_NAME_SZ];
 player_tree_node *nicknamed_player = NULL;
 int count = 0;
 player *tmp = NULL;

 COPY_STR(lowered_name, name, PLAYER_S_NAME_SZ);

 lower_case(lowered_name);

 assert(flags & PLAYER_FIND_NEWBIES);
 flags |= PLAYER_FIND_NEWBIES;
 
 assert(requester || !(flags & (PLAYER_FIND_VERBOSE | PLAYER_FIND_NICKS |
                                PLAYER_FIND_SELF)));

 if (strcasecmp(lowered_name, name))
 {
  if (flags & PLAYER_FIND_VERBOSE)
    player_find_msg_run(PLAYER_FIND_MSG_TYPE_STR_TOO_LONG,
                        requester, name, NULL);
  return (NULL);
 }
 
 if (!strcmp("me", lowered_name) ||
     (requester && requester->saved &&
      !strcmp(requester->saved->lower_name, lowered_name)))
 {
  if (!(flags & PLAYER_FIND_SELF))
  {
   if (flags & PLAYER_FIND_VERBOSE)
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_SELF, requester, name, NULL);
   
   return (NULL);
  }
  
  return (requester);
 }

 if ((flags & PLAYER_FIND_NICKS) && requester)
   if ((nicknamed_player = nickname_player_tree_find(requester, lowered_name)))
   {
    if (P_IS_ON(nicknamed_player) &&
        ((flags & PLAYER_FIND_SELF) || !requester ||
         (requester->saved != nicknamed_player)) &&
        (nicknamed_player->player_ptr->location == requester->location))
      return (nicknamed_player->player_ptr);

    if (flags & PLAYER_FIND_VERBOSE)
    {
     if (requester->saved == nicknamed_player)
       player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_NICKNAME_SELF,
                           requester, name, nicknamed_player->name);
     else
       player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_LOCAL_NICKNAME,
                           requester, name, nicknamed_player->name);
    }
    
    return (NULL);
   }

 do_inorder_room(construct_find_local_name_list_do, requester->location,
                 lowered_name, &count, &tmp, requester, flags);

 if (!count)
 {
  if (requester && (flags & PLAYER_FIND_VERBOSE))
    player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_LOCAL_MATCH_START,
                        requester, name, NULL);

  return (NULL);
 }

 if (count == 1)
   return (tmp);
 
 assert(count > 1);
 assert(requester); 

 if (requester && (flags & PLAYER_FIND_VERBOSE))
   fvtell_player(SYSTEM_T(requester), " and %s.\n", tmp->saved->name);

 return (NULL);
}

/* returns the number of nodes to skip */
static int next_match(player_linked_list *node, const char *name)
{
 if (node && PLAYER_LINK_NEXT(node) &&
     !beg_strcmp(name,
                PLAYER_LINK_SAV_GET(PLAYER_LINK_NEXT(node))->lower_name))
   return (1); /* 0, 1, 2 */
 else
   return (0);
}

static int next_match_skip(player *p, player_linked_list *node,
                           const char *name, int no_self)
{
 if (no_self && p && p->saved &&
     node && PLAYER_LINK_NEXT(node))
   if (PLAYER_LINK_SAV_GET(PLAYER_LINK_NEXT(node)) == p->saved)
     /* tell it to skip one */
     return (next_match(PLAYER_LINK_NEXT(node), name) ? (2) : (0));
 
 return (next_match(node, name));
}

player *player_find_on(player *requester, const char *name, int flags)
{
 char lowered_name[PLAYER_S_NAME_SZ];
 player_tree_node *nicknamed_player = NULL;
 
 COPY_STR(lowered_name, name, PLAYER_S_NAME_SZ);

 lower_case(lowered_name);

 assert(requester || !(flags & (PLAYER_FIND_VERBOSE | PLAYER_FIND_NICKS |
                                PLAYER_FIND_SELF)));

 if (strcasecmp(lowered_name, name))
 {
  if (flags & PLAYER_FIND_VERBOSE)
    player_find_msg_run(PLAYER_FIND_MSG_TYPE_STR_TOO_LONG,
                        requester, name, NULL);
  return (NULL);
 }
 
 if (!strcmp("me", lowered_name) ||
     (requester && requester->saved &&
      !strcmp(requester->saved->lower_name, lowered_name)))
 {
  if (!(flags & PLAYER_FIND_SELF))
  {
   if (flags & PLAYER_FIND_VERBOSE)
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_SELF, requester, name, NULL);
   
   return (NULL);
  }
  
  return (requester);
 }
 
 if ((flags & PLAYER_FIND_NICKS) &&
     (nicknamed_player = nickname_player_tree_find(requester, lowered_name)))
 {
  if (P_IS_ON(nicknamed_player) &&
      ((flags & PLAYER_FIND_SELF) || !requester ||
       (requester->saved != nicknamed_player)))
    return (nicknamed_player->player_ptr);
  
  if (flags & PLAYER_FIND_VERBOSE)
  {
   if (requester->saved == nicknamed_player)
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_NICKNAME_SELF,
                         requester, name, nicknamed_player->name);
   else
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_ON_NICKNAME,
                         requester, name, nicknamed_player->name);
  }
  
  return (NULL);
 }

 if (flags & PLAYER_FIND_EXPAND)
 {
  int saved_cmp;
  player_linked_list *scan = NULL;
  int skip = 0;
  player_tree_node *tmp = player_tree_find(lowered_name);
  
  while (tmp && !((saved_cmp = beg_strcmp(lowered_name, tmp->lower_name)) ||
                  P_IS_ON(tmp)))
  {
   assert(MALLOC_VALID(tmp, sizeof(player_tree_node), PLAYER_TREE_NODE));
   tmp = tmp->next;
  }
  if (tmp && !P_IS_ON(tmp))
    tmp = NULL;

  /* this code "prefers" residents to newbies ... that is a feature */
  if ((flags & PLAYER_FIND_NEWBIES) &&
      (!tmp || strcmp(lowered_name, tmp->lower_name)))
  {
   player_tree_node *new_tmp = NULL;

   if (!tmp)
     new_tmp = player_newbie_find(lowered_name);
   else
     new_tmp = player_newbie_find_exact(lowered_name);
   
   if (new_tmp)
     tmp = new_tmp;
  }

  if (!tmp)
  {
   if (flags & PLAYER_FIND_VERBOSE)
   {
    if (flags & PLAYER_FIND_NEWBIES)
      player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_ON_MATCH_START,
                          requester, name, NULL);
    else
      player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_ON_RESIDENT_MATCH_START,
                          requester, name, NULL);
   }
   
   return (NULL);
  }
  
  assert(MALLOC_VALID(tmp, sizeof(player_tree_node), PLAYER_TREE_NODE));
  
  if (((flags & PLAYER_FIND_PICK_FIRST) ||
       !strcmp(lowered_name, tmp->lower_name)) &&
      ((flags & PLAYER_FIND_SELF) || (requester->saved != tmp)))
    return (tmp->player_ptr);

  flags |= PLAYER_FIND_NEWBIES;
  scan = &tmp->player_ptr->logon_list_alpha.s;
  
  if (!(flags & PLAYER_FIND_SELF) && (requester->saved == tmp))
  {
   if (!next_match(scan, lowered_name))
   {
    if (flags & PLAYER_FIND_VERBOSE)
    {
     if (flags & PLAYER_FIND_NEWBIES) /* always true */
       player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_ON_OTHER_MATCH_START,
                           requester, name, NULL);
     else
       player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_ON_OTHER_RESIDENT_MATCH_START,
                           requester, name, NULL);
    }
    
    return (NULL);
   }
   scan = PLAYER_LINK_NEXT(scan);
  }
  
  if (!(skip = next_match_skip(requester, scan, lowered_name,
                               !(flags & PLAYER_FIND_SELF))))
    return (PLAYER_LINK_GET(scan));

  if (!(flags & PLAYER_FIND_VERBOSE))
    return (NULL);
  
  fvtell_player(SYSTEM_T(requester), "%s", " Multiple matches: ");

  do
  {
   fvtell_player(SYSTEM_T(requester), "%s, ",
                 PLAYER_LINK_SAV_GET(scan)->name);
   
   if (skip == 2)
     scan = PLAYER_LINK_NEXT(scan);
   
   scan = PLAYER_LINK_NEXT(scan);
  } while ((skip = next_match_skip(requester, scan, lowered_name,
                                   !(flags & PLAYER_FIND_SELF))));
  
  fvtell_player(SYSTEM_T(requester), "and %s.\n",
		PLAYER_LINK_SAV_GET(scan)->name);
 }
 else
 {
  player_tree_node *tmp = NULL;
  
  if ((tmp = player_tree_find_exact(lowered_name)) && P_IS_ON(tmp))
  {
   if (!requester || (requester->saved != tmp))
     return (tmp->player_ptr);

   assert(!(flags & PLAYER_FIND_SELF));
   
   if (flags & PLAYER_FIND_VERBOSE)
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_SELF, requester, name, NULL);

   return (NULL);
  }
  else if ((flags & PLAYER_FIND_NEWBIES) &&
           (tmp = player_newbie_find_exact(lowered_name)))
  {   
   if (!requester || (requester->saved != tmp))
     return (tmp->player_ptr);

   assert(!(flags & PLAYER_FIND_SELF));
   
   if (flags & PLAYER_FIND_VERBOSE)
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_SELF, requester, name, NULL);
   
   return (NULL);
  }
  
  if (flags & PLAYER_FIND_VERBOSE)
  {
   if (flags & PLAYER_FIND_NEWBIES)
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_ON_MATCH,
                         requester, name, NULL);
   else
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_ON_RESIDENT_MATCH,
                         requester, name, NULL);
  }
 }

 return (NULL);
}

player_tree_node *player_find_all(player *requester, const char *name,
                                  int flags)
{
 char lowered_name[PLAYER_S_NAME_SZ];
 player_tree_node *tmp = NULL;
 
 COPY_STR(lowered_name, name, PLAYER_S_NAME_SZ);

 lower_case(lowered_name);

 assert(requester || !(flags & (PLAYER_FIND_VERBOSE | PLAYER_FIND_NICKS |
                                PLAYER_FIND_SELF)));

 if (strcasecmp(lowered_name, name))
 {
  if (flags & PLAYER_FIND_VERBOSE)
    player_find_msg_run(PLAYER_FIND_MSG_TYPE_STR_TOO_LONG,
                        requester, name, NULL);
  
  return (NULL);
 }
 
 flags |= PLAYER_FIND_PICK_FIRST;
 
 if (!strcmp("me", lowered_name) ||
     (requester && requester->saved &&
      !strcmp(requester->saved->lower_name, lowered_name)))
 {
  if (!(flags & PLAYER_FIND_SELF))
  {
   if (flags & PLAYER_FIND_VERBOSE)
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_SELF,
                         requester, name, NULL);
   
   return (NULL);
  }
  
  return (requester->saved);
 }
 
 if ((flags & PLAYER_FIND_NICKS) &&
     (tmp = nickname_player_tree_find(requester, lowered_name)))
 {
  if ((flags & PLAYER_FIND_SELF) || !requester || (requester->saved != tmp))
    return (tmp);
  
  if (flags & PLAYER_FIND_VERBOSE)
    player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_NICKNAME_SELF,
                        requester, name, NULL);
  
  return (NULL);
 }

 if (flags & PLAYER_FIND_EXPAND)
 {
  if ((tmp = player_tree_find(lowered_name)))
  {
   if (flags & PLAYER_FIND_SELF)
     return (tmp);

   if (!requester || (requester->saved != tmp))
     return (tmp);
  }

  if ((flags & PLAYER_FIND_NEWBIES) &&
      (tmp = player_newbie_find(lowered_name)))
    return (tmp);
  else if (flags & PLAYER_FIND_VERBOSE)
  {
   if (flags & PLAYER_FIND_SELF)
   {
    if (flags & PLAYER_FIND_NEWBIES)
      player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_MATCH_START,
                         requester, name, NULL);
    else
      player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_RESIDENT_MATCH_START,
                         requester, name, NULL);
   }
   else if (flags & PLAYER_FIND_NEWBIES)
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_OTHER_MATCH_START,
                         requester, name, NULL);
   else
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_OTHER_RESIDENT_MATCH_START,
                         requester, name, NULL);
  }
 }
 else
 {
  if ((tmp = player_tree_find_exact(lowered_name)))
  {
   if (!requester || (requester->saved != tmp))
     return (tmp);

   assert(!(flags & PLAYER_FIND_SELF));
   
   if (flags & PLAYER_FIND_VERBOSE)
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_SELF, requester, name, NULL);
   
   return (NULL);
  }
  else if ((flags & PLAYER_FIND_NEWBIES) &&
           (tmp = player_newbie_find_exact(lowered_name)))
  {
   if (!requester || (requester->saved != tmp))
     return (tmp);

   assert(!(flags & PLAYER_FIND_SELF));
   
   if (flags & PLAYER_FIND_VERBOSE)
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_SELF, requester, name, NULL);
   
   return (NULL);
  }

  if (flags & PLAYER_FIND_VERBOSE)
  {
   if (flags & PLAYER_FIND_NEWBIES)
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_MATCH, requester, name, NULL);
   else
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_NO_RESIDENT_MATCH,
                         requester, name, NULL);
  }
 }

 return (NULL);
}

player *player_find_load(player *requester, const char *name, int flags)
{
 player_tree_node *tmp = NULL;

 log_assert(requester && requester->saved && name);

 if (!(tmp = player_find_all(requester, name, flags)))
   return (NULL);
 
 if (PRIV_SYSTEM_ROOM(tmp) ||
     (tmp->priv_banished && !(flags & PLAYER_FIND_BANISHED)))
 {
  if (tmp->priv_banished)
  {
   if (flags & PLAYER_FIND_VERBOSE)
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_BANISHED,
                         requester, name, NULL);
  }
  else
  {
   if (flags & PLAYER_FIND_VERBOSE)
     player_find_msg_run(PLAYER_FIND_MSG_TYPE_SYSTEM_CHARACTER,
                         requester, name, NULL);
  }

  return (NULL);
 }
 
 if (P_IS_AVL(tmp))
 {
  tmp->a_timestamp = now;
  return (tmp->player_ptr); /* previous load -- or newbie -- or logged on */
 }
 else
 {
  if (player_load(tmp))
  {
   assert(P_IS_AVL(tmp));
   return (tmp->player_ptr);
  }
  else
  {
   log_assert(FALSE); /* should have been picked up above */
   if (requester && (flags & PLAYER_FIND_VERBOSE))
     fvtell_player(SYSTEM_T(requester),
                   " The player name -- ^S^B%s^s -- is not valid.\n", name);
  }
 }
 
 return (NULL);
}

void init_player_find(void)
{
 int tmp = 0;
 
 player_find_msg_del(1);
 tmp = player_find_msg_add(player_find_msg_default);

 IGNORE_PARAMETER(tmp);
 assert(tmp);
}
