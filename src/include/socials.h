#ifndef SOCIALS_H
#define SOCIALS_H

#ifdef SOCIALS_C

/* For passing information around conveniently, lets have a struct */
typedef struct social_obj
{
 player *p;               /* the player calling the social */
 char *str;         /* the string they passed */
 unsigned int info;       /* the information flags for social options */
 const char *pfirst;      /* the plural string that the social uses */
 const char *pafter;      /* the plural after string that the social uses */
 const char *sfirst;      /* the singular string that the social uses */
 const char *safter;      /* the singular after string that the social uses */
 const char *toself;      /* the no action to self string - optional */
 const char *opt_str;     /* the optional, func returned str, thats used */
 const char *opt_end_str; /* the optional, func returned end str, thats used */
 const char *wrap_pre;    /* the string which precedes any optional string */
 const char *wrap_post;   /* the string which procedes any optional string */
 const char *after_name;  /* the string which goes str8 after the name */
 
 /* all bellow elements get initialised by parse_social - INTERNAL only */
 char *search;      /* used to parse the str and track where we are */

 const char *opt_space;   /* set if there's a space before opt_str*/
 const char *end_space;   /* set if there's a space before opt_end_str*/
 player *spouse;          /* only for when going to marriage channel */
 int parsed_info;         /* information int returned by parse_social */
 int multi_list;          /* the multi list of ppl to go to */
 int sub_multi_list;      /* the sub list of people to go to */
 struct channels_base *chan_base;
} social_obj;

/* Now we need all the #defines for the bit flags for the info ints */

/* This first set is the info ints for defining a social */

#define DEFAULT_SOCIAL 0              /*can be redefined if needed later*/
                                      /*for all socials that have no flags*/

#define ADD_DEFAULTS 0                /*the flags which get automatically*/
                                      /*added to a social - better than above*/

#define NO_NAMES (1<<0)               /*no specific ppl (remotes are ok)*/

#define NO_TFOS (1<<2)                /*no tell_friends_of*/
#define NO_MESSAGE (1<<3)             /*no user start messages to social*/
#define NO_REMOTES (1<<4)             /*no remotes with this social*/
#define NO_LOCAL (1<<5)               /*no locals with this social*/
#define NO_MARRIAGE_CHANNEL (1<<6)    /*don't goto marriage channel*/
#define NO_GEN_CHANNEL (1<<7)       /*don't goto any channels */

#define NO_MULTIS (1<<9)              /*don't goto multi's*/
#define NO_MINISTERS (1<<10)          /*don't allow to the minister grp*/
#define NO_SUS (1<<11)                /*don't allow to the sus grp*/
#define NO_EVERYONE (1<<12)           /*don't allow to the everyone grp*/
#define NO_FRIENDS (1<<13)            /*don't allow to the friends grp*/
#define NO_ROOM (1<<14)               /*don't allow to the room group*/
#define NO_NEWBIES (1<<15)            /*don't allow to the newbies group*/
#define NO_TO_NO_ONE (1<<16)          /*must goto someone at least*/
#define NO_NO_MESSAGE (1<<17)         /*must have a user start message*/
#define NO_NO_END_MESSAGE (1<<18)     /*must have a user end message*/
#define NO_END_MESSAGE (1<<19)        /*don't allow messages at the end*/
#define SOCIAL_DISABLED (1<<20)       /*not working atm*/
#define SOCIAL_PRINT_AFTER (1<<21)    /*print afterstr even when a toself*/
#define SOCIAL_NO_PRINT_FIRST (1<<22) /*no print first string when a toself*/
#define SOCIAL_MOVE_OPT (1<<23)       /*print opt str at opt end*/
#define SOCIAL_MOVE_OPT_END (1<<24)   /*print opt end str at opt*/
#define SOCIAL_SWAP_WRAPPERS (1<<25)  /*change which string wrappers use*/
#define SOCIAL_ASSUME_LNAME (1<<26)   /*assume first param are local names*/
#define SOCIAL_ASSUME_RNAME (1<<27)   /*assume first param are remote names*/
#define SOCIAL_ASSUME_END_STR (1<<28) /*assume first param is the end str*/
#define SOCIAL_ASSUME_OPT_STR (1<<29) /*assume first param is the opt_str*/
#define SOCIAL_LAST_END_STR (1<<30)   /*always assume last is end not opt_str*/
#define SOCIAL_AUTO_SLASH (1<<31)     /*always do an automatic -/ */

/* SOCIAL_REORDER only prints the the end string before the to list when
   its a 'remote' type social - thats a tell, multi or channel */

/* here are the alias's for the info int */
#define NO_CHANNELS (NO_MARRIAGE_CHANNEL | NO_GEN_CHANNEL)

#define NO_SHOUT (NO_EVERYONE)
#define NO_NAMED_GROUPS (NO_SUS | NO_MINISTERS | NO_FRIENDS | NO_EVERYONE)
#define NO_GROUPS (NO_CHANNELS | NO_TFOS | NO_MULTIS | NO_NAMED_GROUPS)
#define NO_USER_MESSAGES (NO_END_MESSAGE | NO_MESSAGE)
#define INACTIVE (NO_NAMES | NO_CHANNELS | NO_TFOS | NO_MESSAGE | \
                  OPTIONAL_MESSAGE | NO_REMOTES | NO_MULTIS | \
		  NO_MINISTERS | NO_SUS | NO_EVERYONE | NO_FRIENDS | \
		  NO_TO_NO_ONE | NO_END_MESSAGE | OPTIONAL_END_MESSAGE)
   /*used for valid command, but nothing works - useless but fun :) */


/* These defines are used in the parsed_info int after parsing */

#define TO_MARRIAGE_CHANNEL (1<<0) /*going to marriage 'channel'*/
#if 0
# define TO_SPOD_CHANNEL (1<<1)     /*going to spod channel*/
# define TO_CRAZY_CHANNEL (1<<2)    /*going to crazy channel*/
# define TO_SUS (1<<3)              /*going to sus channel*/
# define TO_MINISTERS (1<<4)        /*going to the ministers grp*/
# define TO_ADMIN (1<<9)            /*going to the admin channel (NOT USED)*/
# define TO_USER_CHANNEL (1<<12)    /*going to a users channel (NOT USED)*/
#else
# define TO_GEN_CHANNEL (1<<4)        /* going to a generic channel */
#endif
#define TO_EVERYONE (1<<5)         /*going to everyone (really?! wow...)*/
#define TO_FRIENDS (1<<6)          /*going to the players friends*/
#define TO_ROOM (1<<7)             /*going to the room (really?! wow...)*/
#define TO_FRIENDS_OF (1<<8)       /*going to someone's friends*/

#define TO_MULTI (1<<10)           /*its actually a multi we're going to*/
#define TO_NEWBIES (1<<11)         /*going to the newbies*/

#define TO_SELF (1<<13)            /*you actually specified yourself*/
#define TO_SINGLE_REMOTE (1<<14)   /*only going to one remote person*/
#define TO_LIST_OF_REMOTES (1<<15) /*actually a user_list of remote ppl*/
#define TO_LIST_OF_LOCAL (1<<16)   /*not used - assume its going locally*/
#define TO_SPECIAL_GROUP (1<<17)   /*to grp/channel thats not multi supported*/
#define TO_NO_ONE (1<<18)          /*gone to no players so far*/
#define TO_NO_ONE_SPECIFIC (1<<19) /*no sublist has been set*/
#define TO_NAME_CONTROLLER (1<<20) /*has been in the name controller (-)*/
#define HAS_SET_MSG (1<<21)        /*checks if an optional msg has been set*/
#define HAS_SET_END_MSG (1<<22)    /*checks if an optional msg has been set*/

#define FAIL (1<<31)               /*internally when the social fails*/


/* A useful INTERNAL define for the default values for the internal
 * bits of the social_obj struct */
#define INTERNAL NULL, NULL, NULL, NULL, 0, 0, 0, NULL

/* These are used in is_it_a_controller(char *) to check that a controller
 * hasn't been 'invalidated' by being typed twice. Takes a char (in 's) and
 * a pointer to a string. A controllers also invalidated by a space after it
 * except in the case of > and < which allow spaces - NO LONGER USED 
#define CHECK_SINGLE(x, y) if ((x) == *(y)) return (FALSE); else return (TRUE);
#define CHECK_DOUBLE(x, y) if ((x) == *(y)) CHECK_SINGLE((x), ((y) + 1))
#define CHECK_NOT_SPACE(x) if (*((x) + 1) == ' ') return (FALSE);
#define CHECK_CONTROLLER(x, y) CHECK_NOT_SPACE(y) else CHECK_DOUBLE((x), (y))
#define CHECK_MSG(x, y) CHECK_DOUBLE((x), (y))

#define OLD_CHECK_CONTROLLER(x, y) if ((x) == *(y)) CHECK_BOTH(x, ((y) + 1))
*/

/* Useful in making the following macro lesss huge! */
#define END_STUFF NULL, NULL, NULL, NULL, NULL, NULL, INTERNAL

#define SOCIAL(name, info, pf, pa, sf, sa, ts, opt, opt_end, wrapf, wraps) \
static void user_social_ ## name (player *p, const char *str) \
{ \
 social_obj the_social = \
   {NULL, NULL, (ADD_DEFAULTS | info), pf, pa, sf, sa, END_STUFF}; \
 /* Removes snide and uneccessary compiler remarks... */ \
 the_social.p = p; \
 the_social.str = str ? strcpy(socials_copy_input, str) : NULL; \
 the_social.toself = ts; /*the toself string*/ \
 the_social.opt_str = opt; the_social.opt_end_str = opt_end; /*string stuff*/ \
 the_social.wrap_pre = wrapf; the_social.wrap_post = wraps; \
 if (!str) /*the player didn't call the function is this is null*/ \
   social_info(&the_social); \
 else /*they did, so run the social*/ \
   social(&the_social); \
}

/* if a parameter is passed as JUST \ then it is treated the same as a NULL
 * I'm not sure if this is needed, but it can easily be removed */
#define GENERIC_SOCIAL_SETUP(x, y, z) do { \
 ++parameter_number; \
 if (get_parameter_parse(parameters, (const char **)&str, parameter_number)) \
 {  int return_length = GET_PARAMETER_LENGTH(parameters, parameter_number); \
 (x) = GET_PARAMETER_STR(parameters, parameter_number); \
 assert(strnlen(x, return_length + 1) == (unsigned) return_length); \
 if ((*(x) == '0') && !*((x) + 1)) (x) = NULL; else \
 if (!GET_PARAMETER_BUF_UNUSED(parameters)) \
 { \
  fvtell_player(NORMAL_T(p), \
                " Your %s parameter (%s) is too long.\n" \
                " Maximum length of the parameter input should be %lu.\n", \
                (y), (z), (unsigned long)parameters->buffer_size); \
  return; } } else (x) = NULL; } while (FALSE)
   
#define SOCIALS_TOP_OPTIONALS_FUNC(f, t, c) do { \
   if (!(the_social->info & SOCIAL_MOVE_OPT) && the_social->opt_str) \
     f(t, \
       "%s%s%s%s", \
       ((the_social->wrap_pre && \
         (the_social->info & SOCIAL_SWAP_WRAPPERS)) ? \
        the_social->wrap_pre : \
        USE_STRING(the_social->opt_space, \
                   the_social->opt_space)), \
       USE_STRING(the_social->opt_str, \
                  the_social->opt_str), \
       (c), \
       USE_STRING((the_social->wrap_post && \
                   (the_social->info & SOCIAL_SWAP_WRAPPERS)), \
                  the_social->wrap_post)); \
 \
   if ((the_social->info & SOCIAL_MOVE_OPT_END) && the_social->opt_end_str) \
     f(t, \
       "%s%s%s%s", \
       ((the_social->wrap_pre && \
         !(the_social->info & SOCIAL_SWAP_WRAPPERS)) ? \
        the_social->wrap_pre : \
        USE_STRING(the_social->end_space, \
                   the_social->end_space)), \
       USE_STRING(the_social->opt_end_str, \
                  the_social->opt_end_str), \
       (c), \
       USE_STRING((the_social->wrap_post && \
                   !(the_social->info & SOCIAL_SWAP_WRAPPERS)), \
                  the_social->wrap_post)); } while (FALSE)

#define SOCIALS_BOTTOM_OPTIONALS_FUNC(f, t, c) do { \
   if ((the_social->info & SOCIAL_MOVE_OPT) && the_social->opt_str) \
     f(t, \
       "%s%s%s%s", \
       ((the_social->wrap_pre && \
         (the_social->info & SOCIAL_SWAP_WRAPPERS)) ? \
        the_social->wrap_pre : \
        USE_STRING(the_social->opt_space, \
                   the_social->opt_space)), \
       USE_STRING(the_social->opt_str, \
                  the_social->opt_str), \
       (c), \
       USE_STRING((the_social->wrap_post && \
                   (the_social->info & SOCIAL_SWAP_WRAPPERS)), \
                  the_social->wrap_post)); \
 \
 if (!(the_social->info & SOCIAL_MOVE_OPT_END) && the_social->opt_end_str) \
   f(t, \
     "%s%s%s%s", \
     ((the_social->wrap_pre && \
       !(the_social->info & SOCIAL_SWAP_WRAPPERS)) ? \
      the_social->wrap_pre : \
      USE_STRING(the_social->end_space, \
                 the_social->end_space)), \
     USE_STRING(the_social->opt_end_str, \
                the_social->opt_end_str), \
     (c), \
     USE_STRING((the_social->wrap_post && \
                 !(the_social->info & SOCIAL_SWAP_WRAPPERS)), \
                the_social->wrap_post)); } while (FALSE)

#define SOCIALS_CHANNEL_FUNC(p, p_1, p_2, p_3, p_4, p_5, p_6, p_7) \
 do_order_misc_on(internal_socials_channel_1, base->players_start, \
                  p, p_1, p_2); \
 \
 if (!(the_social->info & SOCIAL_MOVE_OPT) && the_social->opt_str) \
   do_order_misc_on(internal_socials_channel_2, base->players_start, \
                    p, \
                    ((the_social->wrap_pre && \
                      (the_social->info & SOCIAL_SWAP_WRAPPERS)) ? \
                     the_social->wrap_pre : \
                     USE_STRING(the_social->opt_space, \
                                the_social->opt_space)), \
                    USE_STRING(the_social->opt_str, \
                               the_social->opt_str), \
                    USE_STRING((the_social->wrap_post && \
                                (the_social->info & SOCIAL_SWAP_WRAPPERS)), \
                               the_social->wrap_post)); \
 \
 if ((the_social->info & SOCIAL_MOVE_OPT_END) && the_social->opt_end_str) \
   do_order_misc_on(internal_socials_channel_3, base->players_start, \
                    p, \
                    ((the_social->wrap_pre && \
                      !(the_social->info & SOCIAL_SWAP_WRAPPERS)) ? \
                     the_social->wrap_pre : \
                     USE_STRING(the_social->end_space, \
                                the_social->end_space)), \
                    USE_STRING(the_social->opt_end_str, \
                               the_social->opt_end_str), \
                    USE_STRING((the_social->wrap_post && \
                                !(the_social->info & SOCIAL_SWAP_WRAPPERS)), \
                               the_social->wrap_post)); \
 \
 do_order_misc_on(internal_socials_channel_4, base->players_start, \
                  p, p_3, p_4, p_5, p_6); \
 \
 if ((the_social->info & SOCIAL_MOVE_OPT) && the_social->opt_str) \
   do_order_misc_on(internal_socials_channel_5, base->players_start, \
                    p, \
                    ((the_social->wrap_pre && \
                      (the_social->info & SOCIAL_SWAP_WRAPPERS)) ? \
                     the_social->wrap_pre : \
                     USE_STRING(the_social->opt_space, \
                                the_social->opt_space)), \
                    USE_STRING(the_social->opt_str, \
                               the_social->opt_str), \
                    USE_STRING((the_social->wrap_post && \
                                (the_social->info & SOCIAL_SWAP_WRAPPERS)), \
                               the_social->wrap_post)); \
     \
 if (!(the_social->info & SOCIAL_MOVE_OPT_END) && the_social->opt_end_str) \
   do_order_misc_on(internal_socials_channel_6, base->players_start, \
                    p, \
                    ((the_social->wrap_pre && \
                      !(the_social->info & SOCIAL_SWAP_WRAPPERS)) ? \
                     the_social->wrap_pre : \
                     USE_STRING(the_social->end_space, \
                                the_social->end_space)), \
                    USE_STRING(the_social->opt_end_str, \
                               the_social->opt_end_str), \
                    USE_STRING((the_social->wrap_post && \
                                !(the_social->info & SOCIAL_SWAP_WRAPPERS)), \
                               the_social->wrap_post)); \
 \
 do_order_misc_on(internal_socials_channel_7, base->players_start, \
                  p, p_7)


#define SOCIALS_FULL_STOP_CHECK \
USE_STRING((!(the_social->opt_str || \
              the_social->opt_end_str) || \
            ((!the_social->opt_str || (the_social->opt_str && \
              !(the_social->info & SOCIAL_MOVE_OPT))) && \
             (!the_social->opt_end_str || (the_social->opt_end_str && \
              (the_social->info & SOCIAL_MOVE_OPT_END))) && \
             !(the_social->parsed_info & TO_NO_ONE_SPECIFIC))), \
           ".")

#endif

/* Now we need the ACTUAL social definitions themselves, these are included
 * in the socials_data.h file (from socials.c). */

#endif /*SOCIALS_H*/
