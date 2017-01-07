#define LISTS_C
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


Timer_q_base list_load_player_timer_queue; /* for room lists */

static const char *list_node_name(list_node *current)
{
 player *tmp = player_find_on(NULL, current->name, PLAYER_FIND_DEFAULT);

 if (tmp)
   return (tmp->saved->name);

 return (current->name);
}

#if LIST_DEBUG
int list_check_flag_self(list_node *entry, int ret)
{
 list_self_node *tmp = (list_self_node *)entry;

 assert(tmp->list_type == LIST_TYPE_SELF);
 
 return (ret);
}

int list_check_flag_coms(list_node *entry, int ret)
{
 list_coms_node *tmp = (list_coms_node *)entry;

 assert(tmp->list_type == LIST_TYPE_COMS); 

 return (ret);
}

int list_check_flag_room(list_node *entry, int ret)
{
 list_room_node *tmp = (list_room_node *)entry;

 assert(tmp->list_type == LIST_TYPE_ROOM); 

 return (ret);
}

int list_check_flag_game(list_node *entry, int ret)
{
 list_game_node *tmp = (list_game_node *)entry;

 assert(tmp->list_type == LIST_TYPE_GAME); 

 return (ret);
}

int list_check_flag_chan(list_node *entry, int ret)
{
 list_chan_node *tmp = (list_chan_node *)entry;

 assert(tmp->list_type == LIST_TYPE_CHAN); 

 return (ret);
}
#endif

static int list_player_all_used_entry(int list_type, list_node *entry)
{
 switch (list_type)
 {
  case LIST_TYPE_SELF:
    return (LIST_FLAG(entry, self, article_inform) &&
            LIST_FLAG(entry, self, find) &&
            LIST_FLAG(entry, self, friend) &&
            LIST_FLAG(entry, self, grab) &&
            LIST_FLAG(entry, self, inform) &&
            LIST_FLAG(entry, self, inform_beep) &&
            TRUE);
  case LIST_TYPE_COMS:
    return (LIST_FLAG(entry, coms, says) &&
            LIST_FLAG(entry, coms, tells) &&
            LIST_FLAG(entry, coms, shouts) &&
            LIST_FLAG(entry, coms, multis) &&
            LIST_FLAG(entry, coms, tfs) &&
            LIST_FLAG(entry, coms, tfsof) &&
            LIST_FLAG(entry, coms, channels) &&
            LIST_FLAG(entry, coms, echos) &&
            LIST_FLAG(entry, coms, wakes) &&
            LIST_FLAG(entry, coms, sessions) &&
            LIST_FLAG(entry, coms, comments) &&
            LIST_FLAG(entry, coms, autos) &&
            LIST_FLAG(entry, coms, movement) &&
            TRUE);
  case LIST_TYPE_ROOM:
    return (LIST_FLAG(entry, room, alter) &&
            LIST_FLAG(entry, room, bar) &&
            LIST_FLAG(entry, room, bolt) &&
            LIST_FLAG(entry, room, boot) &&
            LIST_FLAG(entry, room, invite) &&
            LIST_FLAG(entry, room, grant) &&
            LIST_FLAG(entry, room, key) &&
            LIST_FLAG(entry, room, link) &&
            TRUE);

  case LIST_TYPE_GAME:
    return (LIST_FLAG(entry, game, draughts) &&
            LIST_FLAG(entry, game, sps) &&
            LIST_FLAG(entry, game, ttt) &&
            TRUE);

  case LIST_TYPE_CHAN:
    return (LIST_FLAG(entry, chan, boot) &&
            LIST_FLAG(entry, chan, config) &&
            LIST_FLAG(entry, chan, grant) &&
            LIST_FLAG(entry, chan, read) &&
            LIST_FLAG(entry, chan, who) &&
            LIST_FLAG(entry, chan, write) &&
            TRUE);

  default:
    log_assert(FALSE);
    return (FALSE);
 }
}

static void list_player_all_set_entry(int list_type, list_node *entry, int val)
{
 switch (list_type)
 {
  case LIST_TYPE_SELF:
    ((list_self_node *)entry)->article_inform = val;
    ((list_self_node *)entry)->find = val;
    ((list_self_node *)entry)->friend = val;
    ((list_self_node *)entry)->grab = val;
    ((list_self_node *)entry)->inform = val;
    ((list_self_node *)entry)->inform_beep = val;
    return;
  case LIST_TYPE_COMS:
    ((list_coms_node *)entry)->says = val;
    ((list_coms_node *)entry)->tells = val;
    ((list_coms_node *)entry)->multis = val;
    ((list_coms_node *)entry)->shouts = val;
    ((list_coms_node *)entry)->tfs = val;
    ((list_coms_node *)entry)->tfsof = val;
    ((list_coms_node *)entry)->channels = val;
    ((list_coms_node *)entry)->echos = val;
    ((list_coms_node *)entry)->wakes = val;
    ((list_coms_node *)entry)->sessions = val;
    ((list_coms_node *)entry)->comments = val;
    ((list_coms_node *)entry)->autos = val;
    ((list_coms_node *)entry)->movement = val;
    return;
  case LIST_TYPE_ROOM:
    ((list_room_node *)entry)->alter = val;
    ((list_room_node *)entry)->bar = val;
    ((list_room_node *)entry)->bolt = val;
    ((list_room_node *)entry)->boot = val;
    ((list_room_node *)entry)->invite = val;
    ((list_room_node *)entry)->grant = val;
    ((list_room_node *)entry)->key = val;
    ((list_room_node *)entry)->link = val;
    return;

  case LIST_TYPE_GAME:
    ((list_game_node *)entry)->draughts = val;
    ((list_game_node *)entry)->sps = val;
    ((list_game_node *)entry)->ttt = val;
    return;

  case LIST_TYPE_CHAN:
    ((list_chan_node *)entry)->boot = val;
    ((list_chan_node *)entry)->config = val;
    ((list_chan_node *)entry)->grant = val;
    ((list_chan_node *)entry)->read = val;
    ((list_chan_node *)entry)->who = val;
    ((list_chan_node *)entry)->write = val;
    return;

  default:
    log_assert(FALSE);
    return;
 }
}

static int list_flag_func_self_all(player *p, const char *str,
                                   list_node *current)
{ LIST_FLAG_ALL_FUNC_ON_OFF(SELF); }
static int list_flag_func_self_nothing(player *p, const char *str,
                                       list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(flag_nothing, "Nothing", self); }
static int list_flag_func_article_inform(player *p, const char *str,
                                         list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(article_inform, "Article inform", self); }
static int list_flag_func_beep(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(inform_beep, "Inform beep", self); }
static int list_flag_func_find(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(find, "Findable", self); }
static int list_flag_func_friend(player *p,const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(friend, "Friend", self); }
static int list_flag_func_grab(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(grab, "Grabable", self); }
static int list_flag_func_inform(player *p,const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(inform, "Inform", self); }

static int list_flag_func_coms_all(player *p, const char *str,
                                   list_node *current)
{ LIST_FLAG_ALL_FUNC_ON_OFF(COMS); }
static int list_flag_func_coms_nothing(player *p, const char *str,
                                       list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(flag_nothing, "Nothing", coms); }
static int list_flag_func_autos(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(autos, "Autos", coms); }
static int list_flag_func_channels(player *p, const char *str,
                                   list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(channels, "Channels", coms); }
static int list_flag_func_comments(player *p, const char *str,
                                   list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(comments, "Comments", coms); }
static int list_flag_func_echos(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(echos, "Echos", coms); }
static int list_flag_func_movement(player *p, const char *str,
                                   list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(movement, "Movement", coms); }
static int list_flag_func_multis(player *p, const char *str,
                                 list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(multis, "Multis", coms); }
static int list_flag_func_says(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(says, "Says", coms); }
static int list_flag_func_shouts(player *p, const char *str,
                                 list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(shouts, "Shouts", coms); }
static int list_flag_func_sessions(player *p, const char *str,
                                   list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(sessions, "Sessions", coms); }
static int list_flag_func_tells(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(tells, "Tells", coms); }
static int list_flag_func_tfs(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(tfs, "Tell friends", coms); }
static int list_flag_func_tfsof(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(tfsof, "Tell friends of", coms); }
static int list_flag_func_wakes(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(wakes, "Wakes", coms); }

static int list_flag_func_room_all(player *p, const char *str,
                                   list_node *current)
{ LIST_FLAG_ALL_FUNC_ON_OFF(ROOM); }
static int list_flag_func_room_nothing(player *p, const char *str,
                                       list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(flag_nothing, "Nothing", room); }
static int list_flag_func_room_alter(player *p, const char *str,
                                     list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(alter, "Alter", room); }
static int list_flag_func_bar(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(bar, "Bar", room); }
static int list_flag_func_room_boot(player *p, const char *str,
                                    list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(boot, "Boot", room); }
static int list_flag_func_bolt(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(bolt, "Bolt", room); }
static int list_flag_func_room_grant(player *p, const char *str,
                                     list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(grant, "Grant", room); }
static int list_flag_func_invite(player *p, const char *str,
                                 list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(invite, "Invite", room); }
static int list_flag_func_key(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(key, "Key", room); }
static int list_flag_func_link(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(link, "Link", room); }

static int list_flag_func_game_all(player *p, const char *str,
                                   list_node *current)
{ LIST_FLAG_ALL_FUNC_ON_OFF(GAME); }
static int list_flag_func_game_nothing(player *p, const char *str,
                                       list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(flag_nothing, "Nothing", game); }
static int list_flag_func_draughts(player *p, const char *str,
                                   list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(draughts, "Draughts", game); }
static int list_flag_func_sps(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(sps, "Sps", game); }
static int list_flag_func_ttt(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(ttt, "Ttt", game); }


static int list_flag_func_chan_all(player *p, const char *str,
                                   list_node *current)
{ LIST_FLAG_ALL_FUNC_ON_OFF(CHAN); }
static int list_flag_func_chan_nothing(player *p, const char *str,
                                       list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(flag_nothing, "Nothing", chan); }
static int list_flag_func_chan_boot(player *p, const char *str,
                                    list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(boot, "Boot", chan); }
static int list_flag_func_chan_config(player *p, const char *str,
                                     list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(config, "Configure", chan); }
static int list_flag_func_chan_grant(player *p, const char *str,
                                     list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(grant, "Grant", chan); }
static int list_flag_func_read(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(read, "Read", chan); }
static int list_flag_func_who(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(who, "Who", chan); }
static int list_flag_func_write(player *p, const char *str, list_node *current)
{ LIST_FLAG_FUNC_ON_OFF(write, "Write", chan); }


typedef struct list_flag_off
{
 const char *name;
 int (*func)(player *p, const char *, list_node *);
 bitflag dup : 1;
} list_flag_off;

static list_flag_off list_self_flags[] =
{ /* must be in order */
 {"all", list_flag_func_self_all, TRUE},
 {"article inform", list_flag_func_article_inform, FALSE},
 {"article_inform", list_flag_func_article_inform, TRUE},
 {"beep", list_flag_func_beep, TRUE},
 {"bep", list_flag_func_beep, TRUE},
 {"find", list_flag_func_find, FALSE},
 {"friend", list_flag_func_friend, FALSE},
 {"fnd", list_flag_func_friend, TRUE},
 {"grab me", list_flag_func_grab, FALSE},
 {"grab_me", list_flag_func_grab, TRUE},
 {"grabme", list_flag_func_grab, TRUE},
 {"grb", list_flag_func_grab, TRUE},
 {"inform", list_flag_func_inform, FALSE},
 {"inform beep", list_flag_func_beep, FALSE},
 {"inform_beep", list_flag_func_beep, TRUE},
 {"mail inform", list_flag_func_article_inform, TRUE},
 {"mail_inform", list_flag_func_article_inform, TRUE},
 {"news inform", list_flag_func_article_inform, TRUE},
 {"news_inform", list_flag_func_article_inform, TRUE},
 {"nothing", list_flag_func_self_nothing, FALSE},
 {NULL, NULL, FALSE}
};
#define LIST_SELF_FLAG_SZ 7 /* number of distinct flags */

static list_flag_off list_coms_flags[] =
{ /* must be in order */
 {"all", list_flag_func_coms_all, TRUE},
 {"autos", list_flag_func_autos, FALSE},
 {"blk", list_flag_func_tells, TRUE},
 {"block", list_flag_func_tells, TRUE},
 {"channels", list_flag_func_channels, FALSE},
 {"comments", list_flag_func_comments, FALSE},
 {"echos", list_flag_func_echos, FALSE},
 {"movement", list_flag_func_movement, FALSE},
 {"multis", list_flag_func_multis, FALSE},
 {"nothing", list_flag_func_coms_nothing, FALSE},
 {"says", list_flag_func_says, FALSE},
 {"sessions", list_flag_func_sessions, FALSE},
 {"shouts", list_flag_func_shouts, FALSE},
 {"tells", list_flag_func_tells, FALSE},
 {"tfs", list_flag_func_tfs, FALSE},
 {"tfsof", list_flag_func_tfsof, FALSE},
 {"wakes", list_flag_func_wakes, FALSE},
 {NULL, NULL, FALSE}
};
#define LIST_COMS_FLAG_SZ 14 /* number of distinct flags */

static list_flag_off list_room_flags[] =
{ /* must be in order */
 {"all", list_flag_func_room_all, TRUE},
 {"alter", list_flag_func_room_alter, FALSE},
 {"bar", list_flag_func_bar, FALSE},
 {"bolt", list_flag_func_bolt, FALSE},
 {"boot", list_flag_func_room_boot, FALSE},
 {"grant", list_flag_func_room_grant, FALSE},
 {"invite", list_flag_func_invite, FALSE},
 {"key", list_flag_func_key, FALSE},
 {"kick", list_flag_func_room_boot, TRUE},
 {"link", list_flag_func_link, FALSE},
 {"nothing", list_flag_func_room_nothing, FALSE},
 {NULL, NULL, FALSE}
};
#define LIST_ROOM_FLAG_SZ 8 /* number of distinct flags */

static list_flag_off list_game_flags[] =
{ /* must be in order */
 {"all", list_flag_func_game_all, TRUE},
 {"checkers", list_flag_func_draughts, TRUE},
 {"draughts", list_flag_func_draughts, FALSE},
 {"nothing", list_flag_func_game_nothing, FALSE},
 {"sps", list_flag_func_sps, FALSE},
 {"ttt", list_flag_func_ttt, FALSE},
 {NULL, NULL, FALSE}
};
#define LIST_GAME_FLAG_SZ 4 /* number of distinct flags */

static list_flag_off list_chan_flags[] =
{ /* must be in order */
 {"all", list_flag_func_chan_all, TRUE},
 {"boot", list_flag_func_chan_boot, FALSE},
 {"configure", list_flag_func_chan_config, FALSE},
 {"grant", list_flag_func_chan_grant, FALSE},
 {"kick", list_flag_func_chan_boot, TRUE},
 {"nothing", list_flag_func_chan_nothing, FALSE},
 {"read", list_flag_func_read, FALSE},
 {"who", list_flag_func_who, FALSE},
 {"write", list_flag_func_write, FALSE},
 {NULL, NULL, FALSE}
};
#define LIST_CHAN_FLAG_SZ 7 /* number of distinct flags */

/* number of flags that can be changed at once */
#define LIST_FLAG_SZ LIST_COMS_FLAG_SZ

static list_flag_off *list_flags_off(int list_type)
{
 switch (list_type)
 {
  case LIST_TYPE_SELF: return (list_self_flags);
    
  case LIST_TYPE_COMS: return (list_coms_flags);
    
  case LIST_TYPE_ROOM: return (list_room_flags);
    
  case LIST_TYPE_GAME: return (list_game_flags);

  case LIST_TYPE_CHAN: return (list_chan_flags);

  default:
    break;
 }
 
 return (NULL);
}

static int list_player_is_used_entry(int list_type, list_node *entry)
{
 switch (list_type)
 {
  case LIST_TYPE_SELF:
    return (LIST_FLAG(entry, self, article_inform) ||
            LIST_FLAG(entry, self, find) ||
            LIST_FLAG(entry, self, friend) ||
            LIST_FLAG(entry, self, grab) ||
            LIST_FLAG(entry, self, inform) ||
            LIST_FLAG(entry, self, inform_beep) ||
            FALSE);
  case LIST_TYPE_COMS:
    return (LIST_FLAG(entry, coms, says) ||
            LIST_FLAG(entry, coms, tells) ||
            LIST_FLAG(entry, coms, multis) ||
            LIST_FLAG(entry, coms, shouts) ||
            LIST_FLAG(entry, coms, tfs) ||
            LIST_FLAG(entry, coms, tfsof) ||
            LIST_FLAG(entry, coms, channels) ||
            LIST_FLAG(entry, coms, echos) ||
            LIST_FLAG(entry, coms, wakes) ||
            LIST_FLAG(entry, coms, comments) ||
            LIST_FLAG(entry, coms, sessions) ||
            LIST_FLAG(entry, coms, autos) ||
            LIST_FLAG(entry, coms, movement) ||
            FALSE);
  case LIST_TYPE_ROOM:
    return (LIST_FLAG(entry, room, alter) ||
            LIST_FLAG(entry, room, bar) ||
            LIST_FLAG(entry, room, bolt) ||
            LIST_FLAG(entry, room, boot) ||
            LIST_FLAG(entry, room, invite) ||
            LIST_FLAG(entry, room, grant) ||
            LIST_FLAG(entry, room, key) ||
            LIST_FLAG(entry, room, link) ||
            FALSE);

  case LIST_TYPE_GAME:
    return (LIST_FLAG(entry, game, draughts) ||
            LIST_FLAG(entry, game, sps) ||
            LIST_FLAG(entry, game, ttt) ||
            FALSE);

  case LIST_TYPE_CHAN:
    return (LIST_FLAG(entry, chan, boot) ||
            LIST_FLAG(entry, chan, config) ||
            LIST_FLAG(entry, chan, grant) ||
            LIST_FLAG(entry, chan, read) ||
            LIST_FLAG(entry, chan, who) ||
            LIST_FLAG(entry, chan, write) ||
            FALSE);

  default:
    log_assert(FALSE);
    return (FALSE);
 }
}

static int list_is_used_entry(int list_type, list_node *entry)
{
 switch (list_type)
 {
  case LIST_TYPE_SELF:
    return (LIST_FLAG(entry, self, flag_nothing) ||
            list_player_is_used_entry(list_type, entry));
    
  case LIST_TYPE_COMS:
    return (LIST_FLAG(entry, coms, flag_nothing) ||
            list_player_is_used_entry(list_type, entry));
    
  case LIST_TYPE_ROOM:
    return (LIST_FLAG(entry, room, flag_nothing) ||
            list_player_is_used_entry(list_type, entry));

  case LIST_TYPE_GAME:
    return (LIST_FLAG(entry, game, flag_nothing) ||
            list_player_is_used_entry(list_type, entry));

  case LIST_TYPE_CHAN:
    return (LIST_FLAG(entry, chan, flag_nothing) ||
            list_player_is_used_entry(list_type, entry));

  default:
    log_assert(FALSE);
    return (FALSE);
 }
}

static void list_tell_single_entry(player *p, int list_type, list_node *entry)
{
 int count = 0;
 int done = 0;
 list_flag_off *tmp = list_flags_off(list_type);
 char buffer[sizeof("Entry for %s") + PLAYER_S_NAME_SZ];

 log_assert(tmp);
 
 sprintf(buffer, "Entry for %s",
         entry ? (!strcmp(entry->name, p->saved->lower_name) ? "you" :
                  list_node_name(entry)) : "Everyone");
 
 ptell_mid(NORMAL_T(p), buffer, FALSE);

 while (tmp[count].name)
 {
  assert(tmp[count].func);
  
  if (!tmp[count].dup)
  {
   fvtell_player(NORMAL_T(p), "%s: %s\n", tmp[count].name,
                 TOGGLE_ON_OFF(entry &&
                               (*tmp[count].func)(NULL, NULL, entry)));
   ++done;
  }
  
  ++count;
 }

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void list_tell_all_entry(player *p, int list_type, list_node *entry)
{
 int count = 0;
 int done = 0;
 list_flag_off *tmp = list_flags_off(list_type);

 log_assert(tmp);

 fvtell_player(NORMAL_T(p), "%-*s - ",
               PLAYER_S_NAME_SZ, list_node_name(entry));

 if (!list_player_is_used_entry(list_type, entry))
 {
  assert(list_is_used_entry(list_type, entry));
  fvtell_player(NORMAL_T(p), "%s", " ** nothing ** \n");
 }
 else if (list_player_all_used_entry(list_type, entry))
 {
  fvtell_player(NORMAL_T(p), "%s", " ** all ** \n");
 }
 else
 {
  while (tmp[count].name)
  {
   assert(tmp[count].func);
   
   if (!tmp[count].dup)
   {
    if ((*tmp[count].func)(NULL, NULL, entry))
    {
     fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(done, "%s"),
                   tmp[count].name);
     ++done;
    }
   }
   
   ++count;
  }
  
  fvtell_player(NORMAL_T(p), ".\n");
 }
}
 
static int list_get_size(list_node *head)
{
 int number = 0;
 list_node *scan = head;

 while (scan)
 {
  list_node *scan_next = scan->next;

  ++number;

  scan = scan_next;
 }
 
 return (number);
}

static void list_node_init(list_node *current, int list_type, int grouped)
{
 current->name = NULL;

 current->c_timestamp = now;
 
 switch (list_type)
 {
  case LIST_TYPE_SELF:
  {
   list_self_node *tmp = (list_self_node *)current;

   tmp->list_type = LIST_TYPE_SELF;

   tmp->flag_grouped = grouped;
   tmp->flag_nothing = FALSE;

   list_player_all_set_entry(LIST_TYPE_SELF, current, FALSE);
  }
  break;
  
  case LIST_TYPE_COMS:
  {
   list_coms_node *tmp = (list_coms_node *)current;

   tmp->list_type = LIST_TYPE_COMS;

   tmp->flag_grouped = grouped;
   tmp->flag_nothing = FALSE;

   list_player_all_set_entry(LIST_TYPE_COMS, current, FALSE);
  }
  break;
  
  case LIST_TYPE_ROOM:
  {
   list_room_node *tmp = (list_room_node *)current;

   tmp->list_type = LIST_TYPE_ROOM;

   tmp->flag_grouped = grouped;
   tmp->flag_nothing = FALSE;

   list_player_all_set_entry(LIST_TYPE_ROOM, current, FALSE);
  }
  break;

  case LIST_TYPE_GAME:
  {
   list_game_node *tmp = (list_game_node *)current;

   tmp->list_type = LIST_TYPE_GAME;

   tmp->flag_grouped = grouped;
   tmp->flag_nothing = FALSE;

   list_player_all_set_entry(LIST_TYPE_GAME, current, FALSE);
  }
  break;
  
  case LIST_TYPE_CHAN:
  {
   list_chan_node *tmp = (list_chan_node *)current;
   
   tmp->list_type = LIST_TYPE_CHAN;

   tmp->flag_grouped = grouped;
   tmp->flag_nothing = FALSE;

   list_player_all_set_entry(LIST_TYPE_CHAN, current, FALSE);
  }
  break;

default:
    log_assert(FALSE);
    break;
 }
}

static int list_node_grouped(int list_type, list_node *entry)
{
 switch (list_type)
 {
  case LIST_TYPE_SELF:
    return (LIST_FLAG(entry, self, flag_grouped));
  case LIST_TYPE_COMS:
    return (LIST_FLAG(entry, coms, flag_grouped));
  case LIST_TYPE_ROOM:
    return (LIST_FLAG(entry, room, flag_grouped));
  case LIST_TYPE_GAME:
    return (LIST_FLAG(entry, game, flag_grouped));
  case LIST_TYPE_CHAN:
    return (LIST_FLAG(entry, chan, flag_grouped));
    
  default:
    log_assert(FALSE);
    return (FALSE);
 }
}

static void list_malloc_free_name(list_node *node)
{
 const char *name = node->name;
 
 switch (tolower((unsigned char) name[0]))
 {
  case 'a':
    name = "admins";
    break;
  case 'e':
    name = "everyone";
    break;
  case 'k':
    name = "karma";
    break;
  case 'm':
    name = "ministers";
    break;
  case 'n':
    name = "newbie";
    break;
  case 's':
    if (tolower((unsigned char) name[1]) == 'p')
      name = "spods";
    else
    {
     assert(tolower((unsigned char) name[1]) == 'u');
     name = "sus";
    }
    break;
    
  default:
    assert(FALSE);
    return;
 }

 FREE(node->name);
 node->name = (char *)name; /* creates a warning ... although it's ok */
}

static int list_add_entry(list_node **head, int list_type, list_node *new_node)
{
 list_node *scan = NULL;
 player_tree_node *tmp = NULL;
 
 if (list_node_grouped(list_type, new_node))
   list_malloc_free_name(new_node);
 else /* delete old list entries */
   if (!(tmp = player_find_all(NULL, new_node->name, PLAYER_FIND_DEFAULT)) ||
       (difftime(tmp->c_timestamp, new_node->c_timestamp) >= 0))
     return (FALSE);
 
 if (!(scan = *head))
 {
  new_node->next = NULL;
  new_node->prev = NULL;
  *head = new_node;
  return (TRUE);
 }
 
 while (scan->next && (strcmp(new_node->name, scan->name) > 0))
   scan = scan->next;
 
 if (strcmp(new_node->name, scan->name) > 0)
 {
  if ((new_node->next = scan->next))
    new_node->next->prev = new_node;
  
  new_node->prev = scan;
  scan->next = new_node;
 }
 else
 {
  if ((new_node->prev = scan->prev))
    new_node->prev->next = new_node;
  else
    *head = new_node;
  
  new_node->next = scan;
  scan->prev = new_node;
 }

 return (TRUE);
}

static list_node *list_user_add_entry(player *p,
                                      list_node **head, int max_entries,
                                      int list_type, const char *name)
{
 player_tree_node *sp = NULL;
 list_node *new_entry = NULL;
 int grouped = FALSE;
 size_t name_len = 0;
 
 if (list_get_size(*head) >= max_entries)
 {
  if (p)
    fvtell_player(SYSTEM_T(p), " You are already at your maximum "
                  "of -- ^S^B%d -- list entries.\n", p->max_list_entries);
  return (NULL);
 }

 if (p && p->saved &&
     ((!PRIV_STAFF(p->saved) &&
       !(strcmp(name, "admins") && strcmp(name, "sus"))) ||
      (!PRIV_STAFF(p->saved) && !p->saved->priv_minister &&
       !strcmp(name, "ministers")) ||
      (!PRIV_STAFF(p->saved) && !p->saved->priv_spod &&
       !strcmp(name, "spods")) ||
      FALSE))
 {
  fvtell_player(NORMAL_T(p), " You are not allowed to add "
                "the group -- ^S^B%s^s -- to a list.\n",
                name);
  return (NULL);
 }
 
 if (!strcmp(name, "newbies") || !strcmp(name, "karma") ||
     !strcmp(name, "ministers") || !strcmp(name, "spods") ||
     !strcmp(name, "admins") || !strcmp(name, "sus") ||
     !strcmp(name, "everyone"))
   grouped = TRUE;
 else if (!(sp = player_find_all(p, name, p ? PLAYER_FIND_SC_EXTERN :
                                 PLAYER_FIND_DEFAULT)))
   return (NULL);
 else
   name = sp->lower_name;
 
 LIST_MALLOC(list_type, new_entry);
 if (!new_entry)
   goto recover_entry;
 
 list_node_init(new_entry, list_type, grouped);

 name_len = strlen(name);
 if (!(new_entry->name = MALLOC(name_len + 1)))
   goto recover_name;
 
 COPY_STR_LEN(new_entry->name, name, name_len);
 
 if (!list_add_entry(head, list_type, new_entry))
 {
  assert(FALSE);
 }
 
 return (new_entry);

 recover_name:
 LIST_FREE(list_type, new_entry);
 recover_entry:
 if (p)
   P_MEM_ERR(p);

 LOG_MEM_ERR();
 return (NULL);
}

static list_node *list_find_entry(list_node *head, const char *name)
{
 list_node *tmp = head;
 int cmp_sve = 1;
 
 while (tmp && ((cmp_sve = strcmp(tmp->name, name)) < 0))
   tmp = tmp->next;
 
 if (cmp_sve)
   return (NULL);
 else
   return (tmp);
}

static void list_delete_entry(list_node **head, int list_type,
                              list_node *entry)
{
 if (!(head && entry))
 {
  log_assert(FALSE);
  return;
 }

 assert(list_find_entry(*head, entry->name));
 
 if (entry->next)
   entry->next->prev = entry->prev;
 
 if (entry->prev)
   entry->prev->next = entry->next;
 else
   *head = entry->next;

 if (!list_node_grouped(list_type, entry))
   FREE(entry->name);
 LIST_FREE(list_type, entry);
}

void list_load_cleanup(int list_type, list_node *head)
{
 list_node *tmp = head;
 
 while (tmp)
 {
  list_node *tmp_next = tmp->next;
  
  assert(tmp->name);

  if (!list_node_grouped(list_type, tmp))
    FREE(tmp->name);
  LIST_FREE(list_type, tmp);

  tmp = tmp_next;
 }
}

void list_room_local_load_cleanup(room *r)
{
 list_load_cleanup(LIST_TYPE_ROOM, r->list_room_local_start);
 r->list_room_local_start = NULL;
}

void list_room_glob_load_cleanup(player_tree_node *sp)
{
 assert(sp->flag_tmp_list_room_glob_in_core);
  
 list_load_cleanup(LIST_TYPE_ROOM, sp->list_room_glob_start);
 sp->list_room_glob_start = NULL;
 sp->flag_tmp_list_room_glob_in_core = FALSE;
}

void list_player_load_cleanup(player *p)
{
 list_load_cleanup(LIST_TYPE_SELF, p->list_self_start);
 p->list_self_start = NULL;
 
 list_load_cleanup(LIST_TYPE_COMS, p->list_coms_start);
 p->list_coms_start = NULL;

 list_load_cleanup(LIST_TYPE_GAME, p->list_game_start);
 p->list_game_start = NULL;

 p->flag_tmp_dont_save_after_this = TRUE;
}

void list_chan_cleanup(channels_base *base)
{
 list_load_cleanup(LIST_TYPE_CHAN, base->list_start);
 base->list_start = NULL;
}

void list_load(list_node **head, int list_type, file_io *io_player)
{
 int count = 0;
 int number = 0;

 file_section_beg("header", io_player);

 number = file_get_int("number", io_player);

 file_section_end("header", io_player);

 file_section_beg("list", io_player);
 
 while (count < number)
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];
  list_node *scan = NULL;

  LIST_MALLOC(list_type, scan);
  
  if (!scan)
    SHUTDOWN_MEM_ERR();
  
  sprintf(buffer, "%04d", ++count);

  file_section_beg(buffer, io_player);

  scan->c_timestamp = file_get_time_t("c_timestamp", io_player);
  
  file_section_beg("flags", io_player);

  switch (list_type)
  {
   case LIST_TYPE_SELF:
   {
    list_self_node *tmp = (list_self_node *)scan;

    tmp->list_type = LIST_TYPE_SELF;
    
    tmp->article_inform = file_get_bitflag("article_inform", io_player);
    tmp->find = file_get_bitflag("find", io_player);
    tmp->friend = file_get_bitflag("friend", io_player);
    tmp->grab = file_get_bitflag("grab", io_player);
    tmp->flag_grouped = file_get_bitflag("grouped", io_player);
    tmp->inform = file_get_bitflag("inform", io_player);
    tmp->inform_beep = file_get_bitflag("inform_beep", io_player);
   }
   break;
   
   case LIST_TYPE_COMS:
   {
    list_coms_node *tmp = (list_coms_node *)scan;

    tmp->list_type = LIST_TYPE_COMS;
    
    tmp->autos = file_get_bitflag("autos", io_player);
    tmp->channels = file_get_bitflag("channels", io_player);
    tmp->comments = file_get_bitflag("comments", io_player);
    tmp->echos = file_get_bitflag("echos", io_player);
    tmp->flag_grouped = file_get_bitflag("grouped", io_player);
    tmp->movement = file_get_bitflag("movement", io_player);
    tmp->multis = file_get_bitflag("multis", io_player);
    tmp->says = file_get_bitflag("says", io_player);
    tmp->sessions = file_get_bitflag("sessions", io_player);
    tmp->shouts = file_get_bitflag("shouts", io_player);
    tmp->tells = file_get_bitflag("tells", io_player);
    tmp->tfs = file_get_bitflag("tfs", io_player);
    tmp->tfsof = file_get_bitflag("tfsof", io_player);
    tmp->wakes = file_get_bitflag("wakes", io_player);
   }
   break;
   
   case LIST_TYPE_ROOM:
   {
    list_room_node *tmp = (list_room_node *)scan;

    tmp->list_type = LIST_TYPE_ROOM;
    
    tmp->alter = file_get_bitflag("alter", io_player);
    tmp->bar = file_get_bitflag("bar", io_player);
    tmp->bolt = file_get_bitflag("bolt", io_player);
    tmp->boot = file_get_bitflag("boot", io_player);
    tmp->grant = file_get_bitflag("grant", io_player);
    tmp->flag_grouped = file_get_bitflag("grouped", io_player);
    tmp->invite = file_get_bitflag("invite", io_player);
    tmp->key = file_get_bitflag("key", io_player);
    tmp->link = file_get_bitflag("link", io_player);
   }
   break;

   case LIST_TYPE_GAME:
   {
    list_game_node *tmp = (list_game_node *)scan;

    tmp->list_type = LIST_TYPE_GAME;
    
    tmp->draughts = file_get_bitflag("draughts", io_player);
    tmp->flag_grouped = file_get_bitflag("grouped", io_player);
    tmp->sps = file_get_bitflag("sps", io_player);
    tmp->ttt = file_get_bitflag("ttt", io_player);
   }
   break;

   case LIST_TYPE_CHAN:
   {
    list_chan_node *tmp = (list_chan_node *)scan;
    int real_grant = FALSE;
    
    tmp->list_type = LIST_TYPE_CHAN;

    tmp->grant = file_get_bitflag("alter", io_player);
    if (FILE_IO_CREATED(io_player))
      real_grant = TRUE;
    tmp->boot = file_get_bitflag("boot", io_player);
    if (real_grant)
    { /* config/grant/who added as alter was removed */
     tmp->config = file_get_bitflag("config", io_player);
     tmp->grant = file_get_bitflag("grant", io_player);
    }
    else
      tmp->config = tmp->grant;
    tmp->flag_grouped = file_get_bitflag("grouped", io_player);
    tmp->read = file_get_bitflag("read", io_player);
    if (real_grant)
      tmp->who = file_get_bitflag("who", io_player);
    else
      tmp->who = tmp->grant;
    tmp->write = file_get_bitflag("write", io_player);
   }
   break;

   default:
     log_assert(FALSE);
     break;
  }

  file_section_end("flags", io_player);
 
  if (!(scan->name = file_get_malloc("name", NULL, io_player)))
    SHUTDOWN_MEM_ERR();
  lower_case(scan->name); /* mucked up previously -- can remove after a bit */

  if (list_find_entry(*head, scan->name))
  {
   FREE(scan->name);
   LIST_FREE(list_type, scan);
  }
  else if (!list_add_entry(head, list_type, scan))
  {
   assert(!list_node_grouped(list_type, scan));
   
   FREE(scan->name);
   LIST_FREE(list_type, scan);
  }
  
  file_section_end(buffer, io_player);
 }

 file_section_end("list", io_player);
}

static void timed_list_cleanup(int timed_type, void *passed_player_tree_node)
{
 player_tree_node *sp = passed_player_tree_node;
 int do_save = FALSE;
 int do_cleanup = FALSE;
 int do_retime = FALSE;

 TCTRACE("timed_list_cleanup");

 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;

 if (timed_type == TIMER_Q_TYPE_CALL_RUN_ALL)
   do_save = TRUE;
 else if (difftime(now, sp->list_a_timestamp) > LIST_CLEANUP_TIMEOUT_CLEANUP)
 {
  do_save = TRUE;
  do_cleanup = TRUE;
 }
 else if (difftime(now, sp->list_l_timestamp) >
          LIST_CLEANUP_TIMEOUT_SYNC_ANYWAY)
 {
  do_save = TRUE;
  do_retime = TRUE;
  sp->list_l_timestamp = now;
 }
 else
   do_retime = TRUE;
 
 if (do_save && sp->flag_tmp_room_needs_saving)
   room_save(sp);
 
 if (do_cleanup)
   list_room_glob_load_cleanup(sp);
 else if (do_retime)
 {
  struct timeval tv;

  gettimeofday(&tv, NULL);

  TIMER_Q_TIMEVAL_ADD_SECS(&tv, LIST_CLEANUP_TIMEOUT_REDO, 0);
  
  timer_q_add_static_node(&sp->list_load_timer, &list_load_player_timer_queue,
                          sp, &tv, TIMER_Q_FLAG_NODE_SINGLE);
 }
}

static void list_load_timer_start(player_tree_node *sp)
{
 struct timeval tv;
 
 gettimeofday(&tv, NULL);
 
 TIMER_Q_TIMEVAL_ADD_SECS(&tv, LIST_CLEANUP_TIMEOUT_LOAD, 0);
 
 timer_q_add_static_node(&sp->list_load_timer, &list_load_player_timer_queue,
                         sp, &tv, TIMER_Q_FLAG_NODE_SINGLE);
}

void list_room_glob_load(player_tree_node *sp, file_io *io_room)
{
 if (sp->flag_tmp_list_room_glob_in_core)
   return;
 
 list_load(&sp->list_room_glob_start, LIST_TYPE_ROOM, io_room);
 sp->flag_tmp_list_room_glob_in_core = TRUE;
 list_load_timer_start(sp);
}

void list_room_glob_file_load(player_tree_node *sp)
{
 file_io real_io_room;
 file_io *io_room = &real_io_room;
 
 if (sp->flag_tmp_list_room_glob_in_core)
   return;
 
 if (!room_load_open(sp, io_room))
 {
  /* assert(FALSE); */
  return;
 }
 
 file_section_end("rooms", io_room);
 file_read_close(io_room);
}

void list_save(list_node **head, int list_type, file_io *io_player)
{
 int count = 0;
 int number = list_get_size(*head);
 list_node *scan = *head;

 file_section_beg("header", io_player);

 file_put_int("number", number, io_player);

 file_section_end("header", io_player);

 file_section_beg("list", io_player);
 
 while (scan && (count < number))
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];
  
  sprintf(buffer, "%04d", ++count);
  
  file_section_beg(buffer, io_player);

  file_put_time_t("c_timestamp", scan->c_timestamp, io_player);
  
  file_section_beg("flags", io_player);

  switch (list_type)
  {
   case LIST_TYPE_SELF:
   {
    list_self_node *tmp = (list_self_node *)scan;
    
    file_put_bitflag("article_inform", tmp->article_inform, io_player);
    file_put_bitflag("find", tmp->find, io_player);
    file_put_bitflag("friend", tmp->friend, io_player);
    file_put_bitflag("grab", tmp->grab, io_player);
    file_put_bitflag("grouped", tmp->flag_grouped, io_player);
    file_put_bitflag("inform", tmp->inform, io_player);
    file_put_bitflag("inform_beep", tmp->inform_beep, io_player);
   }
   break;
   
   case LIST_TYPE_COMS:
   {
    list_coms_node *tmp = (list_coms_node *)scan;
    
    file_put_bitflag("autos", tmp->autos, io_player);
    file_put_bitflag("channels", tmp->channels, io_player);
    file_put_bitflag("comments", tmp->comments, io_player);
    file_put_bitflag("echos", tmp->echos, io_player);
    file_put_bitflag("grouped", tmp->flag_grouped, io_player);
    file_put_bitflag("movement", tmp->movement, io_player);
    file_put_bitflag("multis", tmp->multis, io_player);
    file_put_bitflag("says", tmp->says, io_player);
    file_put_bitflag("sessions", tmp->sessions, io_player);
    file_put_bitflag("shouts", tmp->shouts, io_player);
    file_put_bitflag("tells", tmp->tells, io_player);
    file_put_bitflag("tfs", tmp->tfs, io_player);
    file_put_bitflag("tfsof", tmp->tfsof, io_player);
    file_put_bitflag("wakes", tmp->wakes, io_player);
   }
   break;
   
   case LIST_TYPE_ROOM:
   {
    list_room_node *tmp = (list_room_node *)scan;
    
    file_put_bitflag("alter", tmp->alter, io_player);
    file_put_bitflag("bar", tmp->bar, io_player);
    file_put_bitflag("bolt", tmp->bolt, io_player);
    file_put_bitflag("boot", tmp->boot, io_player);
    file_put_bitflag("grant", tmp->grant, io_player);
    file_put_bitflag("grouped", tmp->flag_grouped, io_player);
    file_put_bitflag("invite", tmp->invite, io_player);
    file_put_bitflag("key", tmp->key, io_player);
    file_put_bitflag("link", tmp->link, io_player);
   }
   break;

   case LIST_TYPE_GAME:
   {
    list_game_node *tmp = (list_game_node *)scan;
    
    file_put_bitflag("draughts", tmp->draughts, io_player);
    file_put_bitflag("grouped", tmp->flag_grouped, io_player);
    file_put_bitflag("sps", tmp->sps, io_player);
    file_put_bitflag("ttt", tmp->ttt, io_player);
   }
   break;
  
   case LIST_TYPE_CHAN:
   {
    list_chan_node *tmp = (list_chan_node *)scan;

    file_put_bitflag("boot", tmp->boot, io_player);
    file_put_bitflag("config", tmp->config, io_player);
    file_put_bitflag("grant", tmp->grant, io_player);
    file_put_bitflag("grouped", tmp->flag_grouped, io_player);
    file_put_bitflag("read", tmp->read, io_player);
    file_put_bitflag("who", tmp->who, io_player);
    file_put_bitflag("write", tmp->write, io_player);
   }
   break;
  
   default:
     log_assert(FALSE);
     break;
  }

  file_section_end("flags", io_player);
  
  file_put_string("name", scan->name, 0, io_player);
  
  file_section_end(buffer, io_player);

  scan = scan->next;
 }
 assert(!scan && (count == number));

 file_section_end("list", io_player);
}

list_node *list_user_find_entry(player *p, list_node *head, const char *name)
{
 list_node *tmp = head;
 int cmp_sve = 1;
 
 while (tmp && ((cmp_sve = beg_strcmp(name, tmp->name)) > 0))
   tmp = tmp->next;
 
 if (cmp_sve)
   tmp = NULL;

 if (!tmp && p)
   fvtell_player(SYSTEM_T(p), " The string -- ^S^B%s^s -- does not match a "
                 "list entry.\n", name);

 return (tmp);
}

list_node *list_onanywhere(player *from,
                           list_node *head, player_tree_node *current)
{
 list_node *the_entry = NULL;

 BTRACE("list_onanywhere");

 if (!head)
   return (NULL);
 
 log_assert(current && (!from || (P_IS_AVL(from->saved) &&
                                  (from->saved->player_ptr == from))));
 
 if ((the_entry = list_find_entry(head, current->lower_name)))
   return (the_entry);
 else if (from && /* passing NULL works */
          (current->karma_value >= from->karma_cutoff) &&
          (the_entry = list_find_entry(head, "karma")))
   return (the_entry);
 else if (current->priv_lower_admin &&
          (the_entry = list_find_entry(head, "admins")))
   return (the_entry);
 else if (PRIV_STAFF(current) && (the_entry = list_find_entry(head, "sus")))
   return (the_entry);
 else if (current->priv_minister &&
          (the_entry = list_find_entry(head, "ministers")))
   return (the_entry);
 else if (current->priv_spod &&
          (the_entry = list_find_entry(head, "spods")))
   return (the_entry);
 else if (from && /* passing NULL works */
          (!current->priv_base ||
           (from->list_newbie_time > real_total_logon_time(current))) &&
          (the_entry = list_find_entry(head, "newbies")))
   return (the_entry);
 else if ((the_entry = list_find_entry(head, "everyone")))
   return (the_entry);
 
 return (NULL);
}

static list_node *list_create_or_find_entry(player *p,
                                            list_node **head, int max_entries,
                                            int list_type, const char *name)
{
 list_node *tmp = list_user_find_entry(NULL, *head, name);

 if (!tmp)
   return (list_user_add_entry(p, head, max_entries, list_type, name));
 
 return (tmp);
}

static int list_type_player_parse(player *p, const char *str,
                                  list_node ***head, int *list_type,
                                  const char **name)
{ 
 if (!beg_strcmp(str, "communication") ||
     !beg_strcmp(str, "comms") || !beg_strcmp(str, "coms"))
 {
  *list_type = LIST_TYPE_COMS;
  *head = &p->list_coms_start;
  *name = "Coms";
 }
 else if (!beg_strcmp(str, "self"))
 {
  *head = &p->list_self_start;
  *list_type = LIST_TYPE_SELF;
  *name = "Self";
 }
 else if (!beg_strcmp(str, "games"))
 {
  *head = &p->list_game_start;
  *list_type = LIST_TYPE_GAME;
  *name = "Games";
 }
 else if (!beg_strcmp(str, "tmp communication") ||
          !beg_strcmp(str, "tmp_communication") ||
          !beg_strcmp(str, "communication tmp") ||
          !beg_strcmp(str, "communication_tmp") ||
          !beg_strcmp(str, "tcommunication") ||
          !beg_strcmp(str, "tmp comms") ||
          !beg_strcmp(str, "tmp_comms") ||
          !beg_strcmp(str, "comms tmp") ||
          !beg_strcmp(str, "comms_tmp") ||
          !beg_strcmp(str, "coms tmp") ||
          !beg_strcmp(str, "coms_tmp") ||
          !beg_strcmp(str, "tcomms"))
 {
  *list_type = LIST_TYPE_COMS;
  *head = &p->list_coms_tmp_start;
  *name = "Coms tmp";
 }
 else if (!beg_strcmp(str, "tmp self") ||
          !beg_strcmp(str, "tmp_self") ||
          !beg_strcmp(str, "self tmp") ||
          !beg_strcmp(str, "self_tmp") ||
          !beg_strcmp(str, "tself"))
 {
  *head = &p->list_self_tmp_start;
  *list_type = LIST_TYPE_SELF;
  *name = "Self tmp";
 }
 else
   return (FALSE);

 return (TRUE);
}

static int list_type_room_parse(player *p, const char *str,
                                list_node ***head, const char **name)
{ 
 if (!beg_strcmp(str, "local"))
 {
  *head = &p->location->list_room_local_start;
  *name = "$Room-Owner-Name.$Room-Id local";
 }
 else if (!beg_strcmp(str, "personnal") ||
          !beg_strcmp(str, "mine") ||
          !beg_strcmp(str, "me") ||
          !beg_strcmp(str, "my global") ||
          !beg_strcmp(str, "my_global"))
 {
  *head = &p->saved->list_room_glob_start;
  *name = "Your global";
 }
 else if (!beg_strcmp(str, "global"))
 {
  *head = &p->location->owner->list_room_glob_start;
  *name = "$Room-Owner-Name global";
 }
 else
   return (FALSE);

 return (TRUE);
}

static void user_list_player_del(player *p, parameter_holder *params)
{
 int count = 0;
 char *str = NULL;
 int list_type = LIST_TYPE_SELF;
 list_node **head = &p->list_self_start;
 const char *name = NULL;
 char buf[128];
 
 if (params->last_param != 2)
   TELL_FORMAT(p, "<type> <player(s)>");

 lower_case(GET_PARAMETER_STR(params, 1));

 if (!list_type_player_parse(p, GET_PARAMETER_STR(params, 1),
                             &head, &list_type, &name))
   TELL_FORMAT(p, "<self [tmp]|games|communication [tmp]> <player(s)>");
 
 str = GET_PARAMETER_STR(params, 2);
 while (str)
 {
  char *end_name = next_parameter(str, ',');
  
  if (end_name)
    *end_name++ = 0;

  if (*str)
  {
   list_node *tmp = list_user_find_entry(p, *head, str);
   
   if (tmp)
   {
    ++count;
    fvtell_player(NORMAL_T(p), " Entry removed for '^S^B%s^s'.\n",
                  list_node_name(tmp));

    list_delete_entry(head, list_type, tmp);
   }
  }

  str = end_name;
 }

 switch (count)
 {
  case 0:
    fvtell_player(NORMAL_T(p), " Deleted ^S^Bno^s %s list entries.\n", name);
    break;
  case 1:
    fvtell_player(NORMAL_T(p), " Deleted ^S^Bone^s %s list entry.\n", name);
    break;
    
  default:
    fvtell_player(NORMAL_T(p), " Deleted ^S^B%s^s %s list entries.\n",
                  word_number_base(buf, 128, NULL, count,
                                   FALSE, word_number_def), name);
 }
}

void user_list_room_del(player *p, parameter_holder *params)
{
 int count = 0;
 char *str = NULL;
 int list_type = LIST_TYPE_ROOM;
 list_node **head = &p->list_self_start;
 const char *name = NULL;
 char buf[128];
 
 if (params->last_param != 2)
   TELL_FORMAT(p, "<type> <player(s)>");

 lower_case(GET_PARAMETER_STR(params, 1));

 if (!list_type_room_parse(p, GET_PARAMETER_STR(params, 1), &head, &name))
   TELL_FORMAT(p, "<global|local> <player(s)>");
 
 str = GET_PARAMETER_STR(params, 2);
 while (str)
 {
  char *end_name = next_parameter(str, ',');
  
  if (end_name)
    *end_name++ = 0;

  if (*str)
  {
   list_node *tmp = list_user_find_entry(p, *head, str);
   
   if (tmp)
   {
    ++count;
    fvtell_player(NORMAL_T(p), " Entry removed for '^S^B%s^s'.\n",
                  list_node_name(tmp));

    list_delete_entry(head, list_type, tmp);
   }
  }

  str = end_name;
 }

 switch (count)
 {
  case 0:
    fvtell_player(NORMAL_T(p), " Deleted ^S^Bno^s %s list entries.\n", name);
    break;
  case 1:
    fvtell_player(NORMAL_T(p), " Deleted ^S^Bone^s %s list entry.\n", name);
    break;
    
  default:
    fvtell_player(NORMAL_T(p), " Deleted ^S^B%s^s %s list entries.\n",
                  word_number_base(buf, 128, NULL, count,
                                   FALSE, word_number_def), name);
 }
}

void user_list_chan_del(player *p, parameter_holder *params)
{
 int count = 0;
 char *str = NULL;
 int list_type = LIST_TYPE_CHAN;
 list_node **head = NULL;
 const char *name = NULL;
 channels_base *chan_base = NULL;
 char buf[128];
 
 if (params->last_param != 2)
   TELL_FORMAT(p, "<channel> <player(s)>");

 str = GET_PARAMETER_STR(params, 1);
 if (!(chan_base = channels_user_find_grant_base(p, str)))
   return;
 head = &chan_base->list_start;
 
 str = GET_PARAMETER_STR(params, 2);
 while (str)
 {
  char *end_name = next_parameter(str, ',');
  
  if (end_name)
    *end_name++ = 0;

  if (*str)
  {
   list_node *tmp = list_user_find_entry(p, *head, str);
   
   if (tmp)
   {
    ++count;
    fvtell_player(NORMAL_T(p), " Entry removed for '^S^B%s^s'.\n",
                  list_node_name(tmp));

    list_delete_entry(head, list_type, tmp);
   }
  }

  str = end_name;
 }

 switch (count)
 {
  case 0:
    fvtell_player(NORMAL_T(p), " Deleted ^S^Bno^s %s list entries.\n", name);
    break;
  case 1:
    fvtell_player(NORMAL_T(p), " Deleted ^S^Bone^s %s list entry.\n", name);
    channels_timed_save(chan_base);
    break;
    
  default:
    fvtell_player(NORMAL_T(p), " Deleted ^S^B%s^s %s list entries.\n",
                  word_number_base(buf, 128, NULL, count,
                                   FALSE, word_number_def), name);
    channels_timed_save(chan_base);
 }
}

static int list_flags_get_offset(int list_type, const char *str)
{
 int count = 0;
 list_flag_off *tmp = list_flags_off(list_type);

 log_assert(tmp);
 
 while (tmp[count].name)
 {
  int save_cmp = 0;
  
  assert(tmp[count].func);
  if (!(save_cmp = beg_strcmp(str, tmp[count].name)))
    return (count);
  else if (save_cmp < 0)
    return (-1);

  ++count;
 }

 return (-1);
}

static int list_flag_change_pre(player *p, parameter_holder *params,
                                int list_type,
                                int *flag_offsets, size_t *flag_offset_count)
{
 char *flags = NULL;

 log_assert(params->last_param >= 2);
 
 if (!(*GET_PARAMETER_STR(params, 1) && *GET_PARAMETER_STR(params, 2)))
   return (FALSE);
 
 lower_case(GET_PARAMETER_STR(params, 1));
 lower_case(GET_PARAMETER_STR(params, 2));

 flags = GET_PARAMETER_STR(params, 2);
 while (flags)
 {
  int list_flag_offset = -1;
  char *tmp = next_parameter(flags, ',');

  if (tmp)
    *tmp++ = 0;
  
  if ((list_flag_offset = list_flags_get_offset(list_type, flags)) == -1)
  {
   *flag_offset_count = 0;
   if (p)
     fvtell_player(NORMAL_T(p),
                   " Bad list flag -- ^S^B%s^s -- no changes made.\n", flags);
   else
     log_assert(FALSE);
   return (FALSE);
  }

  flag_offsets[*flag_offset_count] = list_flag_offset;
  ++*flag_offset_count;

  if (*flag_offset_count >= LIST_FLAG_SZ)
  {
   *flag_offset_count = 0;
   if (p)
     fvtell_player(NORMAL_T(p), "%s",
                   " Too many list flags no changes made.\n");
   else
     log_assert(FALSE);
   return (FALSE);
  }
  
  flags = tmp;
 }

 return (TRUE);
}

static int list_flags_run_offset(player *p, const char *str, int offset,
                                 int list_type, list_node *entry)
{
 list_flag_off *tmp = list_flags_off(list_type);

 log_assert(tmp);
 
 return ((*tmp[offset].func)(p, str, entry));
}

static int list_flag_change_post(player *p, parameter_holder *params,
                                 int list_type, list_node **head,
                                 int max_entries,
                                 int *flag_offsets, size_t flag_offset_count)
{
 char *str = NULL;
 int ret = FALSE;
 
 assert(params->last_param > 1);

 str = GET_PARAMETER_STR(params, 1);
 while (str)
 {
  list_node *entry = NULL;
  char *str_after = next_parameter(str, ',');
  size_t count = 0;
  
  if (str_after)
    *str_after++ = 0;
  assert(!*(str + strcspn(str, ALPHABET_UPPER)));
  
  if (!(entry = list_create_or_find_entry(p, head, max_entries,
                                          list_type, str)))
    return (ret);

  while (count < flag_offset_count)
    list_flags_run_offset(p, GET_PARAMETER_STR(params, 3),
                          flag_offsets[count++], list_type, entry);

  if (!list_is_used_entry(list_type, entry))
    list_delete_entry(head, list_type, entry);

  ret = TRUE; /* always return true atm. better if we can return true only
                 * when it has changed though -- save saves */
  str = str_after;
 }

 return (ret);
}

#if 0 /* might want to add these in ... with a parse type */
static void user_list_set(player *p, parameter_holder *params)
{
 int flag_offsets[LIST_FLAG_SZ];
 size_t flag_offset_count = 0;
 const char *cun_on = "on";

 if (params->last_param != 2)
   TELL_FORMAT(p, "<player(s)> <flag(s)>");
 get_parameter_parse(params, &cun_on, 3);

 list_flag_change_pre(p, params, flag_offsets, &flag_offset_count);
 if (!flag_offset_count)
   return;

 list_flag_change_post(p, params, flag_offsets, flag_offset_count);
}

static void user_list_remove(player *p, parameter_holder *params)
{
 int flag_offsets[LIST_FLAG_SZ];
 size_t flag_offset_count = 0;
 const char *cun_off = "off";

 if (params->last_param != 2)
   TELL_FORMAT(p, "<player(s)> <flag(s)>");
 get_parameter_parse(params, &cun_off, 3);
 
 list_flag_change_pre(p, params, flag_offsets, &flag_offset_count);
 if (!flag_offset_count)
   return;
 
 list_flag_change_post(p, params, flag_offsets, flag_offset_count);
}

static void user_list_toggle(player *p, parameter_holder *params)
{
 int flag_offsets[LIST_FLAG_SZ];
 size_t flag_offset_count = 0;
 const char *cun_toggle = "toggle";

 if (params->last_param != 2)
   TELL_FORMAT(p, "<player(s)> <flag(s)>");
 get_parameter_parse(params, &cun_toggle, 3);
 
 list_flag_change_pre(p, params, flag_offsets, &flag_offset_count);
 if (!flag_offset_count)
   return;
 
 list_flag_change_post(p, params, flag_offsets, flag_offset_count);
}
#endif

static void user_list_player_change(player *p, parameter_holder *params)
{
 int flag_offsets[LIST_FLAG_SZ];
 size_t flag_offset_count = 0;
 const char *cun_toggle = "toggle";
 const char *cun_on = "on";
 const char *cun_off = "off";
 int list_type = LIST_TYPE_SELF;
 list_node **head = &p->list_self_start;
 const char *name = "Self";
 
 switch (params->last_param)
 {
  case 4:
    break;
  
  case 3:
  {
   switch (*GET_PARAMETER_STR(params, 3))
   {
    case '=':
      /* FIXME: unsupported ... */
      /* FALLTHROUGH */
    case '+':
      ++GET_PARAMETER_STR(params, 3);
      --GET_PARAMETER_LENGTH(params, 3);
      get_parameter_parse(params, &cun_on, 4);
      break;
    case '-':
      ++GET_PARAMETER_STR(params, 3);
      --GET_PARAMETER_LENGTH(params, 3);
      get_parameter_parse(params, &cun_off, 4);
      break;
    case '~':
      ++GET_PARAMETER_STR(params, 3);
      --GET_PARAMETER_LENGTH(params, 3);
      
    default:
      get_parameter_parse(params, &cun_toggle, 4);
   }
  }
  break;
  
  default:
    TELL_FORMAT(p, "<type> <player(s)> <flag(s)> <on|off|toggle>");
 }

 lower_case(GET_PARAMETER_STR(params, 1));
 
 if (!list_type_player_parse(p, GET_PARAMETER_STR(params, 1),
                             &head, &list_type, &name))
   TELL_FORMAT(p, "<self [tmp]|games|communication [tmp]> <player(s)> "
               "<flag(s)> <on|off|toggle>");
 get_parameter_shift(params, 1);
 
 list_flag_change_pre(p, params, list_type, flag_offsets, &flag_offset_count);
 if (!flag_offset_count)
   return;
 
 list_flag_change_post(p, params, list_type, head, p->max_list_entries,
                       flag_offsets, flag_offset_count);
}

void user_list_room_change(player *p, parameter_holder *params)
{
 int flag_offsets[LIST_FLAG_SZ];
 size_t flag_offset_count = 0;
 const char *cun_toggle = "toggle";
 const char *cun_on = "on";
 const char *cun_off = "off";
 int list_type = LIST_TYPE_ROOM;
 list_node **head = &p->location->list_room_local_start;
 const char *name = "Local";
 
 switch (params->last_param)
 {
  case 4:
    break;
  
  case 3:
  {
   switch (*GET_PARAMETER_STR(params, 3))
   {
    case '=':
      /* FIXME: unsupported ... */
      /* FALLTHROUGH */
    case '+':
      ++GET_PARAMETER_STR(params, 3);
      --GET_PARAMETER_LENGTH(params, 3);
      get_parameter_parse(params, &cun_on, 4);
      break;
    case '-':
      ++GET_PARAMETER_STR(params, 3);
      --GET_PARAMETER_LENGTH(params, 3);
      get_parameter_parse(params, &cun_off, 4);
      break;
    case '~':
      ++GET_PARAMETER_STR(params, 3);
      --GET_PARAMETER_LENGTH(params, 3);
      
    default:
      get_parameter_parse(params, &cun_toggle, 4);
   }
  }
  break;
  
  default:
    TELL_FORMAT(p, "<type> <player(s)> <flag(s)> <on|off|toggle>");
 }

 lower_case(GET_PARAMETER_STR(params, 1));
 
 if (!list_type_room_parse(p, GET_PARAMETER_STR(params, 1), &head, &name))
   TELL_FORMAT(p, "<global|local> <player(s)> <flag(s)> <on|off|toggle>");
 get_parameter_shift(params, 1);

 list_flag_change_pre(p, params, list_type, flag_offsets, &flag_offset_count);
 if (!flag_offset_count)
   return;
 
 if (list_flag_change_post(p, params, list_type, head, p->max_list_entries,
                           flag_offsets, flag_offset_count))
   p->location->owner->flag_tmp_room_needs_saving = TRUE;
}

void user_list_chan_change(player *p, parameter_holder *params)
{
 int flag_offsets[LIST_FLAG_SZ];
 size_t flag_offset_count = 0;
 const char *cun_toggle = "toggle";
 const char *cun_on = "on";
 const char *cun_off = "off";
 int list_type = LIST_TYPE_CHAN;
 list_node **head = NULL;
 channels_base *chan_base = NULL;
 char *str = NULL;
 
 switch (params->last_param)
 {
  case 4:
    break;
  
  case 3:
  {
   switch (*GET_PARAMETER_STR(params, 3))
   {
    case '=': /* FIXME: unsupported ... */
      /* FALLTHROUGH */
    case '+':
      ++GET_PARAMETER_STR(params, 3);
      --GET_PARAMETER_LENGTH(params, 3);
      get_parameter_parse(params, &cun_on, 4);
      break;
    case '-':
      ++GET_PARAMETER_STR(params, 3);
      --GET_PARAMETER_LENGTH(params, 3);
      get_parameter_parse(params, &cun_off, 4);
      break;
    case '~':
      ++GET_PARAMETER_STR(params, 3);
      --GET_PARAMETER_LENGTH(params, 3);
      
    default:
      get_parameter_parse(params, &cun_toggle, 4);
   }
  }
  break;
  
  default:
    TELL_FORMAT(p, "<channel> <player(s)> <flag(s)> <on|off|toggle>");
 }

 str = GET_PARAMETER_STR(params, 1);
 lower_case(str);
 
 if (!(chan_base = channels_user_find_grant_base(p, str)))
   return;
 get_parameter_shift(params, 1);
 head = &chan_base->list_start;
 
 list_flag_change_pre(p, params, list_type, flag_offsets, &flag_offset_count);
 if (!flag_offset_count)
   return;
 
 if (list_flag_change_post(p, params, list_type, head, p->max_list_entries,
                           flag_offsets, flag_offset_count))
   channels_timed_save(chan_base);
}

int list_system_change(list_node **head, int list_type, const char *str)
{
 int flag_offsets[LIST_FLAG_SZ];
 size_t flag_offset_count = 0;
 parameter_holder params;
 
 get_parameter_init(&params);
 if (!get_parameter_parse(&params, &str, 3))
   return (FALSE);
 
 list_flag_change_pre(NULL, &params, list_type,
                      flag_offsets, &flag_offset_count);
 if (!flag_offset_count)
   return (FALSE);

 return (list_flag_change_post(NULL, &params, list_type, head, INT_MAX,
                               flag_offsets, flag_offset_count));
}

static void user_list_inform(player *p, parameter_holder *params)
{ MAKE_LIST_FLAG_FUNC("inform", LIST_TYPE_SELF, &p->list_self_start,
                      p->max_list_entries); }
static void user_list_grab(player *p, parameter_holder *params)
{ MAKE_LIST_FLAG_FUNC("grab", LIST_TYPE_SELF, &p->list_self_start,
                      p->max_list_entries); }
static void user_list_friend(player *p, parameter_holder *params)
{ MAKE_LIST_FLAG_FUNC("friend", LIST_TYPE_SELF, &p->list_self_start,
                      p->max_list_entries); }
static void user_list_find(player *p, parameter_holder *params)
{ MAKE_LIST_FLAG_FUNC("find", LIST_TYPE_SELF, &p->list_self_start,
                      p->max_list_entries); }
static void user_list_beep(player *p, parameter_holder *params)
{ MAKE_LIST_FLAG_FUNC("inform_beep", LIST_TYPE_SELF, &p->list_self_start,
                      p->max_list_entries); }

static void user_list_ignore(player *p, parameter_holder *params)
{ MAKE_LIST_FLAG_FUNC("all", LIST_TYPE_COMS, &p->list_coms_start,
                      p->max_list_entries); }
static void user_list_tignore(player *p, parameter_holder *params)
{ MAKE_LIST_FLAG_FUNC("all", LIST_TYPE_COMS, &p->list_coms_tmp_start,
                      p->max_list_entries); }

static void user_list_bar(player *p, parameter_holder *params)
{ MAKE_LIST_FLAG_FUNC("bar", LIST_TYPE_ROOM, &p->saved->list_room_glob_start,
                      p->max_list_entries); }
static void user_list_invite(player *p, parameter_holder *params)
{ MAKE_LIST_FLAG_FUNC("invite", LIST_TYPE_ROOM, &p->saved->list_room_glob_start,
                      p->max_list_entries); }
static void user_list_key(player *p, parameter_holder *params)
{ MAKE_LIST_FLAG_FUNC("key", LIST_TYPE_ROOM, &p->saved->list_room_glob_start,
                      p->max_list_entries); }

static void list_show_all(player *p, int list_type, list_node *head,
                          const char *name)
{
 list_node *scan = head;
 list_node *grouped[8];
 char buffer[sizeof("%s list, %d of %d") + 1024 + (2 * BUF_NUM_TYPE_SZ(int))];
 unsigned int count = 0;

 assert(strlen(name) < 1024);
 
 if (!scan)
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You do not have any names on your list.\n");
  return;
 }
 
 sprintf(buffer, "%s list, %d of %d",
         name, list_get_size(head), p->max_list_entries);
 ptell_mid(NORMAL_T(p), buffer, FALSE);

 while (scan)
 {
  if (list_node_grouped(list_type, scan))
    grouped[count++] = scan;
  else
    list_tell_all_entry(p, list_type, scan);
  
  scan = scan->next;
 }

 if (count)
 {
  unsigned int tmp = 0;
  
  ptell_mid(NORMAL_T(p), "Groups", FALSE);
  
  while ((tmp < count) && (tmp < (sizeof(grouped) / sizeof(grouped[0]))))
    list_tell_all_entry(p, list_type, grouped[tmp++]);
 }

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void user_list_player_view(player *p, parameter_holder *params)
{
 int list_type = LIST_TYPE_SELF;
 list_node **head = &p->list_self_start;
 const char *name = "Self";
 player_tree_node *sp = NULL;
 list_node *entry = NULL;
 
 switch (params->last_param)
 {
  case 2:
    lower_case(GET_PARAMETER_STR(params, 1));
    
    if (!list_type_player_parse(p, GET_PARAMETER_STR(params, 1),
                                &head, &list_type, &name))
      TELL_FORMAT(p, "<communication [tmp]|games|self [tmp]> <player>");

    if (!strcasecmp(GET_PARAMETER_STR(params, 2), "newbies") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "karma") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "ministers") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "spods") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "admins") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "sus") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "everyone"))
      entry = list_find_entry(*head, GET_PARAMETER_STR(params, 2));
    else
    {
     if (!(sp = player_find_all(p, GET_PARAMETER_STR(params, 2),
                                PLAYER_FIND_SC_EXTERN)))
       return;
     entry = list_onanywhere(p, *head, sp);
    }
    
    list_tell_single_entry(p, list_type, entry);
   break;
   
  case 1:
    lower_case(GET_PARAMETER_STR(params, 1));

    if (!list_type_player_parse(p, GET_PARAMETER_STR(params, 1),
                                &head, &list_type, &name))
      TELL_FORMAT(p, "<communication [tmp]|games|self [tmp]> <player>");
    
  case 0: /* _try_ to maintain compat with EW */
    list_show_all(p, list_type, *head, name);
    
    pager(p, PAGER_DEFAULT);
    break;
    
  default:
    TELL_FORMAT(p, "<type> [player]");
 }
}

void user_list_other_player_view(player *p, parameter_holder *params)
{
 int list_type = LIST_TYPE_SELF;
 list_node **head = NULL;
 const char *name = "Self";
 player *p2 = NULL;
 list_node *entry = NULL;
 
 switch (params->last_param)
 {
  case 2:
    lower_case(GET_PARAMETER_STR(params, 1));

    if (!(p2 = player_find_load(p, GET_PARAMETER_STR(params, 2),
                                PLAYER_FIND_SC_EXTERN)))
      return;

    head = &p2->list_self_start;
    
    if (!list_type_player_parse(p, GET_PARAMETER_STR(params, 1),
                                &head, &list_type, &name))
      TELL_FORMAT(p, "<communication [tmp]|games|self [tmp]> <player>");
    
    entry = list_onanywhere(p2, *head, p->saved);
    
    list_tell_single_entry(p, list_type, entry);
   break;
   
  default:
    TELL_FORMAT(p, "<type> <player>");
 }
}

void user_list_room_view(player *p, parameter_holder *params)
{
 int list_type = LIST_TYPE_ROOM;
 list_node **head = &p->location->owner->list_room_glob_start;
 const char *name = "$Room-Owner-Name global";
 player_tree_node *sp = NULL;
 list_node *entry = NULL;

 switch (params->last_param)
 {
  case 2:
    lower_case(GET_PARAMETER_STR(params, 1));
    
    if (!list_type_room_parse(p, GET_PARAMETER_STR(params, 1), &head, &name))
      TELL_FORMAT(p, "<global|local> <player>");

    if (!strcasecmp(GET_PARAMETER_STR(params, 2), "newbies") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "karma") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "ministers") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "spods") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "admins") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "sus") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "everyone"))
      entry = list_find_entry(*head, GET_PARAMETER_STR(params, 2));
    else
    {
     if (!(sp = player_find_all(p, GET_PARAMETER_STR(params, 2),
                                PLAYER_FIND_SC_EXTERN)))
       return;
     entry = list_onanywhere(p, *head, sp);
    }
    
    list_tell_single_entry(p, list_type, entry);
   break;
   
  case 1:
    lower_case(GET_PARAMETER_STR(params, 1));
    
    if (!list_type_room_parse(p, GET_PARAMETER_STR(params, 1), &head, &name))
      TELL_FORMAT(p, "<global|local> <player>");
    
  case 0: /* _try_ to maintain compat with EW */
    list_show_all(p, list_type, *head, name);
    
    pager(p, PAGER_DEFAULT);
    break;
    
  default:
    TELL_FORMAT(p, "<global|local> <player>");
 }
}

void user_list_chan_view(player *p, parameter_holder *params)
{
 int list_type = LIST_TYPE_CHAN;
 list_node **head = NULL;
 const char *name = "Channel";
 player_tree_node *sp = NULL;
 list_node *entry = NULL;
 channels_base *chan_base = NULL;
 
 switch (params->last_param)
 {
  case 2:
    lower_case(GET_PARAMETER_STR(params, 1));
    
    if (!(chan_base = channels_user_find_base(p,
                                              GET_PARAMETER_STR(params, 1),
                                              TRUE)))
      TELL_FORMAT(p, "<channel> <player(s)>");
    head = &chan_base->list_start;
    
    if (!strcasecmp(GET_PARAMETER_STR(params, 2), "newbies") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "karma") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "ministers") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "spods") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "admins") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "sus") ||
        !strcasecmp(GET_PARAMETER_STR(params, 2), "everyone"))
      entry = list_find_entry(*head, GET_PARAMETER_STR(params, 2));
    else
    {
     if (!(sp = player_find_all(p, GET_PARAMETER_STR(params, 2),
                                PLAYER_FIND_SC_EXTERN)))
       return;
     entry = list_onanywhere(p, *head, sp);
    }
    
    list_tell_single_entry(p, list_type, entry);
   break;
   
  case 1:
    lower_case(GET_PARAMETER_STR(params, 1));
    
    if (!(chan_base = channels_user_find_base(p,
                                              GET_PARAMETER_STR(params, 1),
                                              TRUE)))
      TELL_FORMAT(p, "<channel> <player(s)>");
    head = &chan_base->list_start;
    
    list_show_all(p, list_type, *head, name);
    
    pager(p, PAGER_DEFAULT);
    break;
    
  default:
    TELL_FORMAT(p, "<channel> <player(s)>");
 }
}

static void user_list_friendof(player *p, const char *str)
{
 tmp_output_list_storage tmp_save; 
 int count = 0;
 int out_flags = 0;
  
 save_tmp_output_list(p, &tmp_save);

 DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
               (construct_name_list_do,
                current_players, &count,
                ((str && (*str == '-')) ? 0 : CONSTRUCT_NAME_USE_PREFIX) |
                CONSTRUCT_NAME_USE_LIST_ENT_THEM,
                (priv_test_list_type)NULL, list_self_priv_test_friend,
                &out_flags,
                NORMAL_FT(OUTPUT_BUFFER_TMP | RAW_OUTPUT, p)));
 
 if (out_flags & CONSTRUCT_NAME_OUT_MID_LINE)
   fvtell_player(NORMAL_FT(OUTPUT_BUFFER_TMP, p), "%s", "\n");
 
 if (!count)
 {
  output_node *tmp = output_list_grab(p);
  
  fvtell_player(SYSTEM_T(p), "%s", " You aren't the friend of anyone.\n");
  output_list_cleanup(&tmp);
  load_tmp_output_list(p, &tmp_save);
 }
 else
 {
  output_node *tmp = output_list_grab(p);
  char buf[128];
  
  load_tmp_output_list(p, &tmp_save);
  
  if (count == 1)
    fvtell_player(NORMAL_T(p),
                  " You are the friend of one person...\n");
  else
    fvtell_player(NORMAL_T(p), " You are the friend of %s people...\n",
                  word_number_base(buf, 128, NULL, count,
                                   FALSE, word_number_def));
  
  output_list_linkin(p, 3, &tmp, INT_MAX);
 }
}

static void user_list_grabable(player *p, const char *str)
{
 tmp_output_list_storage tmp_save; 
 int count = 0;
 int out_flags = 0;
  
 save_tmp_output_list(p, &tmp_save);

 DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
               (construct_name_list_do,
                current_players, &count,
                ((str && (*str == '-')) ? 0 : CONSTRUCT_NAME_USE_PREFIX) |
                CONSTRUCT_NAME_USE_LIST_ENT_THEM,
                (priv_test_list_type)NULL, list_self_priv_test_grab,
                &out_flags,
                NORMAL_FT(OUTPUT_BUFFER_TMP | RAW_OUTPUT, p)));
 
 if (out_flags & CONSTRUCT_NAME_OUT_MID_LINE)
   fvtell_player(NORMAL_FT(OUTPUT_BUFFER_TMP, p), "%s", "\n");
 
 if (!count)
 {
  output_node *tmp = output_list_grab(p);

  fvtell_player(SYSTEM_T(p), "%s", " You can't grab anyone.\n");
  output_list_cleanup(&tmp);
  load_tmp_output_list(p, &tmp_save);
 }
 else
 {
  output_node *tmp = output_list_grab(p);
  char buf[128];
  
  load_tmp_output_list(p, &tmp_save);
  
  if (count == 1)
    fvtell_player(NORMAL_T(p),
                  " You can grab one person...\n");
  else
    fvtell_player(NORMAL_T(p), " You can grab %s people...\n",
                  word_number_base(buf, 128, NULL, count,
                                   FALSE, word_number_def));
  
  output_list_linkin(p, 3, &tmp, INT_MAX);
 }
}

#if 0
static void user_invites_list(player *p, const char *str)
{
 tmp_output_list_storage tmp_save; 
 int count = 0;
 int out_flags = 0;
  
 save_tmp_output_list(p, &tmp_save);

 DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
               (construct_name_list_do,
                current_players, &count,
                ((str && (*str == '-')) ? 0 : CONSTRUCT_NAME_USE_PREFIX) |
                CONSTRUCT_NAME_USE_LIST_ENT_THEM,
                (priv_test_list_type)NULL, list_self_priv_test_invite,
                &out_flags,
                NORMAL_FT(OUTPUT_BUFFER_TMP | RAW_OUTPUT, p)));
 
 if (out_flags & CONSTRUCT_NAME_OUT_MID_LINE)
   fvtell_player(NORMAL_FT(OUTPUT_BUFFER_TMP, p), "%s", "\n");
 
 if (!count)
 {
  output_node *tmp = output_list_grab(p);
  
  fvtell_player(SYSTEM_T(p), "%s",
                " You haven't got an invite off anyone.\n");
  output_list_cleanup(&tmp);
  load_tmp_output_list(p, &tmp_save);
 }
 else
 {
  output_node *tmp = output_list_grab(p);
  load_tmp_output_list(p, &tmp_save);
  char buf[128];
  
  if (count == 1)
    fvtell_player(NORMAL_T(p),
                  " You have invites from one person...\n");
  else
    fvtell_player(NORMAL_T(p), " You have invites off %s people.\n",
                  word_number_base(buf, 128, NULL, count,
                                   FALSE, word_number_def));

  output_list_linkin(p, 3, &tmp, INT_MAX);
 }
}
#endif

static void user_list_change_newbie_time(player *p, parameter_holder *params)
{ /* FIXME: make lsn a resi command ? -- spod command ? */
 unsigned long newbie_time = 0;
 int err = FALSE;
 char buf[256];
 
 if (params->last_param != 1)
   TELL_FORMAT(p, "<time>");

 newbie_time = word_time_parse(GET_PARAMETER_STR(params, 1),
                               WORD_TIME_PARSE_ERRORS, &err);

 if (err)
 {
  fvtell_player(NORMAL_T(p), " The string -- ^S^B%s^s -- isn't a valid "
                "time value.\n", GET_PARAMETER_STR(params, 1));
  return;
 }
 
 if (!PRIV_STAFF(p->saved))
 {
  if (newbie_time < MK_HOURS(2))
  {
   fvtell_player(SYSTEM_T(p), "%s",
                 " You are not allowed to set a time lower "
                 "than -- ^S^B2 hours^s --.\n");
   return;
  }
 }
 if (newbie_time > (unsigned)real_total_logon_time(p->saved))
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You are not allowed to set a time higher "
                "than your unidle logon time.\n");
  return;
 }

 p->list_newbie_time = newbie_time;

 fvtell_player(NORMAL_T(p),
               " Newbies will be classed as all those people who are "
               "not residents or have less than '^S^B%s^s' logon time.\n",
               word_time_long(buf, sizeof(buf),
                              newbie_time, WORD_TIME_DEFAULT));
}

void init_list(void)
{
 timer_q_add_static_base(&list_load_player_timer_queue, timed_list_cleanup,
                         TIMER_Q_FLAG_BASE_INSERT_FROM_END |
                         TIMER_Q_FLAG_BASE_RUN_ALL);

}

void cmds_init_list(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("beep", user_list_beep, PARSE_PARAMS, LIST);
 CMDS_PRIV(command_list);
 CMDS_ADD("clist", user_list_player_del, PARSE_PARAMS, LIST);
 CMDS_PRIV(command_list);
 CMDS_ADD("find", user_list_find, PARSE_PARAMS, LIST);
 CMDS_PRIV(command_list);
 CMDS_ADD("friend", user_list_friend, PARSE_PARAMS, LIST);
 CMDS_PRIV(command_list);
 CMDS_ADD("grabme", user_list_grab, PARSE_PARAMS, LIST);
 CMDS_PRIV(command_list);
 CMDS_ADD("inform", user_list_inform, PARSE_PARAMS, LIST);
 CMDS_PRIV(command_list);

 CMDS_ADD("ignore", user_list_ignore, PARSE_PARAMS, COMMUNICATION);
 CMDS_PRIV(command_list); CMDS_XTRA_SECTION(LIST);
 CMDS_ADD("tignore", user_list_tignore, PARSE_PARAMS, COMMUNICATION);
 CMDS_PRIV(command_list); CMDS_XTRA_SECTION(LIST);

 CMDS_ADD("bar", user_list_bar, PARSE_PARAMS, LOCAL);
 CMDS_PRIV(command_list); CMDS_XTRA_SECTION(LIST);
 CMDS_ADD("invite", user_list_invite, PARSE_PARAMS, LOCAL);
 CMDS_PRIV(command_list); CMDS_XTRA_SECTION(LIST);
 CMDS_ADD("key", user_list_key, PARSE_PARAMS, LOCAL);
 CMDS_PRIV(command_list); CMDS_XTRA_SECTION(LIST);

 CMDS_ADD("friendof", user_list_friendof, CONST_CHARS, INFORMATION);
 CMDS_PRIV(base); CMDS_XTRA_SECTION(LIST);
 CMDS_ADD("grabable", user_list_grabable, CONST_CHARS, INFORMATION);
 CMDS_XTRA_SECTION(LIST);

 CMDS_ADD("list", user_list_player_view, PARSE_PARAMS, PERSONAL_INFO);
 CMDS_PRIV(command_list); CMDS_XTRA_SECTION(LIST);
 CMDS_ADD("list_change", user_list_player_change, PARSE_PARAMS, PERSONAL_INFO);
 CMDS_PRIV(command_list); CMDS_XTRA_SECTION(LIST);

 CMDS_ADD("newbie_time", user_list_change_newbie_time, PARSE_PARAMS, LIST);
 CMDS_PRIV(command_list);
}
