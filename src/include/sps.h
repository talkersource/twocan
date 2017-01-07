#ifndef SPS_H
#define SPS_H

typedef struct sps_struct
{
 int types[3];
 int amount_at_last;
} sps_struct;

#ifdef SPS_C

/* defines for each type */
#define SCISSORS 1
#define PAPER 2
#define STONE 3

/* how many games crazylands will play (per hour) before disabling you */
#define MAX_SPS_PLAYED_BY_CL 25

/* maximum hours you can get added to your disabled time */
#define RANDOM_SPS_HOURS 15
/* time that it gets disabled for after you've over played the game */
#define TIME_SPS_DISABLED_FOR(rnd_time) (MK_HOURS(15) + MK_HOURS(rnd_time))

/* defines for each of the pictures */
         
#define PAPER_L1 " ___ " 
#define PAPER_L2 "| ..L\\"
#define PAPER_L3 "| ... |"
#define PAPER_L4 "| ... |"
#define PAPER_L5 "|_____|"

#define SCISSORS_L1 ""
#define SCISSORS_L2 "O   O"
#define SCISSORS_L3 " \\./" 
#define SCISSORS_L4 " / \\"
#define SCISSORS_L5 "/   \\"

#define STONE_L1 " "
#define STONE_L2 "   ___"
#define STONE_L3 " _'   `_"
#define STONE_L4 "'  .'   `."
#define STONE_L5 "`___,_._,'"

/* array for quickly referencing which line is required */
const char *sps_pictures[5][3] = {{SCISSORS_L1, PAPER_L1, STONE_L1},
                                  {SCISSORS_L2, PAPER_L2, STONE_L2},
                                  {SCISSORS_L3, PAPER_L3, STONE_L3},
                                  {SCISSORS_L4, PAPER_L4, STONE_L4},
                                  {SCISSORS_L5, PAPER_L5, STONE_L5}};

/* array for working out who won, from lefts point of view. 0 = draw,
   1 = win and -1 = lose */
const int sps_win_table[3][3] = {{0, 1, -1}, {-1, 0, 1}, {1, -1, 0}};

/* gets the current line being asked for for the picture */
#define GET_LINE(type, line) (sps_pictures[(line - 1)][(type - 1)])

/* gets the string for the type */
#define GET_SPS_STRING(type, caps) ((type == SCISSORS) ? \
                                (caps ? "Scissors" : "scissors") : \
                                 ((type == PAPER) ? \
                                  (caps ? "Paper" : "paper") : \
                                  ((type == STONE) ? \
                                   (caps ? "Stone" : "stone") : "ERROR")))
   
#endif /* sps_c */

#endif /* sps_h */
