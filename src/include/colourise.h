#ifndef COLOURISE_H
#define COLOURISE_H

#ifdef COLOURISE_C
/* turn colour off */
# define SYS_COLOUR(x) (x)->flag_no_colour_from_others = TRUE
/* turn wands off */
# define SYS_SPECIALS(x) (x)->flag_no_specials_from_others = TRUE
/* both off */
# define WANDS_OFF(x) do { \
 (x)->flag_no_colour_from_others = TRUE; \
 (x)->flag_no_specials_from_others = TRUE; \
 (x)->flag_just_normal_hilight = TRUE; } while (FALSE)
     
/* both on... use WANDS_ON(p); SYS_COLOUR(p); to turn just wands on etc.. */
# define WANDS_ON(x) do { \
 (x)->flag_no_colour_from_others = FALSE; \
 (x)->flag_no_specials_from_others = FALSE; \
 (x)->flag_just_normal_hilight = FALSE; } while (FALSE)
#endif

#define USER_COLOUR_SAY        "^R00" /* room msg */
#define OUTPUT_COLOUR_SAY          0
#define USER_COLOUR_ECHO       "^R01" /* echo msg */
#define OUTPUT_COLOUR_ECHO         1
#define USER_COLOUR_TELL       "^R02" /* personal msg */
#define OUTPUT_COLOUR_TELL         2
#define USER_COLOUR_TFRIENDS   "^R03" /* friends msg */
#define OUTPUT_COLOUR_TFRIENDS     3
#define USER_COLOUR_TFOF       "^R04" /* friends of */
#define OUTPUT_COLOUR_TFOF         4
#define USER_COLOUR_OLD_SPOD "^R05" /* old spod chan -- inform */
#define OUTPUT_COLOUR_OLD_SPOD 5
#define USER_COLOUR_OLD_SUS "^R06" /* old su chan -- format */
#define OUTPUT_COLOUR_OLD_SUS 6
#define USER_COLOUR_MINE       "^R07" /* your tells */
#define OUTPUT_COLOUR_MINE         7
#define USER_COLOUR_OLD_MAIN_CHAN "^R08" /* old main chan */
#define OUTPUT_COLOUR_OLD_MAIN_CHAN 8
#define USER_COLOUR_SHOUTS     "^R09" /* shouts */
#define OUTPUT_COLOUR_SHOUTS       9
#define USER_COLOUR_SOCIALS    "^R10" /* socials */
#define OUTPUT_COLOUR_SOCIALS     10
#define USER_COLOUR_SOCIALS_ME "^R11" /* socials to me */
#define OUTPUT_COLOUR_SOCIALS_ME  11
#define USER_COLOUR_DRAUGHTS   "^R12" /* draughts / checkers */
#define OUTPUT_COLOUR_DRAUGHTS    12
#define USER_COLOUR_RECHO      "^R13" /* recho */
#define OUTPUT_COLOUR_RECHO       13
#define USER_COLOUR_MARRIAGE   "^R14" /* marriage channel */
#define OUTPUT_COLOUR_MARRIAGE    14
#define USER_COLOUR_CHAN_1     "^R15" /* user channels colour */
#define OUTPUT_COLOUR_CHAN_1      15
#define USER_COLOUR_CHAN_2     "^R16" /* user channels colour */
#define OUTPUT_COLOUR_CHAN_2      16
#define USER_COLOUR_CHAN_3     "^R17" /* user channels colour */
#define OUTPUT_COLOUR_CHAN_3      17
#define USER_COLOUR_CHAN_4     "^R18" /* user channels colour */
#define OUTPUT_COLOUR_CHAN_4      18
#define USER_COLOUR_CHAN_5     "^R19" /* user channels colour */
#define OUTPUT_COLOUR_CHAN_5      19
#define USER_COLOUR_CHAN_6     "^R20" /* user channels colour */
#define OUTPUT_COLOUR_CHAN_6      20
#define USER_COLOUR_CHAN_7     "^R21" /* user channels colour */
#define OUTPUT_COLOUR_CHAN_7      21
#define USER_COLOUR_CHAN_8     "^R22" /* user channels colour */
#define OUTPUT_COLOUR_CHAN_8      22
/* leave some spare upto ... 46 (32 seperate colours if you want to) ...
 * Also need to add in table view of colours */
#define OUTPUT_COLOUR_SZ 23

#define EVERYTHING_OFF 0

/* colours... */
#define FOREGROUND_ONE 1
#define FOREGROUND_TWO 2
#define FOREGROUND_THREE 3
#define FOREGROUND_FOUR 4
#define FOREGROUND_FIVE 5
#define FOREGROUND_SIX 6
#define FOREGROUND_SEVEN 7
#define FOREGROUND_EIGHT 8
#define FOREGROUND_ALL 15
#define FOREGROUND_OFF ~FOREGROUND_ALL

/* used bottom 4 bits... do same for background using next 4 bits */
#define BACKGROUND_ONE (FOREGROUND_ONE << 4)
#define BACKGROUND_TWO (FOREGROUND_TWO << 4)
#define BACKGROUND_THREE (FOREGROUND_THREE << 4)
#define BACKGROUND_FOUR (FOREGROUND_FOUR << 4)
#define BACKGROUND_FIVE (FOREGROUND_FIVE << 4)
#define BACKGROUND_SIX (FOREGROUND_SIX << 4)
#define BACKGROUND_SEVEN (FOREGROUND_SEVEN << 4)
#define BACKGROUND_EIGHT (FOREGROUND_EIGHT << 4)
#define BACKGROUND_ALL (FOREGROUND_ALL << 4)
#define BACKGROUND_OFF ~BACKGROUND_ALL

/* wands... as they are called... using above the first 8 bits... */
#define BOLD_ON (1<<8)
#define BOLD_OFF (~(1<<8))
#define FLASHING_ON (1<<9)
#define FLASHING_OFF (~(1<<9))
#define UNDERLINE_ON (1<<10)
#define UNDERLINE_OFF (~(1<<10))
#define INVERSE_ON (1<<11)
#define INVERSE_OFF (~(1<<11))
#define DIM_ON (1<<12)
#define DIM_OFF (~(1<<12))

#define SAVE_OUTPUT_TYPE (1<<28)
#define RESTORE_OUTPUT_TYPE (~(1<<28))
#define CLEAR_OUTPUT (1<<29)
#define RESET_OUTPUT (1<<30)

#endif
