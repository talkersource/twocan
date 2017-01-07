#define TTT_C
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



static void ttt_print_board(player *p, player *p2)
{
 int count = 1;
 player *noughts = p->ttt_noughts ? p : p2;
 player *crosses = p->ttt_noughts ? p2 : p; 

 while (count < 10)
 {
  switch (count)
  {
   case 1:
     fvtell_player(NORMAL_T(p), "\n");
     break;
 
   case 4:
   case 7:
     if (count == 4)
       fvtell_player(NORMAL_T(p), "     1 | 2 | 3");
     else
       fvtell_player(NORMAL_T(p), "     4 | 5 | 6");
     
     fvtell_player(NORMAL_T(p), "\n---+---+---    ---+---+---\n");
     break;
     
   default:  
     fvtell_player(NORMAL_T(p), "|");
     break;
  }
  
  assert(!((noughts->ttt_board & TTT_BIT(count)) &&
           (crosses->ttt_board & TTT_BIT(count))));
  
  fvtell_player(NORMAL_T(p),
                (noughts->ttt_board & TTT_BIT(count)) ? " ^S^2o^s " :
                (crosses->ttt_board & TTT_BIT(count)) ? " ^S^7x^s " : "   ");

  ++count;
 }

 fvtell_player(NORMAL_T(p), "     7 | 8 | 9");
 fvtell_player(NORMAL_T(p), "\n\n");
}

static void ttt_start_game(player *p1, player *p2)
{
 p1->ttt_playing = TRUE;
 p2->ttt_playing = TRUE;
 
 p1->ttt_noughts = p1->ttt_my_go = FALSE;
 p2->ttt_noughts = p2->ttt_my_go = TRUE;
 
 p1->ttt_board = p2->ttt_board = 0;

 ttt_print_board(p1, p2);
 ttt_print_board(p2, p1);
 
 fvtell_player(SYSTEM_T(p2), " It's you to go first, as %s.\n",
               "^S^2Noughts^s");
}

static void ttt_end_win_game(player *winner, player *loser)
{
 winner->ttt_won++;
 winner->ttt_played++;
 loser->ttt_played++;
 ttt_print_board(winner, loser);
 ttt_print_board(loser, winner);
 fvtell_player(SYSTEM_T(loser), " You lost that game of tic tac toe, as %s.\n",
               loser->ttt_noughts ? "^S^2Noughts^s" : "^S^7Crosses^s");
 fvtell_player(SYSTEM_T(winner), " You won that game of tic tac toe, as %s.\n",
               winner->ttt_noughts ? "^S^2Noughts^s" : "^S^7Crosses^s");
 winner->ttt_playing = FALSE;
 loser->ttt_playing = FALSE;
 winner->ttt_oponent = NULL;
 loser->ttt_oponent = NULL;
 *winner->ttt_op_name = 0;
 *loser->ttt_op_name = 0;
}

static void ttt_end_draw_game(player *p, player *p2)
{
 p->ttt_played++;
 p2->ttt_played++;
 p->ttt_drawn++;
 p2->ttt_drawn++;
 ttt_print_board(p, p2);
 ttt_print_board(p2, p);
 fvtell_player(SYSTEM_T(p), " You drew that game of tic tac toe, as %s.\n",
               p->ttt_noughts ? "^S^2Noughts^s" : "^S^7Crosses^s");
 fvtell_player(SYSTEM_T(p2), " You drew that game of tic tac toe, as %s.\n",
               p2->ttt_noughts ? "^S^2Noughts^s" : "^S^7Crosses^s");
 p->ttt_playing = FALSE;
 p2->ttt_playing = FALSE;
 p->ttt_oponent = NULL;
 p2->ttt_oponent = NULL;
 *p->ttt_op_name = 0;
 *p2->ttt_op_name = 0;
}

static void user_ttt_start(player *p, const char *str)
{ /* offer and accept */
 player *p2 = NULL;
 
 if (p->ttt_playing)
 {
  fvtell_player(SYSTEM_T(p), "%s", " You can only play one game at once.\n");
  return;
 }

 if (!(p2 = player_find_on(p, str,
                           PLAYER_FIND_VERBOSE |
                           PLAYER_FIND_EXPAND |
                           PLAYER_FIND_NICKS |
                           PLAYER_FIND_PICK_FIRST)))
   return;
 
 qstrcpy(p->ttt_op_name, p2->saved->lower_name);
 p->ttt_oponent = p2;
 
 if (!strcmp(p2->ttt_op_name, p->saved->lower_name))
 { /* they offered first */
  ttt_start_game(p2, p);
  return;
 }
 else
 {
  LIST_GAME_CHECK_FLAG_START(p2, p->saved);
  if (LIST_GAME_CHECK_FLAG_DO(ttt))
  {
   fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking "
                 "tic tac toe from you.\n", p2->saved->name);
   return;
  }
  LIST_GAME_CHECK_FLAG_END();
 }
 
 fvtell_player(NORMAL_T(p2),
               " %s%s offer%s you a game of tic tac toe.\n",
               gender_choose_str(p->gender, "", "", "The ", "The "),
               p->saved->name, (p->gender == GENDER_PLURAL) ? "" : "s");
 fvtell_player(NORMAL_T(p),
               " You offer ^S^B%s^s a game of tic tac toe.\n",
               p2->saved->name);
}

static void user_ttt_decline(player *p, const char *str)
{
 if (p->ttt_playing)
   fvtell_player(SYSTEM_T(p), "%s", " You are _already_ playing a game.\n");
 else
 {
  player *p2 = player_find_on(p, str, PLAYER_FIND_VERBOSE |
                              PLAYER_FIND_EXPAND |
                              PLAYER_FIND_NICKS |
                              PLAYER_FIND_PICK_FIRST);

  if (!p2)
    return;
  
  if (!strcmp(p2->ttt_op_name, p->saved->name))
  {
   p2->ttt_oponent = NULL;
   *p2->ttt_op_name = 0;
   fvtell_player(NORMAL_T(p), " You decline the offer of a game of "
                 "tic tac toe from ^B%s^s.\n", p2->saved->name);
   fvtell_player(NORMAL_T(p2),
                 " -=> %s declines your offer of a game of tic tac toe.\n",
                 p->saved->name);
  }
  else
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- isn't offering "
                  "you a game at the moment.\n", p2->saved->name);
 }
}

void user_ttt_quit(player *p)
{
 if (p->ttt_playing)
   ttt_end_win_game(p->ttt_oponent, p);
 else
   fvtell_player(SYSTEM_T(p), "%s",
                 " You aren't playing a game of tic tac toe at the moment.\n");
}

static int ttt_check_won(player *p)
{
 int count = 1;
 
 if (TTT_WON_D_1(p->ttt_board))
   return (TRUE);

 if (TTT_WON_D_2(p->ttt_board))
   return (TRUE);

 while (count < 4)
 {
  if (TTT_WON_H(p->ttt_board, count))
    return (TRUE);
  if (TTT_WON_V(p->ttt_board, count))
    return (TRUE);
  
  ++count;
 }

 return (FALSE);
}

static void user_ttt_command(player *p, const char *str)
{
 if (p->ttt_playing)
 {
  int place = atoi(str);
  
  if (*str)
  { 
   if (*(str + strspn(str, "123456789")))
     TELL_FORMAT(p, "[ 1 - 9 ]");

   if (!((place > 0) && (place < 10)))
   { /* be nicer about our format message */
    fvtell_player(SYSTEM_T(p), "%s",
                  " That is not a valid tic tac toe number.\n"
                  " Valid numbers are in the range 1 to 9 inclusive.\n");
    return;
   }

   if (!p->ttt_my_go)
   {
    fvtell_player(SYSTEM_T(p), "%s",
                  " You can have a go, when it's your turn.\n");
    return;
   }
   
   if (TTT_BIT(place) & (p->ttt_board | p->ttt_oponent->ttt_board))
   {
    fvtell_player(SYSTEM_T(p), " Your %spiece is already in that place.\n",
                  (TTT_BIT(place) & p->ttt_board) ? "" : "oponents ");
    return;
   }
   
   p->ttt_board |= TTT_BIT(place);
   p->ttt_my_go = FALSE;
   p->ttt_oponent->ttt_my_go = TRUE;
   
   if (ttt_check_won(p))
     ttt_end_win_game(p, p->ttt_oponent);
   else
     if (TTT_IS_DRAW(p->ttt_board | p->ttt_oponent->ttt_board))
       ttt_end_draw_game(p, p->ttt_oponent);
     else
     {
      fvtell_player(NORMAL_T(p), " You place a piece at board position %d.\n",
                    place);
      
      if (p->ttt_oponent->flag_ttt_brief)
        fvtell_player(NORMAL_T(p->ttt_oponent),
                      " It is your go at tic tac toe, "
                      "opponent went in position %d.\n", place);
      else
        ttt_print_board(p->ttt_oponent, p);
     }
  }
  else
    ttt_print_board(p, p->ttt_oponent); /* show yourself the board */
 }
 else
   fvtell_player(SYSTEM_T(p), "%s",
                 " You aren't playing a game of tic tac toe at the moment.\n");
}

void user_toggle_ttt_brief(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_ttt_brief, TRUE,
                       " You will %sget brief games of tic tac toe.\n",
                       " You will %sget non-brief games of tic tac toe.\n",
                       TRUE);
}

void user_configure_game_ttt_use(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, configure.game_ttt_use, TRUE,
                       " The ttt game is %splayable.\n",
                       " The ttt game is %snot playable.\n", TRUE);

 configure_save(FALSE);
}

void cmds_init_ttt(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("ttt", user_ttt_command, CONST_CHARS, GAME);
 CMDS_PRIV(configure_game_ttt_base);
 CMDS_ADD("ttt_brief", user_toggle_ttt_brief, CONST_CHARS, GAME);
 CMDS_PRIV(configure_game_ttt_base);
 CMDS_ADD("ttt_decline", user_ttt_decline, CONST_CHARS, GAME);
 CMDS_PRIV(configure_game_ttt_base);
 CMDS_ADD("ttt_start", user_ttt_start, CONST_CHARS, GAME);
 CMDS_PRIV(configure_game_ttt_base);
 CMDS_ADD("ttt_quit", user_ttt_quit, NO_CHARS, GAME);
 CMDS_PRIV(configure_game_ttt_base);
}
