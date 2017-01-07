#define GAMES_C
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

static void user_games_info(player *p, const char *str)
{
 player *p2 = NULL;
 
 if (!*str)
   p2 = p;
 else if (!(p2 = player_find_load(p, str, PLAYER_FIND_SC_EXTERN_ALL)))
   return;
 
 fvtell_player(NORMAL_T(p),
               "%s ^S^B%s%s games sheet.^s\n",
               DASH_LEN,
               ((p == p2) ? "Your" : p2->saved->name),
               ((p == p2) ? "" : "'s"));

 if (configure.game_draughts_use)
 {
  ptell_mid(NORMAL_T(p), "Draughts/Checkers Information", FALSE);
  
  fvtell_player(NORMAL_T(p),
                "Total number of games played: % 4d\n"
                "Total number of games won:    % 4d\n"
                "Total number of games lost:   % 4d\n",
                p2->draughts_played,
                p2->draughts_won,
                p2->draughts_played - p2->draughts_won);
  
  if (draughts_is_playing_game(p2))
    fvtell_player(NORMAL_T(p),
                  "%s %s currently playing a game of %s.\n",
                  ((p == p2) ? "You" :
                   gender_choose_str(p2->gender, "He", "She", "They", "It")),
                  ((p == p2) ? "are" : "is"),
                  get_game_type(p2, 0));
  
  if (draughts_is_watching_game(p2))
    fvtell_player(NORMAL_T(p),
                  "%s %s currently watching a game of %s.\n",
                  ((p == p2) ? "You" :
                   gender_choose_str(p2->gender, "He", "She", "They", "It")),
                  ((p == p2) ? "are" : "is"),
                  get_game_type(p2, 0)); 
 }

 if (configure.game_sps_use)
 {
  ptell_mid(NORMAL_T(p), "Scissors, Paper, Stone Information", FALSE);
  fvtell_player(NORMAL_T(p),
                "Total number of games played: % 4d\n"
                "Total number of games won:    % 4d (%.1f%%)\n"
                "Total number of games lost:   % 4d (%.1f%%)\n"
                "Total number of games drawn:  % 4d (%.1f%%)\n",
                p2->sps_played,
                p2->sps_won, p2->sps_played ?
                (((double)p2->sps_won /
                  (double)p2->sps_played) * 100) : 0.0,
                p2->sps_played - p2->sps_won - p2->sps_drawn,
                p2->sps_played ?
                (((double)(p2->sps_played - p2->sps_won - p2->sps_drawn) /
                  (double)p2->sps_played) * 100) : 0.0,
                p2->sps_drawn, p2->sps_played ?
                (((double)p2->sps_drawn /
                  (double)p2->sps_played) * 100) : 0.0); 
 }

 if (configure.game_ttt_use)
 {
  ptell_mid(NORMAL_T(p), "Tic Tac Toe Information", FALSE);
  
  fvtell_player(NORMAL_T(p),
                "Total number of games played: % 6d\n"
                "Total number of games won:    % 6d\n"
                "Total number of games lost:   % 6d\n"
                "Total number of games drawn:  % 6d\n",
                p2->ttt_played,
                p2->ttt_won,
                p2->ttt_played - p2->ttt_won - p2->ttt_drawn,
                p2->ttt_drawn);
  
  fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 }
}

static void user_toggle_games_brief(player *p, const char *str)
{
 if (configure.game_sps_use)
   user_toggle_sps_brief(p, str);
 
 if (configure.game_ttt_use)
   user_toggle_ttt_brief(p, str);
}

void cmds_init_games(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("brief_games", user_toggle_games_brief, CONST_CHARS, GAME);
 CMDS_PRIV(configure_games_base);

 CMDS_ADD("games_info", user_games_info, CONST_CHARS, GAME);
 CMDS_PRIV(configure_games);
}
