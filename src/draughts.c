#define DRAUGHTS_C
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

/* variables */

static player_linked_list *draughts_games_list = 0;


/* functions */



/* internals */
static void draughts_game_init(draughts_game *current)
{
 current->black = NULL;
 current->white = NULL;

 current->black_locations = 0;
 current->black_kings = 0;
 current->black_moves = 0;

 current->white_locations = 0;
 current->white_kings = 0;
 current->white_moves = 0;

 current->flags = 0;

 current->last_moved = now;

 current->to_move = NULL;

 current->started_at = now;

 current->watchers = NULL;
}

/* returns the game type for the player (used externally) */
const char *get_game_type(player *p, int caps)
{
 return (GAME_TYPE(p, caps));
}
 
/* check to see if p is watching (only) a game, retuns true if they are */
int draughts_is_watching_game(player *p)
{
 if (p->draughts_game)
   if (IS_PLAYER(p, p->draughts_game))
     return (FALSE);
   else
     return (TRUE);
 else
   return (FALSE);
}

/* checks to see if p is a player of the game, returns true if they are */
int draughts_is_playing_game(player *p)
{
 if (p->draughts_game)
   if (IS_PLAYER(p, p->draughts_game))
     return (TRUE);
   else
     return (FALSE);
 else
   return (FALSE);
}

/* returns a pointer which is the opponent player */
static player *get_opponent(player *p)
{
 assert(draughts_is_playing_game(p)); /* pre checked */

 if (p->draughts_game->black == p)
   return (p->draughts_game->white);
 else
   return (p->draughts_game->black);
}

/* counts how many draughts games are currently public (on the list) */
static int count_number_of_draughts_games(void)
{
 if (draughts_games_list)
 {
  player_linked_list *current = draughts_games_list;
  int number_of_games = 0;

  do
    number_of_games++;
  while (PLAYER_LINK_NEXT(current));

  return (number_of_games);
 }

 return (0);
}

/* frees all the watchers for a game (destroys the list) */
static void free_watchers_list(draughts_game *game)
{
 player_linked_list *next = 0;
 player_linked_list *current = 0;
 
 assert(game->watchers);

 current = game->watchers;
 
 /* free each previous element */
 while (current)
 {
  next = PLAYER_LINK_NEXT(current); /* get the next struct */
  assert(PLAYER_LINK_GET(current));
  PLAYER_LINK_GET(current)->draughts_game = NULL; /* stop player watching */
  /* call the func to free an element */
  player_link_del(&game->watchers, PLAYER_LINK_SAV_GET(current), NULL,
                  PLAYER_LINK_DEFAULT, NULL); 
  current = next; /* make the next one in the list the current */
 }
}

/* counts the number of people watching a draughts game */
static const char *people_watching_draughts(draughts_game *game, int none)
{
 static char number[BUF_NUM_TYPE_SZ(int)];
 int ppl_watching = 0;
 player_linked_list *current = game->watchers;
 assert (game);

 if (game->watchers)
 {
  for(; current; current = PLAYER_LINK_NEXT(current))
    ppl_watching++;
   
  sprintf(number, "%d", ppl_watching);
  return (number);
 }
 else
   if (none)
     return ("None");
   else
     return ("no");
}

static void destroy_draughts_game(draughts_game *game)
{
 assert(game);

 if (!(game->flags & DRAUGHTS_PROPOSED))
   if (player_link_find(draughts_games_list, game->black->saved, NULL,
                        PLAYER_LINK_DEFAULT))
     player_link_del(&draughts_games_list, game->black->saved, NULL,
                     PLAYER_LINK_DEFAULT, NULL);
 
 /* get rid of the watchers list if there is one */
 if (game->watchers)
   free_watchers_list(game);

 /* get rid of the actual game struct */
 XFREE(game, DRAUGHTS);
}

void draughts_stop_watching_game(player *p, int verb)
{
 if (draughts_is_watching_game(p))
 {
  player_link_del(&p->draughts_game->watchers, p->saved, NULL,
                  PLAYER_LINK_DEFAULT, NULL);
  p->draughts_game = NULL;
  if (verb)
    fvtell_player(NORMAL_T(p),
                  " You stop watching the %s game.\n",
                  GAME_TYPE(p, 0));
 }
 else if (verb)
   fvtell_player(NORMAL_T(p),
                 " You are not watching a %s game.\n", GAME_TYPE(p, 0));
}

/* rogue command thats called from an internal */
/* withdraws an OFFER of a game only - doesn't assume that player offered is
   on the program */
static void user_draughts_withdraw(player *p)
{
 player *p2 = 0;
 
 if (p->draughts_game && IS_PLAYER(p, p->draughts_game))
 {
  if (p->draughts_game->flags & DRAUGHTS_PROPOSED)
  {
   if ((p2 = p->draughts_game->white)) /* must be the oposition! */
     /* tell p2 (white) its been withdrawn */
     fvtell_player(SYSTEM_T(p2),
                   "%s -=> %s withdraws his offer of a game of %s.^N\n",
                   USER_COLOUR_DRAUGHTS, p->saved->name, GAME_TYPE(p, 0));
   
   /* free it */
   destroy_draughts_game(p->draughts_game);
   p->draughts_game = NULL;
   
   /* tell you its all done */
   fvtell_player(NORMAL_T(p), "%s", " You withdraw your offer of a game");
   
   if (p2)
     fvtell_player(NORMAL_T(p), "with %s.\n",
                   p2->saved->name);
   else
     fvtell_player(NORMAL_T(p), ".\n");
  }
  else
  {
   fvtell_player(NORMAL_T(p), "%s",
                 " You cannot withdraw an offer of a game now it's been "
                 "accepted!\n");
  }
 }
 else
   fvtell_player(NORMAL_T(p), "%s",
                 " You cannot withdraw an offer of a game when you haven't "
                 "offered anyone one!\n");
}

/* internals again... */
/* declines the offer of a game, if there is one - verbose int */
static void decline_draughts_game(player *p, const char *str, int verb_on_err)
{
 player *p2 = 0;

 if (*str)
   if ((p2 = player_find_on(p, str,
                            PLAYER_FIND_SC_EXTERN | PLAYER_FIND_PICK_FIRST)))
   {
    if (p2->draughts_game && IS_PLAYER(p, p2->draughts_game) &&
        IS_PLAYER(p2, p2->draughts_game))
      if (p2->draughts_game->flags & DRAUGHTS_PROPOSED)
      { /* decline it */
       int do_msg = TRUE;
       
       fvtell_player(NORMAL_T(p),
                     " You decline %s%s offer of a %s game.\n",
                     p2->saved->name,
                     (p->gender == GENDER_PLURAL) ? "" : "'s",
                     GAME_TYPE(p, 0));
       
       LIST_GAME_CHECK_FLAG_START(p2, p->saved);
       if (LIST_GAME_CHECK_FLAG_DO(draughts))
         do_msg = FALSE;
       LIST_GAME_CHECK_FLAG_END();
       
       if (do_msg)
         fvtell_player(SYSTEM_T(p2),
                       "%s -=> %s declines your offer of a game of %s.^N\n",
                       USER_COLOUR_DRAUGHTS, p->saved->name, GAME_TYPE(p2, 0));
       /* destroy their offer */
       destroy_draughts_game(p2->draughts_game);
       p2->draughts_game = NULL;
      }
      else
      {
       assert(verb_on_err); /* should never get here if this is false */
       assert(p->draughts_game); /* they must be a player in p2's game */
       fvtell_player(NORMAL_T(p),
                     " You are already in a game with %s, it's too late to "
                     "decline %s offer!\n", p2->saved->name,
                     gender_choose_str(p2->gender, "him", "her",
                                       "them", "it"));
      }
    else if (verb_on_err)
    {
     if (p2 == p)
       fvtell_player(NORMAL_T(p), "%s",
                     " You can't even offer youself a game, how can you "
                     "decline one?!\n");
     else
       fvtell_player(NORMAL_T(p),
                     " %s hasn't offered you a game of %s.\n",
                     p2->saved->name, GAME_TYPE(p, 0));
    }
   }
   else if (verb_on_err) /* shouldn't happen if this int is off anyhow */
     fvtell_player(NORMAL_T(p),
                   " Cannot find anyone of the name -- ^S^B%s^s -- to "
                   "decline.\n", str);
   else
     TELL_FORMAT(p, "<name>");
 else if (verb_on_err)
     TELL_FORMAT(p, "<player_to_decline>");
}

/* when you quit crazy or stop watching a game, this is called */
static void user_draughts_stop_watching(player *p)
{
 draughts_stop_watching_game(p, 1);
}

/* checks if you're in a game, and then loses it for you, called from ignore */
void check_if_on_draughts_game(player *p, player *p2)
{
 /* p2 is ignored player pointer */
 if (p->draughts_game)
 {
  if ((IS_PLAYER(p, p->draughts_game)) && (IS_PLAYER(p2, p->draughts_game)))
  {
   if (p->draughts_game->flags & DRAUGHTS_PROPOSED)
     user_draughts_withdraw(p); /* shortcut */
   else
   {
    fvtell_player(NORMAL_T(p),
		  " You ignore %s and therefore resign the game you "
		  "are playing with %s.\n", p2->saved->name,
                  gender_choose_str(p2->gender, "him", "her", "them", "it"));
    draughts_lose_game(p, DRAUGHTS_LOST_RESIGNED); /* resign the game */
   }
  }
 }
 else
   decline_draughts_game(p, p2->saved->lower_name, 0);
}

/* sets the flag in the player file for the game name for the player */
static void set_game_type(player *p)
{
 assert(p);
 
 if (current_command)
 {
  if (strcasecmp(current_command, "checkers"))
    p->draughts_as_checkers = FALSE;
  else
    p->draughts_as_checkers = TRUE;
 }
}

/* counts the number of flags set in an int = num of pieces left */
static int number_of_pieces(int pieces)
{
 return (number_of_flags(pieces));
}

/* counts the number of flags set in an int = num of kings left - as
   above function, but adds more readability to the code elsewhere */
static int number_of_kings(int kings)
{
 return (number_of_flags(kings));
}

/* tells something to everyone involved in the game - players, watchers */
static void vtell_all_involved_in_game(draughts_game *game, player *from,
                                       const char *fmt, ...)
{
 player_linked_list *current = game->watchers;
 VA_R_DECL(r_ap);
 
 VA_R_START(r_ap, fmt);

 if (game->white)
 {
  int do_chat = TRUE;

  if (from)
  {
   LIST_COMS_CHECK_FLAG_START(game->white, from->saved);
   if (LIST_COMS_CHECK_FLAG_DO(channels))
     do_chat = FALSE;
   LIST_COMS_CHECK_FLAG_END();
  }
  
  if (do_chat)
  {
   VA_C_DECL(ap);
   
   VA_C_START(ap, fmt);
   
   VA_C_COPY(ap, r_ap);
   
   vfvtell_player(TALK_T(from->saved, game->white), fmt, ap);
   
   VA_C_END(ap);
  }
 }
 
 if (game->black)
 {
  int do_chat = TRUE;
  
  if (from)
  {
   LIST_COMS_CHECK_FLAG_START(game->black, from->saved);
   if (LIST_COMS_CHECK_FLAG_DO(channels))
     do_chat = FALSE;
   LIST_COMS_CHECK_FLAG_END();
  }
  
  if (do_chat)
  {
   VA_C_DECL(ap);
   
   VA_C_START(ap, fmt);
   
   VA_C_COPY(ap, r_ap);
   
   vfvtell_player(TALK_T(from->saved, game->black), fmt, ap);
     
   VA_C_END(ap);
  }
 }
 
 while (current) /* wall to the watchers */
 {
  int do_chat = TRUE;
  
  if (from)
  {
   LIST_COMS_CHECK_FLAG_START(PLAYER_LINK_GET(current), from->saved);
   if (LIST_COMS_CHECK_FLAG_DO(channels))
     do_chat = FALSE;
   LIST_COMS_CHECK_FLAG_END();
  }
  
  if (do_chat)
  {
   VA_C_DECL(ap);
   
   VA_C_START(ap, fmt);
   
   VA_C_COPY(ap, r_ap);
   
   vfvtell_player(TALK_T(from->saved, PLAYER_LINK_GET(current)), fmt, ap);
   
   VA_C_END(ap);
  }
 }

 VA_R_END(r_ap);
}

/* tells the particular persons game type to each watcher */
static void wall_watchers_games_types(draughts_game *game, int caps)
{
 player_linked_list *current = game->watchers;
 
 while (current) /* wall to the watchers */
 {
  fvtell_player(SYSTEM_T(PLAYER_LINK_GET(current)),
		"%s",
                GAME_TYPE(PLAYER_LINK_GET(current), caps));   
  current = PLAYER_LINK_NEXT(current);
 }
}

/* tells the particular persons involved in the game their game type */
static void wall_all_involved_games_types(player *from, draughts_game *game,
                                           int caps)
{
 player_linked_list *current = game->watchers;
 int do_msg = TRUE;

 if (from)
 {
  LIST_GAME_CHECK_FLAG_START(game->white, from->saved);
  if (LIST_GAME_CHECK_FLAG_DO(draughts))
    do_msg = FALSE;
  LIST_GAME_CHECK_FLAG_END();
 }
 
 if (do_msg)
   fvtell_player(TALK_T(from->saved, game->white), "%s",
                 GAME_TYPE(game->white, caps));

 do_msg = TRUE;
 
 if (from)
 {
  LIST_GAME_CHECK_FLAG_START(game->black, from->saved);
  if (LIST_GAME_CHECK_FLAG_DO(draughts))
    do_msg = FALSE;
  LIST_GAME_CHECK_FLAG_END();
 }

 if (do_msg)
   fvtell_player(TALK_T(from->saved, game->black), "%s",
                 GAME_TYPE(game->black, caps));

 while (current) /* wall to the watchers */
 {
  do_msg = TRUE;

  if (from)
  {
   LIST_GAME_CHECK_FLAG_START(PLAYER_LINK_GET(current), from->saved);
   if (LIST_GAME_CHECK_FLAG_DO(draughts))
     do_msg = FALSE;
   LIST_GAME_CHECK_FLAG_END();
  }
  
  if (do_msg)
    fvtell_player(TALK_T(from->saved, PLAYER_LINK_GET(current)), "%s",
                  GAME_TYPE(PLAYER_LINK_GET(current), caps));
  
  current = PLAYER_LINK_NEXT(current);
 }
}

/* tells something to all the people watching the game (on the list) */
static void vwall_watchers(draughts_game *game, const char *fmt, ...)
{
 player_linked_list *current = game->watchers;

 while (current) /* wall to the watchers */
 {
  va_list ap;
  
  va_start(ap, fmt);
  
  vfvtell_player(SYSTEM_T(PLAYER_LINK_GET(current)), fmt, ap);   
 
  va_end(ap);

  current = PLAYER_LINK_NEXT(current);
 }
}

/* outputs all the stats for the game to the relevant people */
static void output_draughts_stats(player *winner, player *loser, int reason)
{
 int winners_locations = 0;
 int losers_locations = 0;
 int winners_kings = 0;
 int losers_kings = 0;
 int pieces_left = 0;
 int kings_left = 0;

 if (winner == winner->draughts_game->white)
 {
  winners_locations = winner->draughts_game->white_locations;
  losers_locations = winner->draughts_game->black_locations;
  winners_kings = winner->draughts_game->white_kings;
  losers_kings = winner->draughts_game->black_kings;
 }
 else
 {
  winners_locations = winner->draughts_game->black_locations;
  losers_locations = winner->draughts_game->white_locations;
  winners_kings = winner->draughts_game->black_kings;
  losers_kings = winner->draughts_game->white_kings;
 }
 
 vtell_all_involved_in_game(winner->draughts_game, 0,
                            "     ^S^UStats:^s\n"
                            "     %s, (black) made %d move%s/jump%s and %s, "
                            "(white) made %d move%s/jump%s.\n",
                            winner->draughts_game->black->saved->name,
                            winner->draughts_game->black_moves,
                            (winner->draughts_game->black_moves == 1 ?
                             "" : "s"),
                            (winner->draughts_game->black_moves == 1 ?
                             "" : "s"),
                            winner->draughts_game->white->saved->name,
                            winner->draughts_game->white_moves,
                            (winner->draughts_game->black_moves == 1 ?
                             "" : "s"),
                            (winner->draughts_game->white_moves == 1 ?
                             "" : "s"));

 kings_left = number_of_kings(winners_kings);

 if ((pieces_left = number_of_pieces(winners_locations)) > 1)
 {
  vtell_all_involved_in_game(winner->draughts_game, 0,
                             "     %s had %d pieces",
                             winner->saved->name, pieces_left);
  
  if (kings_left)
    vtell_all_involved_in_game(winner->draughts_game, 0,
                               " and %d king%s, at the end.\n",
                               kings_left, (kings_left == 1 ? "" : "s"));
  else
    vtell_all_involved_in_game(winner->draughts_game, 0,
                               ", none of which are kings, at the end.\n");
 }
 else if (pieces_left == 1)
 {
  vtell_all_involved_in_game(winner->draughts_game, 0,
                             "     %s had one piece left at the end",
                             winner->saved->name);

  if (kings_left)
    vtell_all_involved_in_game(winner->draughts_game, 0,
                               " which is a king.\n");
  else
    vtell_all_involved_in_game(winner->draughts_game, 0,
                               ".\n");
 }
 else
    vtell_all_involved_in_game(winner->draughts_game, 0,
                               "     %s had no pieces left at the end.\n",
                               winner->saved->name);

 if (reason & (DRAUGHTS_LOST_RESIGNED | DRAUGHTS_LOST_TIME |
               DRAUGHTS_LOST_TRAPPED))
 {
  kings_left = number_of_kings(losers_kings);
  
  if ((pieces_left = number_of_pieces(losers_locations)) > 1)
  {
   vtell_all_involved_in_game(loser->draughts_game, 0,
                              "     %s had %d pieces",
                              loser->saved->name, pieces_left);
   
   if (kings_left)
     vtell_all_involved_in_game(loser->draughts_game, 0,
                                " and %d king%s, at the end.\n",
                                kings_left, (kings_left == 1 ? "" : "s"));
   else
     vtell_all_involved_in_game(loser->draughts_game, 0,
                                ", none of which are kings, at the end.\n");
  }
  else if (pieces_left == 1)
  {
   vtell_all_involved_in_game(loser->draughts_game, 0,
                              "     %s had one piece left at the end",
                              loser->saved->name);
   
   if (kings_left)
     vtell_all_involved_in_game(loser->draughts_game, 0,
                                " which is a king.\n");
   else
     vtell_all_involved_in_game(loser->draughts_game, 0,
                                ".\n");
  }
  else
    vtell_all_involved_in_game(loser->draughts_game, 0,
                               "     %s had no pieces left at the end.\n",
                               loser->saved->name);
 }
}			     

/* outputs all the stats for the game to the relevant people */
static void output_draughts_stats_midway(player *p)
{
 int winners_locations = p->draughts_game->black_locations;
 int losers_locations = p->draughts_game->white_locations;
 int winners_kings = p->draughts_game->black_kings;
 int losers_kings = p->draughts_game->white_kings;
 int pieces_left = 0;
 int kings_left = 0;
 player *winner = p->draughts_game->black;
 player *loser = p->draughts_game->white;
 
 fvtell_player(NORMAL_T(p),
               " ^S^UStats:^s\n"
               " %s, (black) has made %d move%s/jump%s and %s, "
               "(white) has made %d move%s/jump%s.\n",
               winner->draughts_game->black->saved->name,
               winner->draughts_game->black_moves,
               (winner->draughts_game->black_moves == 1 ?
                "" : "s"),
               (winner->draughts_game->black_moves == 1 ?
                "" : "s"),
               winner->draughts_game->white->saved->name,
               winner->draughts_game->white_moves,
               (winner->draughts_game->black_moves == 1 ?
                "" : "s"),
               (winner->draughts_game->white_moves == 1 ?
                "" : "s"));
 
 kings_left = number_of_kings(winners_kings);
 
 if ((pieces_left = number_of_pieces(winners_locations)) > 1)
 {
  fvtell_player(NORMAL_T(p),
                " %s has %d pieces",
                winner->saved->name, pieces_left);
  
  if (kings_left)
    fvtell_player(NORMAL_T(p),
                  " %d of which %s king%s.\n",
                  kings_left,
                  (kings_left == 1 ? "is a" : "are"),
                  (kings_left == 1 ? "" : "s"));
  else
    fvtell_player(NORMAL_T(p), "%s", ", none of which are kings.\n");
 }
 else if (pieces_left == 1)
 {
  fvtell_player(NORMAL_T(p),
                " %s has one piece left",
                winner->saved->name);
  
  if (kings_left)
    fvtell_player(NORMAL_T(p), "%s", " which is a king.\n");
  else
    fvtell_player(NORMAL_T(p), "%s", ".\n");
 }
 else
   fvtell_player(NORMAL_T(p),
                 " %s has no pieces left.\n",
                 winner->saved->name);
 
 kings_left = number_of_kings(losers_kings);
 
 if ((pieces_left = number_of_pieces(losers_locations)) > 1)
 {
  fvtell_player(NORMAL_T(p),
                " %s has %d pieces",
                loser->saved->name, pieces_left);
  
  if (kings_left)
    fvtell_player(NORMAL_T(p),
                  " %d of which %s king%s.\n",
                  kings_left,
                  (kings_left == 1 ? "is a" : "are"),                  
                  (kings_left == 1 ? "" : "s"));
  else
    fvtell_player(NORMAL_T(p), "%s", ", none of which are kings.\n");
 }
 else if (pieces_left == 1)
 {
  fvtell_player(NORMAL_T(p),
                " %s has one piece left",
                loser->saved->name);
  
  if (kings_left)
    fvtell_player(NORMAL_T(p), "%s", " which is a king.\n");
  else
    fvtell_player(NORMAL_T(p), "%s", ".\n");
 }
 else
   fvtell_player(NORMAL_T(p),
                 " %s has no pieces left.\n",
                 loser->saved->name);
}

/* when you lose the game, tidies up - quitting = TRUE then you just quit,
   resigned = TRUE then you resigned from the game, declared = TRUE then
   you declared a win cause the other player didn't move for > MAX_MOVE mins */
void draughts_lose_game(player *p, int reason)
{
 player *winner = 0;

 assert (p);
 
 /* p is the LOSER of the game! */
 
 if (p->draughts_game)
   if (IS_PLAYER(p, p->draughts_game))
   {
    if (sys_flag.panic) /* we're crashing or shutting down */
    {
     if (p->draughts_game->flags & DRAUGHTS_FRIENDLY)
       p->draughts_played--; /* any player looses one games worth */
     return;
    }

    assert(p->draughts_game->white && p->draughts_game->black);
    
    /* who lost/withdrew/quit etc? */
    winner = get_opponent(p); /* winner is the winner */
    
    /* just check if its still a proposal */
    if (p->draughts_game->flags & DRAUGHTS_PROPOSED)
    {
     if (winner == winner->draughts_game->black)
       fvtell_player(SYSTEM_T(winner),
                     "%s -=> %s quits, therefore declining your offer of "
                     "a %s game.\n", USER_COLOUR_DRAUGHTS,
                     p->saved->name, GAME_TYPE(p, 0));
     else
       fvtell_player(SYSTEM_T(winner),
                     "%s -=> %s withdraws %s offer of a game of %s.^N\n",
                     USER_COLOUR_DRAUGHTS, p->saved->name,
                     gender_choose_str(p->gender, "he", "she", "they", "it"),
                     GAME_TYPE(p, 0));
     
     destroy_draughts_game(p->draughts_game);
     p->draughts_game = NULL;
     winner->draughts_game = NULL;
     return;
    }

    /* tell everyone the results */
    if (reason & DRAUGHTS_LOST_QUIT)
    {
     fvtell_player(SYSTEM_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, winner),
		   "%s -=> %s has quit $Talker-Name and therefore "
                   "forefeit the game! ^BYou win!^N\n",
                   USER_COLOUR_DRAUGHTS, p->saved->name);
     
     vwall_watchers(winner->draughts_game,
		    "%s -=> %s quit $Talker-Name and therefore lost the ",
		    USER_COLOUR_DRAUGHTS, p->saved->name);
     wall_watchers_games_types(winner->draughts_game, 0);
     vwall_watchers(winner->draughts_game,
		    " game!\n     %s wins the game!^N\n",
		    winner->saved->name);
    }
    else if (reason & DRAUGHTS_LOST_RESIGNED)
    {
     fvtell_player(SYSTEM_T(winner),
		   "%s -=> %s has resigned the %s game, therefore, "
		   "you win!^N\n",
		   USER_COLOUR_DRAUGHTS, p->saved->name, GAME_TYPE(winner, 0));

     fvtell_player(NORMAL_T(p),
		   "%s -=> You have resigned the %s game and therefore "
		   "%s wins.^N\n", USER_COLOUR_DRAUGHTS,
                   GAME_TYPE(p, 0), winner->saved->name);

     vwall_watchers(winner->draughts_game,
		    "%s -=> %s resigned the ",
		    USER_COLOUR_DRAUGHTS, p->saved->name);
     wall_watchers_games_types(winner->draughts_game, 0);     
     vwall_watchers(winner->draughts_game,
		    " game, therefore %s wins!^N\n", winner->saved->name);
    }
    else if (reason & DRAUGHTS_LOST_TIME)
    {
     fvtell_player(NORMAL_T(winner), "%s", /* tell the winner */
                   " -=> You call the time and win the game!\n");
     
     fvtell_player(SYSTEM_T(p), /* tell the loser */
		   "%s -=> You fail to move quick enough and %s call%s the "
		   "time and wins the game!^N\n", USER_COLOUR_DRAUGHTS,
                   winner->saved->name,
                   (winner->gender == GENDER_PLURAL) ? "" : "s");

     /* tell the watchers */
     vwall_watchers(winner->draughts_game,
		    "%s -=> %s doesn't move quick enough, %s call%s the time "
		    " and wins the ", USER_COLOUR_DRAUGHTS,
                    p->saved->name, winner->saved->name,
                    (winner->gender == GENDER_PLURAL) ? "" : "s");
     wall_watchers_games_types(winner->draughts_game, 0);
     vwall_watchers(winner->draughts_game,
		    " game!^N\n");
    }
    else if (reason & DRAUGHTS_LOST_TRAPPED)
    {
     fvtell_player(SYSTEM_T(winner),
		   "%s -=> Congratulations, you have won the game against "
		   "%s, by trapping %s pieces!^N\n",
                   USER_COLOUR_DRAUGHTS, p->saved->name,
                   gender_choose_str(p->gender, "his", "her", "their", "its"));

     fvtell_player(SYSTEM_T(p),
		   "%s -=> %s %s in a position which traps all your "
                   "remaining pieces.\n     %s %s therefore won.^N\n",
		   USER_COLOUR_DRAUGHTS, winner->saved->name,
                   (winner->gender == GENDER_PLURAL) ? "are" : "is",
                   winner->saved->name,
                   (p->gender == GENDER_PLURAL) ? "have" : "has");
     
     vwall_watchers(winner->draughts_game,
		    "%s -=> %s won the ", USER_COLOUR_DRAUGHTS,
                    winner->saved->name);
     wall_watchers_games_types(winner->draughts_game, 0);
     vwall_watchers(winner->draughts_game,
		    " game against %s, by trapping all %s pieces!^N\n",
                    p->saved->name,
                    gender_choose_str(p->gender, "his", "her",
                                      "their", "its"));
    }
    else /* assume reason & DRAUGHTS_LOST_PIECES */
    {
     fvtell_player(SYSTEM_T(winner),
		   "%s -=> Congratulations, you have won the game against "
		   "%s!^N\n", USER_COLOUR_DRAUGHTS, p->saved->name);

     fvtell_player(SYSTEM_T(p),
		   "%s -=> %s take%s your last piece and wins the game.^N\n",
		   USER_COLOUR_DRAUGHTS, winner->saved->name,
                   (winner->gender == GENDER_PLURAL) ? "" : "s");
     
     vwall_watchers(winner->draughts_game,
		    "%s -=> %s won the ", USER_COLOUR_DRAUGHTS,
                    winner->saved->name);
     wall_watchers_games_types(winner->draughts_game, 0);
     vwall_watchers(winner->draughts_game,
		    " game against %s!^N\n", p->saved->name);
    }
    
    /* sort the score */
    if (!(p->draughts_game->flags & DRAUGHTS_FRIENDLY))
    {
     fvtell_player(SYSTEM_T(winner),
		   "     You get the points for this game, "
		   "well done!\n");
     winner->draughts_won++;
    }

    if (!(reason & DRAUGHTS_LOST_QUIT))
      output_draughts_stats(winner, p, reason);
    
    /* cleanup */
    destroy_draughts_game(winner->draughts_game);
    p->draughts_game = NULL;
    winner->draughts_game = NULL;
   }
}

/* returns the column number (1 - 8) for a letter passed, 0 on bad column */
static int get_col_from_letter(char letter)
{
 int ret_val = 0;

 ret_val = (ALPHA_LOWER_OFFSET(letter) + 1);

 if (RANGE(ret_val, 1, 8))
   return (ret_val);
 else
   return (0);
}

/* gets the atoi, checks the range and returns the row - returns 0 on fail */
static int get_row_from_letter(const char *num)
{
 int row = 0;

 row = atoi(num);

 if (RANGE(row, 1, 8))
   return (9 - row);
 else
   return (0);
}

/* returns the effective column value for a column */
static int get_effective_col(int col)
{
 if (col == 1)
   return (0);
 else
   return ((int) (col / 2));
}

/* checks if the position is a valid number - returns TRUE if so, FALSE else */
static int valid_board_position(int position)
{
 if (RANGE(position, 1, 35)) /* is it in range? */
   if ((position == 9) || (position == 18) || (position == 27))
     return (FALSE); /* yes, but its an invalid one */
   else
     return (TRUE); /* yes, and its not invalid */
 else
   return (FALSE); /* not in range, therefore not valid */
}

/* returns the board position for internal working, based on row and col - if
   its not in a board place (ie:, not in positions 1 to 35), returns 0 */
static int get_draught_board_position(int col, int row)
{
 int effective_row = 0;
 int row_used = (9 - row);

 /* check if we're finding out about a possible board position */
 if (row_used % 2) /* odd row */
 {
  if (!(col % 2)) /* not an odd column */
    return (0); /* its not a valid board position */
 }
 else
   if (col % 2) /* odd column */
     return (0); /* its not a valid board position */
 
 effective_row = ((row_used * 4) + (int) (row_used / 2));

 assert(RANGE(effective_row, 4, 36));
 
 return ((effective_row - get_effective_col(col)));
}

/* returns the bit flag for the row and column, -1 if an non bit flag  */
static int get_draught_location_bit(int col, int row, int board_position)
{ 
 int board_position_found = 0;

 if (board_position)
   board_position_found = board_position;
 else
   board_position_found = get_draught_board_position(col, row);

 /* if its not a board position at all */
 if (board_position_found < 1)
   return (-1);
 
 assert(RANGE(board_position_found, 1, 35));
 
 if (RANGE(board_position_found, 1, 8))
   return ((board_position_found - 1));
 else if (RANGE(board_position_found, 10, 17))
   return ((board_position_found - 2));
 else if (RANGE(board_position_found, 19, 26))
   return ((board_position_found - 3));
 else if (RANGE(board_position_found, 28, 35))
   return ((board_position_found - 4));
 else
   return (board_position_found);
}

/* checks to see if a piece is at the row or column and returns the
   appropriate character - if !for_black then its for the white player.
   Used only in the standard board so far. colour int switches colour on */
static const char *is_piece(int actual_col, int actual_row,
                            draughts_game *game, int for_white, int colour)
{
 int row = actual_row;
 int col = actual_col;
 int bit = 0;

 if (for_white) /* flip the board if you're white */
 {
  col = (9 - actual_col);
  row = (9 - actual_row);
 }
 
 bit = get_draught_location_bit(col, row, 0);

 /* if its not possibly a piece */
 if (bit == -1)
 {
  if (colour)
    return ("^98^8_^N"); /* its no piece at all, so default board */
  else
    return ("_"); /* its no piece at all, so default board */
 }
 
 /* if its a black piece */
 if (game->black_locations & (1<<bit))
 {
  if (game->black_kings & (1<<bit)) /* if its a king */
  {
   if (colour)
     return ("^91^3^BO^N");
   else
     return ("B");
  }
  else /* its not a king */
  {
   if (colour)
     return ("^91^3^Bo^N");
   else
     return ("b");
  }
 }
 
 /* if its a white piece */
 if (game->white_locations & (1<<bit))
 {
  if (game->white_kings & (1<<bit)) /* if its a king */
  {
   if (colour)
     return ("^91^8^BO^N");
   else
     return ("W");
  } 
  else /* its not a king */
  {
   if (colour)
     return ("^91^8^Bo^N");
   else
     return ("w");
  }
 }
 
 /* its no piece at all, so default board */
 if (colour)
   return ("^91^1_^N");
 else
   return ("_");
}

/* same as above function, but used in the large board */
static const char *is_piece_large(int actual_col, int actual_row,
                                  draughts_game *game, int for_white,
                                  int colour)
{
 int row = actual_row;
 int col = actual_col;
 int bit = 0;

 if (for_white) /* flip the board if you're white */
 {
  col = (9 - actual_col);
  row = (9 - actual_row);
 }
 
 bit = get_draught_location_bit(col, row, 0);

 /* if its not possibly a piece */
 if (bit == -1)
 {
  if (colour)
    return ("^98^8   ^N"); /* its no piece at all, so default board */
  else
    return ("   "); /* its no piece at all, so default board */
 }
 
 /* if its a black piece */
 if (game->black_locations & (1<<bit))
 {
  if (game->black_kings & (1<<bit)) /* if its a king */
  {
   if (colour)
     return ("^91^3^B O ^N");
   else
     return (" B ");
  }
  else /* its not a king */
  {
   if (colour)
     return ("^91^3^B o ^N");
   else
     return (" b ");
  }
 }
 
 /* if its a white piece */
 if (game->white_locations & (1<<bit))
 {
  if (game->white_kings & (1<<bit)) /* if its a king */
  {
   if (colour)
     return ("^91^8^B O ^N");
   else
     return (" W ");
  }
  else /* its not a king */
  {
   if (colour)
     return ("^91^8^B o ^N");
   else
     return (" w ");
  }
 }
 
 /* its no piece at all, so default board */
 if (colour)
   return ("^91^1   ^N");
 else
   return ("   ");
}

/* prints a fairly large colour only board */
/* assumes they have a pointer to a game */
static void show_colour_only_draughts_board(player *p, int white, int coloured)
{
 int col = 1;
 int row = 1;
 draughts_game *game = p->draughts_game;

 assert(game);
 
 if (white)
   fvtell_player(NORMAL_T(p), "%s",
                 "     h  g  f  e  d  c  b  a\n");
 else
   fvtell_player(NORMAL_T(p), "%s",
                 "     a  b  c  d  e  f  g  h\n");
 
 for (; row < 9; row++)
 {
  fvtell_player(NORMAL_T(p),
                "  %d ", (white ? row : (9 - row)));
  
  for (col = 1; col < 9; col++)
    fvtell_player(NORMAL_T(p),
                  "%s", is_piece_large(col, row, game, white, coloured));
  
  fvtell_player(NORMAL_T(p),
                " %d\n", (white ? row : (9 - row)));
 }

 if (white)
   fvtell_player(NORMAL_T(p), "%s",
                 "     h  g  f  e  d  c  b  a\n");
 else
   fvtell_player(NORMAL_T(p), "%s",
                 "     a  b  c  d  e  f  g  h\n"); 
}

/* prints a large colour or non-colour draughts board to player p */
/* assumes they have a pointer to a game */
static void show_large_draughts_board(player *p, int white, int coloured)
{
 int col = 1;
 int row = 1;
 draughts_game *game = p->draughts_game;

 assert(game);
 
 if (white)
   fvtell_player(NORMAL_T(p), "%s",
                 "      h   g   f   e   d   c   b   a\n");
 else
   fvtell_player(NORMAL_T(p), "%s",
                 "      a   b   c   d   e   f   g   h\n");

 fvtell_player(NORMAL_T(p), "%s", LINE_OF_LARGE_DRAUGHTS_BOARD);
 
 for (; row < 9; row++)
 {
  fvtell_player(NORMAL_T(p),
                "  %d |", (white ? row : (9 - row)));
  
  for (col = 1; col < 9; col++)
    fvtell_player(NORMAL_T(p),
                  "%s|", is_piece_large(col, row, game, white, coloured));
  
  fvtell_player(NORMAL_T(p),
                " %d\n", (white ? row : (9 - row)));

  fvtell_player(NORMAL_T(p), "%s", LINE_OF_LARGE_DRAUGHTS_BOARD);
 }

 if (white)
   fvtell_player(NORMAL_T(p), "%s",
                 "      h   g   f   e   d   c   b   a\n");
 else
   fvtell_player(NORMAL_T(p), "%s",
                 "      a   b   c   d   e   f   g   h\n"); 
}

/* prints a standard colour or non-colour draughts board to player p */
/* assumes they have a pointer to a game */
static void show_standard_draughts_board(player *p, int white, int coloured)
{
 int col = 1;
 int row = 1;
 draughts_game *game = p->draughts_game;

 assert(game);
 
 if (white)
   fvtell_player(NORMAL_T(p), "%s",
                 "      h g f e d c b a\n");
 else
   fvtell_player(NORMAL_T(p), "%s",
                 "      a b c d e f g h\n");

 fvtell_player(NORMAL_T(p), "%s",
               "      _ _ _ _ _ _ _ _\n");
 
 for (; row < 9; row++)
 {
  fvtell_player(NORMAL_T(p),
               "  %d  |", (white ? row : (9 - row)));

  for (col = 1; col < 9; col++)
    fvtell_player(NORMAL_T(p),
                  "%s|", is_piece(col, row, game, white, coloured));

  fvtell_player(NORMAL_T(p),
                "  %d\n", (white ? row : (9 - row)));
 }

 if (white)
   fvtell_player(NORMAL_T(p),
                 "\n"
                 "      h g f e d c b a\n");
 else
   fvtell_player(NORMAL_T(p),
                 "\n"
                 "      a b c d e f g h\n");
}

/* sets up the initial position for all the black and white pieces */
static void setup_start_pieces(draughts_game *game)
{
 int i = 0;
 
 /* setup white (top of board) */
 for (i = 31; i > 19; i--) /* top 12 pieces */
   game->white_locations |= (1<<i);
			   
 /* setup black (bottom of board) */
 for (i = 0; i < 12; i++) /* bottom 12 pieces */
   game->black_locations |= (1<<i);
}

/* prints the users choice of board, calling the correct function. Must be
   called only when user is playing draughts or watching draughts. Calls the
   sub function with the player and an int which = 1 for white player */
static void show_draughts_board(player *p, int white, int showing)
{
 draughts_game show_game;
 draughts_game *current = NULL;
 int board_to_show = 0;
 
 assert(p);
 assert((showing) ||
        (draughts_is_playing_game(p) || draughts_is_watching_game(p)));

 if (showing) /* just doing a demo of the board */
 {
  draughts_game_init(&show_game);
 
  if (p->draughts_game)
    current = p->draughts_game;
  p->draughts_game = &show_game;
  setup_start_pieces(&show_game);
  board_to_show = (showing - 1);
 }
 else
   board_to_show = p->draughts_board;
 
 /* be sure to update NUMBER_OF_BOARDS in the .h file */
 switch (board_to_show)
 {
  case 0:
    show_standard_draughts_board(p, white, 0);
    break;
    
  case 1:
    show_standard_draughts_board(p, white, 1);
    break;

  case 2:
    show_large_draughts_board(p, white, 0);
    break;

  case 3:
    show_large_draughts_board(p, white, 1);
    break;

  case 4:
    show_colour_only_draughts_board(p, white, 1);
    break;
    
  default:
    show_standard_draughts_board(p, white, 0);
 }

 pager(p, PAGER_DEFAULT);
 
 if (showing)
   p->draughts_game = current;
}

/* walls the board to everyone who needs it, if white is TRUE then its
   coming FROM the white players perspective, else its a black player */
static void wall_board(draughts_game *game, int white)
{
 player_linked_list *current = game->watchers;
 
 assert(game);

 /* show the watchers - always see default black board */
 while (current)
 {
  if (!PLAYER_LINK_GET(current)->flag_draughts_no_auto_show_watch)
    show_draughts_board(PLAYER_LINK_GET(current), 0, 0);
  current = PLAYER_LINK_NEXT(current);
 }

 /* show the you and the oponent */
 if (white)
 { /* whites move */
  if (!game->black->flag_draughts_no_auto_show_player)
    show_draughts_board(game->black, 0, 0); /* black sees opponents move */
  
  if (!game->white->flag_draughts_no_auto_show_my_move)
    show_draughts_board(game->white, 1, 0); /* white see's his/her move */
 }
 else
 { /* blacks move */
  if (!game->black->flag_draughts_no_auto_show_my_move)
    show_draughts_board(game->black, 0, 0); /* black see's his move */
  
  if (!game->white->flag_draughts_no_auto_show_player)
    show_draughts_board(game->white, 1, 0); /* white see's his/her opponents */
 }
}
 
static void setup_draughts_game(player *p, const char *str, int friendly)
{
 player *p2 = 0;
 
 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_EXTERN & ~PLAYER_FIND_SELF)))
   return;
 
 if (p2 == p)
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You can't play yourself at the game, thats cheating!\n");
  return;
 }
 
 if (!p->draughts_game)
 {
  LIST_GAME_CHECK_FLAG_START(p2, p->saved);
  if (LIST_GAME_CHECK_FLAG_DO(draughts))
  {
   fvtell_player(NORMAL_T(p),
                 " You cannot play %s with %s as they don't want to "
                 "play.\n", GAME_TYPE(p, 0), p2->saved->name);
   return;
  }
  LIST_GAME_CHECK_FLAG_END();
  
  if (p2->draughts_game) /* they have a game */
  {
   if (p2->draughts_game->white == p) /* has the player been offered */
     /* its an offer and not an actual game already in progress? */
     if (p2->draughts_game->flags & DRAUGHTS_PROPOSED) 
     { /* its a response then */
      
      /* stop proposal and setup p's game */
      p2->draughts_game->flags &= ~DRAUGHTS_PROPOSED;
      p->draughts_game = p2->draughts_game;
      
      /* reset the friendly int correctly when its a response */
      friendly = (p->draughts_game->flags & DRAUGHTS_FRIENDLY);
      
      fvtell_player(SYSTEM_T(p2),
                    "%s -=> %s accept%s your offer of a %sgame which "
                    "will commence now.\n     You play ^S^Ublack^s "
                    "and move first.^N\n",
                    USER_COLOUR_DRAUGHTS,
                    p->saved->name,
                    (p->gender == GENDER_PLURAL) ? "" : "s",
                    (p2->draughts_game->flags &
                     DRAUGHTS_PRIVATE_BLACK ? "(private) " : ""));
      fvtell_player(NORMAL_T(p),
                    " You accept the offer of a %sgame from %s "
                    "which will commence now.\n %s play%s ^Ublack^u "
                    "and moves first.\n",
                    (p->draughts_game->flags &
                     DRAUGHTS_PRIVATE_BLACK ? "(private) " : ""),
                    p2->saved->name, p2->saved->name,
                    (p->gender == GENDER_PLURAL) ? "" : "s");
      
      /* setup the pieces in the start positions */
      setup_start_pieces(p2->draughts_game);
      
      /* setup the last mover */
      p->draughts_game->to_move = p2;
      p->draughts_game->last_moved = now;
      
      /* starting the game! */
      p->draughts_game->started_at = now;
      
      /* check for if this counts as a game */
      if (!(p->draughts_game->flags & DRAUGHTS_FRIENDLY))
      { /* count it */
       p->draughts_played++;
       p2->draughts_played++;
      }
      
      /* if its an offered private proposal */
      if (p->draughts_game->flags & DRAUGHTS_PRIVATE_BLACK)
        p->draughts_game->flags |= DRAUGHTS_PRIVATE_WHITE;
      
      /* check if the player wants private option set */
      if (p->flag_draughts_auto_private)
      {
       if (!(p->draughts_game->flags & DRAUGHTS_PRIVATE_BLACK))
       {
        fvtell_player(NORMAL_T(p),
                      " Warning: This game can be watched by any "
                      "resident. You request for it to be "
                      "private.\n");
        fvtell_player(SYSTEM_T(p2),
                      "     %s request%s that this game be a private "
                      "one.\n", p->saved->name,
                      (p->gender == GENDER_PLURAL) ? "" : "s");
        
        p->draughts_game->flags |= DRAUGHTS_PRIVATE_WHITE;
       }
      }
      
      /* check for if this game can be watched */
      if (!(p->draughts_game->flags &
            (DRAUGHTS_PRIVATE_WHITE|DRAUGHTS_PRIVATE_BLACK)))
        /* yes, so add proposer to the master list */
        player_link_add(&draughts_games_list, p2->saved, NULL,
                        PLAYER_LINK_DEFAULT, NULL);
     }
     else
     {
      /* can't get here, cause p->draughts_game already
         checked, and the user must have a game with p2 to
         get this far */
      assert(FALSE);
     }
   else
     if (IS_PLAYER(p2, p2->draughts_game))
       fvtell_player(NORMAL_T(p),
                     " You cannot offer a game of %s to %s as "
                     "they're already playing!\n", GAME_TYPE(p, 0),
                     p2->saved->name);
     else
       fvtell_player(NORMAL_T(p),
                     " You cannot play %s with %s as they are "
                     "currently watching a game, they must stop "
                     "first.\n",
                     GAME_TYPE(p, 0), p2->saved->name);
  }
  else
  {
   if ((p->draughts_game = XMALLOC(sizeof(draughts_game), DRAUGHTS)))
   { /* setup game */
    draughts_game_init(p->draughts_game);
    
    /* setup players */
    p->draughts_game->black = p;
    p->draughts_game->white = p2;
    
    /* setup flags */
    p->draughts_game->flags |= DRAUGHTS_PROPOSED;
    
    if (p->flag_draughts_auto_private)
      p->draughts_game->flags |= (DRAUGHTS_PRIVATE_BLACK);
    
    if (friendly)
      p->draughts_game->flags |= DRAUGHTS_FRIENDLY;
     
    fvtell_player(SYSTEM_T(p2),
                  "%s -=> %s offer%s you a game of %s.\n"
                  "     To accept type: "
                  "%s%s%splay %s^N\n",
                  USER_COLOUR_DRAUGHTS,
                  p->saved->name,
                  (p->gender == GENDER_PLURAL) ? "" : "s",
                  (MODE_IN_MODE(p2, DRAUGHTS) ? GAME_TYPE(p2, 0)
                   : GAME_TYPE(p, 0)),
                  (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
                  (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "),
                  (friendly ? "friendly_" : ""),
                  p->saved->lower_name);
    fvtell_player(NORMAL_T(p),
                  " You have offered %s a game of %s.\n",
                  p2->saved->name,
                  GAME_TYPE(p, 0));
    
    if (p->draughts_game->flags & DRAUGHTS_FRIENDLY)
    {
     fvtell_player(SYSTEM_T(p2),
                   "     %sThe game will be a ^S^Ufriendly^s (the "
                   "winner doesn't get a score).^N\n",
                   USER_COLOUR_DRAUGHTS);
     fvtell_player(NORMAL_T(p), "%s",
                   "     ^BThe game will be a ^Ufriendly^u (the "
                   "winner doesn't get a score).^b\n");
    }
    
    if (p->draughts_game->flags & (DRAUGHTS_PRIVATE_BLACK))
    {
     fvtell_player(NORMAL_T(p), "%s",
                   " The game will be private "
                   "(no one can watch it).\n");
     fvtell_player(SYSTEM_T(p2),
                   "     %sThe game will be private (no one can "
                   "watch it).^N\n",
                   USER_COLOUR_DRAUGHTS);
    }
   }
   else
     P_MEM_ERR(p);
  }
 }
 else
 { /* you're already playing or watching */
  if (IS_PLAYER(p, p->draughts_game))
    fvtell_player(NORMAL_T(p),
                  " You're currently playing a %s game, you must "
                  "finish this one first.\n", GAME_TYPE(p, 0));
  else
    fvtell_player(NORMAL_T(p),
                  " You're currently watching a game of %s, you must "
                  "stop this watching first.\n", GAME_TYPE(p, 0));
 }
}

static void check_game_call_start(player *p, const char *str, int friendly)
{
 if (p->draughts_game)
   if (IS_PLAYER(p, p->draughts_game))
     fvtell_player(NORMAL_T(p),
		  " You are already playing %s, you must stop "
		  "the current game first.\n", GAME_TYPE(p, 0));
   else     
     fvtell_player(NORMAL_T(p),
		  " You are currently watching a game, "
		  "first use: %s%swatch off\n",
		   (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
		   (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "));
 else
   setup_draughts_game(p, str, friendly);
}

/* walls the move to all players and watchers */
static void wall_move(draughts_game *game, char *from, char *to, int hops,
		      int pieces_taken, int white)
{
 player *p = 0;
 player *opponent = 0;
 player_linked_list *current = game->watchers;
 
 if (white)
 {
  p = game->white;
  opponent = game->black;
 }
 else
 {
  p = game->black;
  opponent = game->white;
 }
 
 /* first tell the players */
 /* tell the opponent */
 fvtell_player(SYSTEM_T(opponent),
	       "%s -=> %s%s move%s from %s to %s",
               USER_COLOUR_DRAUGHTS,
               gender_choose_str(p->gender, "", "", "The ", "The "),
	       p->saved->name, (p->gender == GENDER_PLURAL) ? "" : "s",
               from, to);
 if (hops > 1) /* then pieces_taken MUST be > 0 */
 {
  assert(pieces_taken);
  fvtell_player(SYSTEM_T(opponent),
		", using %d jumps and taking %d of your pieces.^N\n",
		hops, pieces_taken);
 }
 else if (pieces_taken)
   fvtell_player(SYSTEM_T(opponent), "%s",
                 " taking one of your pieces.^N\n");
 else
   fvtell_player(SYSTEM_T(opponent), ".^N\n");
 
 /* tell the moving player */
 fvtell_player(NORMAL_T(p),
	       " You move your piece from %s to %s", from, to);
 if (pieces_taken)
   fvtell_player(NORMAL_T(p), ", taking %d enemy piece%s.\n", pieces_taken,
		 (pieces_taken > 1 ? "s" : ""));
 else
   fvtell_player(NORMAL_T(p), ".\n");

 /* now tell the watchers of the game */
 while (current)
 {
  fvtell_player(SYSTEM_T(PLAYER_LINK_GET(current)),
		"%s -=> %s%s move%s from %s to %s",
                USER_COLOUR_DRAUGHTS,
                gender_choose_str(p->gender, "", "", "The ", "The "),
                p->saved->name, (p->gender == GENDER_PLURAL) ? "" : "s",
                from, to);
  if (pieces_taken)
   fvtell_player(SYSTEM_T(PLAYER_LINK_GET(current)),
		 ", taking %d of %s%s%s pieces.%s\n", pieces_taken,
		 gender_choose_str(opponent->gender, "", "", "the ", "the "),
                 opponent->saved->name,
                 (opponent->gender == GENDER_PLURAL) ? "" : "'s", "^N");
  else
    fvtell_player(SYSTEM_T(PLAYER_LINK_GET(current)), ".^N\n");

  current = PLAYER_LINK_NEXT(current);
 }
}

/* checks that the distance is a valid distance, sets the taking bit if its
   only valid for taking, and sets the position being taken, also takes
   account of kings - returns TRUE if valid, FALSE otherwise */
static int valid_distances(int board_from, int board_to, int *taking,
			   int *board_taken, int kings, int bit_of_from,
			   int white)
{
 int size = 0;
 int forward = 1;

 assert(board_to - board_from); /* can never be the same square, pre checked */
 /* HAS to be valid piece */
 assert(RANGE(bit_of_from, (1<<0), (1<<30)) || (bit_of_from == (1<<31)));
	
 /* setup defaults */
 *taking = 0;
 *board_taken = 0;

 if (white)
 {
  /* if its forwards, must be a king */
  if ((size = (board_to - board_from)) > 0) 
    if (!(kings & bit_of_from)) /* is it? */
      return (FALSE); /* forwards move, without a king */
 }
 else
 {
  /* if its backwards, must be a king */
  if ((size = (board_to - board_from)) < 0) 
    if (!(kings & bit_of_from)) /* is it? */
      return (FALSE); /* backwards move, without a king */
 }
 
 if ((size < 0) && !(white))
   forward = 0;

 if ((size > 0) && (white))
   forward = 0;
 
 size = abs(size);

 if (size % 5) /* its not going that way, by any */
   if (size % 4) /* its not going that way by any */
     return (FALSE); /* bad move */
   else /* its going right, or left backwards */
     if (size > 4) /* its trying to take */
       if (size > 8) /* its going to far */
	 return (FALSE);
       else /* valid take, valid move */
       {
	*taking = 1;
	if (forward)
	  if (white)
	    *board_taken = (board_from - 4);
	  else
	    *board_taken = (board_from + 4);
	else
	  if (white)
	    *board_taken = (board_from + 4);
	  else
	    *board_taken = (board_from - 4);
	return (TRUE);
       }
     else /* just moving one square */
       return (TRUE); /* valid move */
 else /* its going left */
   if (size > 5) /* its trying to take */
     if (size > 10) /* its going to far */
       return (FALSE);
     else /* valid take, valid move */
     {
      *taking = 1;
      if (forward)
	if (white)
	  *board_taken = (board_from - 5);
	else
	  *board_taken = (board_from + 5);
      else
	if (white)
	  *board_taken = (board_from + 5);
	else
	  *board_taken = (board_from - 5);
      return (TRUE);
     }
   else /* just moving one square */
     return (TRUE); /* valid move */
}

/* checks there's a piece at the board position, and inits the bit for it -
   returns TRUE if there's a piece there, false otherwise */
static int piece_at_position(int pieces, int position, int *bit_of_piece)
{
 int bit = 0;

 if ((bit = get_draught_location_bit(0, 0, position)) == position)
   assert(0); /* should be a valid position so bit should be different! */

 *bit_of_piece = (1<<bit);

 if (pieces & (1<<bit))
   return (TRUE);
 else
   return (FALSE);
}

/* checks that you're ACTUALLY taking a piece from the enemy now you've tried
   to take a piece, just checks the enemy_pieces for the position,
   returns TRUE if there's one there or FALSE on fail - alias for simplicity */
static int check_if_taking(int pieces, int position)
{
 int dummy = 0;

 return (piece_at_position(pieces, position, &dummy));
}

/* finds out if all pieces are trapped - returns the player who's pieces
   are trapped if it finds them to be all trapped */
static int trapped_pieces(int enemy_pieces, int pieces, int kings,
                          int white_just_moved)
{
 /* sets up if its white to move next or not ? */
 int white = !(white_just_moved);
 int row = 1;
 int col = 1;
 int position = 0;
 int temp_position = 0;
 int bit = 0;
 int dummy = 0;
 int forward_five = 5;
 int forward_four = 4;
 int backward_five = -5;
 int backward_four = -4;

 if (white) /* silly things to do if we're white moving next */
 {
  forward_five = -5;
  forward_four = -4;
  backward_five = 5;
  backward_four = 4;
 }
 
 for (; row < 9; row++)
   for (col = 1; col < 9; col ++) /* check if valid position */
     if ((position = get_draught_board_position(col, row))) 
       if (piece_at_position(pieces, position, &bit)) /* check for piece */
       { /* player has a piece at this square */

        /* first check normal single moves */
        
        temp_position = (position + forward_five); /* check the forward_five */
        if (valid_board_position(temp_position))
          if ((!(piece_at_position(pieces, temp_position, &dummy))) &&
              (!(piece_at_position(enemy_pieces, temp_position, &dummy))))
            return (FALSE);
        
        temp_position = (position + forward_four); /* check the forward_four */
        if (valid_board_position(temp_position))
          if ((!(piece_at_position(pieces, temp_position, &dummy))) &&
              (!(piece_at_position(enemy_pieces, temp_position, &dummy))))
            return (FALSE);

        if (kings & bit) /* if its a king, check backwards */
        {
         temp_position = (position + backward_five); /* check backward_five */
         if (valid_board_position(temp_position))
           if ((!(piece_at_position(pieces, temp_position, &dummy))) &&
               (!(piece_at_position(enemy_pieces, temp_position, &dummy))))
             return (FALSE);
         
         temp_position = (position + backward_four); /* check backward_four */
         if (valid_board_position(temp_position))
           if ((!(piece_at_position(pieces, temp_position, &dummy))) &&
               (!(piece_at_position(enemy_pieces, temp_position, &dummy))))
             return (FALSE);
        }

        /* so far there's no normal left or right move, so check jumps */

        temp_position = (position + forward_five); /* check the forward_five */
        if (valid_board_position(temp_position))
          if (valid_board_position(temp_position + forward_five))
            if ((piece_at_position(enemy_pieces, temp_position, &dummy)) &&
                (!(piece_at_position(pieces, (temp_position + forward_five),
                                     &dummy))) &&
                (!(piece_at_position(enemy_pieces,
                                     (temp_position + forward_five), &dummy))))
              return (FALSE);
        
        temp_position = (position + forward_four); /* check the forward_four */
        if (valid_board_position(temp_position))
          if (valid_board_position(temp_position + forward_four))
            if ((piece_at_position(enemy_pieces, temp_position, &dummy)) &&
                (!(piece_at_position(pieces, (temp_position + forward_four),
                                     &dummy))) &&
                (!(piece_at_position(enemy_pieces,
                                     (temp_position + forward_four), &dummy))))
              return (FALSE);
        
        if (kings & bit) /* again if its a king, check backwards */
        {
         temp_position = (position + backward_five); /* check backward_five */
         if (valid_board_position(temp_position))
           if (valid_board_position(temp_position + backward_five))
             if ((piece_at_position(enemy_pieces, temp_position, &dummy)) &&
                 (!(piece_at_position(pieces, (temp_position + backward_five),
                                      &dummy))) &&
                 (!(piece_at_position(enemy_pieces,
                                      (temp_position + backward_five),
                                      &dummy))))
               return (FALSE);
         
         temp_position = (position + backward_four); /* check backward_four */
         if (valid_board_position(temp_position))
           if (valid_board_position(temp_position + backward_four))
             if ((piece_at_position(enemy_pieces, temp_position, &dummy)) &&
                 (!(piece_at_position(pieces, (temp_position + backward_four),
                                      &dummy))) &&
                 (!(piece_at_position(enemy_pieces,
                                      (temp_position + backward_four),
                                      &dummy))))
               return (FALSE);
        }
       }
 
 /* else we've got to here and not returned - therefore we're trapped */
 return (TRUE);
}

/* find out if its posiible to take from the current position, returns TRUE
 if you can, or FALSE otherwise */
static int can_i_take_from(int board_from, int bit_from, int pieces,
                           int enemy_pieces, int kings, int white)
{
 int forward_five = 5;
 int forward_four = 4;
 int backward_five = -5;
 int backward_four = -4;
 int temp_position = 0;
 int dummy = 0;

 if (white) /* things to do if we're white */
 {
  forward_five = -5;
  forward_four = -4;
  backward_five = 5;
  backward_four = 4;
 }

 temp_position = (board_from + forward_five); /* check forward five */

 if (valid_board_position(temp_position + forward_five)) /* valid to jump to */
   if ((!(piece_at_position(pieces, (temp_position + forward_five),
                                     &dummy))) &&
       (!(piece_at_position(enemy_pieces, (temp_position + forward_five),
                                           &dummy)))) /* no pieces at dest */
     if (valid_board_position(temp_position)) 
       if (piece_at_position(enemy_pieces, temp_position, &dummy)) 
         return (TRUE); /* theres a piece to take */
 
 temp_position = (board_from + forward_four); /* check forward four */

 if (valid_board_position(temp_position + forward_four)) /* valid to jump to */
   if ((!(piece_at_position(pieces, (temp_position + forward_four),
                                     &dummy))) &&
       (!(piece_at_position(enemy_pieces, (temp_position + forward_four),
                                           &dummy)))) /* no pieces at dest */
     if (valid_board_position(temp_position)) 
       if (piece_at_position(enemy_pieces, temp_position, &dummy)) 
         return (TRUE); /* theres a piece to take */

 if (kings & bit_from) /* its a queen, so check backwards too */
 {
  temp_position = (board_from + backward_five); /* check forward five */
  
  if (valid_board_position(temp_position + backward_five)) /* valid jump to */
    if ((!(piece_at_position(pieces, (temp_position + backward_five),
                             &dummy))) &&
        (!(piece_at_position(enemy_pieces, (temp_position + backward_five),
                             &dummy)))) /* no pieces at dest */
      if (valid_board_position(temp_position)) 
        if (piece_at_position(enemy_pieces, temp_position, &dummy)) 
          return (TRUE); /* theres a piece to take */
  
  temp_position = (board_from + backward_four); /* check forward four */
  
  if (valid_board_position(temp_position + backward_four)) /* valid jump to */
    if ((!(piece_at_position(pieces, (temp_position + backward_four),
                             &dummy))) &&
        (!(piece_at_position(enemy_pieces, (temp_position + backward_four),
                             &dummy)))) /* no pieces at dest */
      if (valid_board_position(temp_position)) 
        if (piece_at_position(enemy_pieces, temp_position, &dummy)) 
          return (TRUE); /* theres a piece to take */
 }
 
 /* else we've got to here and not returned - therefore we can't take */
 return (FALSE);
}

/* find out if you CAN take an enemy piece ANYWHERE on the board, returns
   TRUE if you can, FALSE otherwise */
static int can_i_take(int pieces, int enemy_pieces, int kings, int white)
{
 /* step through each of my pieces and see if there's a) an enemy piece at
    +5 or +4 and there's a space at +10 or + 8 */

 int row = 1;
 int col = 1;
 int position = 0;
 int temp_position = 0;
 int bit = 0;
 int dummy = 0;
 int forward_five = 5;
 int forward_four = 4;
 int backward_five = -5;
 int backward_four = -4;

 if (white) /* silly things to do if we're white */
 {
  forward_five = -5;
  forward_four = -4;
  backward_five = 5;
  backward_four = 4;
 }
 
 for (; row < 9; row++)
   for (col = 1; col < 9; col ++) /* check if valid position */
     if ((position = get_draught_board_position(col, row))) 
       if (piece_at_position(pieces, position, &bit)) /* check for piece */
       {
        temp_position = (position + forward_five); /* check the forward_five */
        if (valid_board_position(temp_position))
          /* check the forward_ten */
          if (valid_board_position(temp_position + forward_five)) 
          { /* if there's an enemy piece to be taken and nothing at +/-10 */
           if (piece_at_position(enemy_pieces, temp_position, &dummy))
             if (!(piece_at_position(pieces, (temp_position + forward_five),
                                     &dummy)) &&
                 !(piece_at_position(enemy_pieces, 
                                     (temp_position + forward_five), &dummy)))
               return (TRUE);
          }
        
        temp_position = (position + forward_four); /* check the forward_four */
        if (valid_board_position(temp_position))
          /* check the forward_eight */
          if (valid_board_position(temp_position + forward_four)) 
          { /* if there's an enemy piece to be taken and nothing at +/-8 */
           if (piece_at_position(enemy_pieces, temp_position, &dummy))
             if (!(piece_at_position(pieces, (temp_position + forward_four),
                                     &dummy)) &&
                 !(piece_at_position(enemy_pieces, 
                                     (temp_position + forward_four), &dummy)))
               return (TRUE);
          }
        
        if (kings & (bit)) /* if its a king, we have to check back too */
        {
         /* check the backward_five */
         temp_position = (position + backward_five); 
         if (valid_board_position(temp_position))
           /* check the backward_five */
           if (valid_board_position(temp_position + backward_five)) 
           { /* if there's an enemy piece to be taken and nothing at -/+10 */
            if (piece_at_position(enemy_pieces, temp_position, &dummy))
              if (!(piece_at_position(pieces, (temp_position + backward_five),
                                      &dummy)) &&
                  !(piece_at_position(enemy_pieces, 
                                      (temp_position + backward_five),
                                      &dummy)))
                return (TRUE);
           }
        }

        if (kings & (bit)) /* if its a king, we have to check back too */
        {
         /* check the backward_four */
         temp_position = (position + backward_four); 
         if (valid_board_position(temp_position))
           /* check the backward_eight */
           if (valid_board_position(temp_position + backward_four)) 
           { /* if there's an enemy piece to be taken and nothing at -/+8 */
            if (piece_at_position(enemy_pieces, temp_position, &dummy))
              if (!(piece_at_position(pieces, (temp_position + backward_four),
                                      &dummy)) &&
                  !(piece_at_position(enemy_pieces, 
                                      (temp_position + backward_four),
                                      &dummy)))
                return (TRUE);
           }
        }
       }

 /* else we've got to here and not returned - therefore we can't take */
 return (FALSE); 
}

/* checks the coor description of pos, then initialises the col and row values
   to their numerical equivalent, returns TRUE on success, FALSE otherwise */
static int check_position_describer(const char *pos, int *col, int *row)
{
 assert(pos && *pos);
 
 if (strnlen(pos, 3) == 2)
   if (isalpha((unsigned char) *pos))
     if (isdigit((unsigned char) *(pos + 1)))
       if ((*col = get_col_from_letter(*pos)))
	 if ((*row = get_row_from_letter((pos + 1))))
           return (TRUE); /* if we're here, then things are valid */

 /* else, its not a valid describer */
 *col = 0;
 *row = 0;
 return (FALSE);
}

/* sets up the new flags in the required ints - checks for kings */
static void setup_new_state(int *pieces, int *enemy_pieces, int *kings,
			    int *enemy_kings, int taking, int bit_from,
			    int bit_to, int board_taken, int white,
                            int *crowned)
{
 int bit_taken = 0;

 *crowned = 0; /* reset whether we've been crowned or not */
 
 *pieces &= ~bit_from; /* move the piece */
 *pieces |= bit_to;
 if (*kings & bit_from) /* move the king */
 {
  *kings &= ~bit_from;
  *kings |= bit_to;
 }

 if (taking) /* if we're removing an enemy piece */
 {
  assert(board_taken);
  bit_taken = get_draught_location_bit(0, 0, board_taken);
  *enemy_pieces &= ~(1<<bit_taken);
  if (*enemy_kings & (1<<bit_taken)) /* remove it if its a king */
    *enemy_kings &= ~(1<<bit_taken);
 }

 /* check if the new piece gets crowned king */
 if (white)
   switch (bit_to)
   {
    case (1<<0):
    case (1<<1):
    case (1<<2):
    case (1<<3):
      *kings |= bit_to;
      *crowned = 1;
    default:
      break;
   }
 else
   switch (bit_to)
   {
    case (1<<31):
    case (1<<30):
    case (1<<29):
    case (1<<28):
      *kings |= bit_to;
      *crowned = 1;
    default:
      break;
   }   
}

/* sets up the final state in the actualy game after a move has been
   completed succesfully */
static void make_final_state(draughts_game *game, int white, int pieces,
			     int enemy_pieces, int kings, int enemy_kings,
			     int number_of_hops)
{
 if (white) /* update whites information, black is enemy */
 {
  game->white_locations = pieces;
  game->black_locations = enemy_pieces;
  game->white_kings = kings;
  game->black_kings = enemy_kings;
  game->white_moves += number_of_hops;
  game->to_move = game->black;  
 }
 else /* update blacks information, white is enemy */
 {
  game->white_locations = enemy_pieces;
  game->black_locations = pieces;
  game->white_kings = enemy_kings;
  game->black_kings = kings;
  game->black_moves += number_of_hops;
  game->to_move = game->white;
 }

 /* regardless of player... */
 game->last_moved = now;
}

/* commands */

/* for testing only - sets up a boards pieces - must be playing a game
   - no checking!!! */
static void user_draughts_test_setup(player *p, const char *str)
{
#ifdef DRAUGHTS_DEBUG
 char *black = str;
 char *white;
 char *wking;
 char *bking;
 int newint = 0;
 
 if (*str)
 {
  if ((white = next_parameter_no_seperators(black, ' ')))
  {
   newint = atoi(black);
   p->draughts_game->black_locations = newint;
   if ((bking = next_parameter_no_seperators(white, ' ')))
   {
    newint = atoi(white);
    p->draughts_game->white_locations = newint;
    if ((wking = next_parameter_no_seperators(bking, ' ')))
    {
     newint = atoi(bking);
     p->draughts_game->black_kings = newint;

     newint = atoi(wking);
     p->draughts_game->white_kings = newint;
    }
    else
      TELL_FORMAT(p, "<black> <white> <bkings> <wkings>");
   }
   else
     TELL_FORMAT(p, "<black> <white> <bkings> <wkings>");
  }
  else
    TELL_FORMAT(p, "<black> <white> <bkings> <wkings>");
 }
 else
   TELL_FORMAT(p, "<black> <white> <bkings> <wkings>");
#else
 IGNORE_PARAMETER(p && str);
 fvtell_player(NORMAL_T(p), "%s",
               " This command is not in effect.\n");
#endif
}

static void user_draughts_stats(player *p)
{
 if (draughts_is_playing_game(p) || draughts_is_watching_game(p))
   output_draughts_stats_midway(p);
 else
   fvtell_player(NORMAL_T(p),
                 " You must be watching a game of %s to get the stats for "
                 "it.\n", GAME_TYPE(p, 0));
}

static void user_draughts_auto_private_game(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_draughts_auto_private, TRUE,
                       " You will %sautomatically start or request a private "
                       "$R-Nationality(us(checkers) def(draughts)) game.\n",
                       " You will ^S^Unot^s %sautomatically start or request "
                       "a private $R-Nationality(us(checkers) def(draughts)) game.\n", 
                       TRUE);
}

static void user_draughts_chat(player *p, const char *str)
{
 if (draughts_is_playing_game(p) || draughts_is_watching_game(p))
   if (*str)
   {
    vtell_all_involved_in_game(p->draughts_game, p,
                               "%s[", USER_COLOUR_DRAUGHTS);
    wall_all_involved_games_types(p, p->draughts_game, 0);
    vtell_all_involved_in_game(p->draughts_game, p,
                               "] %s chat%s '%s%s'.^N\n",
                               p->saved->name,
                               (p->gender == GENDER_PLURAL) ? "" : "s", str,
                               USER_COLOUR_DRAUGHTS);

    fvtell_player(NORMAL_T(p),
                  "%s[%s] %s chat%s '%s%s'.%s\n",
                  USER_COLOUR_MINE, GAME_TYPE(p, 0), p->saved->name,
                  (p->gender == GENDER_PLURAL) ? "" : "s", str,
                  USER_COLOUR_MINE, "^N");
   }
   else
     fvtell_player(SYSTEM_FT(SYSTEM_INFO, p),
                   " Format: %s%s%s <message>\n",
                   (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
                   (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "),
                   current_sub_command);
 else
   fvtell_player(NORMAL_T(p),
                 " You must be playing or watching a game of %s to chat to "
                 "the people of a game.\n", GAME_TYPE(p, 0));
}

static void user_draughts_emote_chat(player *p, const char *str)
{
 if (draughts_is_playing_game(p) || draughts_is_watching_game(p))
   if (*str)
   {
    vtell_all_involved_in_game(p->draughts_game, p,
                               "%s[", USER_COLOUR_DRAUGHTS);
    wall_all_involved_games_types(p, p->draughts_game, 0);
    vtell_all_involved_in_game(p->draughts_game, p,
                               "] %s%s%s^N\n",
                               p->saved->name, isits1(str), isits2(str));

    fvtell_player(NORMAL_T(p),
                  "%s[%s] %s%s%s^N\n",
                  USER_COLOUR_MINE, GAME_TYPE(p, 0), p->saved->name,
                  isits1(str), isits2(str));
   }
   else
     fvtell_player(SYSTEM_FT(SYSTEM_INFO, p),
                   " Format: %s%s%s <message>\n",
                   (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
                   (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "),
                   current_sub_command);
 else
   fvtell_player(NORMAL_T(p),
                 " You must be playing or watching a game of %s to chat to "
                 "the people of a game.\n", GAME_TYPE(p, 0));
}

static void user_draughts_watching_check(player *p)
{
 player_linked_list *current = 0;
 
 if (draughts_is_playing_game(p) || draughts_is_watching_game(p))
   if ((current = p->draughts_game->watchers))
   {
    fvtell_player(NORMAL_T(p),
                  " People watching this %s game are:\n   ", GAME_TYPE(p, 0));
    while (current)
    {
     fvtell_player(SYSTEM_T(p), /* names should go from system (sorta) */
                   "%s", PLAYER_LINK_GET(current)->saved->name);
     if ((current = PLAYER_LINK_NEXT(current)))
       fvtell_player(NORMAL_T(p), "%s", ", ");
    }

    fvtell_player(NORMAL_T(p), "%s", "\n");
   }
   else
     fvtell_player(NORMAL_T(p),
                   " No one is watching this %s game.\n", GAME_TYPE(p, 0));
 else
   fvtell_player(NORMAL_T(p),
                 " You must be watching a %s game to see who's "
                 "watching it.\n", GAME_TYPE(p, 0));
}

/* declares a win if the other player hasn't moved for over 20 mins and its
   their move to be made - they may have lagged out or just being nasty! */
static void user_draughts_decl_win(player *p)
{
 player *enemy = 0;
 time_t last_move = 0;
 
 if (draughts_is_playing_game(p))
   if (p->draughts_game->to_move != p)
   {
    char buf[256];
    
    last_move = p->draughts_game->last_moved;
    enemy = get_opponent(p);
    if (difftime(now, last_move) > MAX_MOVE) /* enemy looses, declared time */
      draughts_lose_game(enemy, DRAUGHTS_LOST_TIME); 
    else
      fvtell_player(NORMAL_T(p),
		    " %s still has %s left to make %s move.\n",
		    enemy->saved->name,
		    word_time_long(buf, sizeof(buf),
                                   (MAX_MOVE - difftime(now, last_move)),
                                   (WORD_TIME_MINUTES | WORD_TIME_SECONDS)),
                    gender_choose_str(enemy->gender, "his", "her",
                                      "their", "its"));
   }
   else
     fvtell_player(NORMAL_T(p), "%s",
                   " Its your turn to move!\n");
 else
   fvtell_player(NORMAL_T(p), " You have to be playing a %s game to delcare "
		 "a win!\n", GAME_TYPE(p, 0));
}

/* offers or accepts an offer of a game */
static void user_draughts_play(player *p, const char *str)
{
 if (!*str)
   fvtell_player (SYSTEM_FT(SYSTEM_INFO, p),
                  " Format: %s%s%s <player_to_play>\n",
		  (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
		  (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "),
                  current_sub_command);
 else
   /* start the game if its okay - not friendly */
   check_game_call_start(p, str, 0);
}

static void user_draughts_quit(player *p)
{
 if (p->draughts_game)
   if (IS_PLAYER(p, p->draughts_game)) /* resign the game - p quits */
    draughts_lose_game(p, DRAUGHTS_LOST_RESIGNED); 
   else
     fvtell_player(NORMAL_T(p),
		   " You cannot quit a game you are watching.\n "
		   "To stop watching the game, use: %s%swatch off\n",
		   (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
		   (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "));
 else
   fvtell_player(NORMAL_T(p),
		 " You cannot quit a %s game when you're not playing one!\n",
		 GAME_TYPE(p, 0));      
}

static void user_move_draughts(player *p, parameter_holder *params)
{
 char wordth[10];
 int bit_from = 0;
 int bit_to = 0;
 int board_from = 0;
 int board_to = 0;
 int from_row = 0;
 int from_col = 0;
 int white = 0;
 int pieces = 0;
 int kings = 0;
 int enemy_pieces = 0;
 int enemy_kings = 0;
 int board_taken = 0;
 int number_of_hops = 1;
 int successful_move = 0;
 int pieces_taken = 0;
 unsigned int count = 0;
 
 if (!draughts_is_playing_game(p))
 {
  fvtell_player(NORMAL_T(p),
		" You cannot move anything, as you're not playing %s.\n",
		GAME_TYPE(p, 0));
  return;
 }
 
 if (p->draughts_game->to_move != p)
 {
  fvtell_player(NORMAL_T(p), "%s", " Its not your turn to move!\n");
  return;
 }

 /* setup WHO we're dealing with */
 if (p->draughts_game->white == p)
 {
  white = 1; /* used for quickly checking status */
  pieces = p->draughts_game->white_locations;
  kings = p->draughts_game->white_kings;
  enemy_pieces = p->draughts_game->black_locations;
  enemy_kings = p->draughts_game->black_kings;
 }
 else
 {
  pieces = p->draughts_game->black_locations;
  kings = p->draughts_game->black_kings;
  enemy_pieces = p->draughts_game->white_locations;
  enemy_kings = p->draughts_game->white_kings;
 }

 if (params->last_param < 2)
   fvtell_player (SYSTEM_FT(SYSTEM_INFO, p), " Format: %s%s%s <from> <to> "
                  "[<to> [<to> [<to>]]]\n",
		  (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
		  (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "),
                  current_sub_command);
 
 if (!check_position_describer(GET_PARAMETER_STR(params, 1),
                               &from_col, &from_row))
 {
  fvtell_player(NORMAL_T(p),
                " The board position -- ^S^B%s^s -- is invalid.\n",
                GET_PARAMETER_STR(params, 1));
  return;
 }
 
 if (!(board_from = get_draught_board_position(from_col, from_row)))
 {
  fvtell_player(NORMAL_T(p),
                " You cannot move from board position '^B%s^N'.\n",
                GET_PARAMETER_STR(params, 1));
  return;
 }
   
 if (!piece_at_position(pieces, board_from, &bit_from))
 {
  fvtell_player(NORMAL_T(p),
                " You do not have a piece at board position '^S^B%s^s'.\n",
                GET_PARAMETER_STR(params, 1));
  return;
 }

 count = 1;
 while (count < params->last_param)
 {
  int to_row = 0;
  int to_col = 0;
  int taking = 0;
  int crowned = 0;
  
  ++count;
  
  if (!check_position_describer(GET_PARAMETER_STR(params, count),
                                &to_col, &to_row))
  {
   fvtell_player(NORMAL_T(p),
                 " The board position -- ^S^B%s^s -- is invalid.\n",
                 GET_PARAMETER_STR(params, count));
   break;
  }
  
  if (!strcasecmp(GET_PARAMETER_STR(params, count - 1),
                  GET_PARAMETER_STR(params, count)))
  {
   fvtell_player(NORMAL_T(p),
                 " The move -- ^S^B%s^s to ^S^B%s^s -- is invalid as "
                 "it moves you nowhere!\n",
                 GET_PARAMETER_STR(params, count - 1),
                 GET_PARAMETER_STR(params, count));
   break;
  }
  
  if (!(board_to = get_draught_board_position(to_col, to_row)))
  {
   fvtell_player(NORMAL_T(p),
                 " The move -- ^S^B%s^s to ^S^B%s^s -- is an "
                 "invalid one.\n",
                 GET_PARAMETER_STR(params, count - 1),
                 GET_PARAMETER_STR(params, count));
   break;
  }
  
  if (piece_at_position(pieces, board_to, &bit_to))
  {
   fvtell_player(NORMAL_T(p),
                 " You have a piece at board position '^S^B%s^s', so "
                 "you can't move there.\n", GET_PARAMETER_STR(params, count));
   break;
  }         
  
  if (piece_at_position(enemy_pieces, board_to, &bit_to))
  {
   fvtell_player(NORMAL_T(p),
                 " %s %s a piece at board position '^S^B%s^s', so "
                 "you can't move there.\n",
                 (GET_ENEMY_POINTER(p, p->draughts_game)->saved->name),
                 (GET_ENEMY_POINTER(p, p->draughts_game)->gender ==
                  GENDER_PLURAL) ? "have" : "has",
                 GET_PARAMETER_STR(params, count));
   break;
  }
  
  if (!valid_distances(board_from, board_to, &taking,
                       &board_taken, kings, bit_from, white))
  {
   fvtell_player(NORMAL_T(p),
                 " The move -- ^S^B%s^s to ^S^B%s^s -- is an "
                 "invalid one.\n",
                 GET_PARAMETER_STR(params, count - 1),
                 GET_PARAMETER_STR(params, count));	  
   break;
  }
  
  if (taking)
  {
   if (!(check_if_taking(enemy_pieces, board_taken)))
   {
    fvtell_player(NORMAL_T(p),
                  " The move -- ^S^B%s^s to ^S^B%s^s -- is an "
                  "invalid one.\n",
                  GET_PARAMETER_STR(params, count - 1),
                  GET_PARAMETER_STR(params, count));	  
    break;
   }
  }
  else if (number_of_hops > 1)
  { /* not taking, and second hop */
   word_number_th(wordth, 10, number_of_hops, FALSE);
   fvtell_player(NORMAL_T(p),
                 " You cannot move a ^S^B%s^s jump unless it is "
                 "to take an enemy piece.\n", wordth);
   break;
  }
  else /* not taking - so check if I can */
    if (pieces_taken)
    {
     if (can_i_take_from(board_to, bit_to, pieces, enemy_pieces,
                         kings, white))
     { /* so now check if I can take a piece from here ? */
      char buf[128];
      
      fvtell_player(NORMAL_T(p), "%s",
                    " It is possible to take an enemy piece");
      if ((number_of_hops > 1) || pieces_taken)
        fvtell_player(NORMAL_T(p), " after jump %s",
                      word_number_base(buf, 128, NULL, number_of_hops,
                                       FALSE, word_number_def));
      
      fvtell_player(NORMAL_T(p), "%s",
                    ", therefore you must.\n");
      break;
     }
    }
    else if (can_i_take(pieces, enemy_pieces, kings, white))
    {
     char buf[128];
     
     fvtell_player(NORMAL_T(p), "%s",
                   " It's possible to take an enemy piece");
     if ((number_of_hops > 1) || pieces_taken)
       fvtell_player(NORMAL_T(p), " after jump %s",
                     word_number_base(buf, 128, NULL, number_of_hops,
                                      FALSE, word_number_def));
     
     fvtell_player(NORMAL_T(p), "%s",
                   ", therefore you must.\n");
     break;
    }
  
  /* if we're here, everything is okay, therefore I setup the new
     to, the new state and so on */
  setup_new_state(&pieces, &enemy_pieces, &kings, &enemy_kings,
                  taking, bit_from, bit_to, board_taken, white,
                   &crowned);
  
  if (taking) /* if we've taken a piece */
    pieces_taken++;
  
  if ((count < params->last_param) && (number_of_hops < 4) && !crowned)
  {
   board_from = board_to; /* make the old to, the new from */
   bit_from = bit_to;
   from_col = to_col;
   from_row = to_row;
   
   board_to = 0; /* none of the next five really needed */
   bit_to = 0;
   
   number_of_hops++; /* made more than one, so increment */
  } /* we're at the end of the move if this else is checked */
  else if (pieces_taken && (number_of_hops < 4) && !crowned)
  { /* we've taken_one_piece, and made less than the max hops */
   if (can_i_take_from(board_to, bit_to, pieces, enemy_pieces,
                       kings, white))
   { /* so now check if I can take a piece from here ? */
    char buf[128];
    
    fvtell_player(NORMAL_T(p), "%s",
                  " It is possible to take an enemy piece");
    if ((number_of_hops > 1) || pieces_taken)
      fvtell_player(NORMAL_T(p), " after jump %s",
                    word_number_base(buf, 128, NULL, number_of_hops,
                                     FALSE, word_number_def));
    
    fvtell_player(NORMAL_T(p), "%s", ", therefore you must.\n");
    break;
   }
   else /* moves have finished, and its a good move, so do it */
   { /* in this call, I've actually taken a piece */
    successful_move = 1;
    make_final_state(p->draughts_game, white, pieces, enemy_pieces,
                     kings, enemy_kings, number_of_hops);
   }
  }             
  else /* moves have finished, and its a good move, so do it */
  { /* in this call, no pieces have been taken */
   successful_move = 1;
   make_final_state(p->draughts_game, white, pieces, enemy_pieces,
                    kings, enemy_kings, number_of_hops);
  }
 }

 if (!successful_move)
   return;
 
 if (number_of_pieces(enemy_pieces) < 1)
 { /* this move means the last piece has been taken */
  wall_move(p->draughts_game, GET_PARAMETER_STR(params, 1),
            GET_PARAMETER_STR(params, count - 1), number_of_hops,
            pieces_taken, white);
  draughts_lose_game(GET_ENEMY_POINTER(p, p->draughts_game),
                     DRAUGHTS_LOST_PIECES);
 }
 else if (trapped_pieces(pieces, enemy_pieces, enemy_kings, white))
 { /* this move means that player trapped all enemies pieces */
  wall_move(p->draughts_game, GET_PARAMETER_STR(params, 1),
            GET_PARAMETER_STR(params, count - 1), number_of_hops,
            pieces_taken, white);
  draughts_lose_game(GET_ENEMY_POINTER(p, p->draughts_game),
                     DRAUGHTS_LOST_TRAPPED);
 } 
 else
 { /* inform players of the move */
  wall_move(p->draughts_game, GET_PARAMETER_STR(params, 1),
            GET_PARAMETER_STR(params, count - 1), number_of_hops,
            pieces_taken, white);
  wall_board(p->draughts_game, white);
 }
}

static void user_draughts_decline(player *p, const char *str)
{
 decline_draughts_game(p, str, 1);
}

/* sets the close flag for you, if both players have closed the game, then
   it exits and the score doesn't count (draughts_played-- if not friendly) */
static void user_draughts_close(player *p)
{
 draughts_game *game = p->draughts_game;
 player *p2;
 int flag_you_closed = 0;
 int flag_them_closed = 0;
 
 if (draughts_is_playing_game(p))
   if (!(game->flags & DRAUGHTS_FRIENDLY))
   { 
    if (game->white == p)
    {
     flag_you_closed = DRAUGHTS_WHITE_CLOSED;
     flag_them_closed = DRAUGHTS_BLACK_CLOSED;
     p2 = game->black;
    }
    else
    {
     flag_you_closed = DRAUGHTS_BLACK_CLOSED;
     flag_them_closed = DRAUGHTS_WHITE_CLOSED;
     p2 = game->white;
    }
    
    if (game->flags & flag_you_closed)
      fvtell_player(NORMAL_T(p),
                    " You have already closed the %s game.\n",
                    GAME_TYPE(p, 0));
    else
      if (game->flags & flag_them_closed)
      {
       fvtell_player(NORMAL_T(p),
                     " You close the %s game, agreeing with %s.\n"
                     " The game is therefore over.\n",
                     GAME_TYPE(p, 0), p2->saved->name);
       
       fvtell_player(SYSTEM_T(p2),
                     "%s -=> %s agrees and closes the %s game.\n"
                     "     The game is therefore over and the score "
                     "doesn't count.^N\n",
                     USER_COLOUR_DRAUGHTS,
                     p->saved->name,
                     GAME_TYPE(p2, 0));

       vwall_watchers(game,
                      "%s -=> The players close the %s game, "
                      "therefore the score does not count.\n"
                      "     The game is over.^N\n",
                      USER_COLOUR_DRAUGHTS,
                      GAME_TYPE(p, 0));
       
       /* they both agree to forfeit, therefore no score */
       p->draughts_played--;
       p2->draughts_played--;
       
       /* get rid of the game */
       destroy_draughts_game(game);
       p->draughts_game = NULL;
       p2->draughts_game = NULL;
      }
      else
      {
       p->draughts_game->flags |= flag_you_closed;
       fvtell_player(NORMAL_T(p),
                     " You ask %s if %s would mind closing the %s game.\n",
                     p2->saved->name,
                     gender_choose_str(p2->gender, "he", "she", "they", "it"),
                     GAME_TYPE(p, 0));
       fvtell_player(SYSTEM_T(p2),
                     "%s -=> %s closes the game, if you wish to agree and "
                     "close also, type:\n"
                     "     %s close_game\n"
                     "     Warning, this means that the score will not "
                     "count!^N\n",
                     USER_COLOUR_DRAUGHTS,
                     p->saved->name,
                     GAME_TYPE(p2, 0));
      }
   }
   else
     fvtell_player(NORMAL_T(p),
                   " In a friendly %s game, the score doesn't count so "
                   "there is no need to close it.\n",
                   GAME_TYPE(p, 0));
 else
   fvtell_player(NORMAL_T(p),
                 " You must be playing a game of %s to close it.\n",
                 GAME_TYPE(p, 0));
}

/* toggles the game to make it private (off the watchers list) */
static void user_draughts_private(player *p, const char *passed_str)
{
 int made = 0;
 int your_private_flag = 0;
 int their_private_flag = 0;
 player *p2 = 0;
 draughts_game *game = NULL;
 const char *str = passed_str;
 const char *current_wish = 0;
 
 if (draughts_is_playing_game(p) || draughts_is_watching_game(p))
 {
  game = p->draughts_game;
  
  if (draughts_is_playing_game(p))
  { /* playing a game */
   if (game->white == p)
   {
    your_private_flag = DRAUGHTS_PRIVATE_WHITE;
    their_private_flag = DRAUGHTS_PRIVATE_BLACK;
    p2 = game->black;
   }
   else
   {
    your_private_flag = DRAUGHTS_PRIVATE_BLACK;
    their_private_flag = DRAUGHTS_PRIVATE_WHITE;
    p2 = game->white;
   }

   if (strcasecmp(current_sub_command, "public_game"))
     current_wish = "private";
   else
     current_wish = "public";
   
   /* see if they actually typed the public command */
   if (!(strcasecmp(current_wish, "public")))
     if (*str)
     {
      if (TOGGLE_MATCH_ON(str))
        str = "off";
      else if (TOGGLE_MATCH_OFF(str))
        str = "on";
     }
      
   if (*str)
   { /* theres a string */
    if (!TOGGLE_MATCH_ON(str))
      if (!TOGGLE_MATCH_OFF(str))
        TELL_FORMAT(p, "[on|off]");
      else
      { /* off */
       if (game->flags & your_private_flag)
       {
        if (game->flags & their_private_flag) 
        { /* theirs is on and yours is on already */
         fvtell_player(SYSTEM_T(p2),
                       "%s -=> %s wants the %s game to be public.\n"
                       "     If you agree (you don't mind spectators), "
                       "type: "
                       "     %s%s%s off^N\n",
                       USER_COLOUR_DRAUGHTS,
                       p->saved->name, GAME_TYPE(p2, 0),
                       (MODE_IN_MODE(p2, DRAUGHTS) ? "" : GAME_TYPE(p2, 0)),
                       (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "),
                       current_sub_command);
         fvtell_player(NORMAL_T(p),
                       " You want the game to be public, "
                       "you must wait for %s to agree.\n",
                       p2->saved->name);
         
         game->flags &= ~your_private_flag;
        }
        else
        { /* theirs isn't on and yours is on already */
         if (!player_link_find(draughts_games_list, game->black->saved,
                               NULL, PLAYER_LINK_DEFAULT))
         {            
          fvtell_player(NORMAL_T(p),
                        " You agree to make this %s game public, so "
                        "residents can watch now it.\n",
                        GAME_TYPE(p, 0));
          fvtell_player(SYSTEM_T(p2),
                        "%s -=> %s agrees to make this %s game public.^N\n",
                        USER_COLOUR_DRAUGHTS,
                        p->saved->name, GAME_TYPE(p2, 0));
          
          player_link_add(&draughts_games_list, game->black->saved, NULL,
                          PLAYER_LINK_DEFAULT, NULL);
          made = 1; /* set it this time */
          game->flags &= ~your_private_flag;
         }
         else
         { /* game was on the list */
          fvtell_player(NORMAL_T(p),
                        " You remove your private %s game request.\n",
                        GAME_TYPE(p, 0));
          fvtell_player(SYSTEM_T(p2),
                         "%s -=> %s cancel%s %s request to make the %s "
                        "game a private one.^N\n",
                        USER_COLOUR_DRAUGHTS,
                        p->saved->name,
                        (p->gender == GENDER_PLURAL) ? "" : "'s",
                        gender_choose_str(p->gender, "his", "her",
                                          "their", "its"),
                        GAME_TYPE(p2, 0));
          game->flags &= ~your_private_flag;
         }
        }
       }
       else
       { /* yours is off already */
        if (game->flags & their_private_flag)
          fvtell_player(NORMAL_T(p),
                        " You have already requested that this %s game be a "
                        "public one.\n",
                        GAME_TYPE(p, 0));
        else
        { /* theirs isn't on, and yours is off already */
         fvtell_player(NORMAL_T(p),
                       " This %s game is already a %s one.\n",
                       GAME_TYPE(p, 0), current_wish);
        }
       }
      }
    else /* on */
      if (game->flags & your_private_flag)
      {
       if (game->flags & their_private_flag)
       { /* yours is on already and theirs is on */
        fvtell_player(NORMAL_T(p),
                      " This %s game is already %s.\n", GAME_TYPE(p, 0),
                      current_wish);
       }
       else
       {
        fvtell_player(NORMAL_T(p),
                      " You have re-request that this %s game should be "
                      "private.\n", GAME_TYPE(p, 0));
        fvtell_player(SYSTEM_T(p2),
                      "%s -=> %s re-requests that this %s game be a private "
                      "one.^N\n", USER_COLOUR_DRAUGHTS,
                      p->saved->name, GAME_TYPE(p2, 0));
       }
      }
      else
      { /* your flag isn't already on */
       if (game->flags & their_private_flag)
       { /* theirs is already on */
        if (player_link_find(draughts_games_list, game->black->saved,
                             NULL, PLAYER_LINK_DEFAULT))
        { /* agree that its should be private from now */
          fvtell_player(NORMAL_T(p),
                        " You agree to make this game private, no "
                        "one can watch it from now.\n");
          fvtell_player(SYSTEM_T(p2),
                        "%s -=> %s agree%s to make this %s game private.^N\n",
                        USER_COLOUR_DRAUGHTS,
                        p->saved->name,
                        (p->gender == GENDER_PLURAL) ? "" : "s",
                        GAME_TYPE(p2, 0));
          
          player_link_del(&draughts_games_list, game->black->saved, NULL,
                          PLAYER_LINK_DEFAULT, NULL);
          made = 1; /* set it this time */
          game->flags |= your_private_flag;
        }
        else
        {
         fvtell_player(NORMAL_T(p),
                       " You cancel your request for a public %s game.\n",
                       GAME_TYPE(p, 0));
         fvtell_player(SYSTEM_T(p2),
                       "%s -=> %s cancel%s %s request for a public "
                       "%s game.%s\n",
                       USER_COLOUR_DRAUGHTS,
                       p->saved->name,
                       (p->gender == GENDER_PLURAL) ? "" : "'s",
                       gender_choose_str((p)->gender, "his", "her",
                                         "their", "its"),
                       GAME_TYPE(p2, 0), "^N");
         game->flags |= your_private_flag;
        }
       }
       else
       { /* request a private game - theres is off */
        fvtell_player(SYSTEM_T(p2),
                      "%s -=> %s want%s the %s game to be private.\n"
                      "     If you agree (you don't want spectators), "
                      "type: "
                      "     %s%s%s on^N\n",
                      USER_COLOUR_DRAUGHTS,
                      p->saved->name, (p->gender == GENDER_PLURAL) ? "" : "s",
                      GAME_TYPE(p2, 0),
                      (MODE_IN_MODE(p2, DRAUGHTS) ? "" : GAME_TYPE(p2, 0)),
                      (MODE_IN_MODE(p2, DRAUGHTS) ? "" : " "),
                      current_sub_command);
        fvtell_player(NORMAL_T(p),
                      " You want the game to be private, "
                      "you must wait for %s to agree.\n",
                      p2->saved->name);
        
        game->flags |= your_private_flag;        
       }
      }
   }
  }

  if (!(*str) || made)
  {
   /* explain the situation now */
   if (game->flags & (DRAUGHTS_PRIVATE_BLACK|DRAUGHTS_PRIVATE_WHITE))
     fvtell_player(NORMAL_T(p),
                   " The %s game is %sprivate.\n",
                   GAME_TYPE(p, 0), (made ? "now " : ""));
   else
     fvtell_player(NORMAL_T(p),
                   " The %s game is %spublic.\n",
                   GAME_TYPE(p, 0), (made ? "now " : ""));
  }
 }
 else
   if (*str)
     fvtell_player(NORMAL_T(p),
                   " You must be playing a game of %s "
                   "to make it private/public.\n", GAME_TYPE(p, 0));
   else
     fvtell_player(NORMAL_T(p),
                   " You must be playing or watching a game of %s "
                   "to check if it's private/public.\n", GAME_TYPE(p, 0));
}

/* shows you the board of the game being played / watched */
static void user_draughts_view_board(player *p, const char *str)
{
 int white = 0;

 if (p->draughts_game)
   if (!(p->draughts_game->flags & DRAUGHTS_PROPOSED))
   {
    if (p->draughts_game->white == p) /* they white ? */
      if (beg_strcmp(str, "flipped")) /* yes, typed flipped ? */
        white = 1; /* no, see normal */
      else
        white = 0; /* yes, see black's way */
    else /* they're black */
      if (beg_strcmp(str, "flipped")) /* typed flipped ? */
        white = 0; /* no, see normal */
      else
        white = 1; /* yes, see black's way */
    
    fvtell_player(NORMAL_T(p), "%s",
                  " The current board is...\n\n");

    show_draughts_board(p, white, 0);

    fvtell_player(NORMAL_T(p),
                  "\n"
                  " It's %s (%s) to move next.\n",
                  p->draughts_game->to_move->saved->name,
                  (p->draughts_game->to_move == p->draughts_game->white ?
                   "white" : "black"));
   }
   else
     fvtell_player(NORMAL_T(p),
                   " There must be a game of %s in progress to "
                   "view a board.\n", GAME_TYPE(p, 0));
 else
   fvtell_player(NORMAL_T(p),
		 " You must be player or watching a game of %s to "
		 "view a board.\n", GAME_TYPE(p, 0));
}

/* offers or accepts an offer of a friendly game */
static void user_draughts_friendly_play(player *p, const char *str)
{
 if (!*str)
   fvtell_player (SYSTEM_FT(SYSTEM_INFO, p),
                  " Format: %s%s%s <player_to_play>\n",
		  (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
		  (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "),
                  current_sub_command);
 else
   /* start the game if its okay - not friendly */
   check_game_call_start(p, str, 1);
}

static void user_draughts_auto_view_board(player *p, parameter_holder *params)
{
 int made = 0;
 int on = FALSE;

 switch (params->last_param)
 {
  case 2:
    if (TOGGLE_MATCH_ON(GET_PARAMETER_STR(params, 2)))
      on = TRUE;
    else if (!TOGGLE_MATCH_OFF(GET_PARAMETER_STR(params, 2)))
      TELL_FORMAT(p, "[own_move|opponents_move|watched_move] [on|off]");

    if ((beg_strcasecmp(GET_PARAMETER_STR(params, 1), "opponents_move")))
      if ((beg_strcasecmp(GET_PARAMETER_STR(params, 1), "own_move")))
        if ((beg_strcasecmp(GET_PARAMETER_STR(params, 1), "watched_move")))
          TELL_FORMAT(p, "[own_move|opponents_move|watched_move] [on|off]");
        else /* watched move */
          if (!on)
          {
           made = 1;
           p->flag_draughts_no_auto_show_watch = TRUE;
          }
          else
          {
           made = 1;
           p->flag_draughts_no_auto_show_watch = FALSE;
          }
      else /* own move */
        if (!on)
        {
         made = 1;
         p->flag_draughts_no_auto_show_my_move = TRUE;
        }
        else
        {
         made = 1;
         p->flag_draughts_no_auto_show_my_move = FALSE;
        }
    else /* opponents move */
      if (!on)
      {
       made = 1;
       p->flag_draughts_no_auto_show_player = TRUE;
      }
      else
      {
       made = 1;
       p->flag_draughts_no_auto_show_player = FALSE;
      }
    get_parameter_shift(params, 1);
  case 0:
    break;
    
  default:
    TELL_FORMAT(p, "[own_move|opponents_move|watched_move] [on|off]");
 }
 
 fvtell_player(NORMAL_T(p),
               " You will ^S^B%s^sautomatically get the following:\n"
               " %s shown the board on your move.\n"
               " %s shown the board on an opponents move.\n"
               " %s shown the board when a player of a game "
               "you're watching moves.\n",
               (made ? "now " : ""),
               (p->flag_draughts_no_auto_show_my_move ?
                "^S^BNot get^s" : "^S^BGet^s"),
               (p->flag_draughts_no_auto_show_player ?
                "^S^BNot get^s" : "^S^BGet^s"),
               (p->flag_draughts_no_auto_show_watch ?
                "^S^BNot get^s" : "^S^BGet^s"));
}

static void user_draughts_see_boards(player *p, const char *str)
{
 int board_to_show = 0;
 
 if (*str)
 {
  if (NUMBER_OF_BOARDS < 2)
  {
   fvtell_player(NORMAL_T(p), "%s",
                 " There is only one board to choose from, sorry!\n");
   return;
  }
  
  board_to_show = atoi(str);

  if (RANGE(board_to_show, 1, NUMBER_OF_BOARDS))
  {
   fvtell_player(NORMAL_T(p),
                 " %s board number %d looks like this:\n\n",
                 GAME_TYPE(p, 1), board_to_show);
   show_draughts_board(p, 0, board_to_show);
  }
  else
    fvtell_player(NORMAL_T(p),
                  " Please specify a board in the range 1 to %d.\n",
                  NUMBER_OF_BOARDS);
 }
 else
 {
  if (NUMBER_OF_BOARDS > 1)
    fvtell_player(NORMAL_T(p),
                  " There are %d %s boards available (1 to %d).\n",
                  NUMBER_OF_BOARDS,
                  GAME_TYPE(p, 0), NUMBER_OF_BOARDS);
  else
    fvtell_player(NORMAL_T(p), " There is one %s board available.\n",
                  GAME_TYPE(p, 0));
  fvtell_player(NORMAL_T(p),
                " To see another board, type:      %s%sshow_board <number>\n"
                " To select a board, type:         %s%schoose_board "
                "[<number>]\n"
                " To see your current board, type: %s%schoose_board\n",
                (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
                (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "),
                (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
                (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "),
                (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
                (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "));
 }
}

static void user_draughts_choose_board(player *p, const char *str)
{
 int board = 0;
 
 if (*str)
 {
  board = atoi(str);
  
  if (RANGE(board, 1, (NUMBER_OF_BOARDS + 1)))
  {
   p->draughts_board = (board - 1);
   fvtell_player(NORMAL_T(p),
                 " You select %s board number %d.\n",
                 GAME_TYPE(p, 0), (p->draughts_board + 1));
  }
  else
    fvtell_player(NORMAL_T(p),
                  " You must choose a board in the range 1 to %d.\n",
                  NUMBER_OF_BOARDS);
 }
 else
 {
  fvtell_player(NORMAL_T(p),
                " Your current %s board (number %d) looks like this:\n\n",
                GAME_TYPE(p, 0), p->draughts_board);
  show_draughts_board(p, 0, (p->draughts_board + 1));
  fvtell_player(NORMAL_T(p),
                "\n"
                " To see other boards, type: %s%sshow_board [<number>]\n"
                " To select a board, type:   %s%schoose_board <number>\n",
                (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
                (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "),
                (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
                (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "));
 }
}

static void user_draughts_list_games(player *p)
{
 player_linked_list *current = draughts_games_list;
 player *pgame = 0;
 int game_count = 1;
 
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 
 if (current)
 {
  fvtell_player(NORMAL_T(p),
                "^B%-4s  %-20s %-20s  %-9s   %-7s   %-8s^N\n",
                "Game", "Black Player", "White Player", "#Watching",
                "Started", "Friendly");
  fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
  
  while (current)
  {
   pgame = PLAYER_LINK_GET(current);
   fvtell_player(NORMAL_T(p),
                 "%-4d  %-20s %-20s  %-9s   %-8s  %-8s",
                 game_count, 
                 pgame->draughts_game->black->saved->name,
                 pgame->draughts_game->white->saved->name,
                 people_watching_draughts(pgame->draughts_game, 1),
                 DISP_TIME_P_STD(pgame->draughts_game->started_at, p),
                 (pgame->draughts_game->flags & DRAUGHTS_FRIENDLY ?
                  "Yes" : "No"));
   game_count++;
   if ((current = PLAYER_LINK_NEXT(current)))
     fvtell_player(NORMAL_T(p), "\n");
  }

  fvtell_player(NORMAL_T(p), "\n");
 }
 else
   fvtell_player(NORMAL_T(p),
                 " There are currently no public %s games in progress.\n",
                 GAME_TYPE(p, 0));

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void user_draughts_watch(player *p, const char *str)
{
 int game = 0;
 int changed = 0;
 int moved_to = 1;
 player_linked_list *current = draughts_games_list;

 if (draughts_is_playing_game(p))
 {
  fvtell_player(NORMAL_T(p),
                " You are currently playing a %s game, you cannot watch "
                "one aswell.\n", GAME_TYPE(p, 0));
 }
 else
   if (*str)
   {
    if (draughts_games_list)
    {
     game = atoi(str); /* get the game number */
     
     if (draughts_is_watching_game(p) && !game)
     {
      draughts_stop_watching_game(p, 1);
      return;
     }
     
     if (RANGE(game, 1, count_number_of_draughts_games()))
     {
      if (draughts_is_watching_game(p))
      {
       changed = 1;
       player_link_del(&p->draughts_game->watchers, p->saved, NULL,
                       PLAYER_LINK_DEFAULT, NULL);
      }
      
      for (; current && (moved_to < game); current = PLAYER_LINK_NEXT(current))
        moved_to++;
      
      /* add the player to their watchers list */
      player_link_add(&PLAYER_LINK_GET(current)->draughts_game->watchers,
                      p->saved, NULL, PLAYER_LINK_DEFAULT, NULL);
      p->draughts_game = PLAYER_LINK_GET(current)->draughts_game;
      
      fvtell_player(NORMAL_T(p),
                    " You %s watching %s game number %d.\n",
                    (changed ? "switch to watching" : "start"),
                    GAME_TYPE(p, 0), game);
     }
     else
     {
      fvtell_player(NORMAL_T(p), "%s",
                    " You must specify ");
      if (count_number_of_draughts_games() > 1)
        fvtell_player(NORMAL_T(p),
                      "a %s game in the range 1 to %d.\n",
                      GAME_TYPE(p, 0), count_number_of_draughts_games());
      else
        fvtell_player(NORMAL_T(p),
                      "%s game number 1.\n", GAME_TYPE(p, 0));
     }
    }
    else
      fvtell_player(NORMAL_T(p),
                    " There are currently no public %s games to watch.\n",
                    GAME_TYPE(p, 0));
   }
   else
     if (draughts_games_list)
       fvtell_player(SYSTEM_FT(SYSTEM_INFO, p),
                     " Format: %s%s%s <game_number>\n"
                     " For a list of %s games: %s%slist_games\n",
                     (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
                     (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "),
                     current_sub_command,
                     GAME_TYPE(p, 0),
                     (MODE_IN_MODE(p, DRAUGHTS) ? "" : GAME_TYPE(p, 0)),
                     (MODE_IN_MODE(p, DRAUGHTS) ? "" : " "));
     else
       fvtell_player(NORMAL_T(p),
                     " There are currently no public %s games to watch.\n",
                     GAME_TYPE(p, 0));
}


/* mode things */


static void draughts_rejoin_func(player *p)
{
 fvtell_player(NORMAL_T(p), 
               " Re-Entering %s mode. Use ^Bhelp %s^N for help\n"
               "%s\n"
               " To run ^Bnormal commands^N type /<command>\n"
               " Use '^Bend^N' to ^Bleave^N.\n",
               GAME_TYPE(p, 0), GAME_TYPE(p, 0),
               (p->draughts_game ?
                " If you 'end' the current mode you will not quit "
                "your game."
                : " You are not currently playing a game."));
}

static int user_draughts_command(player *p, const char *str, size_t length)
{
 ICTRACE("draughts_command");
 
 if (MODE_IN_MODE(p, DRAUGHTS))
   MODE_HELPER_COMMAND();
 else
   if (!*str)
   {
    cmds_function tmp_cmd;
    cmds_function tmp_rejoin;
    
    CMDS_FUNC_TYPE_RET_CHARS_SIZE_T((&tmp_cmd), user_draughts_command);
    CMDS_FUNC_TYPE_NO_CHARS((&tmp_rejoin), draughts_rejoin_func);

    if (mode_add(p, "$R-Nationality(us(Checkers)def(Draughts)) Mode-> ",
                 MODE_ID_DRAUGHTS, 0, &tmp_cmd, &tmp_rejoin, NULL))
      fvtell_player(NORMAL_T(p), 
                    " Entering %s mode. Use ^Bhelp %s^N for help\n"
                    "%s\n"
                    " To run ^Bnormal commands^N type /<command>\n"
                    " Use '^Bend^N' to ^Bleave^N.\n",
                    GAME_TYPE(p, 0), GAME_TYPE(p, 0),
                    (p->draughts_game ?
                     " If you 'end' the current mode you will not quit "
                     "your game."
                     : " You are not currently playing a game."));
    else
      fvtell_player(SYSTEM_T(p), 
                    " You cannot enter draughts mode as you are in too many "
                    "other modes.\n");
    return (TRUE);
   }
   else
     set_game_type(p);

 return (cmds_sub_match(p, str, length,
                        &cmds_sub[CMDS_SUB_OFFSET(CMDS_SECTION_DRAUGHTS)]));
}

static void user_draughts_view_commands(player *p)
{
 user_cmds_show_section(p, "draughts");
}

static void user_draughts_end_mode(player *p)
{
 if (!MODE_IN_MODE(p, DRAUGHTS))
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You just tried to exit draughts mode "
                "while not being in it?\n");
  return;
 }
 
 fvtell_player(NORMAL_T(p),
	       " Leaving draughts mode%s\n",
	       (p->draughts_game ?
		", your game is still active."
		: "."));

 mode_del(p);
}

void user_configure_game_draughts_use(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, configure.game_draughts_use, TRUE,
                       " The draughts game is %splayable.\n",
                       " The draughts game is %snot playable.\n", TRUE);

 configure_save(FALSE);
}

void cmds_init_draughts(void)
{
 CMDS_BEGIN_DECLS();
  
#define CMDS_SECTION_SUB CMDS_SECTION_DRAUGHTS
 
 CMDS_ADD_SUB("auto_private_game", user_draughts_auto_private_game, CONST_CHARS);
 CMDS_ADD_SUB("auto_view_board", user_draughts_auto_view_board, PARSE_PARAMS);
 CMDS_ADD_SUB("chat", user_draughts_chat, CONST_CHARS);
 CMDS_ADD_SUB("chemote", user_draughts_emote_chat, CONST_CHARS);
 CMDS_ADD_SUB("choose_board", user_draughts_choose_board, CONST_CHARS);
 CMDS_ADD_SUB("close_game", user_draughts_close, NO_CHARS);
 CMDS_FLAG(no_expand);
 CMDS_ADD_SUB("commands", user_draughts_view_commands, NO_CHARS);
 CMDS_ADD_SUB("declare_win", user_draughts_decl_win, NO_CHARS);
 CMDS_ADD_SUB("decline_offer", user_draughts_decline, CONST_CHARS);
 CMDS_ADD_SUB("emote_chat", user_draughts_emote_chat, CONST_CHARS);
 CMDS_ADD_SUB("end", user_draughts_end_mode, NO_CHARS);
 CMDS_PRIV(mode_draughts);
 
 CMDS_ADD_SUB("friendly_play", user_draughts_friendly_play, CONST_CHARS);
 CMDS_ADD_SUB("list_games", user_draughts_list_games, NO_CHARS);
 CMDS_ADD_SUB("list_watching", user_draughts_watching_check, NO_CHARS);
 CMDS_ADD_SUB("move", user_move_draughts, PARSE_PARAMS);
 CMDS_ADD_SUB("offer", user_draughts_play, CONST_CHARS); /* alias */
 CMDS_ADD_SUB("offer_friendly", user_draughts_friendly_play, CONST_CHARS); /* alias */
 CMDS_ADD_SUB("play", user_draughts_play, CONST_CHARS);
 CMDS_ADD_SUB("play_friendly", user_draughts_friendly_play, CONST_CHARS);
 CMDS_ADD_SUB("private_game", user_draughts_private, CONST_CHARS);
 CMDS_ADD_SUB("public_game", user_draughts_private, CONST_CHARS);
 CMDS_ADD_SUB("quit_game_now", user_draughts_quit, NO_CHARS);
 CMDS_FLAG(no_expand);
 CMDS_ADD_SUB("resign_game_now", user_draughts_quit, NO_CHARS); /* alias */
 CMDS_FLAG(no_expand);
 CMDS_ADD_SUB("setup", user_draughts_test_setup, CONST_CHARS);
 CMDS_FLAG(no_expand); CMDS_PRIV(admin);
 CMDS_ADD_SUB("show_board", user_draughts_see_boards, CONST_CHARS);
 CMDS_ADD_SUB("stats", user_draughts_stats, NO_CHARS);
 CMDS_ADD_SUB("stop_watching", user_draughts_stop_watching, NO_CHARS);
 CMDS_ADD_SUB("view_board", user_draughts_view_board, CONST_CHARS);
 CMDS_ADD_SUB("watch_game", user_draughts_watch, CONST_CHARS);
 CMDS_ADD_SUB("withdraw_offer", user_draughts_withdraw, NO_CHARS);
 
#undef CMDS_SECTION_SUB

 CMDS_ADD("draughts", user_draughts_command, RET_CHARS_SIZE_T, GAME);
 CMDS_PRIV(configure_game_draughts_base);
}
