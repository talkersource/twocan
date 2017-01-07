#ifndef DRAUGHTS_H
#define DRAUGHTS_H

/* NOTE: dont' forget to change draughts_game_init */
typedef struct draughts_game
{
 struct player *black; /* challenger */
 struct player *white; /* opponent */

 int black_locations;
 int black_kings;
 int black_moves;
 
 int white_locations;
 int white_kings;
 int white_moves;
 
 int flags;
 time_t last_moved;
 struct player *to_move;

 time_t started_at;

 struct player_linked_list *watchers;
} draughts_game;

/* flags for lose_game, to determine HOW and WHY the player lost */
#define DRAUGHTS_LOST_RESIGNED (1<<0)   /* player resigned the game */
#define DRAUGHTS_LOST_QUIT (1<<1)       /* player quit the talker */
#define DRAUGHTS_LOST_PIECES (1<<2)     /* players pieces were all taken */
#define DRAUGHTS_LOST_TIME (1<<3)       /* player ran out of time */
#define DRAUGHTS_LOST_TRAPPED (1<<4)    /* players pieces are all trapped */

#ifdef DRAUGHTS_C

/* flags for game - internal */
#define DRAUGHTS_PRIVATE_WHITE (1<<0)
#define DRAUGHTS_PRIVATE_BLACK (1<<1)
#define DRAUGHTS_FRIENDLY (1<<2)
#define DRAUGHTS_WHITE_CLOSED (1<<3)
#define DRAUGHTS_BLACK_CLOSED (1<<4)
#define DRAUGHTS_PROPOSED (1<<5)

#define GAME_TYPE(p, caps) ((p)->draughts_as_checkers ? \
			    ((caps) ? "Checkers" : "checkers") : \
			    ((caps) ? "Draughts" : "draughts"))


#define IS_INLINE_WHITE(p, game) (((game)->white == (p)) ? 1 : 0)
#define IS_PLAYER(p, game) (((game)->white == (p)) || ((game)->black == (p)))
#define GET_POINTER(p, game) (IS_INLINE_WHITE((p), (game)) ? game->white : \
                              game->black)
#define GET_ENEMY_POINTER(p, game) (IS_INLINE_WHITE((p), (game)) ? \
                                    game->black : game->white)

/* how many boards are available for players to choose from */
#define NUMBER_OF_BOARDS 5

/* defines for drawing the boards */
#define LINE_OF_LARGE_DRAUGHTS_BOARD "    +---+---+---+---+---+---+---+---+\n"
   
/* longest the other player has to wait for a move before being allowed to
   move */
#define MAX_MOVE (10 * 60) /* 10 mins */

#endif

#endif
