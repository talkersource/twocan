#ifndef HANGMAN_H
#define HANGMAN_H
/*
 * EW-too base code addition by PeeKaBoo
 * --------------------------------------------------------------------------
 *
 * Hangman v2.0
 * Header file
 */

#ifdef HANGMAN_C

/* File that the words are contained in */
#define WORDFILE "files/hangman.words"

/* The number of misses in a game */
#define MISSES 5

/* If a monetary system is not present in your code, remove this line.
   
   If you have currency, but do not have soft messages, you could 
   use something like this instead: 
   #define MONEY "shillings"
   
   Finally, if you have a monetary system, but the player struct uses
   something different than 'pennies' (ie. p->pennies) then you will need
   to change all instances of it in hangman.c.
*/

/* #define HAVE_MONETRY_SYSTEM */

#ifdef HAVE_MONETRY_SYSTEM
# define MONEY get_config_msg("cash_name")
#endif

#define HANGMAN_MAGIC_TEST(x) (1 << (((int) (x)) - 97))
#define HANGMAN "hangman"

#ifdef TWOCAN_CODE

# define HIGH "^B"
# define OFF "^N"
# define YELLOW "^4"
# define GREEN "^3"
# define PURPLE "^6"
# define ESCAPE "^"

#else

# define HIGH "^H"
# define OFF "^N"
# define YELLOW "^Y"
# define GREEN "^G"
# define PURPLE "^P"
# define ESCAPE "^"

#endif

#endif

typedef struct hangman_type
{
 char *word;
 int letters;
 int bet;
 int misses;
} hangman_type;

#endif
