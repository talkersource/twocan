#define CONSTRUCT_NAMES_C
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


/* used with do_cronorder_room and do_inorder_logged_on etc... */
int construct_name_list_do(player *scan, va_list ap)
{
 /* params */
 int max_count = va_arg(ap, int);
 int *count = va_arg(ap, int *);
 int func_flags = va_arg(ap, int);
 priv_test_list_type list_priv_test_me = va_arg(ap, priv_test_list_type);
 priv_test_list_type list_priv_test_them = va_arg(ap, priv_test_list_type);
 int *output_flags = va_arg(ap, int *);
 
 player_tree_node *from = va_arg(ap, player_tree_node *);
 player *to = va_arg(ap, player *);
 twinkle_info *info = va_arg(ap, twinkle_info *);
 int flags = va_arg(ap, int);
 time_t my_now = va_arg(ap, time_t);

 /* normal locals */
 unsigned int max_length = 0;
 int names_ammount = 0;
 list_node *entry = NULL;

 if ((func_flags & CONSTRUCT_NAME_NO_ME) && (to->saved == scan->saved))
   return (TRUE);
 
 if (func_flags & CONSTRUCT_NAME_USE_LIST_ENT_ME)
 {
  if (!(entry = list_onanywhere(to, to->list_self_tmp_start, scan->saved)))
    if (!(entry = list_onanywhere(to, to->list_self_start, scan->saved)))
      return (TRUE);
  
  assert(LIST_FLAG(entry, self, find) || !LIST_FLAG(entry, self, find));
  
  if (!(*list_priv_test_me)(entry))
    return (TRUE);
 }
 
 if (func_flags & CONSTRUCT_NAME_USE_LIST_ENT_THEM)
 {
  if (!(entry = list_onanywhere(scan, scan->list_self_tmp_start, to->saved)))
    if (!(entry = list_onanywhere(scan, scan->list_self_start, to->saved)))
      return (TRUE);

  assert(LIST_FLAG(entry, self, find) || !LIST_FLAG(entry, self, find));

  if (!(*list_priv_test_them)(entry))
    return (TRUE);
 }
  
 if ((func_flags & CONSTRUCT_NAME_USE_PREFIX) && !to->flag_no_emote_prefix)
   max_length = (PLAYER_S_NAME_SZ + PLAYER_S_PREFIX_SZ - 2);
 else
 {
  max_length = (PLAYER_S_NAME_SZ - 1);
  func_flags &= ~CONSTRUCT_NAME_USE_PREFIX;
 }

 if (to->term_width > max_length)
   names_ammount = (to->term_width / max_length);
 else
   names_ammount = 1;

 assert(max_count && (*count < max_count));

 IGNORE_PARAMETER(max_count);

 ++*count;

 /* no colours can be present, name and prefix */
 if (func_flags & CONSTRUCT_NAME_USE_PREFIX)
   fvtell_player(ALL_T(from, to, info, flags | RAW_OUTPUT, my_now),
                 "%-*s %-*s", PLAYER_S_PREFIX_SZ - 2, scan->prefix,
                 max_length - (PLAYER_S_PREFIX_SZ - 2), scan->saved->name);
 else
   fvtell_player(ALL_T(from, to, info, flags, my_now),
                 "%-*s", max_length,
                 scan->saved->name);
  
 if (!(*count % names_ammount))
 {
  fvtell_player(ALL_T(from, to, info, flags, my_now), "%s", "\n");
  *output_flags &= ~CONSTRUCT_NAME_OUT_MID_LINE;
 }
 else
   *output_flags |= CONSTRUCT_NAME_OUT_MID_LINE;
 
 return (TRUE);
}
