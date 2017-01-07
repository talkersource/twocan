#define ROOM_C
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

static Timer_q_base room_load_timer_queue;
static Timer_q_base room_automessage_timer_queue;

player_linked_list *do_cronorder_room(int (*func) (player *, va_list),
                                      room *r, ...)
{
 DO_BUILD_ORDER_ON(func, r, r->room_list_cron, TRUE,
                   (PLAYER_LINK_GET(scan), ap));
}

player_linked_list *do_inorder_room(int (*func) (player *, va_list),
                                    room *r, ...)
{
 DO_BUILD_ORDER_ON(func, r, r->room_list_alpha, TRUE,
                   (PLAYER_LINK_GET(scan), ap));
}

static room *room_user_find(player *requester,
                            player_tree_node *current, const char *id)
{
 room *scan = current->rooms_start;

 while (scan)
 {
  if (!strcasecmp(scan->id, id))
    return (scan);

  scan = scan->next;
 }

 if (requester)
   fvtell_player(SYSTEM_T(requester),
                 " The player -- ^S^B%s^s -- does not have a room "
                 "with an id of -- ^S^B%s^s --.\n",
                 current->name, id);
 
 return (NULL);
}

room *room_find_home(player_tree_node *current)
{
 room *scan = current->rooms_start;
 
 while (scan)
 {
  if (scan->flag_home)
    break;
  
  scan = scan->next;
 }

 return (scan);
}

room *room_load_find(player *p, const char *str, unsigned int flags)
{
 const char *scan = NULL;
 player_tree_node *sp = NULL;
 room *r = NULL;
 char name[PLAYER_S_NAME_SZ];
 
 BTRACE("room_load_find");

 assert(!(flags & PLAYER_FIND_VERBOSE) || p);
 
 if (!(scan = C_strchr(str, '.')) || ((scan - str) > (PLAYER_S_NAME_SZ - 1)))
 {
  if (flags & PLAYER_FIND_VERBOSE)
  {
   assert(p);
   fvtell_player(SYSTEM_T(p), "%s",
                 " Rooms should be of the form <owner>.<id>\n");
  }
  
  return (NULL);
 }

 if (scan == str)
 {
  assert(p && p->saved);
  qstrcpy(name, p->saved->lower_name);
 }
 else
   COPY_STR_LEN(name, str, scan - str);
 
 if (!(sp = player_find_all(p, name, flags)))
   return (NULL);

 if (!(r = room_user_find((flags & PLAYER_FIND_VERBOSE) ? p : NULL,
                          sp, ++scan)))
   return (NULL);

 if (!r->flag_tmp_in_core)
   room_load(r, NULL);

 return (r);
}

static void internal_room_add(player_tree_node *current, room *r)
{
 room **scan = &current->rooms_start;

 while (*scan && ((*scan)->number < r->number))
   scan = &(*scan)->next;
 
 r->next = *scan;
 *scan = r;
 
 ++current->rooms_num;
 r->owner = current;
}

static void internal_room_del(player_tree_node *current, room *r)
{
 room **scan = &current->rooms_start;

 assert(room_user_find(NULL, current, r->id));
 
 --current->rooms_num;

 while (*scan && (*scan != r))
   scan = &(*scan)->next;
 
 if (*scan)
 {
  assert(*scan == r);
  *scan = (*scan)->next;
 }

 r->owner = NULL;
}

static void room_player_add(player *p, room *r)
{ 
 p->location = r;
 
 player_link_add(&r->room_list_alpha, NULL, p,
                 PLAYER_LINK_LOGGEDON | PLAYER_LINK_NAME_ORDERED |
                 PLAYER_LINK_DOUBLE,
                 &p->room_list_alpha.s);
 player_link_add(&r->room_list_cron, NULL, p,
                 PLAYER_LINK_LOGGEDON | PLAYER_LINK_DOUBLE,
                 &p->room_list_cron.s);

 ++r->players_num;
  
 if (gettimeofday(&r->last_entered_left, NULL))
 {
  assert(FALSE);
 }
}

void room_player_del(player *p, room *r)
{
 assert(p && (p->location == r));

 player_link_del(&r->room_list_alpha, NULL, p,
                 PLAYER_LINK_LOGGEDON | PLAYER_LINK_NAME_ORDERED |
                 PLAYER_LINK_DOUBLE,
                 &p->room_list_alpha.s);
 player_link_del(&r->room_list_cron, NULL, p,
                 PLAYER_LINK_LOGGEDON | PLAYER_LINK_DOUBLE,
                 &p->room_list_cron.s);

 --r->players_num;

 p->location = NULL;
 
 if (gettimeofday(&r->last_entered_left, NULL))
 {
  assert(FALSE);
 } 
}

/* yum ... namespace */
static int internal_tell_room_wall(player *p, player *scan, const char *fmt,
                                   va_list ap)
{
 vfvtell_player(TALK_T(p ? p->saved : NULL, scan), fmt, ap);

 return (TRUE);
}

player_linked_list *vtell_room_wall(room *current, player *avoid,
                                    const char *fmt, ...)
{
 DO_BUILD_ORDER_FMT(internal_tell_room_wall, avoid, fmt, fmt,
                    current->room_list_alpha,
                    (PLAYER_LINK_GET(scan) != avoid),
                    (avoid, PLAYER_LINK_GET(scan), fmt, ap));
}

static int internal_tell_room_says(player *p, player *scan, const char *fmt,
                                   va_list ap)
{
 LIST_COMS_2CHECK_FLAG_START(scan->saved, p->saved, TRUE);
 if (LIST_COMS_2CHECK_FLAG_DO(says))
   return (TRUE);
 LIST_COMS_2CHECK_FLAG_END();

 return (internal_tell_room_wall(p, scan, fmt, ap));
}

player_linked_list *vtell_room_says(room *current, player *avoid,
                                    const char *fmt, ...)
{
 DO_BUILD_ORDER_FMT(internal_tell_room_says, avoid, fmt, fmt,
                    current->room_list_alpha,
                    (PLAYER_LINK_GET(scan) != avoid),
                    (avoid, PLAYER_LINK_GET(scan), fmt, ap));
}

static int internal_tell_room_autos(player *p, player *scan, const char *fmt,
                                    va_list ap)
{ 
 LIST_COMS_2CHECK_FLAG_START(scan->saved, scan->location->owner, TRUE);
 if (LIST_COMS_2CHECK_FLAG_DO(autos))
   return (TRUE);
 LIST_COMS_2CHECK_FLAG_END();

 return (internal_tell_room_wall(p, scan, fmt, ap));
}

static player_linked_list *vtell_room_autos(room *current, player *avoid,
                                            const char *fmt, ...)
{
 DO_BUILD_ORDER_FMT(internal_tell_room_autos, avoid, fmt, fmt,
                    current->room_list_alpha,
                    (PLAYER_LINK_GET(scan) != avoid),
                    (avoid, PLAYER_LINK_GET(scan), fmt, ap));
}

static int internal_tell_room_movement(player *p, player *scan,
                                       const char *fmt, va_list ap)
{
 LIST_COMS_2CHECK_FLAG_START(scan->saved, p->saved, TRUE);
 if (LIST_COMS_2CHECK_FLAG_DO(movement))
   return (TRUE);
 LIST_COMS_2CHECK_FLAG_END();

 return (internal_tell_room_wall(p, scan, fmt, ap));
}

player_linked_list *vtell_room_movement(room *current, player *avoid,
                                        const char *fmt, ...)
{
 DO_BUILD_ORDER_FMT(internal_tell_room_movement, avoid, fmt, fmt,
                    current->room_list_alpha,
                    (PLAYER_LINK_GET(scan) != avoid),
                    (avoid, PLAYER_LINK_GET(scan), fmt, ap));
}

static exit_node *room_exit_add(room *r, const char *name, const char *id)
{
 exit_node *tmp = NULL;
 exit_node *scan = r->exits_start;
 
 if (!(tmp = XMALLOC(sizeof(exit_node), ROOM_EXIT_NODE)))
   return (NULL);
 
 COPY_STR(tmp->name, name, PLAYER_S_NAME_SZ);
 COPY_STR(tmp->id, id, ROOM_ID_SZ);
 
 if (!scan)
 {
  tmp->next = NULL;
  tmp->prev = NULL;
  r->exits_start = tmp;
 }
 else
 {
  while (scan->next && (strcasecmp(scan->id, id) < 0))
    scan = scan->next;

  if (strcasecmp(scan->id, id) < 0)
  {
   if ((tmp->next = scan->next))
     tmp->next->prev = tmp;

   scan->next = tmp;
   tmp->prev = scan;
  }
  else
  {
   if ((tmp->prev = scan->prev))
     tmp->prev->next = tmp;
   else
     r->exits_start = tmp;
   
   scan->prev = tmp;
   tmp->next = scan;
  } 
 }
  
 r->exits_num++;

 return (tmp);
}

static automessage_node *room_automessage_add(room *r, const char *message)
{
 automessage_node *tmp = NULL;
 
 if (!(tmp = XMALLOC(sizeof(automessage_node), ROOM_AUTOMESSAGE_NODE)))
   return (NULL);
 
 COPY_STR(tmp->message, message, ROOM_AUTOMESSAGE_SZ);
 
 if ((tmp->next = r->automessages_start))
   tmp->next->prev = tmp;
 tmp->prev = NULL;
 r->automessages_start = tmp;

 r->automessages_num++;

 return (tmp);
}

/* feature: if name == NULL, then just search on id */
static exit_node *room_exit_find(room *r, const char *name, const char *id)
{
 exit_node *scan = r->exits_start;
 int s_cmp = -1;
 
 while (scan && ((s_cmp = beg_strcasecmp(id, scan->id)) > 0) &&
        (!name || strcasecmp(name, scan->name)))
   scan = scan->next;

 if (!s_cmp)
   return (scan);
 else
   return (NULL);
}

static automessage_node *room_automessage_find(room *r, int count)
{
 automessage_node *scan = r->automessages_start;

 if ((count > r->automessages_num) || (count < -1))
   return (NULL);
 
 while (scan && (--count > 0))
   scan = scan->next;

 return (scan);
}

static void room_exit_del(room *r, exit_node *to_go)
{
 if (to_go->prev)
   to_go->prev->next = to_go->next;
 else
   r->exits_start = to_go->next;

 if (to_go->next)
   to_go->next->prev = to_go->prev;

 XFREE(to_go, ROOM_EXIT_NODE);

 r->exits_num--;
}

static void room_automessage_del(room *r, automessage_node *to_go)
{
 if (to_go->prev)
   to_go->prev->next = to_go->next;
 else
   r->automessages_start = to_go->next;

 if (to_go->next)
   to_go->next->prev = to_go->prev;

 XFREE(to_go, ROOM_AUTOMESSAGE_NODE);

 r->automessages_num--;
}

static void room_destroy_exits(room *r)
{
 exit_node *scan = r->exits_start;

 while (scan)
 {
  exit_node *scan_next = scan->next;

  room_exit_del(r, scan);
  scan = scan_next;
 }

 assert(!r->exits_start);
 
 r->exits_start = NULL;
}

static void room_destroy_automessages(room *r)
{
 automessage_node *scan = r->automessages_start;

 while (scan)
 {
  automessage_node *scan_next = scan->next;

  room_automessage_del(r, scan);
  scan = scan_next;
 }

 assert(!r->automessages_start);
 
 r->automessages_start = NULL;
}

static void room_init(room *r)
{
 r->owner = NULL;
 r->room_list_alpha = NULL;
 r->room_list_cron = NULL;

 r->list_room_local_start = NULL;
 
 r->description = NULL;
 r->exits_start = NULL;
 r->automessages_start = NULL;
 
 r->where_description = NULL;
 r->enter_msg = NULL;
 
 r->id[0] = 0;

 r->players_num = 0;

 r->automessage_min_seconds = 60;

 r->exits_num = 0;
 r->automessages_num = 0;
 
 r->flag_home = FALSE;
 r->flag_automessage = FALSE;
 r->flag_locked = FALSE;
 r->flag_bolted = FALSE;

 r->flag_tmp_in_core = FALSE;
}

static void room_load_cleanup(room *current)
{
 if (current->owner->flag_tmp_room_needs_saving ||
     current->players_num || P_IS_ON(current->owner))
 {
  assert(current->flag_tmp_in_core);
  return;
 }

 if (!current->flag_tmp_in_core)
   return;

 FREE(current->description);
 current->description = NULL;

 FREE(current->enter_msg);
 current->enter_msg = NULL;

 FREE(current->where_description);
 current->where_description = NULL;

 room_destroy_automessages(current);
 room_destroy_exits(current);

 list_load_cleanup(LIST_TYPE_ROOM, current->list_room_local_start);
 current->list_room_local_start = NULL;
 
 current->flag_tmp_in_core = FALSE;
}

static void timed_room_cleanup(int timed_type, void *passed_room)
{
 room *r = passed_room;
 int do_save = FALSE;
 int do_cleanup = FALSE;
 int do_retime = FALSE;

 TCTRACE("timed_room_cleanup");

 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;
 
 if (timed_type == TIMER_Q_TYPE_CALL_RUN_ALL)
   do_save = TRUE;
 else if (!P_IS_ON(r->owner) && !r->players_num &&
          (difftime(now, r->a_timestamp) > ROOM_CLEANUP_TIMEOUT_CLEANUP))
 {
  do_save = TRUE;
  do_cleanup = TRUE;
 }
 else if (difftime(now, r->l_timestamp) > ROOM_CLEANUP_TIMEOUT_SYNC_ANYWAY)
 {
  do_save = TRUE;
  do_retime = TRUE;
  r->l_timestamp = now;
 }
 else
   do_retime = TRUE;
 
 if (do_save && r->owner->flag_tmp_room_needs_saving)
   room_save(r->owner);
 
 if (do_cleanup)
   room_load_cleanup(r);
 else if (do_retime)
 {
  struct timeval tv;

  gettimeofday(&tv, NULL);

  TIMER_Q_TIMEVAL_ADD_SECS(&tv, ROOM_CLEANUP_TIMEOUT_REDO, 0);

  timer_q_add_static_node(&r->load_timer, &room_load_timer_queue,
                          r, &tv, TIMER_Q_FLAG_NODE_SINGLE);
 }
}

static void room_load_timer_start(room *r)
{
 if (!timer_q_find_data(&room_load_timer_queue, r))
 {
  struct timeval tv;
  
  gettimeofday(&tv, NULL);
  
  TIMER_Q_TIMEVAL_ADD_SECS(&tv, ROOM_CLEANUP_TIMEOUT_LOAD, 0);
  
  timer_q_add_static_node(&r->load_timer, &room_load_timer_queue,
                          r, &tv, TIMER_Q_FLAG_NODE_SINGLE);

  r->l_timestamp = now;
 }
 else
   log_assert(FALSE);
}

int room_load_open(player_tree_node *current, file_io *io_room)
{
 char room_file[sizeof("files/player_data/a/a/.room") + PLAYER_S_NAME_SZ];
 
 sprintf(room_file, "%s/%c/%c/%s.room", "files/player_data",
         *current->lower_name, *(current->lower_name + 1),
         current->lower_name);
 
 if (!file_read_open(room_file, io_room))
   return (FALSE);
 
 file_section_beg("list", io_room);
 
 list_room_glob_load(current, io_room);
 
 file_section_end("list", io_room);
 
 file_section_beg("rooms", io_room);
 
 return (TRUE);
}

void room_load(room *current, file_io *io_room)
{
 file_io local_io_room;
 int number_of_exits = 0;
 int number_of_autos = 0;
 int count = 0;
 char room_num[BUF_NUM_TYPE_SZ(int)];

 current->a_timestamp = now;
 if (current->flag_tmp_in_core)
 {
  list_room_glob_file_load(current->owner);
  return;
 }
 
 assert(current->number > 0);
 
 if (!io_room) /* speedup for loading all the rooms... */
 {
  io_room = &local_io_room;
  if (!room_load_open(current->owner, io_room))
    return;
 }

 room_load_timer_start(current);
 
 sprintf(room_num, "%05d", current->number);

 file_section_beg(room_num, io_room);
 
 file_section_beg("header", io_room);
 
 number_of_autos = file_get_int("number_of_autos", io_room);
 number_of_exits = file_get_int("number_of_exits", io_room);
 
 file_section_end("header", io_room);

 file_section_beg("room", io_room);
 
 file_section_beg("autos", io_room);

 count = 0;
 while (count < number_of_autos)
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];
  char automessage[ROOM_AUTOMESSAGE_SZ];
  
  sprintf(buffer, "%05d", ++count);

  file_section_beg(buffer, io_room);

  file_get_string("automessage", automessage, ROOM_AUTOMESSAGE_SZ, io_room);

  if (!room_automessage_add(current, automessage))
    SHUTDOWN_MEM_ERR();
  
  file_section_end(buffer, io_room);
 }
 
 file_section_end("autos", io_room);

 if (!(current->description = file_get_malloc("description", NULL, io_room)))
   SHUTDOWN_MEM_ERR();

 if (!(current->enter_msg = file_get_malloc("enter_msg", NULL, io_room)))
   SHUTDOWN_MEM_ERR();
 
 file_section_beg("exits", io_room);

 count = 0;
 while (count < number_of_exits)
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];
  char name[PLAYER_S_NAME_SZ];
  char id[ROOM_ID_SZ];
  player_tree_node *sp = NULL;
  
  sprintf(buffer, "%05d", ++count);

  file_section_beg(buffer, io_room);

  file_get_string("id", id, ROOM_ID_SZ, io_room);
  file_get_string("name", name, PLAYER_S_NAME_SZ, io_room);

  if ((sp = player_find_all(NULL, name, PLAYER_FIND_DEFAULT)) &&
      room_user_find(NULL, sp, id))
  {
   if (!room_exit_add(current, name, id))
     SHUTDOWN_MEM_ERR();
  }
  
  file_section_end(buffer, io_room);
 } 

 file_section_end("exits", io_room);

 file_section_beg("list", io_room);
 
 list_load(&current->list_room_local_start, LIST_TYPE_ROOM, io_room);

 file_section_end("list", io_room);
 
 if (!(current->where_description = file_get_malloc("where_description",
                                                    NULL, io_room)))
   SHUTDOWN_MEM_ERR();
  
 file_section_end("room", io_room);

 file_section_end(room_num, io_room);
 
 current->flag_tmp_in_core = TRUE;

 if (io_room == &local_io_room)
 {
  file_section_end("rooms", io_room);
  file_read_close(io_room);
 } 
}

static void room_load_all(player_tree_node *current)
{
 room *players_rooms = NULL;
 file_io local_io_room;
  
 assert(current);
 if (!(players_rooms = current->rooms_start))
   return;

 if (!room_load_open(current, &local_io_room))
   return;
 
 while (players_rooms)
 {
  room *next = players_rooms->next;
  
  room_load(players_rooms, &local_io_room);
  
  players_rooms = next;
 }

 file_section_end("rooms", &local_io_room);

 file_read_close(&local_io_room);
}

void room_load_saved(player_tree_node *current)
{
 file_io io_room;
 int ammount = 0;
 int count = 0;
 char room_file[sizeof("files/player_data/a/a/.room") + PLAYER_S_NAME_SZ];

 sprintf(room_file, "%s/%c/%c/%s.room", "files/player_data",
         *current->lower_name, *(current->lower_name + 1),
         current->lower_name);
 
 if (!file_read_open(room_file, &io_room))
   return;

 file_section_beg("a_saved", &io_room);
 
 file_section_beg("header", &io_room);
 
 ammount = file_get_int("ammount", &io_room);
 
 file_section_end("header", &io_room);
  
 file_section_beg("rooms", &io_room);
 while (count < ammount)
 {
  room *new_room = XMALLOC(sizeof(room), ROOM);
  char buffer[BUF_NUM_TYPE_SZ(int)];

  if (!new_room)
    SHUTDOWN_MEM_ERR();

  sprintf(buffer, "%05d", ++count);

  file_section_beg(buffer, &io_room);

  file_section_beg("room", &io_room);
  
  room_init(new_room);

  new_room->number = count;
  
  new_room->automessage_min_seconds = file_get_int("automessage_min_seconds",
                                                   &io_room);
  file_section_beg("flags", &io_room);
  
  new_room->flag_automessage = file_get_bitflag("automessage", &io_room);
  new_room->flag_bolted = file_get_bitflag("bolted", &io_room);
  new_room->flag_home = file_get_bitflag("home", &io_room);
  new_room->flag_locked = file_get_bitflag("locked", &io_room);
  
  file_section_end("flags", &io_room);
  
  file_get_string("id", new_room->id, ROOM_ID_SZ, &io_room);
   
  new_room->owner = current;
  
  internal_room_add(current, new_room);

  file_section_end("room", &io_room);
  
  file_section_end(buffer, &io_room);

  assert(!new_room->owner->flag_tmp_room_needs_saving);
  assert(!new_room->flag_tmp_in_core);
 }
 assert(count == ammount);

 file_section_end("rooms", &io_room);

 file_section_end("a_saved", &io_room);
 
 file_read_close(&io_room);
}

void room_save(player_tree_node *current)
{
 file_io local_io_room;
 file_io *io_room = &local_io_room;
 room *scan = current->rooms_start;
 int room_count = 0;
 char room_file[sizeof("files/player_data/a/a/.room.tmp") + PLAYER_S_NAME_SZ];
 char room_file_ren[sizeof("files/player_data/a/a/.room") + PLAYER_S_NAME_SZ];

 if (configure.talker_read_only)
   return;

 if (!current->flag_tmp_room_needs_saving)
   return;
 
 room_load_all(current); /* all rooms have to be in_core */
 
 sprintf(room_file_ren, "%s/%c/%c/%s.room", "files/player_data",
         *current->lower_name, *(current->lower_name + 1),
         current->lower_name);
 sprintf(room_file, "%s.tmp", room_file_ren);
 
 if (!file_write_open(room_file, ROOM_PLAYER_FILE_VERSION, io_room))
 {
  log_assert(FALSE);
  return;
 }

 file_section_beg("a_saved", io_room);

 file_section_beg("header", io_room);
 
 file_put_int("ammount", current->rooms_num, io_room);
 
 file_section_end("header", io_room);

 file_section_beg("rooms", io_room);

 while (scan && ((unsigned int)room_count < current->rooms_num))
 {
  char room_num[BUF_NUM_TYPE_SZ(int)];

  sprintf(room_num, "%05d", ++room_count);
  assert(room_count > 0);
  
  scan->number = room_count;

  file_section_beg(room_num, io_room);
  
  file_section_beg("room", io_room);
  
  file_put_int("automessage_min_seconds",
               scan->automessage_min_seconds, io_room);
  
  file_section_beg("flags", io_room);
  
  file_put_bitflag("automessage", scan->flag_automessage, io_room);
  file_put_bitflag("bolted", scan->flag_bolted, io_room);
  file_put_bitflag("home", scan->flag_home, io_room);
  file_put_bitflag("locked", scan->flag_locked, io_room);
  
  file_section_end("flags", io_room);
  
  file_put_string("id", scan->id, 0, io_room);

  file_section_end("room", io_room);
  
  file_section_end(room_num, io_room);
  
  scan = scan->next;
 }
 assert(!scan && ((unsigned int)room_count == current->rooms_num));

 file_section_end("rooms", io_room);
 file_section_end("a_saved", io_room);

 file_section_beg("list", io_room);
 
 assert(current->flag_tmp_list_room_glob_in_core);
 list_save(&current->list_room_glob_start, LIST_TYPE_ROOM, io_room);
 
 file_section_end("list", io_room);
 
 file_section_beg("rooms", io_room);
 scan = current->rooms_start;
 room_count = 0;
 while (scan && ((unsigned int)room_count < current->rooms_num))
 {
  char room_num[BUF_NUM_TYPE_SZ(int)];
  int count = 0;
  automessage_node *auto_scan = NULL;
  exit_node *exit_scan = NULL;
  
  sprintf(room_num, "%05d", ++room_count);
  assert(scan->number == room_count);

  file_section_beg(room_num, io_room);
  
  file_section_beg("header", io_room);
  
  file_put_int("number_of_autos", scan->automessages_num, io_room);
  file_put_int("number_of_exits", scan->exits_num, io_room);
  
  file_section_end("header", io_room);
  
  file_section_beg("room", io_room);
  
  file_section_beg("autos", io_room);
  
  count = 0;
  auto_scan = scan->automessages_start;
  while (auto_scan && (count < scan->automessages_num))
  {
   char buffer[BUF_NUM_TYPE_SZ(int)];
   
   sprintf(buffer, "%05d", ++count);
   
   file_section_beg(buffer, io_room);
   
   file_put_string("automessage", auto_scan->message, 0, io_room);
   
   file_section_end(buffer, io_room);

   auto_scan = auto_scan->next;
  }
  
  file_section_end("autos", io_room);
  
  file_put_string("description", scan->description, 0, io_room);
  
  file_put_string("enter_msg", scan->enter_msg, 0, io_room);
  
  file_section_beg("exits", io_room);
  
  count = 0;
  exit_scan = scan->exits_start;
  while (exit_scan && (count < scan->exits_num))
  {
   char buffer[BUF_NUM_TYPE_SZ(int)];
   
   sprintf(buffer, "%05d", ++count);
   
   file_section_beg(buffer, io_room);
   
   file_put_string("id", exit_scan->id, 0, io_room);
   file_put_string("name", exit_scan->name, 0, io_room);
   
   file_section_end(buffer, io_room);

   exit_scan = exit_scan->next;
  }
  
  file_section_end("exits", io_room);
  
  file_section_beg("list", io_room);
  
  list_save(&scan->list_room_local_start, LIST_TYPE_ROOM, io_room);
  
  file_section_end("list", io_room);

  file_put_string("where_description", scan->where_description, 0, io_room);
  
  file_section_end("room", io_room);
  
  file_section_end(room_num, io_room);

  scan->owner->flag_tmp_room_needs_saving = FALSE;
  
  scan = scan->next;
 }
 assert(!scan && ((unsigned int)room_count == current->rooms_num));
 
 file_section_end("rooms", io_room);

 if (file_write_close(io_room))
   rename(room_file, room_file_ren);
}

static room *room_add(player_tree_node *current, const char *id)
{
 room *new_room = NULL;
 
 BTRACE("room_add");
 
 assert(current);
 
 if (!(new_room = XMALLOC(sizeof(room), ROOM)))
   goto malloc_failed_1;

 room_init(new_room);

 if (!(new_room->where_description = MALLOC(ROOM_WHERE_DESCRIPTION_SZ)))
   goto malloc_failed_2;
 
 if (!(new_room->enter_msg = MALLOC(ROOM_ENTER_MSG_SZ)))
   goto malloc_failed_3;

 if (!(new_room->description = MALLOC(ROOM_DESCRIPTION_CHARS_SZ)))
   goto malloc_failed_4;

 sprintf(new_room->where_description, "in somewhere belonging to %s.",
         current->name);
 sprintf(new_room->enter_msg,
         "$F-Gender(pl(go) def(goes)) to a room belonging to %s.",
         current->name);

 sprintf(new_room->description, "\n A bare room, belonging to %s.\n"
         " Isn't it time to write a description ?\n", current->name);

 new_room->automessage_min_seconds = 30;
 new_room->owner = current;

 internal_room_add(current, new_room);

 new_room->number = -1;
 
 COPY_STR(new_room->id, id, ROOM_ID_SZ);
 
 new_room->flag_tmp_in_core =
   new_room->owner->flag_tmp_room_needs_saving = TRUE;

 room_load_timer_start(new_room);
 
 return (new_room);

 malloc_failed_4:
 FREE(new_room->enter_msg);
 malloc_failed_3:
 FREE(new_room->where_description);
 malloc_failed_2:
 XFREE(new_room, ROOM);
 malloc_failed_1:
 return (NULL);
}

static void user_room_add(player *p, parameter_holder *params)
{
 room *r = NULL;
 const char *id = NULL;
 
 if (p->saved->rooms_num >= (unsigned int)p->max_rooms)
 {
  fvtell_player(SYSTEM_T(p), " You are already at your maximum "
                "of -- ^S^B%d -- room limit.\n", p->max_rooms);
  return;
 }

 switch (params->last_param)
 {
  case 1:
    id = GET_PARAMETER_STR(params, 1);
    break;
  case 0:
    id = "unnamed";
    break;
  default:
    TELL_FORMAT(p, "<sys_player_name> <sys_id_name>");
 }
 
 if ((r = room_add(p->saved, id)))
   fvtell_player(NORMAL_T(p), " New room created with an id "
                 "of '^S^B%s.%s^s'\n", p->saved->name, r->id);
 else
   P_MEM_ERR(p);
}

static player_tree_node *room_add_system_char(const char *name)
{
 char player_file[sizeof("files/player_data/a/a/.player") + PLAYER_S_NAME_SZ];
 char lowered_name[PLAYER_S_NAME_SZ];
 file_io real_io_player;
 file_io *io_player = &real_io_player;
 player_tree_node *sys_char = NULL;
 
 COPY_STR(lowered_name, name, PLAYER_S_NAME_SZ);
 
 lower_case(lowered_name);

 sprintf(player_file, "%s/%c/%c/%s.player", "files/player_data",
         *lowered_name, *(lowered_name + 1), lowered_name);

 if (!file_write_open(player_file, 1, io_player))
   return (NULL);

 file_section_beg("a_saved", io_player);
 
 file_section_beg("bits", io_player);

 file_put_time_t("c_timestamp", now, io_player);
 
 file_section_beg("privs", io_player);

 file_put_bitflag("system_room", TRUE, io_player);

 file_section_end("privs", io_player);
 file_section_end("bits", io_player);
 file_section_end("a_saved", io_player);

 if (!file_write_close(io_player))
   return (NULL);
 
 sys_char = player_load_saved(lowered_name);
 sys_char->flag_tmp_list_room_glob_in_core = TRUE;
 
 player_save_index();

 return (sys_char);
}

static void user_room_system_create(player *p, parameter_holder *params)
{
 player_tree_node *sys_char = NULL;
 room *r = NULL;
 
 if (params->last_param != 2)
   TELL_FORMAT(p, "<sys_player_name> <sys_id_name>");
 
 if (!(sys_char = player_find_all(p, GET_PARAMETER_STR(params, 1),
                                  PLAYER_FIND_SELF)))
 {
  if (!(sys_char = room_add_system_char(GET_PARAMETER_STR(params, 1))))
  {
   P_MEM_ERR(p);
   return;
  }
 }
 else if (!PRIV_SYSTEM_ROOM(sys_char))
 {
  fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- isn't a system "
                "character.\n", sys_char->name);
  return;
 }

 if ((r = room_add(sys_char, GET_PARAMETER_STR(params, 2))))
 {
  fvtell_player(NORMAL_T(p), " New room created with an id of %s.%s\n",
                sys_char->name, r->id);

  sys_char->flag_tmp_room_needs_saving = TRUE;
  room_save(sys_char);
 }
 else
   P_MEM_ERR(p);
}

static void user_room_exit_add(player *p, const char *str)
{
 room *r = NULL;
 
 if (!(r = room_load_find(p, str, PLAYER_FIND_SC_EXTERN)))
   return;

 if (r == p->location)
 {
  fvtell_player(SYSTEM_T(p), " The room -- ^S^B%s^s --, owned "
                "by -- ^S^B%s^s -- is your current location.\n",
                r->id, r->owner->name);
  return;
 }

 if (room_exit_find(p->location, r->owner->lower_name, r->id))
 {
  fvtell_player(SYSTEM_T(p), " The room -- ^S^B%s^s --, owned "
                "by -- ^S^B%s^s -- already has an exit from this room.\n",
                r->id, r->owner->name);
  return;
 }

 /* stop people adding loads of exits to other people's rooms */
 if ((p->location->exits_num > 3) && (p->saved != p->location->owner) &&
     !p->saved->priv_admin)
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " Sorry, there are already 4 exits from this room."
                " Only the owner can create more.\n");
  return;
 }
 
 if (p->location->exits_num >= p->max_exits)
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " Sorry, you have reached you maximum exit limit "
                "in this room.\n");
  return;
 }

 if (room_exit_add(p->location, r->owner->lower_name, r->id))
   fvtell_player(NORMAL_T(p), "%s", " New exit added.\n");
 else
   fvtell_player(NORMAL_T(p), "%s", " Addition of new exit failed.\n");

 p->saved->flag_tmp_room_needs_saving = TRUE;
}

static exit_node *room_user_exit_find(player *p, room *r, const char *str)
{
 exit_node *to_go = NULL;
 parameter_holder params;
 char *name = NULL;
 char *id = NULL;
 
 get_parameter_init(&params);
 get_parameter_parse(&params, &str, 3);

 switch (params.last_param)
 {
  case 1:
  {
   name = GET_PARAMETER_STR(&params, 1);
   if ((id = N_strchr(name, '.')))
     *id++ = 0;
   else
   {
    id = name;
    name = NULL;
   }
  }
  break;
  
  case 2:
    name = GET_PARAMETER_STR(&params, 1);
    id = GET_PARAMETER_STR(&params, 2);
    break;
    
  default:
    TELL_FORMAT_NO_RETURN(p, "<exit-name> | <player-name> <exit-name>");
    return (NULL);
 }
 
 if (!(to_go = room_exit_find(r, name, id)))
 {
  fvtell_player(SYSTEM_T(p),
                " No such exit -- ^S^B%s^s --", id);
  if (name)
    fvtell_player(SYSTEM_T(p),
                  ", owned by -- ^S^B%s^s --.\n", name);
  fvtell_player(SYSTEM_T(p), "%s", ".\n");
 }

 return (to_go);
}

static void user_room_exit_del(player *p, const char *str)
{
 exit_node *to_go = room_user_exit_find(p, p->location, str);

 if (!to_go)
   return;
 
 room_exit_del(p->location, to_go);
 
 p->saved->flag_tmp_room_needs_saving = TRUE;
 
 fvtell_player(NORMAL_T(p), "%s", " Exit removed.\n");
}

void user_room_check_flags(player *p)
{ 
 room *r = p->location;

 room_load(r, NULL);

 ptell_mid(NORMAL_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p),
           "Room: $Room-Owner-Name.$Room-Id", FALSE);
 
 fvtell_player(NORMAL_T(p), "Where description: %s\n",
               r->where_description);
 
 fvtell_player(NORMAL_T(p), "Enter-msg: Someone%s%.*s^N\n",
               isits1(r->enter_msg),
               OUT_LENGTH_ENTER_MSG, isits2(r->enter_msg));
 
 fvtell_player(NORMAL_T(p), "Bolted: ^S^B%s^s\n",
               TOGGLE_YES_NO(r->flag_bolted));

 fvtell_player(NORMAL_T(p), "Locked: ^S^B%s^s\n",
               TOGGLE_YES_NO(r->flag_locked));
 
 fvtell_player(NORMAL_T(p), "Home: ^S^B%s^s\n",
               TOGGLE_YES_NO(r->flag_home));
 
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void user_check_exits(player *p)
{ /* put it into a ptell_mid/DASH_LEN box ? */
 exit_node *scan = NULL;
 room *from_room = p->location;
 char buf[128];
 
 scan = from_room->exits_start;
 if (!scan)
 {
  fvtell_player(SYSTEM_T(p), "%s", " This room has no exits.\n");
  return;
 } 

 if (from_room->exits_num == 1)
   fvtell_player(NORMAL_T(p), "%s", "\n There is one exit from here.\n\n");
 else if (from_room->exits_num == 0)
   fvtell_player(NORMAL_T(p), "%s", "\n There are no exits from here\n\n");
 else
   fvtell_player(NORMAL_T(p), "\n There are %s exits from here.\n\n",
                 word_number_base(buf, 128, NULL, from_room->exits_num,
                                  FALSE, word_number_def));
  
 while (scan)
 {
  room *to_room = NULL;
  exit_node *scan_next = scan->next;
  char tmp[PLAYER_S_NAME_SZ + ROOM_ID_SZ];

  sprintf(tmp, "%s.%s", scan->name, scan->id);
  
  if (!(to_room = room_load_find(p, tmp, PLAYER_FIND_SC_SYS)))
  {
   fvtell_player(NORMAL_T(p), "%s", " Exit pointing no-where so deleted.\n");
   room_exit_del(from_room, scan);
  }
  else
  {
   char locked = ' ';
   char bolted = ' ';

   room_load(to_room, NULL);
   
   fvtell_player(NORMAL_FT(RAW_OUTPUT, p), "%*s ", ROOM_ID_SZ, to_room->id);

   if (to_room->flag_locked)
     locked = 'L';
   else
     locked = '-';

   if (to_room->flag_bolted)
     bolted = 'B';
   else
     bolted = '-';
   
   fvtell_player(NORMAL_T(p), "-%c%c- %s\n", locked, bolted,
                 to_room->where_description);
  }

  scan = scan_next;
 }
}

static void user_room_auto_add(player *p, const char *str)
{
 room *r = p->location;

 if (!*str)
   TELL_FORMAT(p, "<string>");

 if (strnlen(str, ROOM_AUTOMESSAGE_SZ) >= ROOM_AUTOMESSAGE_SZ)
 {
  fvtell_player(SYSTEM_T(p),
                " That is too long for an automessage. The maximum "
                "is -- ^S^B%d^s -- characters.\n", ROOM_AUTOMESSAGE_SZ);
  return;
 }
 
 if (r->automessages_num >= p->max_autos)
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " Sorry but you have already reached your maximum "
                "number of automessages.\n");
  return;
 }

 if (room_automessage_add(r, str))
   fvtell_player(NORMAL_T(p), "%s", " New message added.\n");
 else
   fvtell_player(NORMAL_T(p), "%s", " New message ^S^BNOT^s added.\n");

 r->owner->flag_tmp_room_needs_saving = TRUE;
}

static void user_room_auto_del(player *p, const char *str)
{
 automessage_node *to_go = NULL;
 int count = atoi(str);
 room *r = p->location;

 if (!count)
   TELL_FORMAT(p, "<number>"); 

 if (!r->automessages_start)
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " There are no automessages in this room to delete.\n");
  return;
 }

 if (!(to_go = room_automessage_find(r, count)))
 {
  fvtell_player(SYSTEM_T(p), " No automessage number -- ^S^B%d^s --.\n",
                count);
  return;
 }
 
 room_automessage_del(r, to_go); 

 r->owner->flag_tmp_room_needs_saving = TRUE;
 
 fvtell_player(NORMAL_T(p), "%s", " Message removed.\n");
}

void user_room_check_autos(player *p)
{
 automessage_node *scan = p->location->automessages_start;
 int number = 1;
 char buf[256];
 
 if (!scan)
 {
  fvtell_player(SYSTEM_T(p), " The room -- ^S^B%s^s --, owned "
                "by -- ^S^B%s^s -- doesn't have any automessages.\n",
                p->location->id, p->location->owner->name);
  return;
 }

 ptell_mid(NORMAL_T(p), "Automessages", FALSE);
 
 fvtell_player(NORMAL_T(p), "Active: ^S^B%s^s.\n",
               TOGGLE_YES_NO(p->location->flag_automessage));
 
 fvtell_player(NORMAL_T(p), "Minimum time: %s\n",
               word_time_long(buf, sizeof(buf),
                              p->location->automessage_min_seconds,
                              WORD_TIME_DEFAULT));
 
 fvtell_player(NORMAL_T(p), "%s", "\n");
 
 while (scan)
 {
  fvtell_player(NORMAL_WFT(7, p), "[%02d] %.*s\n", number,
                OUT_LENGTH_AUTOMESSAGE, scan->message);

  ++number;
  scan = scan->next;
 }

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void room_start_automessage_timer(room *r)
{
 struct timeval tv;
 
 gettimeofday(&tv, NULL);
 
 TIMER_Q_TIMEVAL_ADD_SECS(&tv,
                          (r->automessage_min_seconds + (rand() & 63)), 0);
 
 timer_q_add_static_node(&r->automessage_timer, &room_automessage_timer_queue,
                         r, &tv, TIMER_Q_FLAG_NODE_SINGLE);
}

static void user_check_toggle_autos(player *p, const char *str)
{
 int old_autos = p->location->flag_automessage;
 
 if (!*str)
 {
  user_room_check_autos(p);
  return;
 }
 
 TOGGLE_COMMAND_ON_OFF(p, str, p->location->flag_automessage, TRUE,
                       " Auto messages are %sturned ^S^Bon^s in this room.\n",
                       " Auto messages are %sturned off in this room.\n",
                       TRUE);
                       
 if (!old_autos && p->location->flag_automessage)
   room_start_automessage_timer(p->location);
 else if (old_autos && !p->location->flag_automessage)
   timer_q_del_node(&room_automessage_timer_queue,
                    &p->location->automessage_timer);
}

void user_room_check_all(player *p, const char *str)
{
 char buffer[sizeof("Used %u, of %d rooms") + (BUF_NUM_TYPE_SZ(int) * 2)];
 room *scan = NULL; /* tmp ptr for rooms */
 player *p2 = p;
 
 if (*str && p->saved->priv_normal_su)
 {
  if (!(p2 = player_find_load(p, str, PLAYER_FIND_SC_SU)))
    return;
 }

 sprintf(buffer, "Used %u, of %d rooms", p2->saved->rooms_num, p2->max_rooms);
 ptell_mid(NORMAL_T(p), buffer, FALSE);

 if (p != p2)
   fvtell_player(INFO_TP(p), "Owner: %s\n", p2->saved->name);
 
 if (p2->saved->rooms_num)
 {
  fvtell_player(INFO_TP(p), "%s", "  Num   Id             hLB   Name\n");

  room_load_all(p2->saved);
  
  for (scan = p2->saved->rooms_start; scan; scan = scan->next)
  {
   int count = scan->players_num;
   output_node *tmp_list = NULL;
   tmp_output_list_storage tmp_save;

   save_tmp_output_list(p, &tmp_save);
   fvtell_player(INFO_FTP(OUTPUT_BUFFER_TMP, p), "  % 3d     ", count);
   tmp_list = output_list_grab(p);
   load_tmp_output_list(p, &tmp_save);

   output_list_linkin(p, 3, &tmp_list, 8);
   
   fvtell_player(INFO_TP(p), "%-*s", ROOM_ID_SZ, scan->id);
   
   fvtell_player(INFO_TP(p), "%c%c%c %s\n",
                 (scan->flag_home ? 'h' : '-'),
                 (scan->flag_locked ? 'L' : '-'),
                 (scan->flag_bolted ? 'B' : '-'),
                 scan->where_description);
  }
 }
 
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void room_rejoin_func(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", 
               " Re-Entering room mode. Use ^Bhelp room^N for help\n"
               " To run ^Bnormal commands^N type /<command>\n"
               " Use '^Bend^N' to ^Bleave^N.\n");
}

static int room_command(player *p, const char *str, size_t length)
{
 ICTRACE("room_command");
 
 if (MODE_IN_MODE(p, ROOM))
   MODE_HELPER_COMMAND();
 
 if (!MODE_IN_MODE(p, ROOM) && !*str)
 {
  cmds_function tmp_cmd;
  cmds_function tmp_rejoin;
  
  CMDS_FUNC_TYPE_RET_CHARS_SIZE_T((&tmp_cmd), room_command);
  CMDS_FUNC_TYPE_NO_CHARS((&tmp_rejoin), room_rejoin_func);

  if (mode_add(p, "Room Mode-> ", MODE_ID_ROOM, 0,
               &tmp_cmd, &tmp_rejoin, NULL))
    fvtell_player(NORMAL_T(p), "%s", 
                  " Entering room mode. Use ^Bhelp room^N for help\n"
                  " To run ^Bnormal commands^N type /<command>\n"
                  " Use '^Bend^N' to ^Bleave^N.\n");
  else
    fvtell_player(SYSTEM_T(p), 
                  " You cannot enter room mode as you are in too many "
                  "other modes.\n");
  return (TRUE);
 }
 
 return (cmds_sub_match(p, str, length,
                        &cmds_sub[CMDS_SUB_OFFSET(CMDS_SECTION_ROOM)]));
}

static void user_room_view_commands(player *p)
{
 user_cmds_show_section(p, "room");
}

static void room_exit_mode(player *p)
{
 assert(MODE_IN_MODE(p, ROOM));
  
 fvtell_player(NORMAL_T(p), "%s", " Leaving room mode.\n");

 mode_del(p);
}

static void user_room_change_where_description(player *p, const char *str,
                                               size_t len)
{
 char *text = NULL;
 
 room_load(p->location, NULL);
 
 if (!*str)
 {
  fvtell_player(NORMAL_T(p), " The room ^S^B%s.%s^s has a name of:\n"
                "%s\n", p->location->owner->name, p->location->id, 
                p->location->where_description);
  return;
 }

 if (len >= ROOM_WHERE_DESCRIPTION_SZ)
 {
  fvtell_player(NORMAL_T(p), " Roms can only have a where description of "
                "-- ^S^B%d^s -- bytes.\n", ROOM_WHERE_DESCRIPTION_SZ - 1);
  return;
 }
 
 if (!(text = MALLOC(len + 1)))
 {
  P_MEM_ERR(p);
  return;
 }
 FREE(p->location->where_description);
 p->location->where_description = NULL;
 
 COPY_STR_LEN(text, str, len);
 p->location->where_description = text;
 
 fvtell_player(NORMAL_T(p), " The room ^S^B%s.%s^s %shas a name of:\n"
               "%s\n", p->location->owner->name, p->location->id,
               "^S^Bnow^s ", p->location->where_description);
}

static void user_room_change_id(player *p, const char *str)
{
 if (!*str)
 {
  fvtell_player(NORMAL_T(p), " Current id for this room is '^S^B%s.%s^s'\n",
                p->location->owner->name, p->location->id);
  return;
 }
 
 if (room_user_find(NULL, p->location->owner, str))
 {
  fvtell_player(SYSTEM_T(p), " The room -- ^S^B%s^s --, owned "
                "by -- ^S^B%s^s -- already exists.\n",
                p->location->id, p->location->owner->name);
  return;
 }

 if (*(str + strspn(str, ALPHABET_UPPER ALPHABET_LOWER "0123456789")))
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " A room-id can contain alphanumeric characters"
                " only.\n");
  return;
 }
 
 COPY_STR(p->location->id, str, ROOM_ID_SZ);
 
 fvtell_player(NORMAL_T(p),
               " The id of the room is now '^S^B%s.%s^s'.\n",
               p->saved->name, p->location->id);
}

static void user_room_set_enter_msg(player *p, const char *str, size_t len)
{
 char *text = NULL;
 
 room_load(p->location, NULL);
 
 if (!*str)
 {
  fvtell_player(NORMAL_T(p), " The room ^S^B%s.%s^s has an enter msg of:\n"
                " Someone%s%.*s^N\n",
                p->location->owner->name, p->location->id, 
                isits1(p->location->enter_msg),
                OUT_LENGTH_ENTER_MSG, isits2(p->location->enter_msg));
  return;
 }
 
 if (len >= ROOM_ENTER_MSG_SZ)
 {
  fvtell_player(NORMAL_T(p), " Rooms can only have an enter message of "
                "-- ^S^B%d^s -- bytes.\n", ROOM_ENTER_MSG_SZ - 1);
  return;
 }
 
 if (!(text = MALLOC(len + 1)))
 {
  P_MEM_ERR(p);
  return;
 }
 FREE(p->location->enter_msg);
 p->location->enter_msg = NULL;
 
 COPY_STR_LEN(text, str, len);
 p->location->enter_msg = text;
 
 fvtell_player(NORMAL_T(p), " The room ^S^B%s.%s^s %shas an enter msg of:\n"
               " Someone%s%.*s^N\n", p->location->owner->name,
               p->location->id, "^S^Bnow^s ",
               isits1(p->location->enter_msg),
               OUT_LENGTH_ENTER_MSG, isits2(p->location->enter_msg));
}

static int internal_room_print_name_title(player *scan, va_list ap)
{
 player_tree_node *from = va_arg(ap, player_tree_node *);
 player *p = va_arg(ap, player *);
 twinkle_info *info = va_arg(ap, twinkle_info *);
 int flags = va_arg(ap, int);
 time_t my_now = va_arg(ap, time_t);

 assert(!from);
#ifndef DEBUG
 IGNORE_PARAMETER(from);
#endif
 
 if (scan != p)
   fvtell_player(ALL_T(scan->saved, p, info, flags, my_now), "%s%s%.*s^N\n",
                 scan->saved->name, isits1(scan->title),
                 OUT_LENGTH_TITLE, isits2(scan->title));

 return (TRUE);
}

static void user_room_look(player *p, const char *str)
{
 player_tree_node *from = p ? p->saved : NULL;
 int flags = 3;
 time_t look_now = now;

 room *r = NULL;
 int count = 0;
 twinkle_info the_info;
 twinkle_info *info = &the_info;

 setup_twinkle_info(info);
 
 if (!str)
 { /* login look is the calling function, now just where is that AXE */
  from = NULL;
  flags = 3 | SYSTEM_INFO;
  look_now = 0;
 }
 
 if (!(r = p->location))
 {
  fvtell_player(ALL_T(from, p, NULL, flags, look_now), "%s",
                " Strange, you don't seem to actually be anywhere ?!\n");
  return;
 }
 
 room_load(r, NULL);
 
 info->returns_limit = UINT_MAX;
 
 count = r->players_num;
 
 /* str can be NULL, say when a person logs in */
 if ((str && !strcmp(str, "-")) || (!str && p->flag_room_enter_brief))
   fvtell_player(ALL_T(from, p, info, flags, look_now),
		 "\n^N You are %s^N\n", p->location->where_description);
 else
   fvtell_player(ALL_T(r->owner, p, info, flags, look_now),
		 "^N%s^N", r->description);
 
 if (count > 1)
 {
  char buf[128];
  
  if (count == 2)
    fvtell_player(ALL_T(from, p, NULL, flags, look_now), "%s",
                  "\n\nThere is only one other person here ...\n");
  else
    fvtell_player(ALL_T(from, p, NULL, flags, look_now),
                  "\n\nThere are %s other people here ...\n",
                  word_number_base(buf, 128, NULL, count - 1,
                                   FALSE, word_number_def));
  
  if (count > ROOM_PLAYERS_LIST_LIMIT)
  {
   int tmp_count = 0;
   int out_flags = 0;

   DO_ORDER_TEST(room, p->flag_list_show_inorder, in, cron,
                 (construct_name_list_do, r, count, &tmp_count,
                  CONSTRUCT_NAME_USE_PREFIX | CONSTRUCT_NAME_NO_ME,
                  (priv_test_list_type)NULL, (priv_test_list_type)NULL,
                  &out_flags,
                  ALL_T(from, p, NULL, flags | RAW_OUTPUT, look_now)));

   assert(tmp_count == count);
   
   if (out_flags & CONSTRUCT_NAME_OUT_MID_LINE)
     fvtell_player(NORMAL_T(p), "%s", "\n");
  }
  else
  {
   DO_ORDER_TEST(room, p->flag_list_show_inorder, in, cron,
                 (internal_room_print_name_title, r,
                  ALL_T(NULL, p, NULL, flags, look_now)));
  }
 }
 else
   fvtell_player(ALL_T(from, p, NULL, flags, look_now), "%s",
                 "\n\n There is nobody here but you.\n");
}

int room_can_enter(player *p, room *r, int verbose)
{
 BTRACE("room_can_enter");

 assert(r && p);

 if (p->location == r)
   return (TRUE);
 
 if (timer_q_find_data(&timer_queue_player_jail, p->saved) ||
     timer_q_find_data(&timer_queue_player_no_move, p->saved))
 {
  if (verbose)
    fvtell_player(SYSTEM_T(p), "%s",
                  " You cannot move anywhere at the moment.\n");
  return (FALSE);
 }

 room_load(r, NULL);
 
 if (r->owner == p->saved)
   return (TRUE);

 if (PRIV_SYSTEM_ROOM(r->owner) && p->saved->priv_system_room)
   return (TRUE);

 LIST_ROOM_CHECK_FLAG_START(r, p->saved);

 if (LIST_ROOM_CHECK_FLAG_DO(bolt))
 {
  if (verbose)
    fvtell_player(SYSTEM_T(p), " The room -- ^S^B%s^s --, owned "
                  "by -- ^S^B%s^s -- is bolted.\n",
                  r->id, r->owner->name);
  return (FALSE);
 }

 if (LIST_ROOM_CHECK_FLAG_DO(alter))
   return (TRUE);
 
 if (LIST_ROOM_CHECK_FLAG_DO(bar))
 {
  if (verbose)
    fvtell_player(SYSTEM_T(p), " The room -- ^S^B%s^s --, owned "
                  "by -- ^S^B%s^s -- is barred to you.\n",
                  r->id, r->owner->name);
  return (FALSE);
 }
  
 if (LIST_ROOM_CHECK_FLAG_DO(key))
   return (TRUE);
 
 if (r->flag_locked)
 {
  if (verbose)
    fvtell_player(SYSTEM_T(p), " The room -- ^S^B%s^s --, owned "
                  "by -- ^S^B%s^s -- is locked and you don't have a key.\n",
                  r->id, r->owner->name);
  return (FALSE);
 }

 if (LIST_ROOM_CHECK_FLAG_DO(invite))
   return (TRUE);

 LIST_ROOM_CHECK_FLAG_END();

 if (verbose)
   fvtell_player(SYSTEM_T(p), " The room -- ^S^B%s^s --, owned "
                 "by -- ^S^B%s^s -- is barred to you.\n",
                 r->id, r->owner->name);
 return (FALSE);
}

/* This is all a bit special ... because when someone moves out of a room
 * following someone then you have to restart scanning through the room
 * Ie.
 * room a = a, b, c, d
 *
 * a moves to rom b
 *
 * b and d are following a, and c is following b */
static int internal_room_follow_do(player *scan, va_list ap)
{
 player *mover = va_arg(ap, player *);

 if (!strcasecmp(scan->follow, mover->saved->lower_name))
 {
  room *old_room = scan->location;

  if (!room_can_enter(scan, mover->location, TRUE))
    return (TRUE);
  
  if (room_player_transfer(scan, mover->location, ROOM_TRANS_NO_OWNER_EVENT |
                           ROOM_TRANS_NO_LEAVE_MSG))
  {
   fvtell_player(SYSTEM_T(scan), " You follow ^S^B%s%s^s into ^S^B%s.%s^s.\n",
                 gender_choose_str(mover->gender, "", "", "The ", "The "),
                 mover->saved->name,
                 mover->location->owner->name, mover->location->id);
   
   vtell_room_movement(old_room, mover, " %s%s follow%s %s%s.\n^N\n",
                       gender_choose_str(scan->gender, "", "", "The ", "The "),
                       scan->saved->name,
                       (scan->gender == GENDER_PLURAL) ? "" : "s",
                       gender_choose_str(mover->gender, "", "",
                                         "The ", "The "),
                       mover->saved->name);
   
   ROOM_EVENT_OWNER_INFORM(scan, "followed someone in");

   return (FALSE);
  }
 }

 return (TRUE);
}

int room_player_transfer(player *p, room *r, int flags)
{
 room *old_room = p->location;

 BTRACE("room_player_transfer");
 
 if (r == p->location)
 {
  if (flags & ROOM_TRANS_VERBOSE)
    fvtell_player(SYSTEM_T(p), " You are already in the "
                  "room -- ^S^B%s^s --, owned by -- ^S^B%s^s --.\n",
                  r->id, r->owner->name);

  return (FALSE);
 }
 
 room_load(r, NULL);
 
 if (p->location)
 {
  room_load(p->location, NULL);
  
  if (!(flags & ROOM_TRANS_NO_LEAVE_MSG))
    vtell_room_movement(p->location, p,
                        " %s%s%.*s^N\n", p->saved->name, isits1(r->enter_msg),
                        OUT_LENGTH_ENTER_MSG, isits2(r->enter_msg));
  
  room_player_del(p, p->location);
 }
 
 room_player_add(p, r);

 if (flags & ROOM_TRANS_LOGON)
 {
  const char *connect_msg = p->connect_msg;
  
  if (!connect_msg[0])
    connect_msg = configure.room_connect_msg;

  vtell_room_movement(p->location, p, " >> %s%s%.*s^N <<\n",
                      p->saved->name, 
                      isits1(connect_msg), OUT_LENGTH_CONNECT_MSGS,
                      isits2(connect_msg));
 }
 
 if (!(flags & ROOM_TRANS_NO_ENTER_MSG))
   vtell_room_movement(p->location, p, " %s%s%.*s^N\n", p->saved->name,
                       isits1(p->enter_msg), OUT_LENGTH_ENTER_MSG,
                       isits2(p->enter_msg));

 if (flags & ROOM_TRANS_LOGON)
 {
  assert(!old_room);
  
  if (!(flags & ROOM_TRANS_NO_OWNER_EVENT))
    ROOM_EVENT_OWNER_INFORM(p, "logged on into");

  user_room_look(p, NULL);
 }
 else
 {
  if (!(flags & ROOM_TRANS_NO_OWNER_EVENT))
    ROOM_EVENT_OWNER_INFORM(p, "entered");
 
  user_room_look(p, "");  
 }
 
 if (p->flag_room_exits_show)
   user_check_exits(p);
 
 if (old_room && !(flags & ROOM_TRANS_NO_FOLLOW))
   do
   { /* nothing */
   } while (do_inorder_room(internal_room_follow_do, old_room, p));
 
 return (TRUE);
}

int room_user_player_transfer(player *p, const char *str, int flags)
{
 room *r = NULL;
 int find_flags = PLAYER_FIND_SC_SYS;

 if (flags & ROOM_TRANS_VERBOSE)
   find_flags |= PLAYER_FIND_VERBOSE;
 
 if (!(r = room_load_find(p, str, find_flags)))
   return (FALSE);
 
 if (!(ROOM_TRANS_NO_PERMS & flags) &&
     !room_can_enter(p, r, (flags & ROOM_TRANS_VERBOSE)))
   return (FALSE);
 
 return (room_player_transfer(p, r, flags));
}

static void user_room_player_transfer(player *p, const char *str)
{ 
 if (!*str)
   TELL_FORMAT(p, "[someone]<.room-id>");

 room_user_player_transfer(p, str, ROOM_TRANS_VERBOSE);
}

static void user_room_player_go(player *p, const char *str)
{
 char room_id[PLAYER_S_NAME_SZ + ROOM_ID_SZ + 1];
 exit_node *scan = room_user_exit_find(p, p->location, str);
 room *r = NULL;
 
 if (!scan)
   return;
 
 sprintf(room_id, "%s.%s", scan->name, scan->id);

 if (!(r = room_load_find(p, room_id, PLAYER_FIND_SELF)))
 { /* does room_user_transfer_player, by hand */
  room_exit_del(r, scan);
  fvtell_player(SYSTEM_T(p), " The room -- ^S^B%s^s -- owned by "
                "-- ^S^B%s^s -- no longer exists, exit deleted.\n",
                scan->id, scan->name);
  return;
 }

 if (!room_can_enter(p, r, TRUE))
   return;
 
 room_player_transfer(p, r, ROOM_TRANS_DEFAULT);
}

static void timed_room_automessage(int timed_type, void *passed_room)
{
 room *r = passed_room;
 int count = 0;
 automessage_node *scan = NULL;

 BTRACE("timed_automessage");

 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;
 
 scan = r->automessages_start;
 if (!scan)
 {
  r->flag_automessage = FALSE;
  return;
 }

 count = rand();
 if (count)
   count %= r->automessages_num;
 while ((count > 0) && scan)
 {
  --count;
  scan = scan->next;
 } 
 assert(!count && scan);

 if (scan)
   vtell_room_autos(r, NULL, "%.*s\n", OUT_LENGTH_AUTOMESSAGE, scan->message);

 if (r->players_num)
   room_start_automessage_timer(r);
}

void init_rooms(void)
{
 player_tree_node *sys_char = NULL;
 room *r = NULL;
 
 timer_q_add_static_base(&room_load_timer_queue, timed_room_cleanup,
                         TIMER_Q_FLAG_BASE_DEFAULT);
 timer_q_add_static_base(&room_automessage_timer_queue, timed_room_automessage,
                         TIMER_Q_FLAG_BASE_DEFAULT);

 /* make sure we have at least system.void */
 if ((r = room_load_find(NULL, "system.void", PLAYER_FIND_DEFAULT)))
   return;
 
 /* NOTE: this isn't an error, when it happens the first time --
  * but after that it's _really_ bad */
 wlog("error", " Creating system.void");

 if (!(sys_char = room_add_system_char("system")))
   SHUTDOWN_MEM_ERR();
 if (!(r = room_add(sys_char, "void")))
   SHUTDOWN_MEM_ERR();

 list_system_change(&sys_char->list_room_glob_start, LIST_TYPE_ROOM,
                    "everyone invite on");
 
 CONST_COPY_STR_LEN(r->where_description, "in the void.");
 CONST_COPY_STR_LEN(r->enter_msg, "fall$F-Conjugate into the void.");
 CONST_COPY_STR_LEN(r->description,
                    " This is the system void.\n"
                    " There is nothing here.\n");

 if (!(r = room_add(sys_char, "prison")))
   SHUTDOWN_MEM_ERR();

 list_system_change(&sys_char->list_room_glob_start, LIST_TYPE_ROOM,
                    "sus invite on");
 
 CONST_COPY_STR_LEN(r->where_description, "in the jail.");
 CONST_COPY_STR_LEN(r->enter_msg, "rot$F-Conjugate in jail.");
 CONST_COPY_STR_LEN(r->description, " This is the jail.\n");
 
 sys_char->flag_tmp_room_needs_saving = TRUE;
 room_save(sys_char); 
}

static int internal_delete_room(player *scan,
                                va_list ap __attribute__ ((unused)))
{
 fvtell_player(SYSTEM_T(scan), "%s",
               " The room around you vanishes !\n");
 if (!room_user_player_transfer(scan, "system.void", ROOM_TRANS_NO_PERMS))
   shutdown_error(" Needed system.void, and it's not there.\n");
 
 return (TRUE);
}

void room_del(room *togo)
{
 assert(!togo->players_num);
 
 assert(timer_q_find_data(&room_load_timer_queue, togo));
 timer_q_del_node(&room_load_timer_queue, &togo->load_timer);

 room_load_cleanup(togo);

 internal_room_del(togo->owner, togo); 
 XFREE(togo, ROOM);
}

static void user_room_del(player *p)
{
 room *togo = NULL;
 
 if (!strcmp("system", p->location->owner->lower_name) &&
     !strcmp("void", p->location->id))
 { /* yum exception code :*/
  fvtell_player(SYSTEM_T(p), "%s", " You can't delete the void !\n");
  return;
 }
 
 togo = p->location;
 togo->owner->flag_tmp_room_needs_saving = TRUE;
 
 do_inorder_room(internal_delete_room, togo);
  
 assert(togo != p->location);

 room_del(togo);

 fvtell_player(NORMAL_T(p), "%s", " Room deleted.\n");
}

int room_can_see_location(player *p, player *p2)
{
 if (!p2->flag_location_hide ||
     p->saved->priv_admin || (p2->location->owner == p->saved) ||
     (p->location == p2->location))
   return (TRUE);
 
 LIST_SELF_CHECK_FLAG_START(p2, p->saved);
 if (LIST_SELF_CHECK_FLAG_DO(find))
   return (TRUE);
 LIST_SELF_CHECK_FLAG_END();

 return (FALSE);
}

static int room_user_can_see_location(player *p, player *p2)
{
 int ret = room_can_see_location(p, p2);
 
 if (!ret)
   fvtell_player(INFO_T(p2->saved, p),
                 " The player -- ^S^B%s^s -- %s in hiding.\n",
                 p2->saved->name,
                 (p2->gender == GENDER_PLURAL) ? "are" : "is");
 
 return (ret);
}

static int internal_where(player *p, player *scan)
{
 room_load(scan->location, NULL); 

 if (!room_user_can_see_location(p, scan))
   return (TRUE);
 
 fvtell_player(INFO_FT(RAW_OUTPUT, scan->saved, p), "%s %s%s %s ",
               (p == scan) ? "*" : " ",
               gender_choose_str(scan->gender, "", "", "The ", "The "),
               scan->saved->name,
               (scan->gender == GENDER_PLURAL) ? "are" : "is");
 
 fvtell_player(INFO_T(scan->saved, p), "%.*s",
               OUT_LENGTH_ROOM_NAME, scan->location->where_description);

 if (scan->flag_location_hide)
  fvtell_player(INFO_T(scan->saved, p), " [hiding]");
  
 
 fvtell_player(INFO_T(scan->saved, p), " %c%s.%s%c",
               scan->flag_location_hide ? '[' : '(',
               scan->location->owner->lower_name, scan->location->id,
               scan->flag_location_hide ? ']' : ')');
 
 fvtell_player(INFO_T(scan->saved, p), "\n");
 
 return (TRUE);
}

/* FIXME: use multis */
static int construct_where_name_list_do(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 return (internal_where(p, scan));
}

static void user_room_where(player *p, const char *str)
{
 player *p2 = NULL;
 
 if (*str)
 {
  if (!strcasecmp(str, "am i"))
  {
   fvtell_player(NORMAL_T(p), " You are %s (%s.%s)\n",
                 p->location->where_description,
                 p->location->owner->lower_name, p->location->id);
   return;
  }

  if ((p2 = player_find_on(p, str, PLAYER_FIND_SC_EXTERN_ALL)))
  {
   ptell_mid(INFO_T(p2->saved, p), "location of $F-Name", FALSE);

   internal_where(p, p2);
   
   fvtell_player(INFO_T(p2->saved, p), "%s", DASH_LEN);
  } 
  return;
 }
 
 if (current_players == 1)
   ptell_mid(INFO_TP(p),
             "There is only you on the program at the moment", FALSE);
 else
   ptell_mid(INFO_TP(p),
             "There are $Current_players-Tostr people on the program", FALSE);

 DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
               (construct_where_name_list_do, p));
 
 fvtell_player(INFO_TP(p), "%s", DASH_LEN);
 
 pager(p, PAGER_DEFAULT);
}

static void user_toggle_room_lock(player *p, const char *str)
{
 int old_locked = p->location->flag_locked;
 
 TOGGLE_COMMAND_ON_OFF(p, str, p->location->flag_locked, TRUE,
                       " The current room is %slocked\n",
                       " The current room is %sunlocked\n", TRUE);

 if (old_locked != p->location->flag_locked)
   vtell_room_wall(p->location, p,
                   " %s%s turn%s the key in the door, %slocking the room.\n",
                   gender_choose_str(p->gender, "", "", "The ", "The "),
                   p->saved->name, (p->gender == GENDER_PLURAL) ? "" : "s",
                   p->location->flag_locked ? "" : "un");
}

static void user_toggle_room_bolt(player *p, const char *str)
{
 int old_bolted = p->location->flag_bolted;
 
 TOGGLE_COMMAND_ON_OFF(p, str, p->location->flag_bolted, TRUE,
                       " Room is %sbolted", " Room is %snot bolted", TRUE);
 
 if (p->location->flag_bolted != old_bolted)
   vtell_room_wall(p->location, p,
                   " %s%s slide%s the bolt for the door, %sbolting the "
                   "room.\n",
                   gender_choose_str(p->gender, "", "", "The ", "The "),
                   p->saved->name, (p->gender == GENDER_PLURAL) ? "" : "s",
                   p->location->flag_bolted ? "" : "un");
}

static void user_room_set_home(player *p)
{
 room *scan = p->location->owner->rooms_start;
 
 assert(scan);
 
 for (; scan; scan = scan->next)
   scan->flag_home = FALSE;
 
 p->location->flag_home = TRUE;
 
 fvtell_player(NORMAL_T(p), " Your room '^S^B%s.%s^s' is now your "
               "home room.\n",
               p->location->owner->name, p->location->id);
}

static void user_room_transfer_home(player *p)
{
 room *r = NULL;
 
 if (!(r = room_find_home(p->saved)))
 {
  fvtell_player(SYSTEM_T(p), "%s", " You don't appear to have a home room.\n");
  return;
 }
 
 room_player_transfer(p, r, ROOM_TRANS_VERBOSE);
}

static void user_room_visit(player *p, const char *str)
{
 player_tree_node *sp = NULL;
 room *scan = NULL;
 char room_id[PLAYER_S_NAME_SZ + ROOM_ID_SZ + 1];
 
 if (!(sp = player_find_all(p, str, PLAYER_FIND_SC_EXTERN)))
   return;
 
 if (!(scan = room_find_home(sp)))
 {
  fvtell_player(SYSTEM_T(p),
                " The player -- %s -- doesn't have a home room.\n",
                sp->name);
  return;
 }
 
 sprintf(room_id, "%s.%s", sp->lower_name, scan->id);

 room_user_player_transfer(p, room_id,
                           ROOM_TRANS_VERBOSE | ROOM_TRANS_NO_OWNER_EVENT);
 ROOM_EVENT_OWNER_INFORM(p, "visit$F-Gender(m(ed) f(ed)) your home");
}

static room *room_find_rand_main(player *p)
{
 room *r = room_load_find(p, configure.room_main, PLAYER_FIND_SC_SYS);
  
 return (r);
}

int room_player_rand_main_transfer(player *p, int flags)
{
 room *r = NULL;
 int ret = FALSE;
 
 if ((r = room_find_rand_main(p)))
 {
  char tmp[PLAYER_S_NAME_SZ + ROOM_ID_SZ];
  
  sprintf(tmp, "%s.%s", r->owner->lower_name, r->id);
  
  if (room_user_player_transfer(p, tmp, flags))
    return (TRUE);
 }

 fvtell_player(SYSTEM_T(p), " There no accessible main rooms, atm.\n");
 if (!p->location)
 {
  ret = room_user_player_transfer(p, "system.void",
                                  flags | ROOM_TRANS_NO_PERMS);
  
  log_assert(ret && p->location);
 }
 
 return (p->location != NULL);
}

void user_configure_room_main(player *p, parameter_holder *params)
{ /* multiple main rooms with privs etc. need to be done eventually */
 if (params->last_param != 1)
   TELL_FORMAT(p, "<player>.<id>");

 if (!room_load_find(p, GET_PARAMETER_STR(params, 1), PLAYER_FIND_VERBOSE |
                     PLAYER_FIND_SC_SYS))
   return;

 COPY_STR(configure.room_main, GET_PARAMETER_STR(params, 1),
          PLAYER_S_NAME_SZ + ROOM_ID_SZ);

 fvtell_player(NORMAL_T(p), " Changed main room to '^S^B%s^s'.\n",
               configure.room_main);
 
 configure_save(FALSE);
}

void user_configure_room_connect_msg(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_ISITS(configure.room_connect_msg, str,
                                  "room connect message",
                                  PLAYER_S_CONNECT_MSG_SZ,
                                  OUT_LENGTH_CONNECT_MSGS);
 configure_save(FALSE);
}

void user_configure_room_disconnect_msg(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_ISITS(configure.room_disconnect_msg, str,
                                  "room disconnect message",
                                  PLAYER_S_DISCONNECT_MSG_SZ,
                                  OUT_LENGTH_CONNECT_MSGS);
 configure_save(FALSE);
}

static void user_room_transfer_main(player *p)
{
 if (PRIV_SYSTEM_ROOM(p->location->owner))
   fvtell_player(SYSTEM_T(p), "%s", " You are already in a main room.\n");
 else
   room_player_rand_main_transfer(p, ROOM_TRANS_DEFAULT);
}

static int room_player_can_grab(player *p, player *p2, int force_grab)
{
 int do_grab = FALSE;
 
 if (force_grab)
   do_grab = TRUE;
 else
 {
  LIST_SELF_CHECK_FLAG_START(p2, p->saved);
  if (LIST_SELF_CHECK_FLAG_DO(grab))
    do_grab = TRUE;
  LIST_SELF_CHECK_FLAG_END();
 }
 
 if (!do_grab)
 {
  fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- isn't allowing"
                " you to grab them.\n", p2->saved->name);
  return (FALSE);
 }
 
 if (p2->location == p->location)
 {
  fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is already the same"
                " room that you are.\n", p2->saved->name);
  return (FALSE);
 }
 
 return (TRUE);
}

static void internal_user_grab(player *p, player *p2, int do_force)
{
 if (!room_player_can_grab(p, p2, do_force))
   return;
 
 fvtell_player(SYSTEM_T(p2), " -=> %s%s stretches out and grabs you!\n",
               gender_choose_str(p->gender, "", "", "The ", "The "),
               p->saved->name);
 fvtell_player(SYSTEM_T(p), " You grab the player '^S^B%s^s'.\n",
               p2->saved->name);
 
 vtell_room_movement(p2->location, p2,
                     " %s%s grab%s %s%s from the room.\n",
                     gender_choose_str(p->gender, "", "", "The ", "The "),
                     p->saved->name,
                     (p->gender == GENDER_PLURAL) ? "" : "s",
                     gender_choose_str(p2->gender, "", "", "The ", "The "),
                     p2->saved->name);
 
 if (!room_player_transfer(p2, p->location, ROOM_TRANS_NO_OWNER_EVENT |
                           ROOM_TRANS_NO_LEAVE_MSG | ROOM_TRANS_NO_ENTER_MSG))
 {
  assert(FALSE);
  return;
 }

 vtell_room_movement(p->location, p,
                     " %s%s grab%s %s%s into the room.\n",
                     gender_choose_str(p->gender, "", "", "The ", "The "),
                     p->saved->name,
                     (p->gender == GENDER_PLURAL) ? "" : "s",
                     gender_choose_str(p2->gender, "", "", "The ", "The "),
                     p2->saved->name);
 
 ROOM_EVENT_OWNER_INFORM(p2, "been grabbed into");
}

static void user_grab(player *p, const char *str)
{
 player *p2 = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<player(s)>");
  
 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_EXTERN_ALL)))
   return;

 internal_user_grab(p, p2, FALSE);
}

static void user_su_room_grab(player *p, const char *str)
{
 player *p2 = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<player(s)>");
  
 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_EXTERN_ALL)))
   return;

 internal_user_grab(p, p2, TRUE);
}

static void room_edit_cleanup(player *p)
{ 
 buffer_room_destroy(p);
}

static void room_edit_quit(player *p)
{
 fvtell_player(NORMAL_T(p), "%s",
               " Leaving editor ^S^BWITHOUT^s keeping changes.\n");

 room_edit_cleanup(p);
}

static void room_edit_end(player *p)
{
 room *r = NULL;

 assert(MODE_IN_MODE(p, EDIT));
 
 if (!(r = room_load_find(p, p->buffers->room_buff->edited_room,
                          PLAYER_FIND_SELF)))
 {
  fvtell_player(SYSTEM_T(p), " The room -- ^S^B%s^s -- no longer exists.\n",
                p->buffers->room_buff->edited_room);
  room_edit_cleanup(p);
  return;
 }

 if (r->description)
   FREE(r->description);

 if (!(r->description = edit_malloc_dump(p, NULL)))
   SHUTDOWN_MEM_ERR();
 
 r->owner->flag_tmp_room_needs_saving = TRUE;

 fvtell_player(NORMAL_T(p), "%s", " Ending edit and ^S^BKEEPING^s changes.\n");

 room_edit_cleanup(p);
}

static void user_room_edit(player *p)
{
 int created = 0;
 
 if ((created = buffer_room_create(p)) > 0)
 {
  P_MEM_ERR(p);
  return;
 }
 else if (created < 0)
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " Cannot edit a room whilst using the current command, "
                "sorry.\n");
  return;
 }
 
 if (edit_start(p, p->location->description))
 {
  assert(MODE_IN_MODE(p, EDIT));
  
  edit_limit_characters(p, ROOM_DESCRIPTION_CHARS_SZ);
  edit_limit_lines(p, ROOM_DESCRIPTION_LINES_SZ);
  
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_quit, room_edit_quit);
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_end, room_edit_end);
 }
 else
   buffer_room_destroy(p);
}

static room *room_random(player *p, room *current)
{
 player_tree_node *scan;
 room *r;
 int i = 0;
 
 while ((scan = player_tree_random()) && (i < 20))
 {
  for (r = scan->rooms_start; r; r = r->next) /* check one person's rooms */
    if (room_can_enter(p, r, FALSE) && (r != current))
      return (r);
  
  ++i;
 }
 
 return (NULL);  /* we have checked 20 ppl's rooms and didn't find one */
}

static void user_room_bounce(player *p)
{
 room *r = NULL;
 
 if ((r = room_random(p, p->location)))
 {
  vtell_room_movement(p->location, p, " %s spins round, and bounces "
                      "into the air !!\n", p->saved->name);
  
  room_player_transfer(p, r, ROOM_TRANS_NO_LEAVE_MSG |
                       ROOM_TRANS_NO_OWNER_EVENT);
  
  ROOM_EVENT_OWNER_INFORM(p, "bounced into");
 }
 else
   fvtell_player(SYSTEM_T(p), "%s",
                 " Tried 20 people and no-one had a single room open.\n");
}

static int internal_room_boot(player *p, player *booted)
{
 room *r = booted->location;
 int do_boot = FALSE;
 
 assert(p && booted);

 if (r->owner != p->saved)
 {
  LIST_ROOM_CHECK_FLAG_START(r, p->saved);
  if (LIST_ROOM_CHECK_FLAG_DO(boot))
    do_boot = TRUE;
  LIST_ROOM_CHECK_FLAG_END();

  if (!do_boot)
  {
   fvtell_player(SYSTEM_T(p),
                 " The player -- ^S^B%s^s -- is not in a room "
                 "that you can boot %s from.\n", booted->saved->name,
                 gender_choose_str(booted->gender, "him", "her",
                                   "them", "it"));
   return (FALSE);
  }
 }

 fvtell_player(SYSTEM_T(booted),
               " %s%s pick%s you up and boot%s you from the room.\n",
               gender_choose_str(p->gender, "", "", "The ", "The "),
               p->saved->name,
               (p->gender == GENDER_PLURAL) ? "" : "s",
               (p->gender == GENDER_PLURAL) ? "" : "s");
 
 if (room_player_rand_main_transfer(booted, ROOM_TRANS_VERBOSE |
                                    ROOM_TRANS_NO_LEAVE_MSG |
                                    ROOM_TRANS_NO_ENTER_MSG))
 {
  ROOM_EVENT_OWNER_INFORM(p, "booted into");
  
  vtell_room_movement(p->location, p,
                      " %s%s %s picked up by %s%s and booted from the room.\n",
                      gender_choose_str(booted->gender, "", "",
                                        "The ", "The "),
                      booted->saved->name,
                      (booted->gender == GENDER_PLURAL) ? "are" : "is",
                      gender_choose_str(p->gender, "", "", "The ", "The "),
                      p->saved->name);
  
  vtell_room_movement(booted->location, booted,
                      " %s%s dusts %s down after getting booted.\n",
                      gender_choose_str(booted->gender, "", "",
                                        "The ", "The "),
                      booted->saved->name,
                      gender_choose_str((p)->gender, "himself", "herself",
                                        "themselves", "itself"));
 }
 
 return (TRUE);
}

static void user_room_boot_out(player *p, const char *str)
{
 player *p2 = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<player(s)>");
  
 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_EXTERN_ALL &
                           ~PLAYER_FIND_SELF)))
   return;
 
 if (!internal_room_boot(p, p2))
   fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- can't be booted "
                 "out of the room by you.\n", p2->saved->name);
 else
   fvtell_player(NORMAL_T(p), " You boot '^S^B%s^s' out of the room.\n",
                 p2->saved->name);
}

static void user_room_join(player *p, const char *str)
{
 player *p2 = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<person>");

 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_EXTERN_ALL)))
   return;
 
 if (!room_user_can_see_location(p, p2))
   return;
 
 if (p2->location == p->location)
 {
  fvtell_player(INFO_T(p2->saved, p),
                " You are already in the same room as %s%s.\n",
                gender_choose_str(p2->gender, "", "", "the ", "the "),
                "$F-Name_full");
  return;
 }

 if (!room_can_enter(p, p2->location, TRUE))
   return;

 if (room_player_transfer(p, p2->location, ROOM_TRANS_NO_OWNER_EVENT))
   ROOM_EVENT_OWNER_INFORM(p, "joined someone in");
}

static void user_room_change_automessage_time(player *p, const char *str)
{
 room *r = p->location;
 int num = word_time_parse(str, WORD_TIME_PARSE_DEFAULT, NULL);
 char buf[256];

 if (!*str)
 {
  fvtell_player(NORMAL_T(p), " The current automessage time is set to "
                "'^S^B%d^s'.\n", r->automessage_min_seconds);
  return;
 }
 
 if (num <= 0)
   TELL_FORMAT(p, "<number>");
 
 r->automessage_min_seconds = num;
 fvtell_player(NORMAL_T(p),
               " Autos set to minimum speed of every '^S^B%s^s'.\n",
               word_time_long(buf, sizeof(buf), num, WORD_TIME_DEFAULT));
}

static void user_room_with(player *p, const char *str)
{
 int count = 0;
 player *p2 = NULL;
 int tmp_count = 0;
 int out_flags = 0;
 
 if (!*str)
   TELL_FORMAT(p, "<player>");
 
 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_EXTERN_ALL)))
   return;

 if (!room_user_can_see_location(p, p2))
   return;

 if (!p->saved->priv_admin && !room_can_enter(p, p2->location, TRUE))
 {
  fvtell_player(SYSTEM_T(p),
                " The player%s -- ^S^B%s^s -- %s in a location that you"
                " cannot enter.\n",
                (p->gender == GENDER_PLURAL) ? "s" : "",
                p2->saved->name,
                (p->gender == GENDER_PLURAL) ? "are" : "is");
  return;
 }

 count = p2->location->players_num;
 if (count == 1)
 {
  fvtell_player(SYSTEM_T(p),
                " The room -- ^S^B%s^s --, owned "
                "by -- ^S^B%s^s -- contains only one player.\n",
                p2->location->id, p2->location->owner->name);
  return;
 }

 ptell_mid(INFO_T(p2->saved, p), "players with $F-Gender(pl(The ))$F-Name",
           FALSE);

 DO_ORDER_TEST(room, p->flag_list_show_inorder, in, cron,
               (construct_name_list_do, p2->location, INT_MAX, &tmp_count,
                CONSTRUCT_NAME_USE_PREFIX | CONSTRUCT_NAME_NO_ME,
                (priv_test_list_type)NULL, (priv_test_list_type)NULL,
                &out_flags,
                INFO_FT(RAW_OUTPUT, p2->saved, p)));
 
 assert(tmp_count == count);
   
 if (out_flags & CONSTRUCT_NAME_OUT_MID_LINE)
   fvtell_player(NORMAL_T(p), "%s", "\n");

 fvtell_player(INFO_T(p2->saved, p), "%s", DASH_LEN);
}

static void user_su_room_reown(player *p, parameter_holder *params)
{
 player_tree_node *sp;
 room *r = p->location;
 
 if (params->last_param != 2)
   TELL_FORMAT(p, "<player>.<room-id> <player>");
 
 if (!(r = room_load_find(p, GET_PARAMETER_STR(params, 1),
                          PLAYER_FIND_SC_SYS)))
   return;

 if (!(sp = player_find_all(p, GET_PARAMETER_STR(params, 2),
                            PLAYER_FIND_SC_SU)))
   return;

 r->owner->flag_tmp_room_needs_saving = TRUE;
 
 internal_room_del(r->owner, r);
 
 internal_room_add(sp, r);

 r->owner->flag_tmp_room_needs_saving = TRUE;
 
 fvtell_player(NORMAL_T(p), "%s", " Room transfered.\n");
}

static int internal_mindseye(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 char buf[256];
 
 fvtell_player(INFO_T(scan->saved, p), "  %s%s %.*s (%s idle)\n",
               gender_choose_str(scan->gender, "", "", "The ", "The "),
               "$F-Name_full",
               OUT_LENGTH_TITLE, scan->title,
               word_time_long(buf, sizeof(buf),
                              difftime(now, scan->last_command_timestamp),
                              WORD_TIME_ALL));

 return (TRUE);
}

static void user_room_mindseye(player *p, const char *str)
{
 room *scan = p->saved->rooms_start;

 /* TODO: do we want to have it availbe for any room owned by the player ? */
 IGNORE_PARAMETER(str);
 
 if (!(scan = room_find_home(p->saved)))
 {
  fvtell_player(SYSTEM_T(p), "%s", " You don't have a home room.\n");
  return;
 }
 
 if (scan == p->location)
 {
  fvtell_player(SYSTEM_T(p), " The room -- ^S^B%s^s --, owned "
                "by -- ^S^B%s^s -- is your current location.\n",
                p->location->id, p->location->owner->name);
  return;
 }
 
 if (!scan->room_list_alpha)
 {
  assert(!scan->room_list_cron);
  fvtell_player(NORMAL_T(p), "%s", " There is no-one in your home room.\n");
  return;
 } 

 ptell_mid(NORMAL_T(p), "Home room", FALSE);

 DO_ORDER_TEST(room, p->flag_list_show_inorder, in, cron,
               (internal_mindseye, scan, p));
 
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void jail_command_del(player *p)
{
 if (p->location) /* if they are loogging off, don't */
   room_player_rand_main_transfer(p, ROOM_TRANS_VERBOSE);
}

static int jail_command(player *p, const char *str, size_t length)
{
 return (cmds_sub_match(p, str, length, &cmds_misc[CMDS_MISC_RESTRICTED]));
}

void room_enter_jail(player *p, int flags)
{
 cmds_function tmp_cmd;
 cmds_function tmp_cleanup;
 
 CMDS_FUNC_TYPE_RET_CHARS_SIZE_T((&tmp_cmd), jail_command);
 CMDS_FUNC_TYPE_NO_CHARS((&tmp_cleanup), jail_command_del);

 room_user_player_transfer(p, "system.prison", flags | ROOM_TRANS_NO_PERMS);
 
 while (!mode_add(p, p->prompt, MODE_ID_JAIL, 0, &tmp_cmd, NULL, &tmp_cleanup))
   mode_del(p);
}

static void user_set_login_room(player *p, const char *str)
{
 room *r = NULL;
 
 if (!*str)
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You will now connect to the main room.\n");
  *p->room_connect = 0;
  p->flag_trans_to_home = FALSE;
  return;
 }
 
 if (!(r = room_load_find(p, str, PLAYER_FIND_SC_SYS)))
   return; 
 
 if (!room_can_enter(p, r, TRUE))
   return;
 
 COPY_STR(p->room_connect, str, PLAYER_S_ROOM_CONNECT_SZ);

 fvtell_player(SYSTEM_T(p), " You will now attempt to connect to "
               "room on logon '^S^B%s^s'.\n", p->room_connect);
 
 p->flag_trans_to_home = FALSE;
}

static void user_su_room_barge(player *p, const char *str)
{
 room *to = NULL;
 
 if ((to = room_load_find(p, str, PLAYER_FIND_SC_SU)))
   room_player_transfer(p, to, ROOM_TRANS_DEFAULT);
}

static void user_toggle_inform_room_enter(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_see_room_events, TRUE,
                       " You will %sbe informed when someone enters "
                       "one of your rooms.\n",
                       " You will %snot be informed when someone enters "
                       "one of your rooms.\n", TRUE);
}

/* Follow command, idea thanks to Slaine among others no doubt */
static void user_follow_player(player *p, const char *str)
{
 player *p2 = NULL;
 
 if (!*str)
 {
  if (p->follow[0] != 0)
  {
   fvtell_player(NORMAL_T(p), " You are currently following %s.\n", p->follow);
   return;
  }
  else
    TELL_FORMAT(p, "<name>/off");
 }
 
 if (TOGGLE_MATCH_OFF(str))
 {  
  fvtell_player(NORMAL_T(p), "%s", " You stop following anyone.\n");
  p->follow[0] = 0;

  if (p->follow[0])
  {
   if ((p2 = player_find_on(p, p->follow, PLAYER_FIND_SC_INTERN_ALL)))
   {
    LIST_COMS_CHECK_FLAG_START(p2, p->saved);
    if (LIST_COMS_CHECK_FLAG_DO(tells))
      return; /* wanted ? */
    LIST_COMS_CHECK_FLAG_END();
    fvtell_player(SYSTEM_T(p2), " -=> ^S^B%s^s stops following you.\n",
                  p->saved->name);
   }
  }
  return;
 }
 
 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_EXTERN_ALL &
                           ~PLAYER_FIND_SELF)))
   return;
 
 if (!room_can_see_location(p, p2))
 {
  fvtell_player(SYSTEM_T(p),
                " The player -- ^S^B%s^s -- %s in hiding, so you "
                "can't follow them.\n",
                p2->saved->name,
                 (p2->gender == GENDER_PLURAL) ? "are" : "is");
  return;
 }
 
 if (p2->flag_follow_block)
 {
  fvtell_player(SYSTEM_T(p),
                " The player -- ^S^B%s^s -- won't let you follow %s.\n",
                p2->saved->name,
                gender_choose_str(p->gender, "him", "her",
                                  "them", "it"));
  return;
 }
  
 if (!strcasecmp(p2->saved->name, p->follow))
 {
  fvtell_player(SYSTEM_T(p),
                " The player -- ^S^B%s^s -- is already being "
                "followed by you.\n", p2->saved->name);
  return;
 }
  
 fvtell_player(NORMAL_T(p), " You start to follow '^S^B%s^s'.\n",
               p2->saved->name);
 COPY_STR(p->follow, p2->saved->name, PLAYER_S_NAME_SZ);

 LIST_COMS_CHECK_FLAG_START(p2, p->saved);
 if (LIST_COMS_CHECK_FLAG_DO(tells))
   return; /* want this ? -- nofollow will do */
 LIST_COMS_CHECK_FLAG_END();
 
 fvtell_player(SYSTEM_T(p2),
               " -=> ^S^B%s^s starts following you.\n", p->saved->name);
}

static void user_toggle_follow_block(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_follow_block, TRUE,
                       " Nobody can follow you %s"
                       "(^S^Bgoaway^s stops those already following).\n",
                       " People can %sfollow you.\n", TRUE);
}

static int internal_follow_check_inorder(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 int *done = va_arg(ap, int *);
 
 if (!strcmp(scan->follow, p->saved->name))
 {
  if (!*done)
    ptell_mid(NORMAL_T(p), "Followed by", FALSE);
  
  fvtell_player(NORMAL_T(p), "%s\n", scan->saved->name);

  ++*done;
 }

 return (FALSE);
}

void user_follow_check(player *p)
{
 int done = FALSE;
 
 do_inorder_logged_on(internal_follow_check_inorder, p, &done);
 
 if (done)
   fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 else
   fvtell_player(SYSTEM_T(p), "%s", " Nobody is following you right now.\n");
}

static int internal_follow_go_away_inorder(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);

 if (strcasecmp(scan->follow, p->saved->name))
   return (TRUE);
 
 scan->follow[0] = 0;
 
 LIST_COMS_CHECK_FLAG_START(scan, p->saved);
 if (LIST_COMS_CHECK_FLAG_DO(tells))
   return (TRUE);
 LIST_COMS_CHECK_FLAG_END();
 
 fvtell_player(SYSTEM_FT(HIGHLIGHT, scan),
               " %s ask%s you to stop following %s.\n",
               p->saved->name,
               gender_choose_str(p->gender, "s", "s", "", ""),
               gender_choose_str(p->gender, "him", "her", "them", "it"));

 return (FALSE);
}

static void user_follow_go_away(player *p, const char *str)
{
 player *p2 = NULL; 

 if (!*str)
   TELL_FORMAT(p, "<name>/'list'/'all'");
 
 if (!strcasecmp(str, "list"))
   user_follow_check(p);
 else if (!strcasecmp(str, "all"))
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You ask everyone to stop following you.\n");

  do_inorder_logged_on(internal_follow_go_away_inorder, p);
 }
 else if ((p2 = player_find_on(p, str, PLAYER_FIND_SC_EXTERN_ALL)))
 {
  if (strcasecmp(p2->follow, p->saved->name))
    fvtell_player(SYSTEM_T(p), "%s", " That person isn't following you.\n");
  else
  {     
   fvtell_player(NORMAL_T(p),
                 " You tell %s to stop following you.\n", p2->saved->name);
   p2->follow[0] = 0;
   
   LIST_COMS_CHECK_FLAG_START(p2, p->saved);
   if (LIST_COMS_CHECK_FLAG_DO(tells))
     return;
   LIST_COMS_CHECK_FLAG_END();
   
   fvtell_player(TALK_FT(HIGHLIGHT, p->saved, p2),
                 " %s asks you to please stop following them.\n",
                 p->saved->name);
  }
 }
}

player_tree_node *room_random_player(room *r)
{
 int count = get_random_num(1, r->players_num);
 player_tree_node *ret = NULL;
 
 assert(r && r->room_list_alpha && r->room_list_cron);

 ret = find_player_room_count(FALSE, r, count);
 
 assert(ret);
 
 return (ret);
}

static void user_toggle_room_brief(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_room_enter_brief, TRUE,
                       " You are %signoring room descriptions.\n",
                       " You are %sseeing room descriptions.\n", TRUE);
}

static void user_toggle_room_hide(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_location_hide, TRUE,
                       " You are %shidden away from prying eyes.\n",
                       " People can %sfind out where you are.\n", TRUE);
 
 if (p->flag_location_hide)
   if (!p->flag_follow_block)
     fvtell_player(NORMAL_T(p), "%s", 
                   " People can still follow you however. Use goaway to"
                   " stop them.\n");
}

static void user_toggle_room_logon_home(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_trans_to_home, TRUE,
                       " You will %sbe taken to your home, "
                       "when you log in.\n",
                       " You will %sbe taken to an entrance room, "
                       "when you log in.\n", TRUE);

 if (p->flag_trans_to_home)
   *p->room_connect = 0;
}

static void user_set_enter_msg(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_ISITS(p->enter_msg, str, "enter message",
                           PLAYER_S_ENTER_MSG_SZ, OUT_LENGTH_ENTER_MSG);
}

static void user_toggle_show_exits(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_room_exits_show, TRUE,
                       " When you enter a room you will %ssee the exits.\n",
                       " When you enter a room you won't %ssee the exits.\n",
                       TRUE);
}

void cmds_init_room(void)
{
 CMDS_BEGIN_DECLS();

#define CMDS_SECTION_SUB CMDS_SECTION_ROOM

 CMDS_ADD_SUB("-auto", user_room_auto_del, CONST_CHARS);
 CMDS_FLAG(no_clever_expand); CMDS_PRIV(command_room_alter);
 CMDS_ADD_SUB("+auto", user_room_auto_add, CONST_CHARS);
 CMDS_FLAG(no_clever_expand); CMDS_PRIV(command_room_alter);
 CMDS_ADD_SUB("-exit", user_room_exit_del, CONST_CHARS);
 CMDS_FLAG(no_clever_expand); CMDS_PRIV(command_room_alter);
 CMDS_ADD_SUB("+exit", user_room_exit_add, CONST_CHARS);
 CMDS_FLAG(no_clever_expand); CMDS_PRIV(command_room_link);
 CMDS_ADD_SUB("autos", user_check_toggle_autos, CONST_CHARS);
 CMDS_PRIV(command_room_alter);
 CMDS_ADD_SUB("bolt", user_toggle_room_bolt, CONST_CHARS);
 CMDS_PRIV(command_room_bolt);
 CMDS_ADD_SUB("check", user_room_check_flags, NO_CHARS);
 CMDS_ADD_SUB("clist", user_list_room_del, PARSE_PARAMS);
 CMDS_PRIV(command_room_grant);
 CMDS_ADD_SUB("commands", user_room_view_commands, NO_CHARS);
 CMDS_ADD_SUB("create", user_room_add, PARSE_PARAMS);
 CMDS_PRIV(command_room);
 CMDS_ADD_SUB("delete", user_room_del, NO_CHARS);
 CMDS_FLAG(no_expand); CMDS_PRIV(command_room_alter);
 CMDS_ADD_SUB("edit", user_room_edit, NO_CHARS);
 CMDS_PRIV(command_room_alter);
 CMDS_ADD_SUB("end", room_exit_mode, NO_CHARS);
 CMDS_FLAG(no_expand);
 CMDS_ADD_SUB("entermsg", user_room_set_enter_msg, CHARS_SIZE_T);
 CMDS_PRIV(command_room_alter);
 CMDS_ADD_SUB("go", user_room_player_go, CONST_CHARS);
 CMDS_PRIV(command_room);
 CMDS_ADD_SUB("home", user_room_transfer_home, NO_CHARS);
 CMDS_ADD_SUB("identify", user_room_change_id, CONST_CHARS);
 CMDS_PRIV(command_room_alter);
 CMDS_ADD_SUB("information", user_room_check_flags, NO_CHARS);
 CMDS_PRIV(command_room);
 CMDS_ADD_SUB("list", user_list_room_view, PARSE_PARAMS);
 CMDS_PRIV(command_room_list);
 CMDS_ADD_SUB("list_change", user_list_room_change, PARSE_PARAMS);
 CMDS_PRIV(command_room_grant);
 CMDS_ADD_SUB("lock", user_toggle_room_lock, CONST_CHARS);
 CMDS_PRIV(command_room_lock);
 CMDS_ADD_SUB("look", user_room_look, CONST_CHARS);
 CMDS_PRIV(command_room);
 CMDS_ADD_SUB("name", user_room_change_where_description, CHARS_SIZE_T);
 CMDS_PRIV(command_room_alter);
 CMDS_ADD_SUB("where_description",
              user_room_change_where_description, CHARS_SIZE_T);
 CMDS_PRIV(command_room_alter);
 CMDS_ADD_SUB("notify", user_toggle_inform_room_enter, CONST_CHARS);
 CMDS_PRIV(command_room);
 CMDS_ADD_SUB("rooms_list", user_room_check_all, CONST_CHARS);
 CMDS_PRIV(command_room);
 CMDS_ADD_SUB("sethome", user_room_set_home, NO_CHARS);
 CMDS_PRIV(command_room_alter);
 CMDS_ADD_SUB("speed_automessages",
              user_room_change_automessage_time, CONST_CHARS);
 CMDS_PRIV(command_room_alter);
 CMDS_ADD_SUB("system_create", user_room_system_create, PARSE_PARAMS);
 CMDS_PRIV(alter_system_room);
 CMDS_ADD_SUB("transfer", user_room_player_transfer, CONST_CHARS);
 CMDS_PRIV(command_room);
 CMDS_ADD_SUB("transfer_ownership", user_su_room_reown, PARSE_PARAMS);
 CMDS_PRIV(senior_su);

#undef CMDS_SECTION_SUB
 
 CMDS_ADD("room", room_command, RET_CHARS_SIZE_T, LOCAL);

 CMDS_ADD("barge", user_su_room_barge, CONST_CHARS, ADMIN);
 CMDS_PRIV(admin);
 CMDS_ADD("bounce", user_room_bounce, NO_CHARS, LOCAL);
 CMDS_ADD("connect_room", user_set_login_room, CONST_CHARS, LOCAL);
 CMDS_PRIV(base);
 CMDS_ADD("connect_home", user_toggle_room_logon_home, CONST_CHARS, SETTINGS);
 CMDS_PRIV(command_room);
 CMDS_ADD("ghome", user_toggle_room_logon_home, CONST_CHARS, HIDDEN);
 CMDS_PRIV(command_room); /* EW compat */
 CMDS_ADD("entermsg", user_set_enter_msg, CONST_CHARS, PERSONAL_INFO);
 CMDS_ADD("exits", user_check_exits, NO_CHARS, LOCAL);
 CMDS_XTRA_SECTION(INFORMATION);
 CMDS_XTRA_SUB(CHECK, CHECK); CMDS_XTRA_SUB(ROOM, ROOM);
 CMDS_ADD("follow_player", user_follow_player, CONST_CHARS, SETTINGS);
 CMDS_ADD("force_grab", user_su_room_grab, CONST_CHARS, LOCAL);
 CMDS_PRIV(senior_su); CMDS_FLAG(no_expand); CMDS_XTRA_SECTION(SU);
 CMDS_ADD("go", user_room_player_go, CONST_CHARS, LOCAL);
 CMDS_ADD("goaway", user_follow_go_away, CONST_CHARS, SETTINGS);
 CMDS_ADD("grab", user_grab, CONST_CHARS, LOCAL);
 CMDS_XTRA_SECTION(MISC);
 CMDS_ADD("hide", user_toggle_room_hide, CONST_CHARS, SETTINGS);
 CMDS_ADD("home", user_room_transfer_home, NO_CHARS, LOCAL);
 CMDS_ADD("join", user_room_join, CONST_CHARS, LOCAL);
 CMDS_XTRA_SECTION(MISC);
 CMDS_ADD("look", user_room_look, CONST_CHARS, LOCAL);
 CMDS_XTRA_MISC(RESTRICTED);
 CMDS_ADD("main", user_room_transfer_main, NO_CHARS, LOCAL);
 CMDS_ADD("mindseye", user_room_mindseye, CONST_CHARS, INFORMATION);
 CMDS_ADD("transfer", user_room_player_transfer, CONST_CHARS, LOCAL);
 CMDS_ADD("visit", user_room_visit, CONST_CHARS, LOCAL);
 CMDS_ADD("where", user_room_where, CONST_CHARS, LOCAL);
 CMDS_XTRA_SECTION(INFORMATION);
 CMDS_ADD("with", user_room_with, CONST_CHARS, INFORMATION);

 CMDS_ADD("nofollow", user_toggle_follow_block, CONST_CHARS, SETTINGS);

 CMDS_ADD("boot", user_room_boot_out, CONST_CHARS, LOCAL);
 CMDS_PRIV(command_room); CMDS_XTRA_SECTION(SU);
 
 CMDS_ADD("brief", user_toggle_room_brief, CONST_CHARS, LOCAL);
 CMDS_XTRA_SECTION(SETTINGS);

 CMDS_ADD("showexits", user_toggle_show_exits, CONST_CHARS, LOCAL);
 CMDS_PRIV(base); CMDS_XTRA_SECTION(SETTINGS);
}
