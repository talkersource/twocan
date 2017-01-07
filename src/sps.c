#define SPS_C
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





/* functions */

/* internals */
/* gets the opposing (in our case next) type */
static int get_next_type(int type)
{
 switch (type) /* choose the winning object */
 {
  case STONE:
    return (PAPER);
  case SCISSORS:
    return (STONE);
  case PAPER:
    return (SCISSORS);
  default:
    assert(0);
 }

 return (0);
}

/* gets the biggest object type if there is one, if there's a tie, return 0 */
static int choose_biggest_type(sps_struct *mem)
{
 int type = 0;
 int count = 0;
 int biggest = -1;
 int next_type = 0;
 
 for (; count < 3; count++)
   if (biggest < mem->types[count])
   {
    type = count;
    biggest = mem->types[count];
   }

 /* if two or more are equal, then return no type biggest */
 next_type = get_next_type(type + 1);
 if ((biggest == mem->types[(next_type - 1)]) ||
     (biggest == mem->types[(get_next_type(next_type) - 1)]))
   return (0);
            
 return ((type + 1));
}

/* gets the choice for the computer when playing CL */
static int get_computer_choice(player *p)
{
 int type = 0;
 int rnd_num = get_random_num(1, 3);
 
 if (p->sps_cl_mem)
 {
  type = choose_biggest_type(p->sps_cl_mem);
  if (type) /* if there's a biggest */
    return (get_next_type(type)); /* get the opposite type */
  else /* else return a random type */
    return (rnd_num);
 }
 else
   return (rnd_num);
}

static int get_object_type(const char *str)
{
 if (beg_strcmp(str, "paper"))
   if (beg_strcmp(str, "scissors"))          
     if (beg_strcmp(str, "stone") && beg_strcmp(str, "rock"))
       return (0);
     else
       return (STONE); /* stone */
   else
     return (SCISSORS); /* scissors */
 else
   return (PAPER); /* paper */
}

/* prints the result of a game, winner == NULL on a draw */
static void print_result(player *p1, player *p2, int sps, int sps2,
                         player *winner)
{
 const char *l1_1 = GET_LINE(sps, 1);
 const char *l2_1 = GET_LINE(sps, 2);
 const char *l3_1 = GET_LINE(sps, 3);
 const char *l4_1 = GET_LINE(sps, 4);
 const char *l5_1 = GET_LINE(sps, 5);
 const char *l1_2 = GET_LINE(sps2, 1);
 const char *l2_2 = GET_LINE(sps2, 2);
 const char *l3_2 = GET_LINE(sps2, 3);
 const char *l4_2 = GET_LINE(sps2, 4);
 const char *l5_2 = GET_LINE(sps2, 5);
 
 if (p1->flag_sps_brief)
   fvtell_player(SYSTEM_T(p1),
                 " You choose ^B%s^b, %s chooses ^B%s^b.",
                 GET_SPS_STRING(sps, FALSE),
                 (p2 ? p2->saved->name : configure.name_long),
                 GET_SPS_STRING(sps2, FALSE));
 else
   fvtell_player(SYSTEM_T(p1),
                 " %-10s                          %-10s\n"
                 " %-10s %-9s                %-10s %-9s\n"
                 " %-10s                          %-10s\n" 
                 " %-10s                          %-10s\n"
                 " %-10s                          %-10s\n\n"
                 " %-8s                           %s\n",
                 l1_1, l1_2, l2_1,
                 GET_SPS_STRING(sps, TRUE),
                 l2_2,
                 GET_SPS_STRING(sps2, TRUE),
                 l3_1, l3_2, l4_1, l4_2, l5_1, l5_2,
                 "You", (p2 ? p2->saved->name : configure.name_long));

 if (p2)
 {
  if (p2->flag_sps_brief)
    fvtell_player(SYSTEM_T(p2),
                  " You choose ^B%s^b, %s chooses ^B%s^b.",
                  GET_SPS_STRING(sps2, FALSE),
                  p1->saved->name,
                  GET_SPS_STRING(sps, FALSE));
  else
    fvtell_player(SYSTEM_T(p2),
                  " %-10s                          %-10s\n"
                  " %-10s %-9s                %-10s %-9s\n"
                  " %-10s                          %-10s\n" 
                  " %-10s                          %-10s\n"
                  " %-10s                          %-10s\n\n"
                  " %-8s                           %s\n",
                  l1_2, l1_1, l2_2,
                  GET_SPS_STRING(sps2, 1),
                  l2_1,
                  GET_SPS_STRING(sps, 1),
                  l3_2, l3_1, l4_2, l4_1, l5_2, l5_1,
                  "You", p1->saved->name);
 }
 
 if (sps != sps2)
 {
  if (p1->flag_sps_brief)
    fvtell_player(SYSTEM_T(p1),
                  " %s win%s.\n",
                  (winner == p1 ? "You" :
                   (p2 ? p2->saved->name : configure.name_long)),
                  (winner == p1 ? "" :
                   (p2 && (p2->gender == GENDER_PLURAL) ? "" : "s")));
  else
    fvtell_player(SYSTEM_T(p1),
                  " %s win%s the game!\n",
                  /* FIXME: add "The " for lemmings etc. */
                  (winner == p1 ? "You" :
                   (p2 ? p2->saved->name : configure.name_long)),
                  (winner == p1 ? "" :
                   (p2 && (p2->gender == GENDER_PLURAL) ? "" : "s")));

  if (p2)
  {
   if (p2->flag_sps_brief)
     fvtell_player(SYSTEM_T(p2),
                   " %s win%s.\n",
                   (winner == p2 ? "You" : p1->saved->name),
                   ((winner == p2) || (p1->gender == GENDER_PLURAL)) ?
                   "" : "s");
   else
     fvtell_player(SYSTEM_T(p2),
                   " %s win%s the game!\n",
                   (winner == p2 ? "You" : p1->saved->name),
                   ((winner == p2) || (p1->gender == GENDER_PLURAL)) ?
                   "" : "s");
  }
 }
 else
 {
  if (p1->flag_sps_brief)
    fvtell_player(SYSTEM_T(p1), "%s", " Its a draw.\n");
  else
    fvtell_player(SYSTEM_T(p1), "%s", " The game is a draw!\n");

  if (p2)
  {
   if (p2->flag_sps_brief)
     fvtell_player(SYSTEM_T(p2), "%s", " Its a draw.\n");
   else
     fvtell_player(SYSTEM_T(p2), "%s", " The game is a draw!\n");
  }
 }
}

/* works out who won sps, and then prints the result */
static void work_out_winner(player *p1, player *p2, int sps, int sps2)
{
 int winner = 0;

 assert(p1); /* p2 may be NULL if playing the computer */
 assert(RANGE(sps, SCISSORS, STONE));
 assert(RANGE(sps2, SCISSORS, STONE));

 /* find out who won */
 winner = sps_win_table[(sps - 1)][(sps2 - 1)];

 if (winner)
   if (winner < 0)
   { /* p2 won */
    print_result(p1, p2, sps, sps2, p2);
    if (p2)
      p2->sps_won++;
    p1->sps_played++;
    if (p2)
      p2->sps_played++;
   }
   else
   { /* p1 won */
    print_result(p1, p2, sps, sps2, p1);
    p1->sps_won++;
    p1->sps_played++;
    if (p2)
      p2->sps_played++;
   }
 else
 { /* draw */
  print_result(p1, p2, sps, sps2, NULL);
  p1->sps_drawn++;
  if (p2)
    p2->sps_drawn++;
  p1->sps_played++;
  if (p2)
    p2->sps_played++;
 }
}

static void sps_play_game(player *p, player *p2, int sps_obj, int accepting)
{
 int sps_obj2 = 0;
 
 LIST_GAME_CHECK_FLAG_START(p2, p->saved);
 if (LIST_GAME_CHECK_FLAG_DO(sps))
 {
  fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking "
                "sps from you.\n", p2->saved->name);
  return;
 }
 LIST_GAME_CHECK_FLAG_END();
  
 if (accepting)
 { /* its an agreement */
  
  /* they must have an object chosen */
  assert(p2->sps_chosen); 
  
  sps_obj2 = p2->sps_chosen;
  work_out_winner(p, p2, sps_obj, sps_obj2);
  p2->sps_chosen = 0;
  p2->sps_playing = NULL;
 }
 else
 { /* its an offer */
  p->sps_playing = p2->saved; /* set up the offer */
  p->sps_chosen = sps_obj;
  fvtell_player(SYSTEM_T(p),
                " You offer ^S^B%s^s a game "
                "with you choosing '^S^B%s^s'.\n",
                p2->saved->name, GET_SPS_STRING(sps_obj, FALSE));
  fvtell_player(SYSTEM_T(p2),
                "^B -=> %s offer%s you a game of scissors, "
                "paper, stone.^N\n"
                "     To accept, type: %s %s "
                "<scissors|paper|stone>\n",
                p->saved->name,
                (p->gender == GENDER_PLURAL) ? "" : "s",
                current_command, p->saved->lower_name);
 }
}

static void user_sps(player *p, parameter_holder *params)
{
 player *p2 = 0;
 int sps_obj = 0;
 int sps_obj2 = 0;
 int random_time = (rand() % (RANDOM_SPS_HOURS + 1));
 
 if (params->last_param != 2)
   TELL_FORMAT(p, "<<player>|-talker> <scissors|paper|stone>");

 lower_case(GET_PARAMETER_STR(params, 2));
 
 if ((sps_obj = get_object_type(GET_PARAMETER_STR(params, 2))))
   if (beg_strcasecmp(GET_PARAMETER_STR(params, 1), "-crazylands") &&
       beg_strcasecmp(GET_PARAMETER_STR(params, 1), "-talker"))
     if (p->sps_playing) /* its a player, already playing ? */
     { /* already playing */
      if (!(p2 = player_find_on(p, GET_PARAMETER_STR(params, 1),
                                PLAYER_FIND_VERBOSE |
                                PLAYER_FIND_EXPAND |
                                PLAYER_FIND_NICKS |
                                PLAYER_FIND_PICK_FIRST)))
        return;
      
      if (p2->sps_playing == p->saved)
        /* accepting another offer from someone */
        sps_play_game(p, p2, sps_obj, 1);
      else /* woops, they're trying to offer someone else a game */
      {
       int do_msg = TRUE;
       if (P_IS_ON(p->sps_playing)) /* FIXME: **** blows on nuke */
       {
        LIST_GAME_CHECK_FLAG_START(p->sps_playing->player_ptr, p->saved);
        if (LIST_GAME_CHECK_FLAG_DO(sps))
        {
         fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking "
                       "sps from you.\n", p->sps_playing->name);
         do_msg = FALSE;
        }
        LIST_GAME_CHECK_FLAG_END();
       }
       
       if (do_msg)
         fvtell_player(SYSTEM_T(p->sps_playing->player_ptr),
                       " -=> %s withdraw%s %s offer of a game of "
                       "scissors, paper, stone.\n", p->saved->name,
                       (p->gender == GENDER_PLURAL) ? "" : "s",
                       gender_choose_str(p->gender, "his", "her",
                                         "their", "its"));
       
       fvtell_player(SYSTEM_T(p),
                     " You remove your offer of a game from ^S^B%s^s.\n",
                     p->sps_playing->name);
       
       p->sps_playing = NULL; /* black the person I was offering */
       
       /* blanked the offer, now play the game */
       sps_play_game(p, p2, sps_obj, 0); /* its an offer */
      }
     }
     else
     { /* its a request or an acceptance */
      if (!(p2 = player_find_on(p, GET_PARAMETER_STR(params, 1),
                                PLAYER_FIND_VERBOSE |
                                PLAYER_FIND_EXPAND |
                                PLAYER_FIND_NICKS |
                                PLAYER_FIND_PICK_FIRST)))
        return;
      
      if (p2->sps_playing == p->saved)
        /* its an agreement */
        sps_play_game(p, p2, sps_obj, 1);
      else /* its an offer */
        sps_play_game(p, p2, sps_obj, 0);
     }
   else /* to the computer */
     if (p->flag_sps_ai_disabled &&
         (difftime(now, p->sps_ai_disabled_till) > 0))
       fvtell_player(SYSTEM_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p), "%s",
                     " Thanks anyhow, but $Talker-Name doesn't want to "
                     "play at the moment.\n");
     else
     { /* go to the computer is allowd */
      sps_obj2 = get_computer_choice(p);
      work_out_winner(p, NULL, sps_obj, sps_obj2);
      
      p->flag_sps_ai_disabled = FALSE;
      
      /* find out if they've played CL before and have a sps struct */
      if (!p->sps_cl_mem) /* init the sps struct */
      {
       if ((p->sps_cl_mem = MALLOC(sizeof(sps_struct))))
         memset(p->sps_cl_mem, 0, sizeof(sps_struct));
       else
         /* FIXME: blows on low mem conditions */
         /* NOTE: tmp fix to put this return in */
         return;
      }
      
      /* now they have (malloc worked?) */
      if (p->sps_cl_mem) /* incrememnt the type */
        p->sps_cl_mem->types[(sps_obj - 1)]++;
      
      /* lets you play MAX_SPS_PLAYED_BY_CL per hour on */
      if ((p->sps_cl_mem->types[0] + p->sps_cl_mem->types[1] +
           p->sps_cl_mem->types[2]) >
          (MAX_SPS_PLAYED_BY_CL *
           ((difftime(now, p->logon_timestamp) + MK_HOURS(1)) /
            MK_HOURS(1))))
      {
       p->sps_ai_disabled_till = disp_time_add(now,
                                          TIME_SPS_DISABLED_FOR(random_time));
       p->flag_sps_ai_disabled = TRUE;
      }
     }
 else /* no object */
   TELL_FORMAT(p, "<<player>|-talker> <scissors|paper|stone>");
}

static void user_sps_withdraw(player *p)
{
 player *p2 = 0;

 if (p->sps_playing)
 { /* withdrawing the game offer */
  p2 = p->sps_playing->player_ptr;
  
  p->sps_playing = NULL;
  p->sps_chosen = 0;
  fvtell_player(SYSTEM_T(p),
                " You withdraw your offer of a game from ^S^B%s^s.\n",
                p2->saved->name);
  
  if (p2)
  {
   LIST_GAME_CHECK_FLAG_START(p2, p->saved);
   if (LIST_GAME_CHECK_FLAG_DO(sps))
   {
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking "
                  "sps from you.\n", p2->saved->name);
    return;
   }
   LIST_GAME_CHECK_FLAG_END();
   
   fvtell_player(SYSTEM_T(p2),
                 " -=> %s withdraw%s %s offer of a game of "
                 "scissors, paper, stone.\n",
                 p->saved->name,
                 (p->gender == GENDER_PLURAL) ? "" : "s",
                 gender_choose_str(p->gender, "him", "her",
                                   "them", "it"));
  }
 }
 else
   fvtell_player(SYSTEM_T(p), "%s",
                 " You're not currently offering anyone a game.\n");
}

void user_toggle_sps_brief(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_sps_brief, TRUE,
                       " You will %sget brief scissors, paper"
                       ", stone games.\n",
                       " You will %snot get brief scissors"
                       ", paper, stone games.\n", TRUE);
}

void user_configure_game_sps_use(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, configure.game_sps_use, TRUE,
                       " The sps game is %splayable.\n",
                       " The sps game is %snot playable.\n", TRUE);

 configure_save(FALSE);
}

void cmds_init_sps(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("brief_sps", user_toggle_sps_brief, CONST_CHARS, GAME);
 CMDS_PRIV(configure_game_sps_base);
 
 CMDS_ADD("sps", user_sps, PARSE_PARAMS, GAME);
 CMDS_PRIV(configure_game_sps_base);
 
 CMDS_ADD("sps_withdraw", user_sps_withdraw, NO_CHARS, GAME);
 CMDS_PRIV(configure_game_sps_base);
}
