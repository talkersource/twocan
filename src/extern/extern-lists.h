#ifndef EXTERN_LISTS_H
#define EXTERN_LISTS_H
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

#ifdef LISTS_C
#define LIST_FLAG_FUNC_ON_OFF(flag, name, type) do { \
 list_ ## type ## _node *tmp = (list_ ## type ## _node *)current; \
 const char *flag_type = ""; \
 \
 assert(LIST_FLAG(current, type, flag) || !LIST_FLAG(current, type, flag)); \
 \
 if (!str) \
   return (tmp->flag); \
 \
 if (TOGGLE_MATCH_ON(str)) \
 { if (!tmp->flag) { flag_type = "Set flag"; tmp->flag = TRUE; } \
   else flag_type = "Kept flag on"; } \
 else \
   if (TOGGLE_MATCH_OFF(str)) \
   { if (tmp->flag) { flag_type = "Removed flag"; tmp->flag = FALSE; } \
     else flag_type = "Kept flag off"; } \
   else \
     if (TOGGLE_MATCH_TOGGLE(str)) \
     { flag_type = "Toggled flag"; tmp->flag = !tmp->flag; } \
     else { if (p) \
           TELL_FORMAT_NO_RETURN(p, "<player(s)> <flag(s)> [on|off|toggle]"); \
           return (-1); } \
 \
 if (p) fvtell_player(NORMAL_T(p), " %s ^S^B%s^s for '^S^B%s^s'\n", \
                      flag_type, name, list_node_name(current)); \
 \
 return (tmp->flag); } while (FALSE)

#define LIST_FLAG_ALL_FUNC_ON_OFF(type) do { \
 const char *flag_type = NULL; \
 \
 if (!str) \
   return (list_player_all_used_entry(LIST_TYPE_ ## type, current)); \
 \
 if (TOGGLE_MATCH_ON(str)) \
 { \
  if (!list_player_all_used_entry(LIST_TYPE_ ## type, current)) \
  { \
   flag_type = "Set ^S^Ball^s flags"; \
   list_player_all_set_entry(LIST_TYPE_ ## type, current, TRUE); \
  } \
  else \
    flag_type = "Kept ^S^Ball^s flags on"; \
 } \
 else if (TOGGLE_MATCH_OFF(str)) \
 { \
  if (list_player_all_used_entry(LIST_TYPE_ ## type, current)) \
  { \
   flag_type = "Removed ^S^Ball^s flags"; \
   list_player_all_set_entry(LIST_TYPE_ ## type, current, FALSE); \
  } \
  else \
    flag_type = "Kept ^S^Ball^s flags off"; \
 } \
 else if (TOGGLE_MATCH_TOGGLE(str)) \
 { \
  if (list_player_all_used_entry(LIST_TYPE_ ## type, current)) \
  { \
   flag_type = "Removed ^S^Ball^s flags"; \
   list_player_all_set_entry(LIST_TYPE_ ## type, current, FALSE); \
  } \
  else \
  { \
   flag_type = "Set ^S^Ball^s flags"; \
   list_player_all_set_entry(LIST_TYPE_ ## type, current, TRUE); \
  } \
 } \
 else \
 { \
  if (p) \
    TELL_FORMAT_NO_RETURN(p, "<player(s)> all [on|off]"); \
  return (-1); \
 } \
 \
 if (p) \
   fvtell_player(NORMAL_T(p), " %s for '^S^B%s^s'\n", \
                 flag_type, list_node_name(current)); \
 \
 return (list_player_all_used_entry(LIST_TYPE_ ## type, current)); \
 } while (FALSE)
     
#define MAKE_LIST_FLAG_FUNC(flag, num_type, head, max_entries) do { \
 int flag_offsets[1]; \
 size_t flag_offset_count = 0; \
 const char *cun_flags = flag ; \
 const char *cun_toggle = "toggle"; \
 \
 if (params->last_param != 1) TELL_FORMAT(p, "<player(s)>"); \
 \
 get_parameter_parse(params, &cun_flags, 2); \
 get_parameter_parse(params, &cun_toggle, 3); \
 list_flag_change_pre(p, params, num_type, flag_offsets, &flag_offset_count); \
 assert(flag_offset_count == 1); \
 \
 if (list_flag_change_post(p, params, num_type, head, max_entries, \
                           flag_offsets, flag_offset_count)) \
   if (num_type == LIST_TYPE_ROOM) \
     p->location->owner->flag_tmp_room_needs_saving = TRUE; \
 } while (FALSE)

#define LIST_MALLOC(t, x) do { switch (t) \
 { \
  case LIST_TYPE_SELF: \
    x = XMALLOC(sizeof(list_self_node), LIST_SELF_NODE); break; \
  \
  case LIST_TYPE_COMS: \
    x = XMALLOC(sizeof(list_coms_node), LIST_COMS_NODE); break; \
  \
  case LIST_TYPE_ROOM: \
    x = XMALLOC(sizeof(list_room_node), LIST_ROOM_NODE); break; \
  \
  case LIST_TYPE_GAME: \
    x = XMALLOC(sizeof(list_game_node), LIST_GAME_NODE); break; \
  \
  case LIST_TYPE_CHAN: \
    x = XMALLOC(sizeof(list_chan_node), LIST_CHAN_NODE); break; \
  \
  default: \
    x = NULL; \
 } } while (FALSE)
 
#define LIST_FREE(t, x) do { switch (t) \
 { case LIST_TYPE_SELF: \
    XFREE(x, LIST_SELF_NODE); break; \
  \
  case LIST_TYPE_COMS: \
    XFREE(x, LIST_COMS_NODE); break; \
  \
  case LIST_TYPE_ROOM: \
    XFREE(x, LIST_ROOM_NODE); break; \
  \
  case LIST_TYPE_GAME: \
    XFREE(x, LIST_GAME_NODE); break; \
  \
  case LIST_TYPE_CHAN: \
    XFREE(x, LIST_CHAN_NODE); break; \
  \
  default: log_assert(FALSE); break; \
 } } while (FALSE)

#endif

extern Timer_q_base list_load_player_timer_queue;

#ifndef LIST_DEBUG
# ifndef NDEBUG
#  define LIST_DEBUG 1
# else
#  define LIST_DEBUG 0
# endif
#endif

#if LIST_DEBUG
extern int list_check_flag_self(list_node *, int);
extern int list_check_flag_coms(list_node *, int);
extern int list_check_flag_room(list_node *, int);
extern int list_check_flag_game(list_node *, int);
extern int list_check_flag_chan(list_node *, int);

# define LIST_FLAG(x, type, y) \
 (list_check_flag_ ## type (x, (((list_ ## type ## _node *) (x)) -> y)))
#else
# define LIST_FLAG(x, type, y) (((list_ ## type ## _node *) (x)) -> y)
#endif

#define LIST_SELF_CHECK_FLAG_START(p, check) do { \
 list_node *local_tmp_entry = NULL; \
 if (!(local_tmp_entry = list_onanywhere(p, \
                                         (p)->list_self_tmp_start, check))) \
   local_tmp_entry = list_onanywhere(p, (p)->list_self_start, check)

#define LIST_SELF_CHECK_FLAG_DO(flag) \
 (local_tmp_entry && LIST_FLAG(local_tmp_entry, self, flag))

#define LIST_SELF_CHECK_FLAG_GROUP(passed_name) \
 (local_tmp_entry && LIST_FLAG(local_tmp_entry, self, flag_grouped) && \
 (!passed_name || !strcmp(passed_name, local_tmp_entry->name)))

#define LIST_SELF_CHECK_FLAG_END() \
 } while (FALSE)

#define LIST_COMS_CHECK_FLAG_START(p, check) do { \
 list_node *local_tmp_entry = NULL; \
 if (!(local_tmp_entry = list_onanywhere(p, \
                                         (p)->list_coms_tmp_start, check))) \
   local_tmp_entry = list_onanywhere(p, (p)->list_coms_start, check)

#define LIST_COMS_CHECK_FLAG_DO(flag) \
 (local_tmp_entry && LIST_FLAG(local_tmp_entry, coms, flag))

#define LIST_COMS_CHECK_FLAG_GROUP(passed_name) \
 (local_tmp_entry && LIST_FLAG(local_tmp_entry, coms, flag_grouped) && \
 (!passed_name || !strcmp(passed_name, local_tmp_entry->name)))
  
#define LIST_COMS_CHECK_FLAG_END() \
 } while (FALSE)

#define LIST_COMS_2CHECK_FLAG_START(from, check, xtra) do { \
 if (xtra) { \
 list_node *local_tmp_1entry = NULL; \
 list_node *local_tmp_2entry = NULL; \
 if (P_IS_AVL(from)) { \
 local_tmp_1entry = list_onanywhere((from)->player_ptr, \
                             (from)->player_ptr->list_coms_tmp_start, check); \
 if (!local_tmp_1entry) \
  local_tmp_1entry = list_onanywhere((from)->player_ptr, \
                                 (from)->player_ptr->list_coms_start, check); \
 } \
 if (P_IS_AVL(check)) { \
 local_tmp_2entry = list_onanywhere((check)->player_ptr, \
                             (check)->player_ptr->list_coms_tmp_start, from); \
 if (!local_tmp_2entry) \
  local_tmp_2entry = list_onanywhere((check)->player_ptr, \
                                 (check)->player_ptr->list_coms_start, from); \
 } \
 local_tmp_1entry = local_tmp_1entry

#define LIST_COMS_2CHECK1_FLAG_DO(flag) \
 (local_tmp_1entry && LIST_FLAG(local_tmp_1entry, coms, flag))
#define LIST_COMS_2CHECK2_FLAG_DO(flag) \
 (local_tmp_2entry && LIST_FLAG(local_tmp_2entry, coms, flag))

#define LIST_COMS_2CHECK1_FLAG_GROUP(passed_name) \
 (local_tmp_1entry && LIST_FLAG(local_tmp_1entry, coms, flag_grouped) && \
 (!passed_name || !strcmp(passed_name, local_tmp_1entry->name)))
#define LIST_COMS_2CHECK2_FLAG_GROUP(passed_name) \
 (local_tmp_2entry && LIST_FLAG(local_tmp_2entry, coms, flag_grouped) && \
 (!passed_name || !strcmp(passed_name, local_tmp_2entry->name)))
  
#define LIST_COMS_2CHECK_FLAG_DO(flag) \
 (LIST_COMS_2CHECK1_FLAG_DO(flag) || LIST_COMS_2CHECK2_FLAG_DO(flag))
  
#define LIST_COMS_2CHECK_FLAG_END() \
 } } while (FALSE)

#define LIST_ROOM_CHECK_FLAG_START(r, check) do { \
 list_node *local_tmp_entry = NULL; \
 if (!(local_tmp_entry = list_onanywhere((r)->owner->player_ptr, \
                                         (r)->list_room_local_start, check))) \
  local_tmp_entry = list_onanywhere((r)->owner->player_ptr, \
                                    (r)->owner->list_room_glob_start, check)

#define LIST_ROOM_CHECK_FLAG_DO(flag) \
 (local_tmp_entry && LIST_FLAG(local_tmp_entry, room, flag))

#define LIST_ROOM_CHECK_FLAG_GROUP(passed_name) \
 (local_tmp_entry && LIST_FLAG(local_tmp_entry, room, flag_grouped) && \
 (!passed_name || !strcmp(passed_name, local_tmp_entry->name)))

#define LIST_ROOM_CHECK_FLAG_END() \
 } while (FALSE)

#define LIST_GAME_CHECK_FLAG_START(p, check) do { \
 list_node *local_tmp_entry = list_onanywhere(p, (p)->list_game_start, check)

#define LIST_GAME_CHECK_FLAG_DO(flag) \
 (local_tmp_entry && LIST_FLAG(local_tmp_entry, game, flag))

#define LIST_GAME_CHECK_FLAG_GROUP(passed_name) \
 (local_tmp_entry && LIST_FLAG(local_tmp_entry, game, flag_grouped) && \
 (!passed_name || !strcmp(passed_name, local_tmp_entry->name)))

#define LIST_GAME_CHECK_FLAG_END() \
 } while (FALSE)

#define LIST_CHAN_CHECK_FLAG_START(base, check) do { \
 list_node *local_tmp_entry = list_onanywhere(NULL, (base)->list_start, \
                                              check)

#define LIST_CHAN_CHECK_FLAG_DO(flag) \
 (local_tmp_entry && LIST_FLAG(local_tmp_entry, chan, flag))

#define LIST_CHAN_CHECK_FLAG_GROUP(passed_name) \
 (local_tmp_entry && LIST_FLAG(local_tmp_entry, chan, flag_grouped) && \
 (!passed_name || !strcmp(passed_name, local_tmp_entry->name)))
  
#define LIST_CHAN_CHECK_FLAG_END() \
 } while (FALSE)

extern void list_load(list_node **, int, file_io *);
extern void list_room_glob_load(player_tree_node *, file_io *);
extern void list_room_glob_file_load(player_tree_node *);
extern void list_player_load(player_tree_node *);

extern void list_load_cleanup(int, list_node *);
extern void list_room_local_load_cleanup(room *);
extern void list_room_glob_load_cleanup(player_tree_node *);
extern void list_player_load_cleanup(player *);

extern void list_chan_cleanup(channels_base *);
 
extern void list_save(list_node **, int, file_io *);

extern list_node *list_user_find_entry(player *, list_node *, const char *);

extern list_node *list_onanywhere(player *, list_node *, player_tree_node *);

extern void user_list_other_player_view(player *, parameter_holder *);

extern void user_list_room_del(player *, parameter_holder *);
extern void user_list_room_change(player *, parameter_holder *);
extern void user_list_room_view(player *, parameter_holder *);

extern void user_list_chan_del(player *, parameter_holder *);
extern void user_list_chan_change(player *, parameter_holder *);
extern void user_list_chan_view(player *, parameter_holder *);

extern int list_system_change(list_node **, int, const char *);

extern void init_list(void);

extern void cmds_init_list(void);

#endif
