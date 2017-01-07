#define PLAYER_TREE_C
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

/* for the tree */
static player_tree_node *tree_root = NULL;

/* for the resi linked list */
static player_tree_node *linked_list_start = NULL;

/* for the newbie linked list */
static player_tree_node *newbie_linked_list_start = NULL;

/* how many ppl are loged on who are newbies */
static int newbie_logged_on = 0;






/* code */

static player_tree_node *left_rotate(player_tree_node *ptr)
{
 int ptr_valid = MALLOC_VALID(ptr, sizeof(player_tree_node),
                              PLAYER_TREE_NODE);
 int ptr_right_valid = MALLOC_VALID(ptr->right, sizeof(player_tree_node),
                     PLAYER_TREE_NODE);
 
 assert(ptr_valid && ptr_right_valid);

 if (ptr_valid)
   if (ptr_right_valid)
   {
    player_tree_node *right_side = ptr->right;
    ptr->right = right_side->left;
    right_side->left = ptr;
    return (right_side);
   }
 
 assert(FALSE);
 return (NULL);
}

static player_tree_node *right_rotate(player_tree_node *ptr)
{
 int ptr_valid = MALLOC_VALID(ptr, sizeof(player_tree_node),
                              PLAYER_TREE_NODE);
 int ptr_left_valid = MALLOC_VALID(ptr->left, sizeof(player_tree_node),
                                   PLAYER_TREE_NODE);
 
 assert(ptr_valid && ptr_left_valid);
 
 if (ptr_valid)
   if (ptr_left_valid)
   {
    player_tree_node *left_side = ptr->left;
    ptr->left = left_side->right;
    left_side->right = ptr;
    return (left_side);
   }
 assert(FALSE);
 return (NULL);
}

static player_tree_node *insert_left_balance(player_tree_node *root,
                                             signed char *taller)
{
 player_tree_node *left_side = root->left;
 
 assert(MALLOC_VALID(root, sizeof(player_tree_node),
		     PLAYER_TREE_NODE));
 assert(MALLOC_VALID(left_side, sizeof(player_tree_node),
		     PLAYER_TREE_NODE));

 switch (left_side->balance)
 {
 case LEFT_HEAVY:
   root->balance = left_side->balance = EQUAL_HEIGHT;
   root = right_rotate(root);
   *taller = FALSE;
   break;
  
 case EQUAL_HEIGHT:
   assert(FALSE);
   break;
   
 case RIGHT_HEAVY:
 {
  player_tree_node *right_of_left_side = left_side->right;
   
  switch(right_of_left_side->balance)
  {
  case LEFT_HEAVY:
    root->balance = RIGHT_HEAVY;
    left_side->balance = EQUAL_HEIGHT;
    break;
    
  case EQUAL_HEIGHT:
    root->balance = left_side->balance = EQUAL_HEIGHT;
     break;
     
  case RIGHT_HEAVY:
    root->balance = EQUAL_HEIGHT;
    left_side->balance = LEFT_HEAVY;
    break;
    
  default:
    assert(FALSE);
  }
  right_of_left_side->balance = EQUAL_HEIGHT; /* new root */
  root->left = left_rotate(left_side);
  root = right_rotate(root);
  assert(root->balance == EQUAL_HEIGHT);
  *taller = FALSE;
 }
 break;
   
 default:
   assert(FALSE);
 }
 return (root);
}

static player_tree_node *insert_right_balance(player_tree_node *root,
                                              signed char *taller)
{
 player_tree_node *right_side = root->right;
 
 assert(MALLOC_VALID(root, sizeof(player_tree_node),
		     PLAYER_TREE_NODE));
 assert(MALLOC_VALID(right_side, sizeof(player_tree_node),
		     PLAYER_TREE_NODE));

 switch (right_side->balance)
 {
 case RIGHT_HEAVY:
   root->balance = right_side->balance = EQUAL_HEIGHT;
   root = left_rotate(root);
   *taller = FALSE;
   break;
   
 case EQUAL_HEIGHT: /* for removing */
   assert(FALSE);
   break;
   
 case LEFT_HEAVY:
 {
  player_tree_node *left_of_right_side = right_side->left;
  
  switch (left_of_right_side->balance)
  {
  case LEFT_HEAVY:
    root->balance = EQUAL_HEIGHT;
    right_side->balance = RIGHT_HEAVY;
    break;
    
  case EQUAL_HEIGHT:
    root->balance = right_side->balance = EQUAL_HEIGHT;
    break;
    
  case RIGHT_HEAVY:
    root->balance = LEFT_HEAVY;
    right_side->balance = EQUAL_HEIGHT;
    break;
    
  default:
    assert(FALSE);
  }
  left_of_right_side->balance = EQUAL_HEIGHT; /* new root */
  root->right = right_rotate(right_side);
  root = left_rotate(root);
  assert(root->balance == EQUAL_HEIGHT);
  *taller = FALSE;
 }
 break;
  
 default:
   assert(FALSE);
 }
 
 return (root);
}

static player_tree_node *insert_node(player_tree_node *root,
                                     player_tree_node *new_node,
                                     signed char *taller)
{
 int strcmp_ans;
  
 if (!root)
 {
  new_node->left = NULL;
  new_node->right = NULL;
  new_node->balance = EQUAL_HEIGHT;
  
  root = new_node;
  /* this sets the balance going */
  *taller = TRUE;
 }
 else
 {
  assert(MALLOC_VALID(root, sizeof(player_tree_node),
		      PLAYER_TREE_NODE));
  if (!(strcmp_ans = strcmp(new_node->lower_name, root->lower_name)))
  {
   /* already found the player.... problem */
   vwlog("player_tree", "tried to add player twice %s", new_node->lower_name);
   
   XFREE(new_node, PLAYER_TREE_NODE);
  }
  else
    if (strcmp_ans < 0)
    {
     root->left = insert_node(root->left, new_node, taller);
     
     if (*taller)
       switch (root->balance)
       {
      case LEFT_HEAVY:
	root = insert_left_balance(root, taller);
	break;
	
      case EQUAL_HEIGHT:
	root->balance = LEFT_HEAVY;
	break;
	
      case RIGHT_HEAVY:
	root->balance = EQUAL_HEIGHT;
	*taller = FALSE;
	break;
	
      default:
        assert(FALSE);
       }
    }
    else
    {
     root->right = insert_node(root->right, new_node, taller);
     
     if (*taller)
       switch (root->balance)
       {
        case RIGHT_HEAVY:
          root = insert_right_balance(root, taller);
          break;
          
        case EQUAL_HEIGHT:
          root->balance = RIGHT_HEAVY;
          break;
          
        case LEFT_HEAVY:
          root->balance = EQUAL_HEIGHT;
          *taller = FALSE;
          break;
          
        default:
          assert(FALSE);
       }
    }
 }
 return (root);
}

static player_tree_node *remove_left_balance(player_tree_node *root,
                                              signed char *shorter)
{
 player_tree_node *left_side = root->left;
  
 assert(MALLOC_VALID(root, sizeof(player_tree_node),
		     PLAYER_TREE_NODE));
 assert(MALLOC_VALID(left_side, sizeof(player_tree_node),
		     PLAYER_TREE_NODE));

 switch (left_side->balance)
 {
 case LEFT_HEAVY:
   root->balance = left_side->balance = EQUAL_HEIGHT;
   root = right_rotate(root);
   break;
    
 case EQUAL_HEIGHT:
   root->balance = LEFT_HEAVY;
   left_side->balance = RIGHT_HEAVY;
   root = right_rotate(root);
   *shorter = FALSE;
   break;
   
 case RIGHT_HEAVY:
 {
  player_tree_node *right_of_left_side = left_side->right;
   
  switch(right_of_left_side->balance)
  {
  case LEFT_HEAVY:
    root->balance = RIGHT_HEAVY;
    left_side->balance = EQUAL_HEIGHT;
    break;
    
  case EQUAL_HEIGHT:
    root->balance = left_side->balance = EQUAL_HEIGHT;
     break;
     
  case RIGHT_HEAVY:
    root->balance = EQUAL_HEIGHT;
    left_side->balance = LEFT_HEAVY;
    break;
    
  default:
    assert(FALSE);
  }
  right_of_left_side->balance = EQUAL_HEIGHT; /* new root */
  root->left = left_rotate(left_side);
  root = right_rotate(root);
  assert(root->balance == EQUAL_HEIGHT);
 }
 break;
   
 default:
   assert(FALSE);
 }
 return (root);
}

static player_tree_node *remove_right_balance(player_tree_node *root,
                                             signed char *shorter)
{
 player_tree_node *right_side = root->right;
  
 assert(MALLOC_VALID(root, sizeof(player_tree_node),
		     PLAYER_TREE_NODE));
 assert(MALLOC_VALID(right_side, sizeof(player_tree_node),
		     PLAYER_TREE_NODE));

 switch (right_side->balance)
 {
 case RIGHT_HEAVY:
   root->balance = right_side->balance = EQUAL_HEIGHT;
   root = left_rotate(root);
   break;

 case EQUAL_HEIGHT:
   root->balance = RIGHT_HEAVY;
   right_side->balance = LEFT_HEAVY;
   root = left_rotate(root);
   *shorter = FALSE;
   break;
    
 case LEFT_HEAVY:
 {
  player_tree_node *left_side_of_right = right_side->left;
  
  switch (left_side_of_right->balance)
   {
   case LEFT_HEAVY:
     root->balance = EQUAL_HEIGHT;
     right_side->balance = RIGHT_HEAVY;
     break;
     
   case EQUAL_HEIGHT:
     root->balance = right_side->balance = EQUAL_HEIGHT;
     break;
     
   case RIGHT_HEAVY:
     root->balance = LEFT_HEAVY;
     right_side->balance = EQUAL_HEIGHT;
     break;
     
   default:
     assert(FALSE);
   }
  left_side_of_right->balance = EQUAL_HEIGHT; /* new root */
  root->right = right_rotate(right_side);
  root = left_rotate(root);
  assert(root->balance == EQUAL_HEIGHT);
 }
 break;
   
  default:
    assert(FALSE);
 }
 return (root);
}

static player_tree_node *remove_node(player_tree_node *root,
				     player_tree_node *old_node,
				     signed char *shorter)
{
 player_tree_node *swap = NULL;
 int strcmp_ans;
  
 if (!root) /* can't find node to delete */
   log_assert(FALSE);
 else
   if (!(strcmp_ans = strcmp(old_node->lower_name, root->lower_name)))
   {
    *shorter = TRUE;

    /* found the player.... del. them */
    if (old_node->left)
      if (old_node->right)
      {
       assert(FALSE);
       return (swap);
      }
      else
	return (old_node->left);
    else
      return (old_node->right); /* can be zero */
   }
   else
     if (strcmp_ans > 0)
     {
      root->right = remove_node(root->right, old_node, shorter);
      
      if (*shorter)
	switch (root->balance)
	{
       case LEFT_HEAVY:
	 root = remove_left_balance(root, shorter);
	 break;
	 
       case EQUAL_HEIGHT:
	 root->balance = LEFT_HEAVY;
         *shorter = FALSE;
	 break;
	 
       case RIGHT_HEAVY:
	 root->balance = EQUAL_HEIGHT;
	 break;
	 
       default:
         assert(FALSE);
	}
     }
     else/* strcmp > 0 */
     {
      root->left = remove_node(root->left, old_node, shorter);
      
      if (*shorter)
	switch (root->balance)
	{
       case RIGHT_HEAVY:
         root = remove_right_balance(root, shorter);
         break;
	 
       case EQUAL_HEIGHT:
	 root->balance = RIGHT_HEAVY;
	 *shorter = FALSE;
	 break;
	 
       case LEFT_HEAVY:
	 root->balance = EQUAL_HEIGHT;
	 break;
	 
       default:
         assert(FALSE);
	}
     }

 return (root);
}

static player_tree_node *internal_find_previous(player_tree_node *root,
                                                player_tree_node *next_node,
                                                signed char *is_right)
{
 player_tree_node *current;
 int strcmp_ans = 0;
 
 assert(MALLOC_VALID(root, sizeof(player_tree_node), PLAYER_TREE_NODE));

 if (*is_right)
   current = root->right;
 else
   current = root->left;

 assert(MALLOC_VALID(current, sizeof(player_tree_node), PLAYER_TREE_NODE));
  
 if (!(strcmp_ans = strcmp(current->lower_name, next_node->lower_name)))
   return (root);
 else if (strcmp_ans < 0) /* if less than we go down right hand side of tree */
   *is_right = TRUE;
 else
   *is_right = FALSE;

 return (internal_find_previous(current, next_node, is_right));
}

static player_tree_node *find_previous(player_tree_node *next_node,
				       signed char *is_right)
{
 int strcmp_ans;
  
 if (!tree_root)
 {
  assert(FALSE);
 }  

 if (!(strcmp_ans = strcmp(tree_root->lower_name, next_node->lower_name)))
   return (NULL);/* root of tree */
 else if (strcmp_ans < 0) /* if less than we go down right hand side of tree */
   *is_right = TRUE;
 else
   *is_right = FALSE;
 
 return (internal_find_previous(tree_root, next_node, is_right));
}

void player_tree_del(player_tree_node *old_node)
{
 signed char shorter = FALSE;
 player *p = NULL;

 assert(MALLOC_VALID(old_node, sizeof(player_tree_node),
		     PLAYER_TREE_NODE));
 
 if ((p = old_node->player_ptr) && (p->saved))
 { /* they are logged on and so quit the player */
  log_assert(FALSE); /* this probably doesn't work :O */
  /* quit_player_resident(p); */
  p->saved = NULL;
 }

 if (old_node->left && old_node->right)
 { /* the node has BOTH ... so change it for another one */
  player_tree_node *swap = old_node->left;
  player_tree_node *parent_old_node = NULL;
  signed char is_right = FALSE;

  while (swap->right)
    swap = swap->right;

  /* swap only has one leaf */
  tree_root = remove_node(tree_root, swap, &shorter);
       
  swap->left = old_node->left;
  swap->right = old_node->right;
  swap->balance = old_node->balance;

  if ((parent_old_node = find_previous(old_node, &is_right)))
    if (is_right)
      parent_old_node->right = swap;
    else
      parent_old_node->left = swap;
  else
    tree_root = swap;
 }
 else
   tree_root = remove_node(tree_root, old_node, &shorter);

 if (old_node->next)
   old_node->next->prev = old_node->prev;

 if (old_node->prev)
   old_node->prev->next = old_node->next;
 else
   linked_list_start = old_node->next;

}

player_tree_node *player_tree_find_exact(const char *lower_name)
{
 int strcmp_ans;
 player_tree_node *current;

 assert(lower_name);
 
 if (!*lower_name)
   return (NULL); 

 current = tree_root;

 while (current)
   if (!(strcmp_ans = strcmp(lower_name, current->lower_name)))
     return (current); /* found */
   else
     if (strcmp_ans < 0)
       current = current->left;
     else
       current = current->right;
 
 return (NULL); /* failed */
}

player_tree_node *player_tree_find(const char *start_of_name)
{
 int strcmp_ans = 0;
 player_tree_node *current = tree_root;
 player_tree_node *tmp = NULL;

 assert(start_of_name);
 
 if (!*start_of_name)
   return (NULL);
 
 while (current)
 {
  if (!(strcmp_ans = beg_strcmp(start_of_name, current->lower_name)))
    tmp = current;
  
  if (strcmp_ans > 0)
    current = current->right;
  else
    current = current->left;
 }

 return (tmp); /* found lowest matching name, or nothing */
}

player_tree_node *player_newbie_start(void)
{ /* ohhh read only variables */
 return (newbie_linked_list_start);
}

player_tree_node *player_newbie_find_exact(const char *lower_name)
{ /* NOTE: no nicknames */
 player_tree_node *finder = newbie_linked_list_start;
 int saved_cmp = 0;
  
 assert(lower_name);
 
 if (!*lower_name)
   return (NULL);

 while (finder && ((saved_cmp = strcmp(lower_name, finder->lower_name)) > 0))
   finder = finder->next;

 if (finder && !saved_cmp)
   return (finder);
 else 
   return (NULL);
}

player_tree_node *player_newbie_find(const char *lower_name)
{ /* NOTE: no nicknames */
 player_tree_node *finder = newbie_linked_list_start;
 int saved_cmp = 0;
  
 assert(lower_name);
 
 if (!*lower_name)
   return (NULL);
  
 while (finder &&
	((saved_cmp = beg_strcmp(lower_name, finder->lower_name)) > 0) )
   finder = finder->next;
 
 if (finder && !saved_cmp)
   return (finder);
 else 
   return (NULL);
}

static player_tree_node *add_list_root_ptr(player_tree_node *current,
					   player_tree_node *top)
{
 player_tree_node *finder = top;
 int cmp_sve = 0;
 
 assert(MALLOC_VALID(current, sizeof(player_tree_node),
		     PLAYER_TREE_NODE));

 if (finder)
 {
  while (finder->next &&
	 (strcmp(finder->lower_name, current->lower_name) < 0))
    finder = finder->next;

  if ((cmp_sve = strcmp(finder->lower_name, current->lower_name)) > 0)
  {
   if ((current->prev = finder->prev))
     current->prev->next = current;
   else
     top = current;

   finder->prev = current;
   current->next = finder;
  }
  else
    if (cmp_sve < 0)
    {
     if ((current->next = finder->next))
       current->next->prev = current;
   
     finder->next = current;
     current->prev = finder;
    }
    else
    {
     assert(FALSE);
    }
 }
 else
 { /* finder == 0 */
  top = current;
  current->next = NULL;
  current->prev = NULL;
 }

 return (top);
}

void player_tree_add(player_tree_node *new_node)
{
 signed char taller = FALSE;

 tree_root = insert_node(tree_root, new_node, &taller);

 linked_list_start = add_list_root_ptr(new_node, linked_list_start);
}

void player_newbie_del(player_tree_node *togo)
{
 assert(MALLOC_VALID(togo, sizeof(player_tree_node), PLAYER_TREE_NODE));
 
 if (togo)
 {
  newbie_logged_on--;
  
  if (togo->next)
    togo->next->prev = togo->prev;
  
  if (togo->prev)
    togo->prev->next = togo->next;
  else
    newbie_linked_list_start = togo->next;
 }
}

int player_newbie_number(void)
{
 if (!newbie_linked_list_start && newbie_logged_on)
 {
  log_assert(FALSE);
  newbie_logged_on = 0;
 }
   
 return (newbie_logged_on);
}

void player_newbie_add(player_tree_node *newbie)
{
 newbie_logged_on++;
 
 newbie_linked_list_start =
   add_list_root_ptr(newbie, newbie_linked_list_start);
}

/* Doesn't use the tree.... so we can destroy from it */
void do_inorder_all(void (*command) (player_tree_node *, va_list), ...)
{
 player_tree_node *current = linked_list_start;
 VA_R_DECL(r_ap);
 
 VA_R_START(r_ap, command);

 while (current)
 {
  player_tree_node *current_next = current->next;
  VA_C_DECL(ap);
  
  VA_C_START(ap, command);
  
  VA_C_COPY(ap, r_ap);

  (*command) (current, ap);
  
  VA_C_END(ap);

  current = current_next;
 }
 VA_R_END(r_ap);
}

/* this can take a _long_ time */
static void internal_do_inorder_all_load(player_tree_node *scan,
                                         void (*command) (player *, va_list),
                                         va_list va)
{
 player *p = NULL;
 
 if (scan->priv_banished || !player_load(scan))
   return;
 
 p = scan->player_ptr;
 
 (*command) (p, va);
}

void do_inorder_all_load(void (*command) (player *, va_list), ...)
{
 player_tree_node *current = linked_list_start;
 VA_R_DECL(r_ap);
 
 VA_R_START(r_ap, command);

 while (current)
 {
  player_tree_node *current_next = current->next;
  VA_C_DECL(ap);
  
  VA_C_START(ap, command);
  
  VA_C_COPY(ap, r_ap);
  
  internal_do_inorder_all_load(current, command, ap);

  VA_C_END(ap);

  current = current_next;
 }
 VA_R_END(r_ap);
}

static void internal_destroy_all_nodes(player_tree_node *current,
                                       va_list va __attribute__ ((unused)))
{
 XFREE(current, PLAYER_TREE_NODE);
}

void player_tree_all_destroy(void)
{
 do_inorder_all(internal_destroy_all_nodes);
}

int player_file_remove(const char *name)
{
 player_tree_node *togo;
 char lowered_name[PLAYER_S_NAME_SZ];
 
 COPY_STR(lowered_name, name, PLAYER_S_NAME_SZ);
 lower_case(lowered_name);

 if ((togo = player_tree_find_exact(lowered_name)))
   player_tree_del(togo);
 else
 {
  log_assert(FALSE);
  vwlog("error", " Tried to remove -- %s -- from save files.", name);
  return (FALSE);
 }

 player_save_index();
 
 return (TRUE);
}

#ifndef USE_SLOW_RANDOM_PLAYER /* pretty sure this is ok */
player_tree_node *player_tree_random(void)
{ /* this _might_ work ... */
 unsigned int levels = highest_powerof_2(no_of_resis) + 1;
 unsigned int gain = 0;
 unsigned int count = 0;
 unsigned int prob = 0;
 player_tree_node *tmp = tree_root;

 while (tmp)
 {
  ++count;
  prob = rand();

  if (levels < count)
    levels = count;
  
  gain = (((count * 100) / levels) / (2 << (levels - count)));
  if (gain >= 100)
  {
   assert(gain == 100);
   assert(count == levels);
   assert(!(tmp->left || tmp->right));

   goto found_random_player;
  }

  if ((prob % 100) < gain)
    return (tmp);

  if (prob / ((RAND_MAX >> 1) + 1)) /* 50 50 chance of going in a direction */
  {
   if (tmp->left)
     tmp = tmp->left;
   else
     goto found_random_player;
  }
  else
  {
   if (tmp->right)
     tmp = tmp->right;
   else
     goto found_random_player;
  }
 }

 return (NULL);

 found_random_player:

 count = 0;
 while (count < 2)
 {
  ++count;
  
  while (tmp)
  {
   if (!tmp->priv_banished && !PRIV_SYSTEM_ROOM(tmp))
     return (tmp);
   
   tmp = tmp->next;
  }

  tmp = tree_root;
 }

 return (NULL);
}

#else
#warning "Using slow random player function"
player_tree_node *player_tree_random(void)
{
 int count = get_random_num(0, no_of_resis - 1);
 player_tree_node *tmp = linked_list_start;

 while (tmp)
 {
  if (!tmp->priv_banished && !PRIV_SYSTEM_ROOM(tmp))
  {
   if (count > 0)
     --count;
   else
   {
    assert(!count);
    return (tmp);
   }
  }
  
  tmp = tmp->next;
 }

 assert(FALSE);

 return (linked_list_start);
}
#endif
