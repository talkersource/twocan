#ifndef MULTI_BASE_H
#define MULTI_BASE_H

/* if you don't include any player names how big can the group be */
#define MULTI_GROUP_NAME_SIZE (10 + (5 + 8 + 1) \
        + (11 + 20 + PLAYER_S_NAME_SZ) \
        + 22 + 16 + 14 + 12 +    21)

/* max ammount of chars for names of multiple says, Ie.
    player1, player2 and you */
#define MAX_NAMES_IN_MULTI_STRING 256

/* want to be able to have at least one groups and one name */
#if (MULTI_GROUP_NAME_SIZE >= (MAX_NAMES_IN_MULTI_STRING - PLAYER_S_NAME_SZ))
# error " Make MAX_NAMES_IN_MULTI_STRING bigger"
#endif

typedef struct multi_node
{
 struct multi_node *next_multi; /* the next multi the parents entries */
 struct multi_node *prev_multi;
 struct multi_node *next_node; /* the next entry in the multi */
 struct multi_node *prev_node;
 struct player_tree_node *parent;

 unsigned int number;
 unsigned int flags;
} multi_node;

typedef struct multi_return
{
 unsigned int multi_number; /* 0 = failed */
 unsigned int players_added; /* players added this time */
 struct player_tree_node *single_player;
 unsigned int error_number; /* an error code saying what went wrong */
 unsigned int error_multi; /* if died on multi ... the number of */
 const char *error_name; /* if died on player ... the name of */
 unsigned int codes; /* the second parameter to multi_add */
 unsigned int find_flags; /* the third parameter to multi_add */
} multi_return;

#define MULTI_DEFAULT 0

/* multi flags Ie. entry->flags */
#define MULTI_THIS_TIME 1     /* means the person should get it this time */
#define MULTI_TO_SELF (1<<1)       /* they added them self, SAME as below */
#define MULTI_AM_SELF (1<<1)       /* they added them self, SAME as above */
#define MULTI_TO_PERSON (1<<2)     /* player added another person by name */
#define MULTI_TO_FRIENDS (1<<3)    /* person who's friends are being used */
#define MULTI_TO_FRIENDS_OF (1<<4) /* person who's friends are being used */
#define MULTI_TO_ROOM (1<<5)       /* player who added the room */
#define MULTI_TO_SUPERS (1<<6)     /* player who added supers */
#define MULTI_TO_EVERYONE (1<<7)   /* player who added everyone */
#define MULTI_TO_MINISTERS (1<<8)  /* player who added ministers */
#define MULTI_TO_NEWBIES (1<<9)    /* player who added newbies */
#define MULTI_AM_PERSON (1<<10)      /* player was added by name */
#define MULTI_AM_FRIEND (1<<11)     /* player is friend */
#define MULTI_AM_FRIEND_OF (1<<12)  /* player is friend */
#define MULTI_AM_ROOM (1<<13)       /* player is in the room */
#define MULTI_AM_SUPER (1<<14)      /* player is a super user */
#define MULTI_AM_EVERYONE (1<<15)   /* player is on program */
#define MULTI_AM_MINISTER (1<<16)   /* player is a minister */
#define MULTI_AM_NEWBIE (1<<17)     /* player is a newbie */
#define MULTI_OWNER (1<<25)         /* player who stopped the multi */
#define MULTI_TMP_BLOCK (1<<26)     /* player is tmp blocking the multi */
#define MULTI_BLOCKED (1<<27)       /* player is blocking the multi */
#define MULTI_ADDED_THIS_TIME (1<<31) /* they were added to the multi */

/* for ease of checking for any flags */
#define MULTI_GROUP_FLAGS_AM (MULTI_AM_SELF | MULTI_AM_PERSON | MULTI_AM_FRIEND | MULTI_AM_FRIEND_OF | MULTI_AM_MINISTER | MULTI_AM_EVERYONE | MULTI_AM_NEWBIE | MULTI_AM_ROOM | MULTI_AM_SUPER)
#define MULTI_GROUP_FLAGS_TO (MULTI_TO_SELF | MULTI_TO_PERSON | MULTI_TO_FRIENDS | MULTI_TO_FRIENDS_OF | MULTI_TO_SUPERS | MULTI_TO_NEWBIES | MULTI_TO_EVERYONE | MULTI_TO_MINISTERS | MULTI_TO_ROOM)
#define MULTI_GROUP_FLAGS_ALL (MULTI_GROUP_FLAGS_TO | MULTI_GROUP_FLAGS_AM)


/* base flags */

#define MULTI_STOPPED 1       /* multi is stopped */
/* 1 << (1 .. 17) from entry flags */
/* from <add to multi> flags */
/* #define MULTI_COMPLETE_IGNORE (1<<28) */
/* #define MULTI_DESTROY_CLEANUP (1<<29) */
/* #define MULTI_IGNORE_BUT_START (1<<30) */
/* #deifne MULTI_ADDED_THIS_TIME (1<<31)  */

/* flags to pass to <add to multi> function */
#define MULTI_NO_PRIVS_NEEDED 1  /* privs not needed for sus/ministers */
#define MULTI_SPARE (1<<1)
#define MULTI_IGNORE_PLAYERS (1<<2)   /* ignore if they say kill the multi */
#define MULTI_KEEP_GROUP_FLAGS (1<<3) /* eg. when using differnt func calls */
#define MULTI_NO_DO_MATCH (1<<4) /* don't do the match at the end */
#define MULTI_REFRESH (1<<5) /* everyone will get it this time */
#define MULTI_AUTO_STOP (1<<6)  /* stop the multi from being added to later */
#define MULTI_USE_NOT_ON    (1<<7) /* allow ppl to be grouped who arn't on */
/* spare... when the group flags are moved */
#define MULTI_STAY_LOCAL    (1<<17) /* only use people in the room */
#define MULTI_VERBOSE       (1<<18) /* produces general error msgs to player */
#define MULTI_DIE_STOPPED   (1<<19) /* adding to a stopped multi */
#define MULTI_DIE_MATCH_NAME (1<<20) /* can't find a name */
#define MULTI_DIE_MATCH_GROUP (1<<21) /* can't find a group, ie no privs */
#define MULTI_DIE_EMPTY_GROUP (1<<22) /* a group with noone in it */
#define MULTI_DIE_MATCH_MULTI (1<<23) /* you used a multi you weren't on */
#define MULTI_LIVE_ON_SMALL (1<<24)  /* allows only two people in multi */
#define MULTI_NO_COMS_CHECKS (1<<25) /* don't check for block_shouts etc. */
#define MULTI_MUST_CREATE (1<<26) /* always use a new multi */
#define MULTI_KEEP_ALIVE (1<<27) /* keep multi from timing out */
#define MULTI_DESTROY_CLEANUP (1<<28) /* destroy the multi on cleanup */
#define MULTI_COMPLETE_IGNORE (1<<29) /* noone can see it */
#define MULTI_IGNORE_BUT_START (1<<30) /* none, apart from the creator, can see it */

/* <add_to_multi> no groups flags */
#define MULTI_NO_NAMES      (1<<8) /* doesn't support names */
#define MULTI_NO_MINISTERS  (1<<9) /* doesn't support ministers group */
#define MULTI_NO_SUS        (1<<10) /* doesn't support sus group */
#define MULTI_NO_NEWBIES    (1<<11) /* doesn't support newbies group */
#define MULTI_NO_FRIENDS    (1<<12) /* doesn't support friends group */
#define MULTI_NO_FRIENDS_OF (1<<13) /* doesn't support friends of */
#define MULTI_NO_ROOM       (1<<14) /* doesn't support room group */
#define MULTI_NO_EVERYONE   (1<<15) /* doesn't support everyone group */
#define MULTI_NO_MULTIS     (1<<16) /* doesn't support EXTRA multis */

/* <do_inorder_for_multi> and cleanup flags */
#define MULTI_SHOW_ALL 1 /* all the ppl o the multi whether should or not */
/* MULTI_USE_NOT_ON (1<<7) */

/* <get_multi_name> flags */
/* #define MULTI_SHOW_ALL 1 */
/* #define MULTI_TO_SELF (1<<1) */
#define MULTI_USE_EMPTY_GROUPS (1<<3)
#define MULTI_ALL_NAMES (1<<4) /* shows names that are in groups */
#define MULTI_MASK_NAMES (1<<5) /* use a mask out the below NO_groups */
#define MULTI_SPEC_NAMES (1<<6) /* shows names that were specified regardless*/
/* #define MULTI_NO_NAMES      (1<<8) */
/* #define MULTI_NO_MINISTERS  (1<<9) */
/* #define MULTI_NO_SUS        (1<<10) */
/* #define MULTI_NO_NEWBIES    (1<<11) */
/* #define MULTI_NO_FRIENDS    (1<<12) */
/* #define MULTI_NO_FRIENDS_OF (1<<13) */
/* #define MULTI_NO_ROOM       (1<<14) */
/* #define MULTI_NO_EVERYONE   (1<<15) */
/* #define MULTI_NO_MULTIS     (1<<16) *//* better name for MULTI_FORCE_NAMES*/
#define MULTI_POSSESSIVE_OUTPUT (1<<17)

/* for ease of setting all NO_groups flags */
#define MULTI_NO_GROUPS (MULTI_NO_NAMES | MULTI_NO_MINISTERS | MULTI_NO_SUS | MULTI_NO_NEWBIES | MULTI_NO_FRIENDS | MULTI_NO_FRIENDS_OF | MULTI_NO_ROOM | MULTI_NO_EVERYONE)

/* flags to return in error struct */
#define MULTI_BAD_FIND (1<<31)          /* no person in group type found */    
#define MULTI_BAD_SEL (1<<30)     /* group type not supported */

/* codes which also use the above 2 flags */
#define SUCCESS (0)      /* no problems */
#define MULTI_CREATED 1  /* created this time ... not error */

/* previous values (2, 3, 4) were not used therefore removed */

#define MULTI_BAD_IGNORE 5  /* you're blocking/removed self from the multi */
#define MULTI_NO_PEOPLE_ADDED 6 /* one ppl matched only */
#define MULTI_NOT_ENOUGH_PEOPLE 7 /* two ppl matched only */
#define MULTI_STOPPED_ALREADY 8 /* can't be added to as it's stopped */
#define MULTI_NO_BASE_CREATED 9 /* can't create struct */
#define MULTI_NO_ENTRY_CREATED 10 /* can't create struct */

#define MULTI_BAD_EVERYONE_SELECTION (1 | MULTI_BAD_SEL) /* no privs */
#define MULTI_BAD_EVERYONE_FIND (1 | MULTI_BAD_FIND)  /* noone else on */
#define MULTI_BAD_NAME_SELECTION (2 | MULTI_BAD_SEL) /* name not allowed */
#define MULTI_BAD_NAME_FIND (2 | MULTI_BAD_FIND)     /* name not found */
#define MULTI_BAD_MULTI_SELECTION (3 | MULTI_BAD_SEL) /* not on multi */
#define MULTI_BAD_MULTI_FIND (3 | MULTI_BAD_FIND)    /* noone on multi */
#define MULTI_BAD_NEWBIE_SELECTION (4 | MULTI_BAD_SEL) /* not enough privs */
#define MULTI_BAD_NEWBIE_FIND (4 | MULTI_BAD_FIND)    /* no newbies on */
#define MULTI_BAD_MINISTER_SELECTION (5 | MULTI_BAD_SEL)  /* no privs etc. */
#define MULTI_BAD_MINISTER_FIND (5 | MULTI_BAD_FIND) /* no ministers on */
#define MULTI_BAD_SU_SELECTION (6 | MULTI_BAD_SEL)     /* no su priv */
#define MULTI_BAD_SU_FIND (6 | MULTI_BAD_FIND)        /* no sus on */
#define MULTI_BAD_FRIENDS_SELECTION (7 | MULTI_BAD_SEL)    /* not allowed */
#define MULTI_BAD_FRIENDS_FIND (7 | MULTI_BAD_FIND)       /* no friends on */
#define MULTI_BAD_FRIENDS_OF_SELECTION (8 | MULTI_BAD_SEL) /* not allowed */
#define MULTI_BAD_FRIENDS_OF_NAME_FIND (9 | MULTI_BAD_FIND)
                                                   /* friendsof name bad */
#define MULTI_BAD_FRIENDS_OF_NOT_ON_LIST (9 | MULTI_BAD_SEL)
                                               /* you're not on their list */
#define MULTI_BAD_FRIENDS_OF_FIND (10 | MULTI_BAD_FIND)  /* none on */
#define MULTI_BAD_FRIENDS_OF_NOT_ON (10 | MULTI_BAD_SEL)
                                        /* they're not found on the talker */
#define MULTI_BAD_ROOM_SELECTION (11 | MULTI_BAD_SEL)  /* not allowed room */
#define MULTI_BAD_ROOM_FIND (11 | MULTI_BAD_FIND)          /* no one in room */

#ifdef MULTI_BASE_C
      /* for removing group flags */

 typedef struct multi_base
 {
  struct multi_base *next;
  struct multi_base *prev;
  struct multi_node *first_node;
  
  time_t last_used;
  unsigned int number;
  unsigned int flags;
  unsigned int total_players;
 } multi_base;

#define IS_ARRAY_BIG_ENOUGH(x, y, z) (((x) + (y) + 2) < (z))

/* for smallness in add_to_multi */
#define MULTI_DIE_IF_EMPTY(x) do { \
 if (values->codes & MULTI_DIE_EMPTY_GROUP) \
 { \
   if (IS_NEW_MULTI(values)) \
     multi_destroy_list(base->number); \
  values->multi_number = 0; \
  values->error_number = MULTI_BAD_ ## x; \
  values->error_name = temp; \
  return (values); \
 } } while (FALSE)

#define MULTI_DIE_IF_MATCH(x) do { \
 if (values->codes & MULTI_DIE_MATCH_GROUP) \
 { \
   if (IS_NEW_MULTI(values)) \
     multi_destroy_list(base->number); \
  values->multi_number = 0; \
  values->error_number = MULTI_BAD_ ## x; \
  values->error_name = temp; \
  return (values); \
 } } while (FALSE)

#define MULTI_TIMEOUT_SECONDS (60 * 60 * 2) /* timeout after 2 hours */

#endif /* multi_base.c specific */

#endif
