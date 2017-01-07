#define SOCIALS_C
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

static char socials_copy_input[INPUT_BUFFER_SZ];
/*
 * All the socials are composed by the SOCIAL macro and are in the .h file
 * All socials call master_social after being parsed
 * All socials call do_social_TYPE from master_social
 */



/* functions... */


/* social_info functions for returning information about ONE social */
#define SOCIAL_SUPPORT_NY(x) (the_social->info & (x) ? "NO" : "YES")
static void social_info(social_obj *the_social)
{
 player *p = the_social->p;

 ptell_left(NORMAL_T(p), "Social Info", TRUE);
 
 if (the_social->info == DEFAULT_SOCIAL)
   fvtell_player(SYSTEM_T(p),
		 " The '^S^B%s^s' social supports all the social "
		 "functions/controllers.\n",
		 current_command);
 else
 {
  int ret = FALSE;
  
  fvtell_player(NORMAL_T(p),
		" The '^S^B%s^s' social supports the following -=>\n"
		" %-18s%15s | %-18s%15s\n"
		" %-18s%15s | %-18s%15s\n"
		" %-18s%15s | %-18s%15s\n"
		" %-18s%15s | %-18s%15s\n"
		" %-18s%15s | %-18s%15s\n"
                " %-18s%15s | %-18s%15s\n"
                " %-18s%15s | %-18s%15s\n"
		" %-18s%15s",
		current_command,
		"Room Socials",      SOCIAL_SUPPORT_NY(NO_LOCAL),
		"Multis",            SOCIAL_SUPPORT_NY(NO_MULTIS),
		"Remote Socials",    SOCIAL_SUPPORT_NY(NO_REMOTES),
		"Everyone Group",    SOCIAL_SUPPORT_NY(NO_EVERYONE),
		"Start Messages",    SOCIAL_SUPPORT_NY(NO_MESSAGE),
		"Room Group",        SOCIAL_SUPPORT_NY(NO_ROOM),
		"End Messages",      SOCIAL_SUPPORT_NY(NO_END_MESSAGE),
		"Friends Group",     SOCIAL_SUPPORT_NY(NO_FRIENDS),
		"Individual Names",  SOCIAL_SUPPORT_NY(NO_NAMES), 
		"Friendsof Group",   SOCIAL_SUPPORT_NY(NO_TFOS),
		"Marriage Channel",  SOCIAL_SUPPORT_NY(NO_MARRIAGE_CHANNEL),
		"Socials To No One", SOCIAL_SUPPORT_NY(NO_TO_NO_ONE),
                "Move Pre String",   SOCIAL_SUPPORT_NY(SOCIAL_MOVE_OPT), 
                "Move Post String",  SOCIAL_SUPPORT_NY(SOCIAL_MOVE_OPT_END),
                "Swap Opt Strings",  SOCIAL_SUPPORT_NY(SOCIAL_SWAP_WRAPPERS));
  
  if (p->saved->priv_base)
  {
   fvtell_player(NORMAL_T(p),
                 "%s %-18s%15s", ret ? "\n" : " |",
                 "Channels",     SOCIAL_SUPPORT_NY(NO_GEN_CHANNEL));
   ret = ret ? FALSE : TRUE;
  }

  if (p->saved->priv_minister) /*ministers group*/
  {
   fvtell_player(NORMAL_T(p),
                 "%s %-18s%15s", ret ? "\n" : " |",
                 "Ministers Group",   SOCIAL_SUPPORT_NY(NO_MINISTERS));
   ret = ret ? FALSE : TRUE;
  }
  
  if (PRIV_STAFF(p->saved)) /*su and newbie groups*/
  {
   fvtell_player(NORMAL_T(p),
                 "%s %-18s%15s", ret ? "\n" : " |",
                 "Sus Group",        SOCIAL_SUPPORT_NY(NO_SUS));
   ret = ret ? FALSE : TRUE;
   fvtell_player(NORMAL_T(p),
                 "%s %-18s%15s", ret ? "\n" : " |",   
		 "Newbies Group",      SOCIAL_SUPPORT_NY(NO_NEWBIES));
  }

  fvtell_player(NORMAL_T(p), "%s", "\n");
 }

 if (the_social->toself)
 {
  if (*the_social->toself)
  {
   fvtell_player(NORMAL_T(p),
                 " The social has a 'toself' message which reads: %s\n",
                 the_social->toself);
   if ((the_social->info & SOCIAL_NO_PRINT_FIRST) &&
       (the_social->pfirst))
     fvtell_player(NORMAL_T(p),
                   " You will not see '%s' before the 'toself' message.\n",
                   the_social->pfirst);
  }
  else
    fvtell_player(NORMAL_T(p),
                  " You are able to just '%s' if you don't "
                  "'%s' with reference to someone.\n", current_command,
                  current_command);
 }
 
 if ((the_social->info & (SOCIAL_ASSUME_LNAME)) && 
     !(the_social->info & (NO_NAMES)))
   fvtell_player(NORMAL_T(p), "%s",
                 " The social assumes any plain string to be a "
                 "local name list.\n");
 else if ((the_social->info & (SOCIAL_ASSUME_RNAME)) && 
          !(the_social->info & (NO_REMOTES)))
   fvtell_player(NORMAL_T(p), "%s",
                 " The social assumes any plain string to be a "
                 "remote name list.\n");
 else if ((the_social->info & (SOCIAL_ASSUME_END_STR)) && 
          !(the_social->info & (NO_END_MESSAGE)))
   fvtell_player(NORMAL_T(p), "%s",
                 " The social assumes any plain string to be an "
                 "end message.\n");
 else if ((the_social->info & (SOCIAL_ASSUME_OPT_STR)) && 
          !(the_social->info & (NO_MESSAGE)))
   fvtell_player(NORMAL_T(p), "%s",
                 " The social assumes any plain string to be a "
                 "start message.\n");

 if (the_social->opt_str)
   if (!(the_social->info & NO_MESSAGE))
     fvtell_player(NORMAL_T(p), "%s",
                   " The social has an optional start message");
   else
     fvtell_player(NORMAL_T(p), "%s",
                   " The social has an start message");
 else
   if (the_social->opt_end_str)
     if (!(the_social->info & NO_END_MESSAGE))
       fvtell_player(NORMAL_T(p), "%s",
                     " The social has an optional end message.\n"); 
     else
       fvtell_player(NORMAL_T(p), "%s",
                     " The social has an end message.\n");
 else
   fvtell_player(NORMAL_T(p), "%s",
                 " The social has no optional messages.\n");

 if (the_social->opt_end_str)
 {
  if (the_social->opt_str)
  {
   if (!(the_social->info & NO_END_MESSAGE))
     fvtell_player(NORMAL_T(p), "%s",
                   " and an optional end message.\n");
   else
     fvtell_player(NORMAL_T(p), "%s",
                   " and an end message.\n");
  }
 }
 else
   if (the_social->opt_str)
     fvtell_player(NORMAL_T(p), "%s", ".\n");
 
 if (the_social->wrap_pre || the_social->wrap_post)
   fvtell_player(NORMAL_T(p),
                 " Optional strings are wrapped by '%s' first and "
                 "'%s' after.\n",
                 (the_social->wrap_pre ? 
                  the_social->wrap_pre : "nothing"),
                 (the_social->wrap_post ? 
                  the_social->wrap_post : "nothing"));
 
 ptell_left(NORMAL_T(p), "End of Social Info", TRUE);
}

static void user_toggle_social_auto_name(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_social_auto_name, TRUE,
                       " You will %sget the name controller for "
                       "socials added automatically.\n",
                       " You %swon't get the name controller for "
                       "socials added automatically.\n",
                       TRUE);
}

/* cleans up the multis in the social should they exist */
static void cleanup_social(social_obj *the_social)
{
 if ((the_social->multi_list) && !(the_social->parsed_info & TO_SPECIAL_GROUP))
   multi_cleanup(the_social->multi_list, MULTI_DEFAULT);

 if (the_social->sub_multi_list &&
     (the_social->sub_multi_list != the_social->multi_list))
   multi_cleanup(the_social->sub_multi_list, MULTI_DEFAULT);
}


/* effectively deals with problems in runnign the social */
static int handle_social_error(social_obj *the_social, const char *error)
{
 assert(FALSE);
 fvtell_player(SYSTEM_T(the_social->p),
	       " An error has occured in the program whilst trying "
	       "to run the social. Sorry, try again.\n");
 vwlog("error", "Error whilst running %s - %s.", current_command, error);

 the_social->parsed_info = (FAIL);

 /*cleanup and multis that exist etc...*/
 cleanup_social(the_social);
 
 return (FALSE);
}

#define FIRST_STR 1
#define AFTER_STR 2
#define TO_OTHER 1
#define TO_OWNER 2
static const char *get_action_str(social_obj *the_social, int which, int to)
{
 player *p = the_social->p;
 
 if (which == FIRST_STR)
   if (p->gender == GENDER_PLURAL)
     return (the_social->pfirst); /* The lemmings WAVE at */
   else
     if (to == TO_OTHER)
       return (the_social->sfirst); /* Fred WAVES at */
     else
       return (the_social->pfirst); /* You WAVE at */
 else if (which == AFTER_STR)
   if (p->gender == GENDER_PLURAL)
     return (the_social->pafter); /* The lemmings BOP */
   else
     if (to == TO_OTHER)     
       return (the_social->safter); /* Fred BOPS */
     else
       return (the_social->pafter); /* You BOP */
 else
 {
  assert(FALSE);
  vwlog("error", "Wrong string requested in social %s - %d",
       current_command, which);
  return ("");
 }
}

#if 0
/* inorder chan social does the work for SU or ADMIN (not used) channels */
static int inorder_chan_social(multi_node *entry, va_list va)
{
 social_obj *the_social = va_arg(va, social_obj *);
 const char *start_chan = va_arg(va, const char *);
 const char *end_chan = va_arg(va, const char *);
 const char *chan_colour = va_arg(va, const char *);
 
 twinkle_info info;
 player *p = the_social->p;
 player_tree_node *sp = entry->parent;
 player *p2 = sp->player_ptr;
 int sub_multi = the_social->sub_multi_list;
 const char *first_other = get_action_str(the_social, FIRST_STR, TO_OTHER);
 const char *after_other = get_action_str(the_social, AFTER_STR, TO_OTHER);
  
 /*no need for check receive as its illeagal to block the sus*/

 /*check for off duty however*/
 if (p2->flag_tmp_su_channel_block)
   return (TRUE);

 /*setup the conjugate twinkle stuff in info*/
 setup_twinkle_info(&info);
 info.output_not_me = TRUE;
 
 /*do the first bit of the channel*/
 fvtell_player(ALL_T(p->saved, p2, &info, 3, now), 
	       "%s%s%s", chan_colour, start_chan, p->saved->name);
 
 /*do the second bit of the channel - lots of ifs more readable than one
   huge inline if as would be needed*/

 if (!sub_multi) /*its just to you effectively*/
   if (the_social->toself && /*if there's a special message*/
       !(the_social->opt_str && the_social->opt_end_str))
   {
    fvtell_player(ALL_T(p->saved, p2, &info, 3, now),
                  "%s%s",
                  USE_STRING((first_other && /* action */
                              !(the_social->info & SOCIAL_NO_PRINT_FIRST)),
                             " "),
                  USE_STRING((first_other &&
                              !(the_social->info & SOCIAL_NO_PRINT_FIRST)),
                             first_other));

    SOCIALS_TOP_OPTIONALS_FUNC(fvtell_player,
                               ALL_T(p->saved, p2, &info, 3, now),
                               chan_colour);
    
    fvtell_player(ALL_T(p->saved, p2, &info, 3, now),
                  "%s%s%s",
                  USE_STRING((!first_other || /* after string */
                              (the_social->info & SOCIAL_PRINT_AFTER)) &&
                             after_other, " "),
                  USE_STRING((!first_other || 
                              (the_social->info & SOCIAL_PRINT_AFTER)) &&
                             after_other, after_other),
                  the_social->toself); /* to self */
    
    SOCIALS_BOTTOM_OPTIONALS_FUNC(fvtell_player,
                                  ALL_T(p->saved, p2, &info, 3, now),
                                  chan_colour);

    fvtell_player(ALL_T(p->saved, p2, &info, 3, now),
                  "%s%s^N\n",
                  chan_colour, end_chan);
   }
   else /*just a normal 'waves at himself' type message*/
   {
    fvtell_player(ALL_T(p->saved, p2, &info, 3, now),
                  "%s%s",
                  USE_STRING(first_other, " "), /* action */
                  USE_STRING(first_other, first_other));
    
    SOCIALS_TOP_OPTIONALS_FUNC(fvtell_player,
                               ALL_T(p->saved, p2, &info, 3, now),
                               chan_colour);
    
    fvtell_player(ALL_T(p->saved, p2, &info, 3, now),
                  "%s%s%s",
                  USE_STRING(after_other, " "), /* after str */
                  USE_STRING(after_other, after_other),
                  gender_choose_str(p->gender, "himself", "herself",
                                    "themselves", "itself"));
    
    SOCIALS_BOTTOM_OPTIONALS_FUNC(fvtell_player,
                                  ALL_T(p->saved, p2, &info, 3, now),
                                  chan_colour);
    
    fvtell_player(ALL_T(p->saved, p2, &info, 3, now),
                  "%s%s^N\n",
                  chan_colour, end_chan);
   }
 else /*send the 'social to whoever' type message*/
 {
  fvtell_player(ALL_T(p->saved, p2, &info, 3, now),
                "%s%s",
                USE_STRING(first_other, " "), /* action */
                USE_STRING(first_other, first_other));
  
  SOCIALS_TOP_OPTIONALS_FUNC(fvtell_player,
                             ALL_T(p->saved, p2, &info, 3, now),
                             chan_colour);
  
  fvtell_player(ALL_T(p->saved, p2, &info, 3, now),
                "%s%s %s",
                USE_STRING(after_other, " "), /* after string */
                USE_STRING(after_other, after_other),		 
                multi_get_names(sub_multi, entry, p->saved, NULL, NULL, 
                                (MULTI_NO_MULTIS|MULTI_TO_SELF))); /* to */

  SOCIALS_BOTTOM_OPTIONALS_FUNC(fvtell_player,
                                ALL_T(p->saved, p2, &info, 3, now),
                                chan_colour);
  
  fvtell_player(ALL_T(p->saved, p2, &info, 3, now),
                "%s%s^N\n",
                chan_colour, end_chan);
 }
 
 return (TRUE); /* have to return true to make do_inorder continue */
}
#endif

/* inorder social does the real work - gets each player from the social list */
static int inorder_social(multi_node *entry, va_list va)
{
 social_obj *the_social = va_arg(va, social_obj *);

 player *p = the_social->p;
 player_tree_node *sp = entry->parent;
 player *p2 = sp->player_ptr;
 int sub_multi = the_social->sub_multi_list;
 int multi = the_social->multi_list;
 int multi_name_flags_main = (MULTI_TO_SELF|MULTI_NO_MULTIS);
 int multi_name_flags_main_sub = (MULTI_TO_SELF|MULTI_NO_MULTIS);
 int multi_name_flags_list = (MULTI_NO_MULTIS);
 int show_multi_num = 1;
 int show_people_list = 0;
 int remote_social = 0;
 int sub_players = 0;
 multi_node *sub_entry = 0;
 const char *first_owner = get_action_str(the_social, FIRST_STR, TO_OWNER);
 const char *after_owner = get_action_str(the_social, AFTER_STR, TO_OWNER);
 const char *first_other = get_action_str(the_social, FIRST_STR, TO_OTHER);
 const char *after_other = get_action_str(the_social, AFTER_STR, TO_OTHER);
 const char *multi_num = multi_get_number(entry);

 /*if there's a sub multi, we need the entry for the player we're going to*/
 if (sub_multi)
   sub_entry = multi_find_entry(entry->parent, sub_multi);

 /*find out if there's a valid multi_number for this person*/
 if (multi_num && !*multi_num)
   show_multi_num = 0;

 /*check if we should show it anyhow...*/
 if (the_social->parsed_info & TO_SINGLE_REMOTE)
   show_multi_num = 0;
 
 /*check to see if we show the people list*/
 if ((multi) && !(the_social->parsed_info & (TO_NO_ONE|TO_LIST_OF_LOCAL)))
 {
  remote_social = 1; /*its a remote social*/
  show_people_list = 1; /*its not just going to the room*/

  /*find out if we want to print a 'report' list, if the multis are the same*/
  if (multi && sub_multi)
  {
   if (multi_compare(multi, sub_multi))
     /* there's no need to, cause the lists are the same*/
     show_people_list = 0;
   else
     sub_players = multi_count_players(sub_multi);
  }
 }

 /*if theres 1 on the remote, its u bud, so don't show the people list*/
 if (multi_count_players(the_social->multi_list) == 1)
   show_people_list = 0;
 
 if (p2 == p) /*its the player who is socialising*/
   if (the_social->parsed_info & TO_NO_ONE_SPECIFIC) /*no sublist*/
     if (the_social->toself && /*if there's a special message*/
         !(the_social->opt_str && the_social->opt_end_str))
     {
      fvtell_player(NORMAL_T(p),
		    "%s%sYou%s%s%s",
		    USER_COLOUR_MINE,
		    (show_multi_num ? multi_num : " "),
		    (p->gender == GENDER_PLURAL ? " all" : ""),
		    USE_STRING((first_owner &&
                                !(the_social->info & SOCIAL_NO_PRINT_FIRST)),
                               " "),
		    USE_STRING((first_owner &&
                                !(the_social->info & SOCIAL_NO_PRINT_FIRST)),
                               first_owner));
      
      SOCIALS_TOP_OPTIONALS_FUNC(fvtell_player, NORMAL_T(p), USER_COLOUR_MINE);
      
      fvtell_player(NORMAL_T(p),
                    "%s%s%s",
                    (first_owner ?
                     (the_social->info & SOCIAL_PRINT_AFTER ?
                      USE_STRING(after_owner, " ") : "") :
                     USE_STRING(after_owner, " ")),
                    (first_owner ?
                     (the_social->info & SOCIAL_PRINT_AFTER ?
                      USE_STRING(after_owner, after_owner) : "") :
                     USE_STRING(after_owner, after_owner)),
                    the_social->toself);

      SOCIALS_BOTTOM_OPTIONALS_FUNC(fvtell_player, NORMAL_T(p),
                                    USER_COLOUR_MINE);
      
      fvtell_player(NORMAL_T(p),
                    "%s%s%s%s^N\n",
                    SOCIALS_FULL_STOP_CHECK,
		    USE_STRING(show_people_list, " (to "),
                    USE_STRING(show_people_list,
                               multi_get_names(multi, entry, p->saved, NULL,
                                               NULL, multi_name_flags_list)),
		    USE_STRING(show_people_list, ")"));
     }
     else /*just a normal 'waves at himself' type message*/
     {
      fvtell_player(NORMAL_T(p),
		    "%s%sYou%s%s%s",
		    USER_COLOUR_MINE,
		    (show_multi_num ? multi_num : " "),
		    (p->gender == GENDER_PLURAL ? " all" : ""),
                    USE_STRING(first_owner, " "),
                    USE_STRING(first_owner, first_owner));
      
      SOCIALS_TOP_OPTIONALS_FUNC(fvtell_player, NORMAL_T(p), USER_COLOUR_MINE);
      
      fvtell_player(NORMAL_T(p),
                    "%s%s yoursel%s",
                    USE_STRING(after_owner, " "),		    
                    USE_STRING(after_owner, after_owner),
                    (p->gender == GENDER_PLURAL ? "ves" : "f"));
      
      SOCIALS_BOTTOM_OPTIONALS_FUNC(fvtell_player, NORMAL_T(p),
                                    USER_COLOUR_MINE);      
      
      fvtell_player(NORMAL_T(p),
                    "%s%s%s%s^N\n",
                    SOCIALS_FULL_STOP_CHECK,
		    USE_STRING(show_people_list, " (to "),
                    USE_STRING(show_people_list,
                               multi_get_names(multi, entry, p->saved, NULL,
                                               NULL,multi_name_flags_list)),
		    USE_STRING(show_people_list, ")"));
     }
   else /*send the 'You social to whoever' type message*/
   {    
    fvtell_player(NORMAL_T(p),
		  "%s%sYou%s%s%s",
		  USER_COLOUR_MINE,
		  (show_multi_num ? multi_num : " "),
		  (p->gender == GENDER_PLURAL ? " all" : ""),
		  USE_STRING(first_owner, " "),
		  USE_STRING(first_owner, first_owner));
    
    SOCIALS_TOP_OPTIONALS_FUNC(fvtell_player, NORMAL_T(p), USER_COLOUR_MINE);
    
    fvtell_player(NORMAL_T(p),
                  "%s%s %s",
                  USE_STRING(after_owner, " "),
                  USE_STRING(after_owner, after_owner),
                  (sub_multi ?
                   multi_get_names(sub_multi, sub_entry, p->saved,
                                   NULL, NULL,
                                   multi_name_flags_main_sub) :
                   multi_get_names(multi, entry, p->saved, NULL, NULL,
                                   multi_name_flags_main)));

    SOCIALS_BOTTOM_OPTIONALS_FUNC(fvtell_player, NORMAL_T(p),
                                  USER_COLOUR_MINE);

    fvtell_player(NORMAL_T(p),
                  "%s%s%s%s^N\n",
                  SOCIALS_FULL_STOP_CHECK,
                  USE_STRING(show_people_list, " (to "),
                  USE_STRING(show_people_list,
                             multi_get_names(multi, entry, p->saved, NULL,
                                             NULL,multi_name_flags_list)),
                  USE_STRING(show_people_list, ")"));
   }
 else if (sub_entry) /*this person is on the sub_multi, there must be one!*/
 { /*send a 'Aperson socials to you, therestoftheppl' type message*/
  int do_social = TRUE;

  LIST_COMS_CHECK_FLAG_START(p2, p->saved);
  if (LIST_COMS_CHECK_FLAG_DO(tells))
    do_social = FALSE;
  LIST_COMS_CHECK_FLAG_END();

  /*special tags for friends or friends of?? I think so...*/
  if (do_social)
  {
   fvtell_player(TALK_FTP(RAW_OUTPUT_VARIABLES, p2), "%s%s%s",
		 USER_COLOUR_SOCIALS_ME,
		 (the_social->parsed_info & (TO_ROOM|TO_LIST_OF_LOCAL) ?
		  SHOW_SOCIALS(p2, "~ ") :
		  (show_multi_num ? multi_num :
		   SHOW_PERSONAL_SOCIAL(p2, "~> ", "> ", "~ "))),
                 gender_choose_str(p->gender, "", "", "The ", "The "));
   fvtell_player(TALK_TP(p2), "%s",
                 "$If( ==(1(N) 2($R-Set-Ign_prefix))"
                 " t($F-Name_full) f($F-Name))");
   fvtell_player(TALK_TP(p2), "%s%s",
		 USE_STRING(first_other, " "),
		 USE_STRING(first_other, first_other));
   
   SOCIALS_TOP_OPTIONALS_FUNC(fvtell_player, TALK_TP(p2),
                              USER_COLOUR_SOCIALS_ME);
   
   fvtell_player(TALK_TP(p2),
                 "%s%s %s",
                 USE_STRING(after_other, " "),
                 USE_STRING(after_other, after_other),
                 (sub_multi ?
                  multi_get_names(sub_multi, sub_entry, p->saved, NULL, NULL,
                                  multi_name_flags_main_sub) :
                  multi_get_names(multi, entry, p->saved, NULL, NULL,
                                  multi_name_flags_main)));
   
   SOCIALS_BOTTOM_OPTIONALS_FUNC(fvtell_player, TALK_TP(p2),
                                 USER_COLOUR_SOCIALS_ME);
   
   fvtell_player(TALK_TP(p2),
                 "%s%s%s%s^N\n",
                 SOCIALS_FULL_STOP_CHECK,
                 USE_STRING(show_people_list, " (to "),
                 USE_STRING(show_people_list,
                            multi_get_names(multi, entry, p->saved, NULL,
                                            NULL,multi_name_flags_list)),
                 USE_STRING(show_people_list, ")"));
  }
 }
 else /*its an observer only*/
   if (the_social->parsed_info & TO_NO_ONE_SPECIFIC)
     if (the_social->toself &&
         !(the_social->opt_str && the_social->opt_end_str))
     {
      int do_social = TRUE;
      
      LIST_COMS_CHECK_FLAG_START(p2, p->saved);
      if (LIST_COMS_CHECK_FLAG_DO(tells))
        do_social = FALSE;
      LIST_COMS_CHECK_FLAG_END();
      
      if (do_social)
      {
       fvtell_player(TALK_FTP(RAW_OUTPUT_VARIABLES, p2), "%s%s%s",
                     USER_COLOUR_SOCIALS,
                     (the_social->parsed_info & (TO_ROOM|TO_LIST_OF_LOCAL) ?
                      SHOW_SOCIALS(p2, "~ ") :
                      (show_multi_num ? multi_num :
                       SHOW_PERSONAL_SOCIAL(p2, "~> ", "> ", "~ "))),
                     gender_choose_str(p->gender, "", "", "The ", "The "));
       fvtell_player(TALK_TP(p2), "%s",
                     "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                     " t($F-Name_full) f($F-Name))");
       fvtell_player(TALK_TP(p2), "%s%s",
                     USE_STRING((first_other &&
                                 !(the_social->info & SOCIAL_NO_PRINT_FIRST)),
                                " "),
                     USE_STRING((first_other &&
                                 !(the_social->info & SOCIAL_NO_PRINT_FIRST)),
                                first_other));
                     
       SOCIALS_TOP_OPTIONALS_FUNC(fvtell_player, TALK_TP(p2),
                                  USER_COLOUR_SOCIALS);
       
       fvtell_player(TALK_TP(p2),
                     "%s%s%s",
                     USE_STRING((!first_other ||
                                 (the_social->info & SOCIAL_PRINT_AFTER)) &&
                                after_other, " "),
                     USE_STRING((!first_other ||
                                 (the_social->info & SOCIAL_PRINT_AFTER)) &&
                                after_other, after_other),
                     the_social->toself);
       
       SOCIALS_BOTTOM_OPTIONALS_FUNC(fvtell_player, TALK_TP(p2),
                                     USER_COLOUR_SOCIALS);
       
       fvtell_player(TALK_TP(p2),
                     "%s%s%s%s^N\n",
                     SOCIALS_FULL_STOP_CHECK,
                     USE_STRING(show_people_list, " (to "),
                     USE_STRING(show_people_list,
                                multi_get_names(multi, entry, p->saved, NULL,
                                                NULL,multi_name_flags_list)),
                     USE_STRING(show_people_list, ")"));
      }
     }
     else /*just a normal 'waves at himself' type message*/
     {
      int do_social = TRUE;
      
      LIST_COMS_CHECK_FLAG_START(p2, p->saved);
      if (LIST_COMS_CHECK_FLAG_DO(tells))
        do_social = FALSE;
      LIST_COMS_CHECK_FLAG_END();
      
      if (do_social)
      {
       fvtell_player(TALK_FTP(RAW_OUTPUT_VARIABLES, p2), "%s%s%s",
                     USER_COLOUR_SOCIALS,
                     (the_social->parsed_info & (TO_ROOM|TO_LIST_OF_LOCAL) ?
                      SHOW_SOCIALS(p2, "~ ") :
                      (show_multi_num ? multi_num :
                       SHOW_PERSONAL_SOCIAL(p2, "~> ", "> ", "~ "))),
                     gender_choose_str(p->gender, "", "", "The ", "The "));
       fvtell_player(TALK_TP(p2), "%s",
                     "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                     " t($F-Name_full) f($F-Name))");
       fvtell_player(TALK_TP(p2), "%s%s",
                     USE_STRING(first_other, " "),
                     USE_STRING(first_other, first_other));
       
       SOCIALS_TOP_OPTIONALS_FUNC(fvtell_player, TALK_TP(p2),
                                  USER_COLOUR_SOCIALS);
       
       fvtell_player(TALK_TP(p2),
                     "%s%s %ssel%s",
                     USE_STRING(after_other, " "),		    
                     USE_STRING(after_other, after_other),
                     gender_choose_str(p->gender, "him", "her",
                                       "them", "it"),
                     (p->gender == GENDER_PLURAL) ? "ves" : "f");
       
       SOCIALS_BOTTOM_OPTIONALS_FUNC(fvtell_player, TALK_TP(p2),
                                     USER_COLOUR_SOCIALS);
       
       fvtell_player(TALK_TP(p2),
                     "%s%s%s%s^N\n",
                     SOCIALS_FULL_STOP_CHECK,
                     USE_STRING(show_people_list, " (to "),
                     USE_STRING(show_people_list,
                                multi_get_names(multi, entry, p->saved, NULL,
                                                NULL,multi_name_flags_list)),
                     USE_STRING(show_people_list, ")"));
      }
     }
   else /*send 'Aperson socials to alistofppl' type message*/
   {
    int do_social = TRUE;
    
    LIST_COMS_CHECK_FLAG_START(p2, p->saved);
    if (LIST_COMS_CHECK_FLAG_DO(tells))
      do_social = FALSE;
    LIST_COMS_CHECK_FLAG_END();

    if (do_social)
    {
     fvtell_player(TALK_FTP(RAW_OUTPUT_VARIABLES, p2), "%s%s%s",
		   USER_COLOUR_SOCIALS,
		   (the_social->parsed_info & (TO_ROOM|TO_LIST_OF_LOCAL) ?
		    SHOW_SOCIALS(p2, "~ ") :
		    (show_multi_num ? multi_num : SHOW_PERSONAL(p2, "> "))),
		   gender_choose_str(p->gender, "", "", "The ", "The "));
     fvtell_player(TALK_TP(p2), "%s",
                   "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                   " t($F-Name_full) f($F-Name))");
     fvtell_player(TALK_TP(p2), "%s%s",
		   USE_STRING(first_other, " "),
		   USE_STRING(first_other, first_other));

     SOCIALS_TOP_OPTIONALS_FUNC(fvtell_player, TALK_TP(p2),
                                USER_COLOUR_SOCIALS);
     
     fvtell_player(TALK_TP(p2),
                   "%s%s %s",
                   USE_STRING(after_other, " "),
                   USE_STRING(after_other, after_other),
                   (sub_multi ?
                    multi_get_names(sub_multi, sub_entry, p->saved, NULL,
                                    NULL, multi_name_flags_main_sub) :
                    multi_get_names(multi, entry, p->saved, NULL, NULL,
                                    multi_name_flags_main)));
     
     SOCIALS_BOTTOM_OPTIONALS_FUNC(fvtell_player, TALK_TP(p2),
                                   USER_COLOUR_SOCIALS);
     
     fvtell_player(TALK_TP(p2),
                   "%s%s%s%s^N\n",
                   SOCIALS_FULL_STOP_CHECK,
                   USE_STRING(show_people_list, " (to "),
                   USE_STRING(show_people_list,
                              multi_get_names(multi, entry, p->saved, NULL,
                                              NULL,multi_name_flags_list)),
                   USE_STRING(show_people_list, ")"));
    }
   }

 if ((p != p2) &&
     ((the_social->parsed_info & TO_SINGLE_REMOTE) ||
     (sub_players > 1)))
   check_receive_state(p->saved, p2);
 
 return (TRUE); /* have to return true to make do_inorder continue */
}

static int internal_socials_channel_1(player_linked_list *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 const char *str_1 = va_arg(ap, const char *);
 const char *str_2 = va_arg(ap, const char *);
 channels_base *base = ((channels_node *)scan)->base;
 const char *col = NULL;
 const char *left_sep = NULL;
 const char *right_sep = NULL;
 twinkle_info info;

 setup_twinkle_info(&info);
 info.output_not_me = TRUE;

 if (p != PLAYER_LINK_GET(scan))
 {
  if (!base->flag_no_blocks)
  {
   LIST_COMS_CHECK_FLAG_START(PLAYER_LINK_GET(scan), p->saved);
   if (LIST_COMS_CHECK_FLAG_DO(channels))
     return (TRUE);
   LIST_COMS_CHECK_FLAG_END();
  }
  
  col = CHANNELS_COLOUR_TYPE((channels_node *)scan);
 }
 else
   col = USER_COLOUR_MINE;
  
 left_sep = CHANNELS_NAME_SEP((channels_node *)scan)[0];
 right_sep = CHANNELS_NAME_SEP((channels_node *)scan)[1];

 fvtell_player(TALK_IT(&info, p->saved, PLAYER_LINK_GET(scan)),
               "%s%s%s%s %s%s%s",
               col, left_sep, base->name, right_sep,
               "$If( ==(1(N) 2($R-Set-Ign_prefix))"
               " t($F-Name_full) f($F-Name))",
               str_1, str_2);
 
 return (TRUE);
}

static int internal_socials_channel_2(player_linked_list *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 const char *str_1 = va_arg(ap, const char *);
 const char *str_2 = va_arg(ap, const char *);
 const char *str_3 = va_arg(ap, const char *);
 channels_base *base = ((channels_node *)scan)->base;
 const char *col = NULL;
 twinkle_info info;

 setup_twinkle_info(&info);
 info.output_not_me = TRUE;
 
 if (p != PLAYER_LINK_GET(scan))
 {
  if (!base->flag_no_blocks)
  {
   LIST_COMS_CHECK_FLAG_START(PLAYER_LINK_GET(scan), p->saved);
   if (LIST_COMS_CHECK_FLAG_DO(channels))
     return (TRUE);
   LIST_COMS_CHECK_FLAG_END();
  }
  
  col = CHANNELS_COLOUR_TYPE((channels_node *)scan);
 }
 else
   col = USER_COLOUR_MINE;

 fvtell_player(TALK_IT(&info, p->saved, PLAYER_LINK_GET(scan)), "%s%s%s%s",
               str_1, str_2, col, str_3);

 return (TRUE);
}

static int internal_socials_channel_3(player_linked_list *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 const char *str_1 = va_arg(ap, const char *);
 const char *str_2 = va_arg(ap, const char *);
 const char *str_3 = va_arg(ap, const char *);
 channels_base *base = ((channels_node *)scan)->base;
 const char *col = NULL;
 twinkle_info info;

 setup_twinkle_info(&info);
 info.output_not_me = TRUE;
 
 if (p != PLAYER_LINK_GET(scan))
 {
  if (!base->flag_no_blocks)
  {
   LIST_COMS_CHECK_FLAG_START(PLAYER_LINK_GET(scan), p->saved);
   if (LIST_COMS_CHECK_FLAG_DO(channels))
     return (TRUE);
   LIST_COMS_CHECK_FLAG_END();
  }
  
  col = CHANNELS_COLOUR_TYPE((channels_node *)scan);
 }
 else
   col = USER_COLOUR_MINE;

 fvtell_player(TALK_IT(&info, p->saved, PLAYER_LINK_GET(scan)), "%s%s%s%s",
               str_1, str_2, col, str_3);
 
 return (TRUE);
}

static int internal_socials_channel_4(player_linked_list *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 const char *str_1 = va_arg(ap, const char *);
 const char *str_2 = va_arg(ap, const char *);
 const char *str_3 = va_arg(ap, const char *);
 const char *str_4 = va_arg(ap, const char *);
 channels_base *base = ((channels_node *)scan)->base;
 twinkle_info info;

 setup_twinkle_info(&info);
 info.output_not_me = TRUE;
 
 if (p != PLAYER_LINK_GET(scan))
 {
  if (!base->flag_no_blocks)
  {
   LIST_COMS_CHECK_FLAG_START(PLAYER_LINK_GET(scan), p->saved);
   if (LIST_COMS_CHECK_FLAG_DO(channels))
     return (TRUE);
   LIST_COMS_CHECK_FLAG_END();
  }
 }

 fvtell_player(TALK_IT(&info, p->saved, PLAYER_LINK_GET(scan)), "%s%s%s%s",
               str_1, str_2, str_3, str_4);

 return (TRUE);
}

static int internal_socials_channel_5(player_linked_list *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 const char *str_1 = va_arg(ap, const char *);
 const char *str_2 = va_arg(ap, const char *);
 const char *str_3 = va_arg(ap, const char *);
 channels_base *base = ((channels_node *)scan)->base;
 const char *col = NULL;
 twinkle_info info;

 setup_twinkle_info(&info);
 info.output_not_me = TRUE;
 
 if (p != PLAYER_LINK_GET(scan))
 {
  if (!base->flag_no_blocks)
  {
   LIST_COMS_CHECK_FLAG_START(PLAYER_LINK_GET(scan), p->saved);
   if (LIST_COMS_CHECK_FLAG_DO(channels))
     return (TRUE);
   LIST_COMS_CHECK_FLAG_END();
  }
  
  col = CHANNELS_COLOUR_TYPE((channels_node *)scan);
 }
 else
   col = USER_COLOUR_MINE;

 fvtell_player(TALK_IT(&info, p->saved, PLAYER_LINK_GET(scan)), "%s%s%s%s",
               str_1, str_2, col, str_3);
 
 return (TRUE);
}

static int internal_socials_channel_6(player_linked_list *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 const char *str_1 = va_arg(ap, const char *);
 const char *str_2 = va_arg(ap, const char *);
 const char *str_3 = va_arg(ap, const char *);
 channels_base *base = ((channels_node *)scan)->base;
 const char *col = NULL;
 twinkle_info info;

 setup_twinkle_info(&info);
 info.output_not_me = TRUE;

 if (p != PLAYER_LINK_GET(scan))
 {
  if (!base->flag_no_blocks)
  {
   LIST_COMS_CHECK_FLAG_START(PLAYER_LINK_GET(scan), p->saved);
   if (LIST_COMS_CHECK_FLAG_DO(channels))
     return (TRUE);
   LIST_COMS_CHECK_FLAG_END();
  }
  
  col = CHANNELS_COLOUR_TYPE((channels_node *)scan);
 }
 else
   col = USER_COLOUR_MINE;

 fvtell_player(TALK_TP(PLAYER_LINK_GET(scan)), "%s%s%s%s",
               str_1, str_2, col, str_3);
 
 return (TRUE);
}

static int internal_socials_channel_7(player_linked_list *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 const char *str_1 = va_arg(ap, const char *);
 channels_base *base = ((channels_node *)scan)->base;
 twinkle_info info;

 setup_twinkle_info(&info);
 info.output_not_me = TRUE;
 
 if (p != PLAYER_LINK_GET(scan))
 {
  if (!base->flag_no_blocks)
  {
   LIST_COMS_CHECK_FLAG_START(PLAYER_LINK_GET(scan), p->saved);
   if (LIST_COMS_CHECK_FLAG_DO(channels))
     return (TRUE);
   LIST_COMS_CHECK_FLAG_END();
  }
 }

 fvtell_player(TALK_IT(&info, p->saved, PLAYER_LINK_GET(scan)), "%s%s\n",
               str_1, "^N");

 return (TRUE);
}

/* master social function to cater for special social types */
static void master_social_no_multi_group(social_obj *the_social)
{
 player *p = the_social->p;
 int sub_multi = the_social->sub_multi_list;
 int get_names_flags = (MULTI_NO_MULTIS|MULTI_TO_SELF);
 const char *first_owner = get_action_str(the_social, FIRST_STR, TO_OWNER);
 const char *after_owner = get_action_str(the_social, AFTER_STR, TO_OWNER);
 const char *first_other = get_action_str(the_social, FIRST_STR, TO_OTHER);
 const char *after_other = get_action_str(the_social, AFTER_STR, TO_OTHER);
 twinkle_info info;

 /*setup all the twinkle hack for conjugate etc*/
 setup_twinkle_info(&info);
 info.output_not_me = TRUE;
 
 if (the_social->parsed_info & TO_MARRIAGE_CHANNEL)
 {
  if (the_social->spouse)
  {
   const char *spouse_str = get_spouse_id(the_social->spouse,
					  !p->flag_no_net_spouse);

   /*to the destination*/
   fvtell_player(NORMAL_FT(RAW_OUTPUT_VARIABLES, the_social->spouse),
		 "%s[m] ",
		 USER_COLOUR_SOCIALS_ME);
   fvtell_player(TALK_TP(the_social->spouse), "%s",
                 "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                 " t($F-Name_full) f($F-Name))");
   fvtell_player(NORMAL_T(the_social->spouse),
		 "%s%s",   
		 USE_STRING(first_other, " "),
		 USE_STRING(first_other, first_other));
   
   SOCIALS_TOP_OPTIONALS_FUNC(fvtell_player, NORMAL_T(the_social->spouse),
                              USER_COLOUR_SOCIALS_ME);
   
   fvtell_player(NORMAL_T(the_social->spouse),
                 "%s%s you",
                 USE_STRING(after_other, " "),
                 USE_STRING(after_other, after_other));
   
   SOCIALS_BOTTOM_OPTIONALS_FUNC(fvtell_player, NORMAL_T(the_social->spouse),
                            USER_COLOUR_SOCIALS_ME);
   
   fvtell_player(NORMAL_T(the_social->spouse),
                 "%s%s\n",
                 SOCIALS_FULL_STOP_CHECK,
		 "^N");

   /*to the sender*/
   fvtell_player(ALL_T(p->saved, p, &info, 3, now),
		 "%s You%s%s",
		 USER_COLOUR_MINE,
		 USE_STRING(first_owner, " "),
		 USE_STRING(first_owner, first_owner));

   SOCIALS_TOP_OPTIONALS_FUNC(fvtell_player,
                              ALL_T(p->saved, p, &info, 3, now),
                              USER_COLOUR_MINE);
   
   fvtell_player(ALL_T(p->saved, p, &info, 3, now),
                 "%s%s your %s",
                 USE_STRING(after_owner, " "),
                 USE_STRING(after_owner, after_owner),
                 spouse_str);
   
   SOCIALS_BOTTOM_OPTIONALS_FUNC(fvtell_player,
                                 ALL_T(p->saved, p, &info, 3, now),
                                 USER_COLOUR_MINE);
   
   fvtell_player(ALL_T(p->saved, p, &info, 3, now),
                 "%s%s\n",
		 SOCIALS_FULL_STOP_CHECK,
		 "^N");     
  }
 }
 else if (the_social->parsed_info & TO_GEN_CHANNEL)
 {
  if (!sub_multi) /*its just to you effectively*/
  {
   if (the_social->toself && /*if there's a special message*/
       !(the_social->opt_str && the_social->opt_end_str))
   {
    channels_base *base = the_social->chan_base;
    
    if (!base)
    {
     log_assert(FALSE);
     return;
    }
    
    SOCIALS_CHANNEL_FUNC(p,
                         USE_STRING((first_other &&
                                  !(the_social->info & SOCIAL_NO_PRINT_FIRST)),
                                    " "),
                         USE_STRING((first_other &&
                                  !(the_social->info & SOCIAL_NO_PRINT_FIRST)),
                                    first_other),
                         (first_other ?
                          (the_social->info & SOCIAL_PRINT_AFTER ?
                           USE_STRING(after_other, " ") : "") :
                          USE_STRING(after_other, " ")),
                         (first_other ?
                          (the_social->info & SOCIAL_PRINT_AFTER ?
                           USE_STRING(after_other, after_other) : "") :
                          USE_STRING(after_other, after_other)),
                         "",
                         the_social->toself,
                         SOCIALS_FULL_STOP_CHECK);
   }
   else /*just a normal 'waves at himself' type message*/
   {
    channels_base *base = the_social->chan_base;
    
    if (!base)
    {
     log_assert(FALSE);
     return;
    }

    SOCIALS_CHANNEL_FUNC(p,
                         USE_STRING(first_other, " "),
                         USE_STRING(first_other, first_other),
                         USE_STRING(after_other, " "),
                         USE_STRING(after_other, after_other),
                         " ",
                         gender_choose_str(p->gender, "himself", "herself",
                                           "themselves", "itself"),
                         SOCIALS_FULL_STOP_CHECK);
   }
  }
  else /*send the 'social to whoever' type message*/
  {
   channels_base *base = the_social->chan_base;
   
   if (!base)
   {
    log_assert(FALSE);
    return;
   }
   
   SOCIALS_CHANNEL_FUNC(p,
                        USE_STRING(first_other, " "),	     
                        USE_STRING(first_other, first_other),
                        USE_STRING(after_other, " "),
                        USE_STRING(after_other, after_other),
                        " ",
                        multi_get_names(sub_multi, NULL, p->saved, NULL,
                                        NULL, get_names_flags),
                        SOCIALS_FULL_STOP_CHECK);
  }
 }
 else
 {
  handle_social_error(the_social, "got to master_social_no_multi_group with "
		      "no valid flag in parsed info");
  fvtell_player(SYSTEM_T(p), " An error has occured in the '%s' social, "
                "please try again with different settings (help socials).\n",
		current_command);
 }

 cleanup_social(the_social);
}

/* master social itself (the new guy) */
static void master_social(social_obj *the_social)
{
#ifdef SOCIAL_DEBUG
 player *p = the_social->p;
 assert (the_social->p);
#endif
 
 if (the_social->parsed_info & FAIL)
   handle_social_error(the_social, "Got to master social with fail flag set!");
 else if (the_social->parsed_info & TO_SPECIAL_GROUP)
   master_social_no_multi_group(the_social);
 else if (the_social->multi_list < 1)
   handle_social_error(the_social, "Got to master social with no multi list!");
#if 0
 else if (the_social->parsed_info & (TO_ADMIN|TO_SUS))
 {
  if (the_social->parsed_info & (TO_ADMIN))
    do_inorder_multi(inorder_chan_social, the_social->multi_list,
                     MULTI_DEFAULT, the_social, "[", "]",
                     USER_COLOUR_MINE); /*no admin chan colour atm*/
  else
    do_inorder_multi(inorder_chan_social, the_social->multi_list,
                     MULTI_DEFAULT, the_social, "<", ">",
                     USER_COLOUR_SUS);    
  cleanup_social(the_social); /*done in all other cases already*/
 }
#endif
 else
 {
  do_inorder_multi(inorder_social, the_social->multi_list, MULTI_DEFAULT,
                   the_social);
  cleanup_social(the_social); /*done in all other cases already*/
 }
  
#ifdef SOCIAL_DEBUG
 fvtell_player(SYSTEM_T(p),
	       " I'm in master_social.\n Social: %s\n Info: %u, "
	       "parsed_info: %u\n Pfirst: %s\n Pafter %s\n Sfirst: %s\n"
	       " Safter: %s\n Opt_str: %s\n Opt_end_str: %s\n Multi: %u\n"
	       " Sub_multi: %u\n", current_command, the_social->info,
	       the_social->parsed_info, the_social->pfirst,
	       the_social->pafter,  the_social->sfirst,  the_social->safter,
	       the_social->opt_str,  the_social->opt_end_str,
	       the_social->multi_list, the_social->sub_multi_list);
#endif
}

static char *next_social_parameter(char *str, int seperator)
{
 return (next_parameter_no_seperators(str, seperator));
}

/*this func checks on the value returned by add to multi when it fails to
  create a multi list and then outputs an appropriate error*/
static int check_error_flags_social(multi_return *values,
				    social_obj *the_social,
				    const char *str)
{
 player *p = the_social->p;

 switch (values->error_number)
 {
  case MULTI_BAD_IGNORE:
    fvtell_player(SYSTEM_T(p),
                  " You can't be socialble to multi -- ^S^B%d^s -- as "
                  "you're blocking it.\n",
                  values->error_multi);
    return (FALSE);
  case MULTI_STOPPED_ALREADY:
    fvtell_player(SYSTEM_T(p),
                  " You cannot be socialble to multi -- ^S^B%d^s -- as "
                  "it has been stopped.\n",
                  values->error_multi);
    return (FALSE);
  case MULTI_BAD_EVERYONE_SELECTION:
    fvtell_player(SYSTEM_T(p), "%s",
                  " You cannot be socialble to everyone whilst "
                  "you're blocking them.\n");
    return (FALSE);
  case MULTI_BAD_EVERYONE_FIND:
    if (the_social->info & NO_EVERYONE)
      fvtell_player(SYSTEM_T(p),
                    " The social -- ^S^B%s^s -- does not support "
                    "the everyone group.\n",
                    current_command);
    else
      fvtell_player(SYSTEM_T(p), "%s",
                    " You can't be socialble to everyone as there's no one "
                    "else around at the moment.\n");
    return (FALSE);
  case MULTI_BAD_MULTI_FIND:
    if (the_social->info & NO_MULTIS)
      fvtell_player(SYSTEM_T(p),
                    " The social -- ^S^B%s^s -- does not support "
                    "socialising with multis.\n", current_command);
    else
      fvtell_player(SYSTEM_T(p),
                    " There is no one on multi -- ^S^B%d^s -- so there's "
                    "no point being sociable to it.\n",
                    values->error_multi);
    return (FALSE);
  case MULTI_BAD_MULTI_SELECTION:
    fvtell_player(SYSTEM_T(p),
                  " You can't be socialble to multi -- ^S^B%d^s -- as "
                  "you're not on it.\n",
                  values->error_multi);
    return (FALSE);    
  case MULTI_BAD_NAME_SELECTION:
    log_assert (the_social->info & NO_NAMES);
    fvtell_player(SYSTEM_T(p),
                  " The social -- ^S^B%s^s -- does not support "
                  "socialising with individual players.\n", current_command);
    return (FALSE);
  case MULTI_BAD_NAME_FIND:
  case MULTI_BAD_FRIENDS_OF_NAME_FIND:
    /* fvtell_player(SYSTEM_T(p),
       " The name -- ^S^B%s^s -- is not a valid name.\n",
       values->error_name); */
    return (FALSE);
  case MULTI_NO_PEOPLE_ADDED:
    fvtell_player(SYSTEM_T(p),
                  " The string -- ^S^B%s^s -- does not match anything.\n",
                  str);
    return (FALSE);
  case MULTI_BAD_MINISTER_SELECTION:
  case MULTI_BAD_SU_SELECTION:
  case MULTI_BAD_NEWBIE_SELECTION:
    fvtell_player(SYSTEM_T(p),
                  " You do not have the ^S^Bprivs^s to socialise with "
                  "all the ^S^B%s's^s as one group.\n", values->error_name);
    return (FALSE);
  case MULTI_BAD_MINISTER_FIND:
  case MULTI_BAD_SU_FIND:
  case MULTI_BAD_NEWBIE_FIND:
    if (the_social->info & NO_NEWBIES)
      fvtell_player(SYSTEM_T(p),
                    " The social -- ^S^B%s^s -- does not support "
                    "socialising with the ^S^B%s's^s all at once.\n",
                    current_command, values->error_name);
    else
      fvtell_player(SYSTEM_T(p),
                    " It is not possible to locate any %s's at the moment.\n",
                    values->error_name);
    return (FALSE);
  case MULTI_BAD_FRIENDS_SELECTION:
    fvtell_player(SYSTEM_T(p), "%s",
                  " You cannot be sociable with friends whilst "
                  "blocking them.\n");
    return (FALSE);
  case MULTI_BAD_FRIENDS_FIND:
    if (the_social->info & NO_FRIENDS)
      fvtell_player(SYSTEM_T(p),
                    " The social -- ^S^B%s^s -- does not support "
                    "socialising with your friends all at once.\n",
                    current_command);
    else 
      fvtell_player(SYSTEM_T(p), "%s",
                    " It was not possible to locate any of your friends.\n");
    return (FALSE);
  case MULTI_BAD_FRIENDS_OF_NOT_ON:
    fvtell_player(SYSTEM_T(p),
                  " You can't socialise with the friends of -- ^S^B%s^s -- "
                  "as they're not logged on.\n", values->error_name);
    return (FALSE);
  case MULTI_BAD_FRIENDS_OF_NOT_ON_LIST:
    fvtell_player(SYSTEM_T(p),
                  " You can't socialise with the friends of -- ^S^B%s^s -- "
                  "you're not on their friends list.\n", values->error_name);
    return (FALSE);
  case MULTI_BAD_FRIENDS_OF_SELECTION:
    fvtell_player(SYSTEM_T(p),
                  " You can't socialise with the friendsof -- ^S^B%s^s -- "
                  "as you're not on their list or they can't be located.\n",
                  values->error_name);
    return (FALSE);
  case MULTI_BAD_FRIENDS_OF_FIND:
    if (the_social->info & NO_TFOS)
      fvtell_player(SYSTEM_T(p),
                    " The social -- ^S^B%s^s -- social does not support "
                    "socialising with the friends of someone.\n",
                    current_command);
    else
      fvtell_player(SYSTEM_T(p),
                    " You can't be sociable to the friends of -- ^S^B%s^s -- "
                    "as none of them can be located.\n",
                    values->error_name);
    return (FALSE);
  case MULTI_BAD_ROOM_SELECTION:
    fvtell_player(SYSTEM_T(p), "%s",
                  " You cannot socialise with the whole room "
                  "at the moment.\n");
    return (FALSE);
  case MULTI_BAD_ROOM_FIND:
    if (the_social->info & NO_ROOM)
      fvtell_player(SYSTEM_T(p),
                    " The social -- ^S^B%s^s -- does not support "
                    "socialising with the room group.\n", current_command);
    else    
      fvtell_player(SYSTEM_T(p), "%s",
                    " There is no one in the room at the moment to socialise "
                    "with.\n");
    return (FALSE);
  default:
    assert(FALSE);
    fvtell_player(SYSTEM_T(p),
                  " Error occured in the social '%s' (%u).\n",
                  current_command, values->error_number);
    vwlog("error", "Error occured in the social %s while doing a %s (%u).",
          current_command, str, values->error_number);
 }

 /* should always return false anyhow */
 return (FALSE);
}

static int check_which_previous_event_and_error(social_obj *the_social,
						const char *active_str,
						int to_flag)
{
 player *p = the_social->p;

 if (the_social->parsed_info & TO_MARRIAGE_CHANNEL)
 {
  if (to_flag & TO_MARRIAGE_CHANNEL)
    return (TRUE);
  else
  {
   fvtell_player(SYSTEM_T(p),
                 " You have tried to be sociable to the marriage "
                 "channel and therefore cannot %s, (%c) aswell.\n",
                 active_str, *the_social->search);
    return (FALSE);
  }
 }
 
 if (the_social->parsed_info & TO_GEN_CHANNEL)
 {
  fvtell_player(SYSTEM_T(p),
                " You have tried to be sociable to a channel "
                "channel and therefore cannot %s, (%c) aswell.\n",
                active_str, *the_social->search);
  return (FALSE);
 }
 
 if (the_social->parsed_info & TO_EVERYONE)
 {
  if (to_flag & TO_EVERYONE)
    return (TRUE);
  else
  {
   fvtell_player(SYSTEM_T(p),
                 " You have tried to be sociable to everyone "
                 "and therefore cannot %s, (%c) aswell.\n",
                 active_str, *the_social->search);
   return (FALSE);
  }
 }
 
 
 if (the_social->parsed_info & TO_FRIENDS)
 {
  if (to_flag & TO_FRIENDS)
    return (TRUE);
  else
  {
   fvtell_player(SYSTEM_T(p),
                 " You have tried to be sociable to your friends "
                 "and therefore cannot %s, (%c) aswell.\n",
                 active_str, *the_social->search);
   return (FALSE);
  }
 }
 
 
 if (the_social->parsed_info & TO_ROOM)
 {
  if (to_flag & TO_ROOM)
    return (TRUE);
  else
  {
   fvtell_player(SYSTEM_T(p),
                 " You have tried to be sociable to the room "
                 "and therefore cannot %s, (%c) aswell.\n",
                 active_str, *the_social->search);
   return (FALSE);
  }
 }

 if (the_social->parsed_info & TO_FRIENDS_OF)
 {
  if (to_flag & TO_FRIENDS_OF)
    return (TRUE);
  else
  {
   fvtell_player(SYSTEM_T(p),
                 " You have tried to be sociable to someone's friends "
                 "and therefore cannot %s, (%c) aswell.\n",
                 active_str, *the_social->search);
   return (FALSE);
  }
 }
 
 if (the_social->parsed_info & TO_MULTI)
 {
  if (to_flag & TO_MULTI)
    return (TRUE);
  else
  {
   fvtell_player(SYSTEM_T(p),
                 " You have tried to be sociable to a multi "
                 "and therefore cannot %s, (%c) aswell.\n",
                 active_str, *the_social->search);
   return (FALSE);
  }
 }
 
 if (the_social->parsed_info & TO_NEWBIES)
 {
  if (to_flag & TO_NEWBIES)
    return (TRUE);
  else
  {
   fvtell_player(SYSTEM_T(p),
                 " You have tried to be sociable to the newbies "
                 "and therefore cannot %s, (%c) aswell.\n",
                 active_str, *the_social->search);
   return (FALSE);
  }
 }
 
 if (the_social->parsed_info & TO_LIST_OF_REMOTES)
 {
  if (to_flag & TO_LIST_OF_REMOTES)
  {
   fvtell_player(SYSTEM_T(p),
                 " You have specified two lists of 'remote' people (%c).\n",
                 *the_social->search);
   return (FALSE);
  }
  else
  {
   fvtell_player(SYSTEM_T(p),
                 " You have tried to be sociable with one list "
                 "of 'remote' people and therefore cannot %s, (%c) aswell.\n",
                 active_str, *the_social->search);
   return (FALSE);
  }
 }
 
 if (the_social->parsed_info & TO_LIST_OF_LOCAL)
 {
  if (to_flag & TO_LIST_OF_LOCAL)
  {
   fvtell_player(SYSTEM_T(p),
                 " You have specified two lists of people (%c).\n",
                 *the_social->search);
   return (FALSE);
  }
  else
  {
   fvtell_player(SYSTEM_T(p),
                 " You have tried to be sociable with one list "
                 "of people and therefore cannot %s, (%c) aswell.\n",
                 active_str, *the_social->search);
   return (FALSE);
  }
 }

 assert(FALSE);
 fvtell_player(SYSTEM_T(p), "%s",
               " There seems to be an internal problem, please try your "
               "social again!\n");
 vwlog("error", "Had a multi_list but no obvious prev action flag, "
      "action: %s, %c! (%s)", active_str, *the_social->search,
      current_command);
 
 return (FALSE);
}

/* This checks that a multi sublist contains people who are on the
   channel specified (CRAZY or SPOD at the moment) and aren't blocking
   etc, else an error is output - returns TRUE if valid, FALSE otherwise */
static int inorder_check_channel(multi_node *entry, va_list va)
{
 social_obj *the_social = va_arg(va, social_obj *);
 int *ret_val = va_arg(va, int *);
 multi_node **person_not_on = va_arg(va, multi_node **);
 channels_node *node = channels_find_node(the_social->chan_base,
                                          entry->parent);
 player *p = the_social->p;
 
 if (node)
 {
  LIST_COMS_CHECK_FLAG_START(entry->parent->player_ptr, p->saved);
  if (!LIST_COMS_CHECK_FLAG_DO(channels))
    return (TRUE);
  LIST_COMS_CHECK_FLAG_END();
 }
 
 *person_not_on = entry;
 return ((*ret_val = FALSE));
}

static int check_valid_channel(social_obj *the_social)
{
 int ret_val = 1;
 multi_node *person_not_on = 0;
 int sub_multi = the_social->sub_multi_list;
 
 do_inorder_multi(inorder_check_channel, sub_multi, MULTI_DEFAULT,
                  the_social, &ret_val, &person_not_on);

 if (!ret_val)
   fvtell_player(SYSTEM_T(the_social->p),
		 " The person -- ^S^B%s^s -- is not on "
                 "the -- ^S^B%s^s -- channel.\n",
		 person_not_on->parent->name, the_social->chan_base->name);
 
 return (ret_val);
}

#if 0
/* This checks that a sublist contains all the people on a main list, else
   its an invalid sublist and an error should be output - returns TRUE if
   a valid sublist, FALSE otherwise */
static int inorder_check_sub_multi(multi_node *entry, va_list va)
{
 int main_multi = va_arg(va, int);
 int *ret_val = va_arg(va, int *);
 multi_node **person_not_on = va_arg(va, multi_node **);

 *person_not_on = entry;
 
 if (multi_find_entry(entry->parent, main_multi))
   return (TRUE);
 else
   return ((*ret_val = 0));   
}

static int check_valid_sublist(social_obj *the_social, int verbose)
{
 int ret_val = 1;
 multi_node *person_not_on = 0;
 int main_multi = the_social->multi_list;
 int sub_multi = the_social->sub_multi_list;
 
 do_inorder_multi(inorder_check_sub_multi, sub_multi, MULTI_DEFAULT,
		      main_multi, &ret_val, &person_not_on);

 if ((!ret_val) && (verbose))
   fvtell_player(SYSTEM_T(the_social->p),
		 " The person '%s' is not in your main list, but ^Uis^u in "
		 "your sublist.\n",
		 person_not_on->parent->name);
 
 return (ret_val);
}
#endif

/* The funcs called from check_appropriate_string to parse var. controllers */

/*this attempts to tag a friends list*/
static int tf_social_parse(social_obj *the_social)
{
 multi_return *values;
 char friendsof[CONST_STRLEN("friendsof:") + PLAYER_S_NAME_SZ];
 char *end_of_names = (the_social->search + 1);
 char *start_of_names = (the_social->search + 1);
 player *p = the_social->p;

 /*check here for imediate (flag) problems*/
 if (the_social->info & NO_TFOS)
 {
  fvtell_player(SYSTEM_T(p),
		" The ^B%s^b social does not support the 'friends of' (%c) "
		"controller.\n", current_command, *the_social->search);
  return (FALSE);
 }

 /*check here for imediate (non flag) fail problems...*/
 if (the_social->multi_list > 0)
   return (check_which_previous_event_and_error(the_social,
						"'tell friends of'",
						TO_FRIENDS_OF));

 /*check for having been in a name list (sublist (-)) controller*/
 if (the_social->parsed_info & (TO_NAME_CONTROLLER))
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You must put a name sublist (-) after other "
                "controllers.\n");
  return (FALSE);
 }
 
 if (!*start_of_names) /*have to have a name of SOME kind - error*/
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You must specify a name after a tell friends of (} or {) "
                "controller.\n");
  return (FALSE);
 }

 
 /*get the end of the name and terminate it*/
 end_of_names = next_social_parameter(end_of_names, ' ');

 sprintf(friendsof, "%s%.*s", "friendsof:",
         PLAYER_S_NAME_SZ - 1, start_of_names);
 
 /* call the 'tag' multi function */
 values = multi_add(p->saved, friendsof,
                    (MULTI_DIE_STOPPED|
                     MULTI_DIE_MATCH_GROUP|
                     MULTI_DIE_EMPTY_GROUP|
                     MULTI_LIVE_ON_SMALL),
                    PLAYER_FIND_SC_COMS | PLAYER_FIND_SELF);

 if ((the_social->multi_list = values->multi_number))
 {
  /*make the loop move on*/
  the_social->search = end_of_names;
  the_social->parsed_info |= (TO_FRIENDS_OF);
  return (TRUE);
 }
 else
   return (check_error_flags_social(values, the_social, "'friends of'"));
}

static int channel_social_parse(social_obj *the_social)
{
 char *channel_end = the_social->search + 1;
 player *p = the_social->p;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 /*checking for imediate (flag) problems won't occur until we know the
   channel type we're socialising with*/
 
 /*check here for imediate (non flag) fail problems...*/
 if (the_social->multi_list > 0)
   return (check_which_previous_event_and_error(the_social,
						"socialise on channels",
						/*any flag which is not an
						  event - has to be a valid
						  one though of course*/
						TO_NO_ONE_SPECIFIC));

 /*check for having been in a name list (sublist (-)) controller*/
 if (the_social->parsed_info & (TO_NAME_CONTROLLER))
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You must put a name sublist (-) after other "
                "controllers.\n");
  return (FALSE);
 }

 if (!get_parameter_parse(&params, (const char **)&channel_end, 1) ||
     !GET_PARAMETER_LENGTH(&params, 1))
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You must specify a channel alias when using a "
                "channel (*) controller.\n");
  return (FALSE);
 }
  
 /*now find out the channel we're trying for*/
 if ((GET_PARAMETER_LENGTH(&params, 1) == 2) &&
     !memcmp(GET_PARAMETER_STR(&params, 1), "m", 2))
 {
  if (the_social->info & NO_MARRIAGE_CHANNEL)
  {
   fvtell_player(SYSTEM_T(p),
		 " The ^B%s^b social does not support socialising on the "
		 "Marriage channel.\n", current_command);
   return (FALSE);
  }
  else if (the_social->sub_multi_list > 0)
  {
   fvtell_player(SYSTEM_T(p), " You have specified a list of people to "
                 "'%s', and then tried to use the Marriage aswell.\n",
                 current_command);
   return (FALSE);
  }
  else if (!p->flag_married || !p->married_to[0])
  {
   fvtell_player(SYSTEM_T(p), "%s",
                 " You cannot socialise on the marriage channel as you're "
                 "not married.\n");
   return (FALSE);
  }
  else if (!(the_social->spouse = player_find_on(p, p->married_to,
                                                 PLAYER_FIND_DEFAULT)))
  {
   fvtell_player(SYSTEM_T(p), "%s",
                 " Your spouse is not currently logged on, so you can't "
                 "socialise with them, sorry.\n");
   return (FALSE);
  }
  
  /*this is a special case - don't use 'multi tags' for this, sorted
    in master social instead*/
  the_social->parsed_info |= (TO_MARRIAGE_CHANNEL|TO_SPECIAL_GROUP);
  /*move the search along past this controller section*/
  the_social->search = channel_end;
  /*set the multi_list to 1 so further checks realise there's been a
    controller so far*/
  return (the_social->multi_list = TRUE);
 }

 if (the_social->info & NO_GEN_CHANNEL)
 {
  fvtell_player(SYSTEM_T(p),
                " The ^B%s^b social does not support socialising on "
                "channels, sorry.\n", current_command);
  return (FALSE);
 }

 the_social->chan_base = channels_user_find_write_base(p,
                                                GET_PARAMETER_STR(&params, 1));
 if (!the_social->chan_base)
   return (FALSE);

 /*this is a special case - don't use 'multi tags' for this, sorted
   in master social instead*/
 the_social->parsed_info |= (TO_GEN_CHANNEL|TO_SPECIAL_GROUP);
 /*move the search along past this controller section*/
 the_social->search = channel_end;
 /*set the multi_list to 1 so further checks realise there's been a
   controller so far*/
 return (the_social->multi_list = TRUE);
}

/* checks to see if there's a message controller in the rest of a string. If
 * there is then checks for a double controller to remove it. If so then
 * removes the controller character */
static void check_for_msg_controller(social_obj *the_social, char controller)
{
 player *p = the_social->p;
 char *msg_left = the_social->search + 1;
 char *prev_character = the_social->search + 1;
 char *moving = 0;
 int out_msg = 0; /*so msgs only get output once as information/warning*/
 int out_end_msg = 0;
 
 while (*msg_left)
 {
  if (*msg_left == controller)
  {
   if (*(msg_left + 1) == controller) /*two controllers*/
   { /*invalidated controller, move message back*/     
    moving = (msg_left + 1);
    prev_character = msg_left;
    while (*moving)
      *prev_character++ = *moving++;
    *prev_character = 0; /*terminate the string*/
   }
   else /*valid controller*/
   {
    prev_character = (msg_left - 1); /*temp pointer to pre-controller*/
    if (*(msg_left + 1) == ' ')
      /*take out first space in msg thingies*/
      msg_left += 2;
    else
      msg_left++; /*move past controller*/
    
    /*set the valid flags - if they tried to set a msg, thats all they can
      do - regardless as to whether they're allowd or not*/
    if (controller == '<')
      the_social->parsed_info |= (HAS_SET_MSG);
    else
      the_social->parsed_info |= (HAS_SET_END_MSG);
    
    /*sort out the opt message pointer if allowd*/
    if (controller == '<')
      if ((the_social->info & NO_MESSAGE) && !out_msg)
      {
       fvtell_player(SYSTEM_T(p),
                     " This ^B%s^b social doesn't support you adding "
                     "start messages (<) to it, sorry.\n", current_command);
       out_msg = 1;
      }
      else
      {
       the_social->opt_space = isits1(msg_left);
       the_social->opt_str = isits2(msg_left);
      }
    else
      if ((the_social->info & NO_END_MESSAGE) && !out_end_msg)
      {
       fvtell_player(SYSTEM_T(p),
                     " This ^B%s^b social doesn't support you adding "
                     "end messages (>) to it, sorry.\n", current_command);
       out_end_msg = 1;
      }
      else
      {
       the_social->end_space = isits1(msg_left);
       the_social->opt_end_str = isits2(msg_left);
      }
    
    /*null controller IF we've actually 'used' it*/
    if (!(out_msg || out_end_msg))
    {
     if (*prev_character == ' ')
        *prev_character = 0; /*null the end of the message*/
     else /*then there's no space at the end of the message, so null the */
       *(prev_character + 1) = 0; /*controller itself instead*/
    }
    
    return; /*valid msg*/
   }
  }
  
  msg_left++; /*else we can move down the string*/
 } /*no controllers found*/
}

static int msg_social_parse(social_obj *the_social)
{
 player *p = the_social->p;
 char *start_of_msg = the_social->search + 1;

 assert(the_social->search && *the_social->search);
 
 if (*start_of_msg == ' ')
   start_of_msg++;

 switch (*the_social->search)
 {
 case '<':
  /*check here for imediate (flag) problems - only NO_MESSAGE*/
  if (the_social->info & NO_MESSAGE)
  {
   fvtell_player(SYSTEM_T(p),
		 " The ^B%s^b social does not support you adding a start "
		 "message (<) to it, sorry.\n", current_command);
   return (FALSE);
  }

  /*set the set message bit*/
  the_social->parsed_info |= (HAS_SET_MSG);
  
  /*check here for imediate (non flag) fail problems... ie: no message*/
  if (!*start_of_msg)
  {
   /*must intend '<' to be the whole of the message!*/
   the_social->opt_str = the_social->search;
   the_social->search = 0; /*end the message loop, we're done here*/
   return (TRUE);
  }

  check_for_msg_controller(the_social, '>'); /*check for other msgs*/
  /*check for the space or not?*/
  the_social->opt_space = isits1(start_of_msg);
  the_social->opt_str = isits2(start_of_msg); /*setup the pointer*/
  the_social->search = 0; /*end the message loop, we're done here*/
  return (TRUE);

  break;
  
 case '>':
  /*check here for imediate (flag) problems*/
  if (the_social->info & NO_END_MESSAGE)
  {
   fvtell_player(SYSTEM_T(p),
		 " The ^B%s^b social does not support you adding an end "
		 "message (>) to it, sorry.\n", current_command);
   return (FALSE);
  }

  /*set the set message bit*/
  the_social->parsed_info |= (HAS_SET_END_MSG);

  /*check here for imediate (non flag) fail problems... ie: no message*/
  if (!*start_of_msg)
  {
   /*must intend '>' to be the whole of the message!*/
   the_social->opt_end_str = the_social->search;
   the_social->search = 0; /*end the message loop, we're done here*/
   return (TRUE);
  }
  
  check_for_msg_controller(the_social, '<');
  /*check for the space or not?*/
  the_social->end_space = isits1(start_of_msg);
  the_social->opt_end_str = isits2(start_of_msg);
  the_social->search = 0; /*end the message loop, we're done here*/
  return (TRUE);

  break;
  
 default:
  handle_social_error(the_social, "Msg_social_parse called with a non "
		      "msg controller.");
 }
 
 return (FALSE);
}

static int remote_social_parse(social_obj *the_social)
{
 multi_return *values;
 int multi_flags = (MULTI_DIE_STOPPED|
		    MULTI_DIE_MATCH_NAME|
		    MULTI_DIE_MATCH_MULTI|
		    MULTI_DIE_MATCH_GROUP|
		    MULTI_DIE_EMPTY_GROUP|
		    MULTI_LIVE_ON_SMALL);
 unsigned int number_to_check = 5;
 char *end_of_names = (the_social->search + 1);
 char *start_of_names = (the_social->search + 1);
 player *p = the_social->p;
 
 /*check here for imediate (flag) problems - remember this can't be a
   sublist for another controller*/
 if (the_social->info & NO_REMOTES)
 {
  fvtell_player(SYSTEM_T(p),
		" The ^B%s^b social does not support remotes (%c), sorry.\n",
		current_command, *the_social->search);
  return (FALSE);
 }
 
 /*check here for imediate (non flag) fail problems...*/
 if (the_social->multi_list > 0)
   return (check_which_previous_event_and_error(the_social,
						"remote",
						TO_LIST_OF_REMOTES));

 /*check for having been in a name list (sublist (-)) controller*/
 if (the_social->parsed_info & (TO_NAME_CONTROLLER))
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You must put a name sublist (-) after other "
                "controllers.\n");
  return (FALSE);
 }
 
 if (!*start_of_names) /*must specify a name of some kind - error*/
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You must specify a name when using a remote (/) "
                "controller.\n");
  return (FALSE);
 }
   
 /*get the end of the name and terminate it*/
 end_of_names = next_social_parameter(end_of_names, ' ');

 /*work out which flags for the add_to_multi func based on the social*/
 if (the_social->info & NO_TFOS)
   multi_flags |= (MULTI_NO_FRIENDS_OF);
 if (the_social->info & NO_MULTIS)
   multi_flags |= (MULTI_NO_MULTIS);
 if (the_social->info & NO_MINISTERS)
   multi_flags |= (MULTI_NO_MINISTERS);
 if (the_social->info & NO_SUS)
   multi_flags |= (MULTI_NO_SUS);
 if (the_social->info & NO_EVERYONE)
   multi_flags |= (MULTI_NO_EVERYONE);
 if (the_social->info & NO_FRIENDS)
   multi_flags |= (MULTI_NO_FRIENDS);
 if (the_social->info & NO_ROOM)
   multi_flags |= (MULTI_NO_ROOM);
 if (the_social->info & NO_NEWBIES)
   multi_flags |= (MULTI_NO_NEWBIES);
 
 /* call the 'tag' multi function */
 values = multi_add(p->saved, start_of_names, multi_flags,
                    PLAYER_FIND_SC_COMS | PLAYER_FIND_SELF);

 /*means a multi has been created, therefore its to a group*/
 if ((the_social->multi_list = values->multi_number))
 {
  /*move the search along past this controller section*/
  the_social->search = end_of_names;
  
  /*check for yourself being specifically added*/
  if (multi_check_for_flag(values->multi_number, MULTI_TO_SELF))
  {
   the_social->parsed_info |= (TO_SELF);
   /*as you count as one, then we check for one less person*/
   number_to_check--;
  }

  /* I'm not sure if this ever works or what I was intending at the time
     - I think under certain conditions I didn't want a multi number, but
     you always get one now and it looks okay - pass! */
  if ((values->players_added < 3) && !(the_social->parsed_info & TO_SELF) &&
      (values->error_number == MULTI_CREATED))
    the_social->parsed_info |= (TO_SINGLE_REMOTE);
  else if (values->players_added < number_to_check)
    the_social->parsed_info |= (TO_LIST_OF_REMOTES);
  else
    the_social->parsed_info |= (TO_LIST_OF_REMOTES|TO_MULTI);
  return (TRUE);
 }
 else
   return (check_error_flags_social(values, the_social, "remote"));
}

static int name_social_parse(social_obj *the_social)
{
 multi_return *values;
 int multi_flags = (MULTI_DIE_STOPPED|
		    MULTI_DIE_MATCH_GROUP|
		    MULTI_DIE_MATCH_NAME|
		    MULTI_DIE_MATCH_MULTI|
		    MULTI_DIE_EMPTY_GROUP|
		    MULTI_IGNORE_PLAYERS|
		    MULTI_MUST_CREATE|
		    MULTI_DESTROY_CLEANUP|
		    MULTI_COMPLETE_IGNORE|
		    MULTI_LIVE_ON_SMALL);
 char *end_of_names = (the_social->search + 1);
 char *start_of_names = (the_social->search + 1);
 player *p = the_social->p;

 /*check here for imediate (flag) problems -
   no_local is checked in master social cause this could be a sub_list*/
 if (the_social->parsed_info & NO_NAMES)
 { /* FIXME: this doesn't work ... use kiss *m -name and %c = '-', which
    * is allowed */
  fvtell_player(SYSTEM_T(p),
		" The social ^B%s^b does not support a name list (%c), "
		"sorry.\n", current_command, *the_social->search);
  return (FALSE);
 }
 
 /*check here for imediate (non flag) fail problems...*/
 if (the_social->sub_multi_list > 0)
   return (check_which_previous_event_and_error(the_social,
						"specify a name list",
						TO_LIST_OF_LOCAL));

 if (!*start_of_names) /*must specify a name of some kind - error*/
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You must specify a player when using a name (-) "
                "controller.\n");
  return (FALSE);
 }
 
 /*get the end of the name and terminate it*/
 end_of_names = next_social_parameter(end_of_names, ' ');

 /*if there's no multi_list then this must be a room list not a sublist -
   means that sublists will have to be at the end of socials only!*/
 if (!(the_social->multi_list))
   multi_flags |= (MULTI_STAY_LOCAL);
 else /*must have multi list for -/ */
   /*check to see if its just a / so we go to the whole remote list*/
   if ((*start_of_names == '/') && !(*(start_of_names + 1)))
   {
    the_social->parsed_info |= (TO_NAME_CONTROLLER);

    /*setup our 'to' list to be the same as our multi list*/
    the_social->sub_multi_list = the_social->multi_list;

    /*now not going 'to no one'*/
    the_social->parsed_info &= ~TO_NO_ONE_SPECIFIC;
    
    /*check to see if you're specifically on the multi (now sub multi)*/
    if (multi_check_for_flag(the_social->sub_multi_list, MULTI_TO_SELF))
      the_social->parsed_info |= (TO_SELF);
    
    /* move along to after -/ */
    the_social->search = end_of_names;
    
    return (TRUE);
   }
		     
 /* call the 'tag' multi function */
 values = multi_add(p->saved, start_of_names, multi_flags,
                    PLAYER_FIND_SC_COMS | PLAYER_FIND_SELF);
 
 if ((the_social->sub_multi_list = values->multi_number))
 {
  if ((the_social->parsed_info & TO_GEN_CHANNEL) &&
      !check_valid_channel(the_social))
    return (FALSE);
  
  /*check for yourself being specifically added*/
  if (multi_check_for_flag(values->multi_number, MULTI_TO_SELF))
    the_social->parsed_info |= (TO_SELF);

  /*now not going to no one*/
  the_social->parsed_info &= ~TO_NO_ONE_SPECIFIC;
  
  /*move the search along past this controller section*/
  the_social->search = end_of_names;
  if (!(the_social->multi_list))
    the_social->parsed_info |= (TO_LIST_OF_LOCAL);
  the_social->parsed_info |= (TO_NAME_CONTROLLER);
  return (TRUE);
 }
 else
   return (check_error_flags_social(values, the_social, ""));
}

/*assumes no controllers and therefore only 'tags' the room*/
static int no_one_social_parse(social_obj *the_social)
{
 char name_buffer[] = "room";
 player *p = the_social->p;
 multi_return *values;
 
 /* call the 'tag' multi function */
 values = multi_add(p->saved, name_buffer,
		       (MULTI_DIE_STOPPED|
			MULTI_DIE_MATCH_GROUP|
		/*	MULTI_DIE_EMPTY_GROUP| Don't want, ok to empty room*/
			MULTI_LIVE_ON_SMALL|
			MULTI_IGNORE_PLAYERS|
			MULTI_MUST_CREATE|
			MULTI_DESTROY_CLEANUP|
			MULTI_COMPLETE_IGNORE|
			MULTI_STAY_LOCAL /*hardly needed, room always local*/
			),
                    PLAYER_FIND_SC_COMS | PLAYER_FIND_SELF);

 if ((the_social->multi_list = values->multi_number))
 {
  the_social->parsed_info |= (TO_ROOM); /*must be going to the room*/
  return (TRUE);
 }
 else /* any other problems are errors! */
   handle_social_error(the_social, "Couldn't tag the room and get multi");

 /* The following should never occur now 
 else
 fvtell_player(SYSTEM_T(p), "%s",
	       " There is currently no one in the room to be "
	       "sociable with.\n"); */

 return (FALSE);
}

/* Based on the controller (pointed to by the_social->search), and previous
 * assignments done, this checks that a valid string is supplied by the user
 * and returns a 1 if it is. If the controller isn't supported then this
 * returns -1. If the user messed up, this returns 0. Outputs appropriate
 * error message to user too. All checks and flag checks should go in the
 * appropriate sub function. */
static int check_appropriate_string_and_tag(social_obj *the_social)
{
 int ret_val = TRUE;
 char *controller = the_social->search;
 player *p = the_social->p;
 
 assert (the_social);
 assert (the_social->search && *the_social->search);
 
 switch (*controller)
 {
  case '}':
  case '{':
    /* previous elements or non flag elements that will cause this to fail are:
       {, }, *, or + (basically a multi_list) */
    ret_val = tf_social_parse(the_social);
    break;
    
  case '+':
  case '*':
    /* previous elements or non flag elements that will cause this to fail are:
       {, }, *, + (basically a multi_list), if no USER_CHANNELS then
       channel !m, !sp, !cc or !su, if *su and no CHANNEL priv, if *s and no
       SPOD priv, if *m and not married, if multi and blocking multis */
    ret_val = channel_social_parse(the_social);
    break;
    
  case '-':
    /* previous elements or non flag elements that will cause this to fail are:
       any of { } * or + WITH - or /, name not > 2 chars, friends and no
       friends, sus or ministers and no CHANNEL, MINISTER or CODER priv,
       / or - and not TO_SPECIAL_GROUP, to room and no location, to everyone
       and blocking shouts, or name thats not on talker */
    ret_val = name_social_parse(the_social);
    break;
    
  case '/':
    /* previous elements or non flag elements that will cause this to fail are:
       any of { } * or + WITH - or /, name not > 2 chars, friends and no
       friends, sus or ministers and no CHANNEL, MINISTER or CODER priv, multi
       digit > than MAX_START_MULTIS, / or - and not TO_SPECIAL_GROUP, to room
       and no location, or to everyone and blocking shouts */
    ret_val = remote_social_parse(the_social);
    break;
    
  case '<':
  case '>':
    /* previous elements or non flag elements that will cause these to fail
       are:
       any previous message (can/should never occur, no check) */
    ret_val = msg_social_parse(the_social);
    break;
    
  default: /* no valid controller found! should have been checked by now! */
    assert(FALSE);
    vwlog("error", "Check_appropriate_string called with invalid controller!"
          " (%s)", current_command);
    fvtell_player(SYSTEM_T(p), "%s",
                  " An error has occured in parsing your social. "
                  "(See help socials).\n");
    ret_val = FALSE;
    break;
 }

 /*search pointer has been incremented in sub func(s) to move search onwards*/
 return (ret_val);
}

/* Returns true if the character passed is a controller, else returns false */
static int a_controller_character(char letter)
{
 char controllers[] = { '{', '}', '*', '-', '/', '<', '>', 0 };
 int i = 0;

 for (; controllers[i] != 0; i++)
   if (letter == controllers[i])
     return (TRUE);

 return (FALSE);
}

/* Check to see if search in the_social points at a controller, return true
 * if its a valid one, false other wise. Moves search on one if its an
 * invaliadted controller (-- for example) */
static int is_it_a_controller(social_obj *the_social)
{
 if (a_controller_character(*the_social->search)) /*is it a controller */
 {
  if ((*the_social->search) == (*the_social->search + 1)) /*its a double*/
  {
   the_social->search++; /*move on*/
   return (FALSE); /*its not valid*/
  }
  else if ((*the_social->search != '<') && /*its not a message (these*/
           (*the_social->search != '>')) /*have special rules)*/
  {
   if (*(the_social->search + 1) != ' ') /*next is not a space*/
     return (TRUE); /*yes, its valid*/
   else /*its a space after it, so...*/
     return (FALSE); /*no, its not valid*/
  }
   else
     return (TRUE); /*its a controller thats valid*/
 }
 
 /* if a non match has occured, a non controller, then return false */
 return (FALSE);
}

/*check to see if the player has auto add name on so that they can use
 socials in a similar fashion to the old system - assumes first thing
 to be a name unless there's a controller there - also does the assume
 local names flag*/
static void check_the_assume_flags_and_modify(social_obj *the_social)
{
 /*note that if a social doesn't allow a local name list for example, 
   then the assume flag will be ignored (as will auto name flags. Also 
   note that flags are checked in order from lowest to highest value*/
 if (((the_social->p->flag_social_auto_name &&
       !((the_social->info & SOCIAL_ASSUME_RNAME) ||
         (the_social->info & (SOCIAL_ASSUME_END_STR)) ||
         (the_social->info & SOCIAL_ASSUME_OPT_STR))) || 
      (the_social->info & SOCIAL_ASSUME_LNAME)) && 
     !(the_social->info & NO_NAMES))
 {
  /*if the first char isn't any kind of controller - and could be a name*/
  if (isalpha((unsigned char) *the_social->str))
  {
   /*add in the minus*/
   *(the_social->str - 1) = '-';
    /*redo the pointers*/
   the_social->str = (the_social->str - 1);
   the_social->search = the_social->str;
  }
 }
 else if ((the_social->info & SOCIAL_ASSUME_RNAME) && 
          !(the_social->info & NO_REMOTES))
 {
  /*if the first char isn't any kind of controller - and could be a name*/
  if (isalpha((unsigned char) *the_social->str))
  {
   /*add in the forward slash*/
   *(the_social->str - 1) = '/';
   /*redo the pointers*/
   the_social->str = (the_social->str - 1);
   the_social->search = the_social->str;
  }
 }
 else if ((the_social->info & SOCIAL_ASSUME_END_STR) && 
          !(the_social->info & NO_END_MESSAGE))
 {
  /*if the first char isn't any kind of controller - and could be a name*/
  if (!a_controller_character(*the_social->str))
  {
   /*add in the greater than*/
   *(the_social->str - 1) = '>';
   /*redo the pointers*/
   the_social->str = (the_social->str - 1);
   the_social->search = the_social->str;
  }
 }
 else if ((the_social->info & SOCIAL_ASSUME_OPT_STR) && 
          !(the_social->info & NO_MESSAGE))
 {
  /*if the first char isn't any kind of controller - and could be a name*/
  if (!a_controller_character(*the_social->str))
  {
   /*add in the less than*/
   *(the_social->str - 1) = '<';
   /*redo the pointers*/
   the_social->str = (the_social->str - 1);
   the_social->search = the_social->str;
  }
 }
}

static int parse_social_with_str(social_obj *the_social)
{
 player *p = the_social->p;
 int ret_val = FALSE;
 
 assert(the_social);
 assert(the_social->p);

 the_social->search = the_social->str;
 
 assert(the_social->search && *(the_social->search));
 
 /*assume that the social is going to no one and UNSET if it succeeds in going
   to someone*/
 the_social->parsed_info |= (TO_NO_ONE|TO_NO_ONE_SPECIFIC);

 /*now check the default assume flags, to modify input*/
 check_the_assume_flags_and_modify(the_social);
 
 while (the_social->search && *the_social->search)
 {
  if (is_it_a_controller(the_social))
    if (check_appropriate_string_and_tag(the_social))
    {
     /* this means that they specified a valid string for their controller */
     /*check for going to someone - anyone!*/
     if ((the_social->sub_multi_list) || (the_social->multi_list))
       the_social->parsed_info &= ~(TO_NO_ONE);
     /*check for more*/
     if (the_social->search && *the_social->search)
       continue; /* carry on with the while loop now */
     else
       ret_val = TRUE; /* else we're done ok! */
    }
    else /* this means that they messed up or controller isn't supported atm */
    {
     ret_val = FALSE;
     /* end loop, error occured due to user */
     the_social->search = N_strchr(the_social->str, 0);
     the_social->parsed_info &= ~(TO_NO_ONE_SPECIFIC);
    }
  else /* by now its just a string that they specified, as a message */
  {
   if (the_social->info & NO_MESSAGE)
   {
    fvtell_player(SYSTEM_T(p),
                  " This ^B%s^b social doesn't support you adding "
                  "start messages (<) to it, sorry.\n", current_command);
    ret_val = FALSE;
   }
   else
   {/*assign the remaining search str to opt_str unless its not allowd */
    if ((the_social->info & (SOCIAL_LAST_END_STR|SOCIAL_ASSUME_END_STR)))
    {
     the_social->parsed_info |= (HAS_SET_END_MSG);
     the_social->end_space = isits1(the_social->search);
     the_social->opt_end_str = isits2(the_social->search);
     check_for_msg_controller(the_social, '<');
    }
    else
    {
     the_social->parsed_info |= (HAS_SET_MSG);
     the_social->opt_space = isits1(the_social->search);
     the_social->opt_str = isits2(the_social->search);
     check_for_msg_controller(the_social, '>');
    }

    /*we're here and the whole social parsed, we may have sublists and so on*/
    ret_val = TRUE;
   } 
   /*everything else has parsed by now, or there's just this accepted or
     regected string, so its 'tag' the social. This is the only condition
     under which an error on the users part (sending a string when one is
     not allowd) is ignored. */   
  }
  the_social->search = N_strchr(the_social->str, 0); /* quit the while loop */
 } /*end of while*/

 /* automatically doing the -/ thing ... ? */
 if ((the_social->info & SOCIAL_AUTO_SLASH) && 
     (the_social->multi_list) &&
     (the_social->parsed_info & TO_NO_ONE_SPECIFIC) &&
     !(the_social->info & NO_NAMES) &&
     !(the_social->parsed_info & TO_SPECIAL_GROUP)) 
 {
  the_social->parsed_info |= (TO_NAME_CONTROLLER);
  
  /*setup our 'to' list to be the same as our multi list*/
  the_social->sub_multi_list = the_social->multi_list;
  
  /*now going to someone*/
  the_social->parsed_info &= ~TO_NO_ONE_SPECIFIC;
  
  /*check to see if you're specifically on the multi (now sub multi)*/
  if (multi_check_for_flag(the_social->sub_multi_list, MULTI_TO_SELF))
    the_social->parsed_info |= (TO_SELF);
 }
 
 if (ret_val) /* success so far! */
   if ((the_social->info & NO_TO_NO_ONE) &&
       (the_social->parsed_info & TO_NO_ONE_SPECIFIC))
   {
    fvtell_player(SYSTEM_T(p),
		  " The ^B%s^b social has to have a sublist to direct it "
		  "at specific player(s), use the - controller.\n",
		  current_command);
    ret_val = (FALSE);
   }
   else if ((the_social->info & NO_NO_MESSAGE) &&
	    !(the_social->parsed_info & HAS_SET_MSG))
   {
    fvtell_player(SYSTEM_T(p),
		  " This ^B%s^b social needs to have a start message "
		  "added to it, try again!\n", current_command);
    ret_val = (FALSE);
   }
   else if ((the_social->info & NO_NO_END_MESSAGE) &&
     	    !(the_social->parsed_info & HAS_SET_END_MSG))
   {
    fvtell_player(SYSTEM_T(p),
		  " This ^B%s^b social needs to have an end message (>) "
		  "added to it, try again!\n", current_command);
    ret_val = (FALSE);
   }
   else
   {   
    /* if its to no one (no names specified) or its to a local list which
       is NOT a _sublist_ of people, then multi list needs to be the people
       in the room - final action for the social parsing function */	
    if ((the_social->parsed_info & TO_NO_ONE) ||
	(the_social->parsed_info & TO_LIST_OF_LOCAL))
      no_one_social_parse(the_social);
   }
 else
   cleanup_social(the_social);
 
 return (ret_val);
}
 
static int parse_social(social_obj *the_social)
{
 player *p = the_social->p;
 int ret_val = FALSE;
 
 assert(the_social);
 assert(the_social->p);

 if (!(the_social->info & SOCIAL_DISABLED))
   if (*the_social->str)
     ret_val = parse_social_with_str(the_social);
   else if (the_social->info & NO_TO_NO_ONE)
     fvtell_player(SYSTEM_T(p),
		   " This ^B%s^b social needs to be directed at a player, "
		   "group or channel, try again.\n", current_command);
   else if (the_social->info & NO_NO_MESSAGE)
     fvtell_player(SYSTEM_T(p),
		   " This ^B%s^b social needs to have a start message "
		   "added to it, try again!\n", current_command);
   else if (the_social->info & NO_NO_END_MESSAGE)
     fvtell_player(SYSTEM_T(p),
		   " This ^B%s^b social needs to have an end message (>) "
		   "added to it, try again!\n", current_command);
   else /*tag the room, and we don't care if there's no one in it*/
   {
    /*going to just the room*/
    the_social->parsed_info |= (TO_NO_ONE|TO_NO_ONE_SPECIFIC); 
    no_one_social_parse(the_social);
    ret_val = TRUE;
   }
 else
   fvtell_player(SYSTEM_T(p),
		 " The ^B%s^b social is currently disabled, sorry.\n",
		 current_command);

 /*if social parsed, then check for final swap wrappers then swap params too*/
 if (ret_val && (the_social->info & SOCIAL_SWAP_WRAPPERS))
 {
  const char *tmp = the_social->opt_str;
  the_social->opt_str = the_social->opt_end_str;
  the_social->opt_end_str = tmp;
  /*need to swap the spaces of course...*/
  tmp = the_social->opt_space;
  the_social->opt_space = the_social->end_space;
  the_social->end_space = tmp;
 }
 
 return (ret_val);
}

/* This is the main function called by every social with the parameter of a
 * social structure contain player information, parameters and social
 * information generally. */
static void social(social_obj *the_social)
{ 
 /*make sure that if there's a system opt str, opt_space knows about it*/
 if (the_social->opt_str)
   the_social->opt_space = " ";
 /*make sure that if there's a system end str, end_space knows about it*/
 if (the_social->opt_end_str)
   the_social->end_space = " ";
 
 /*if social parses, then the struct has been filled out so its ok to execute*/
 if (parse_social(the_social))
   master_social(the_social);
 /*else the error message will already have been told out, so we've finished*/
}


static int parse_generic_social_flags(const char *str, player *p)
{
 int coma_index = 0;
 int info = 0;

 if (!str) /* no flags to process then, I guess */
   return (DEFAULT_SOCIAL);
 
 while ((coma_index = strcspn(str, ",")))
 {
  char buffer[50];

  {
   int space_front = strspn(str, " ");
   str += space_front;
   coma_index -= space_front;
  }

  if (coma_index > 49) /* flag is too big */
    return (DEFAULT_SOCIAL);

  memcpy(buffer, str, coma_index);
  buffer[coma_index] = 0;
  str += coma_index;
  
  if (!beg_strcmp(buffer, "default"))
    info |= DEFAULT_SOCIAL;
  else if (!beg_strcmp(buffer, "no names"))
    info |= NO_NAMES;
  else if (!beg_strcmp(buffer, "no generic channels"))
    info |= NO_GEN_CHANNEL;
  else if (!beg_strcmp(buffer, "no tell friendsof"))
    info |= NO_TFOS;
  else if (!beg_strcmp(buffer, "no message"))
    info |= NO_MESSAGE;
  else if (!beg_strcmp(buffer, "no remotes"))
    info |= NO_REMOTES;
  else if (!beg_strcmp(buffer, "no local"))
    info |= NO_LOCAL;
  else if (!beg_strcmp(buffer, "no marriage channel"))
    info |= NO_MARRIAGE_CHANNEL;
  else if (!beg_strcmp(buffer, "no multis"))
    info |= NO_MULTIS;
  else if (!beg_strcmp(buffer, "no ministers"))
    info |= NO_MINISTERS;
  else if (!beg_strcmp(buffer, "no sus"))
    info |= NO_SUS;
  else if (!beg_strcmp(buffer, "no everyone"))
    info |= NO_EVERYONE;
  else if (!beg_strcmp(buffer, "no friends"))
    info |= NO_FRIENDS;
  else if (!beg_strcmp(buffer, "no room"))
    info |= NO_ROOM;
  else if (!beg_strcmp(buffer, "no newbies"))
    info |= NO_NEWBIES;
  else if (!beg_strcmp(buffer, "no to no one"))
    info |= NO_TO_NO_ONE;
  else if (!beg_strcmp(buffer, "no no message"))
    info |= NO_NO_MESSAGE;
  else if (!beg_strcmp(buffer, "no no end message"))
    info |= NO_NO_END_MESSAGE;
  else if (!beg_strcmp(buffer, "no end message"))
    info |= NO_END_MESSAGE;
  else if (!beg_strcmp(buffer, "disabled"))
    info |= SOCIAL_DISABLED;
  else if (!beg_strcmp(buffer, "print after"))
    info |= SOCIAL_PRINT_AFTER;
  else if (!beg_strcmp(buffer, "assume local names"))
    info |= SOCIAL_ASSUME_LNAME;
  else if (!beg_strcmp(buffer, "assume remote names"))
    info |= SOCIAL_ASSUME_RNAME;
  else if (!beg_strcmp(buffer, "assume end string"))
    info |= SOCIAL_ASSUME_END_STR;
  else if (!beg_strcmp(buffer, "assume opt string"))
    info |= SOCIAL_ASSUME_OPT_STR;
  else if (!beg_strcmp(buffer, "assume last string is end"))
    info |= SOCIAL_LAST_END_STR;
  else if (!beg_strcmp(buffer, "auto slash"))
    info |= SOCIAL_AUTO_SLASH;
  else if (!beg_strcmp(buffer, "move pre"))
    info |= SOCIAL_MOVE_OPT;
  else if (!beg_strcmp(buffer, "move post"))
    info |= SOCIAL_MOVE_OPT_END;
  else if (!beg_strcmp(buffer, "swap opt strings"))
    info |= SOCIAL_SWAP_WRAPPERS;
  else if (!beg_strcmp(buffer, "no channels"))
    info |= NO_CHANNELS;
  else if (!beg_strcmp(buffer, "no named groups"))
    info |= NO_NAMED_GROUPS;
  else if (!beg_strcmp(buffer, "no user messages"))
    info |= NO_GROUPS;
  else if (!beg_strcmp(buffer, "no print first"))
    info |= SOCIAL_NO_PRINT_FIRST;
  else
    fvtell_player(NORMAL_T(p), " Invalid socials flag: %s\n", buffer);
  /*  else if (!beg_strcmp(buffer, "inactive"))
      info |= INACTIVE;
    */
  
  if (!*str)
    break;

  assert(*str == ',');
  ++str;
 }
 
 return (info);
}

/* GENERIC SOCIAL */
static void internal_generic_social(player *p, const char *str, int info)
{
 const char *info_str = NULL;
 int info_int = 0;
 
 const char *pfirst = NULL; /* the plural string that the social uses */
 const char *pafter = NULL; /* the plural after string that the social uses */
 const char *toself = NULL; /* the no action to self string - optional */
 const char *opt_str = NULL; /* the optional, func returned str, thats used */
 const char *opt_end_str = NULL; /* " , func returned end str, thats used */
 const char *wrap_pre = NULL; /* the pre string for any opt end str */
 const char *wrap_post = NULL; /* the post string for any opt end str */
 const char *after_string = NULL; /* currently unused (for after the name) */

 social_obj the_social = {NULL, NULL, ADD_DEFAULTS,
                          NULL, NULL, NULL, NULL, END_STUFF};

 /* get parameter stuff */
 int parameter_number = 0;
 GET_PARAMETER_DECL_CREATE(parameters, (1024 * 4));
 
 GET_PARAMETER_DECL_INIT(parameters, (1024 * 4));
  
 GENERIC_SOCIAL_SETUP(info_str, "first", "the flags");
 GENERIC_SOCIAL_SETUP(pfirst, "second", "the first social string");
 GENERIC_SOCIAL_SETUP(pafter, "third", "the second social string");
 GENERIC_SOCIAL_SETUP(toself, "fourth", "the to self string");
 GENERIC_SOCIAL_SETUP(opt_str, "fifth", "the optional string");
 GENERIC_SOCIAL_SETUP(opt_end_str, "sixth", "the optional end string");
 GENERIC_SOCIAL_SETUP(wrap_pre, "seventh", "the pre-wrap string");
 GENERIC_SOCIAL_SETUP(wrap_post, "eighth", "the post-wrap string");
 GENERIC_SOCIAL_SETUP(after_string, "ninth", "the unused string");

 str += strspn(str, " ");

 info_int = parse_generic_social_flags(info_str, p); 

 the_social.info |= info_int;
 the_social.pfirst = pfirst;
 the_social.pafter = pafter;
 the_social.sfirst = pfirst;
 the_social.safter = pafter;

 the_social.p = p;
 qstrcpy(socials_copy_input, str); the_social.str = socials_copy_input;
 the_social.toself = toself; /*the toself string*/ 
 the_social.opt_str = opt_str;
 the_social.opt_end_str = opt_end_str; /*string stuff*/
 the_social.wrap_pre = wrap_pre;
 the_social.wrap_post = wrap_post; 
 
 if (info)
   social_info(&the_social);
 else
   social(&the_social);
}

static void generic_social(player *p, const char *str)
{
 if (!*str)
   TELL_FORMAT(p, "<social_definition> [<social_parameters>]");
 
 internal_generic_social(p, str, FALSE);
}

static void generic_social_info(player *p, const char *str)
{
 internal_generic_social(p, str, TRUE);
}

/* Functions which return variable strins for use in opt_str's of socials */

static const char *socials_internal_map_def_bop[] = 
{
 "with an over heated monitor.",
 "with a wet cabbage.",
 "with a used hanky.",
 "with a floppy joystick.",
 "with an inflatable banana.",
 "with a soggy copy of the Oxford Dictionary.",
 "with a carton of yak's milk.",
 "with a still damp toilet brush.",
 "with a copy of the Hitch Hikers Guide to the Galaxy (c).",
 "with a lesser spotted Amazon mud eel.",
 "with a toilet door.",
 "with an ewok's angry grandmother.",
 "with Bill Gates bank balance... *OUCH*",
 "with a sticky doughnut (with extra jam).",
 "with a flea ridden matress.",
 "with a squishy computer.",
 "with a deflated inner tube.",
 "with Microsoft's average license aggreement. *SQUISH*",
 "with a bowl of sticky green jello.",
 "with a spods long-distance bill. *woah*",
 "with a giant Stay-Puft Marshmallow Man.",
 "with a defective swipey card.",
 "with a gooey interface. *yuck*",
 "with an Argentinian Antelope.",
 "with a plate of free bird seed from the Road Runner.",
 "with a giant bendy straw.",
 "with a house made out of Lego.",
 "with a greasy rusted bicycle chain.",
 "with the artist formerly known as Prince.",
 "with a tangled slinkey.",
 "with a rather large soldified space.",
 NULL
};

static const char *socials_internal_map_xmas_bop[] =
{
 "a half decorated Christmas tree.",
 "some mistletoe. *wink*",
 "with a partridge in a pear tree.",
 "with a turkey covered in cranberry sauce.",
 "with a stocking full of gifts you'll never use.",
 "with eight smelly reindeer.",
 "with a snowman named Frosty.",
 "with a wreath covered in snow.",
 "with a sleighload of wrapped presents no one will use.",
 "with a string of lights that don't work.",
 "with a creature that isn't stirring.",
 "with a cheezy Christmas ornament.",
 "with a stuffy, crowded shopping mall.",
 "with a half eaten candy cane.",
 "with the green socks Aunt Gertrude gave you.",
 "with a remedial Christmas Caroller.",
 "with the fruitcake you got last year. *OUCH*",
 "with a firey yule log.",
 "with a one-horse open sleigh.",
 "with a grouchy Scrooge.",
 "with frankinscence and myrrh.",
 "with the relatives descending on your house.",
 NULL
};

static const char *bop_str(void)
{
 if (configure.socials_xmas_strings)
   return (CHOOSE_CONST_OFFSET(socials_internal_map_xmas_bop));
 
 return (CHOOSE_CONST_OFFSET(socials_internal_map_def_bop));
}

static const char *socials_internal_map_def_sing[] = 
{
 "Log on, baby...one more time...",
 "If I had a million BPS connection...",
 "Log back on my wayward spod...",
 "And I would do anything to spod...",
 "I want to bop you like an animal...",
 "You've lost that logon feeling...",
 "You take my privs away...",
 "I used to love her, but I had to nuke her...",
 "Spodding next to you...in silent lucidity...",
 "I can't spod...with or without shoes...",
 "All day I dream about spods...",
 "Sitting around writing code all whacked on tuna bake...",
 "I'm only happy when I spod...",
 "Everybody wibbles...sometimes...",
 "How do you talk to an angel...",
 "I adore mi admin...",
 "I'm too sexy for the hub...",
 "Para baliar la CrazyLands...",
 "She's got Linux...and knows how to use it...",
 "She blinded me with socials...",
 "The lag is high and I'm logging off...",
 "Don't...give...me...warns...",
 "I think we're alone now...",
 "Show me logs, show me sweet little logs...",
 "The screen is so bright...I gotta wear shades...",
 "What if Nevyn was one of us...",
 "la la la",
 NULL
};

static const char *sing_str(void)
{
 return (CHOOSE_CONST_OFFSET(socials_internal_map_def_sing));
}

/* functions... */
/* show social info command, uses the coms command list to get a social and
   executes the social function with a null str */
static void user_show_social_info(player *p, parameter_holder *params)
{
 command_base *comlist = &cmds_section[CMDS_SECTION_SOCIAL];
 size_t cmd_count = 0;

 if (params->last_param != 1)
   TELL_FORMAT(p, "<social>");
 
 lower_case(GET_PARAMETER_STR(params, 1));
 
 while (cmd_count < comlist->size)
 {
  if (cmds_do_match(GET_PARAMETER_STR(params, 1), comlist->ptr[cmd_count],
                    p->flag_no_cmd_matching))
  {
   current_command = comlist->ptr[cmd_count]->name;
   cmds_run_func(&comlist->ptr[cmd_count]->func, p, NULL, 0);
   return;
  }
  
  ++cmd_count;
 }
 
 fvtell_player(SYSTEM_T(p), " Social - %s - not found.\n",
               GET_PARAMETER_STR(params, 1));
}

void user_configure_socials_use(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, configure.socials_use, TRUE,
                       " Socials are %senabled.\n",
                       " Socials are %sdisabled.\n", TRUE);

 configure_save(FALSE);
}

void user_configure_socials_xmas_strings(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, configure.socials_xmas_strings, TRUE,
                       " Socials will %stry and use Xmas strings.\n",
                       " Socials will %suse normal strings.\n", TRUE);

 configure_save(FALSE);
}

#include "socials_data.h"

void cmds_init_socials(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("generic_social", generic_social, CONST_CHARS, SYSTEM);
 CMDS_FLAG(no_expand);
 CMDS_PRIV(configure_socials);
 CMDS_ADD("generic_social_info", generic_social_info, CONST_CHARS, SYSTEM);
 CMDS_FLAG(no_expand);
 CMDS_PRIV(configure_socials);
 
 CMDS_ADD("social_information", user_show_social_info, PARSE_PARAMS,
          INFORMATION);
 CMDS_PRIV(configure_socials);
 CMDS_XTRA_SECTION(SYSTEM); CMDS_XTRA_SECTION(MISC);
 CMDS_ADD("social_name_auto", user_toggle_social_auto_name,
          CONST_CHARS, SETTINGS);
 CMDS_PRIV(configure_socials);

 /* automagick for socials... */
#undef EXTERN_SOCIALS_H
#undef SOCIAL
#define SOCIAL(x, ig2, ig3, ig4, ig5, ig6, ig7, ig8, ig9, ig10, ig11) \
 CMDS_ADD( # x , user_social_ ## x , CONST_CHARS, SOCIAL); \
 CMDS_PRIV(configure_socials);
#include "socials_data.h"
}
