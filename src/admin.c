#define ADMIN_C
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




static void user_su_rename_player(player *p, parameter_holder *params)
{
 char old_case_name[PLAYER_S_NAME_SZ + 2];
 player *p2 = NULL;

 if (params->last_param != 2)
   TELL_FORMAT(p, "<old-name> <new-name>");
    
 CHECK_DUTY(p);
 
 if (!(p2 = player_find_on(p, GET_PARAMETER_STR(params, 1),
                           PLAYER_FIND_SC_SU_ALL)))
   return;
 
 if (p2->saved->priv_base)
 {
  fvtell_player(NORMAL_T(p),
                " The player -- ^S^B%s^s -- is a resident.\n",
                p2->saved->name);
  return;
 }

 lower_case(GET_PARAMETER_STR(params, 2));
 
 if (player_tree_find_exact(GET_PARAMETER_STR(params, 2)))
 {
  fvtell_player(NORMAL_T(p), " The player -- ^S^B%s^s -- already exists.\n",
                GET_PARAMETER_STR(params, 2));
  return;
 }

 if (find_player_cronlist_exact(GET_PARAMETER_STR(params, 2)))
 {
  fvtell_player(NORMAL_T(p),
                " There is already another newbie with the name '%s' "
                "logged on, or logging on.\n", GET_PARAMETER_STR(params, 2));
  return;
 }

 if ((GET_PARAMETER_LENGTH(params, 2) > (PLAYER_S_NAME_SZ - 1)) ||
     (GET_PARAMETER_LENGTH(params, 2) < 2))
 {
  fvtell_player(NORMAL_T(p),
                " A player name needs to be between 2 and %d characters.\n",
                PLAYER_S_NAME_SZ - 1);
  return;
 }

 if (*(GET_PARAMETER_STR(params, 2) +
       strspn(GET_PARAMETER_STR(params, 2), ALPHABET_LOWER)))
 {
  fvtell_player(NORMAL_T(p), "%s", " Letters only in names, thanks.\n");
  return;
 }

 multis_destroy_for_player(p2->saved);
 
 player_newbie_del(p2->saved);
 player_list_alpha_del(p2->saved);
 
 strcpy(old_case_name, p2->saved->name);

 COPY_STR_LEN(p2->saved->lower_name,
              GET_PARAMETER_STR(params, 2), GET_PARAMETER_LENGTH(params, 2));
 lower_case(p2->saved->lower_name);

 COPY_STR_LEN(p2->saved->name,
              GET_PARAMETER_STR(params, 2), GET_PARAMETER_LENGTH(params, 2));
 p2->saved->name[0] = toupper((unsigned char) p2->saved->name[0]);

 player_newbie_add(p2->saved);
 player_list_alpha_add(p2->saved);

 multis_init_for_player(p2->saved);

 vtell_room_wall(p2->location, p2,
                 " ^S^B%s%s^s dissolve%s in front of your eyes, and "
                 "rematerialis%s as ^S^B%s%s^s.\n",
                 gender_choose_str(p2->gender, "", "", "The ", "The "),
                 old_case_name,
                 gender_choose_str(p2->gender, "", "", "s", "s"),
                 (p->gender == GENDER_PLURAL) ? "es" : "",
                 gender_choose_str(p2->gender, "", "", "The ", "The "),
                 p2->saved->name);
 
 fvtell_player(SYSTEM_T(p2),
               "\n -=> %s%s %s just changed your name to be '%s'.\n\n",
               gender_choose_str(p->gender, "", "", "The ", "The "),
               p->saved->name,
               (p->gender == GENDER_PLURAL) ? "have" : "has",
               p2->saved->name);
 
 fvtell_player(NORMAL_T(p), "%s", " Done.\n");
 
 channels_wall("staff", 3, NULL,
               " -=> %s%s rename%s the player name ^S^B%s^s to ^S^B%s^s.%s",
               gender_choose_str(p->gender, "", "", "The ", "The "),
               p->saved->name,
               (p->gender == GENDER_PLURAL) ? "" : "s",
               old_case_name, p2->saved->name, "^N");
}

static void user_su_player_stats(player *p, const char *str)
{
 int sus_on = player_list_logon_staff_number();
 int newbies_on = player_newbie_number();

 ptell_mid(NORMAL_T(p), "SU level stats", FALSE);
 if (*str == '-')
   fvtell_player(NORMAL_T(p),
		"     Players on program: %3d\n"
		"           Normal resis: %3d\n\n",
		current_players,
		(current_players - (sus_on + newbies_on)));
else
  fvtell_player(NORMAL_T(p),
	       "     Players on program: %3d\n"
	       "             Newbies on: %3d\n"
	       "              Supers on: %3d\n"
	       "           Normal resis: %3d\n\n",
	       current_players,
	       newbies_on,
	       sus_on,
	       (current_players - (sus_on + newbies_on)));

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void user_su_list_residents(player *p, const char *str)
{
 player_tree_node *current = NULL;
 
 if (!*str || !isalpha((unsigned char) *str))
   TELL_FORMAT(p, "<name>");
 
 if (!(current = player_find_all(p, str,
                                 PLAYER_FIND_EXPAND | PLAYER_FIND_VERBOSE |
                                 PLAYER_FIND_SELF | PLAYER_FIND_PICK_FIRST)))
   return;

 ptell_mid(NORMAL_T(p), "Residents", FALSE);
 fvtell_player(NORMAL_T(p), "%s", "  [NAME]               [Privilages]\n");
 
 while (current && !beg_strcmp(str, current->lower_name))
 {
  if (PRIV_SYSTEM_ROOM(current))
    fvtell_player(NORMAL_T(p), "   %-21s%s\n", 
                  current->lower_name, "System room file.");
  else
  {
   int done = FALSE;

   fvtell_player(NORMAL_T(p), " %s%s%-21s",
                 P_IS_ON(current) ? "*" : " ",
                 P_IS_AVL(current) ? "+" : " ",
                 P_IS_ON(current) ? current->name : current->lower_name);
   
   P_SHOW_PRIV(p, current, 25, admin, "Admin");
   P_SHOW_PRIV(p, current, 25, lower_admin, "Lower Admin");
   P_SHOW_PRIV(p, current, 25, senior_su, "Ssu");
   P_SHOW_PRIV(p, current, 25, basic_su, "Su");
   P_SHOW_PRIV(p, current, 25, su_channel, "Channel");

   P_SHOW_PRIV(p, current, 25, coder, "Coder");
   P_SHOW_PRIV(p, current, 25, minister, "Minister");
   P_SHOW_PRIV(p, current, 25, spod, "Spod");
   P_SHOW_PRIV(p, current, 25, banished, "BANISHED");
   
   fvtell_player(NORMAL_T(p), "%s.\n", !done ? "Normal resident" : "");
  }

  current = current->next;
 }

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 pager(p, PAGER_USE_FORCE);
}

static void user_su_show_staff_list(player *p)
{
 player_linked_list *start = player_list_perm_staff_start();
 int count = 9; /* FIXME: minister aren't listed */

 if (!start)
 {
  fvtell_player(NORMAL_T(p), "%s",
                " There is currently no staff, or at least, no list.\n");
  log_assert(FALSE);
  return;
 }

 ptell_mid(NORMAL_T(p), "Staff list", FALSE);
 
 while (count > 0)
 {
  int done = FALSE;
  player_linked_list *current = start;
  
  switch (count)
  {
   case 9:
     fvtell_player(NORMAL_T(p), " %-12s", "Admin:");
     while (current)
     {
      P_SHOW_STAFF_PRIV(p, PLAYER_LINK_SAV_GET(current), 13, admin, banished);
      
      current = PLAYER_LINK_NEXT(current);
     }
     break; /* do nothing - admin */
     
   case 8:
     fvtell_player(NORMAL_T(p), " %-12s", "LA:");
     while (current)
     {
      P_SHOW_STAFF_PRIV(p, PLAYER_LINK_SAV_GET(current), 13,
                        lower_admin, admin);
      
      current = PLAYER_LINK_NEXT(current);
     }
     break;
     
   case 7:
     fvtell_player(NORMAL_T(p), " %-12s", "SSU:");
     while (current)
     {
      P_SHOW_STAFF_PRIV(p, PLAYER_LINK_SAV_GET(current), 13,
                        senior_su, lower_admin);
      
      current = PLAYER_LINK_NEXT(current);
     }
     break;
     
   case 6:
     fvtell_player(NORMAL_T(p), " %-12s", "SU:");
     while (current)
     {
      P_SHOW_STAFF_PRIV(p, PLAYER_LINK_SAV_GET(current), 13,
                        normal_su, senior_su);
      
      current = PLAYER_LINK_NEXT(current);
     }
     break;
     
   case 5:
     fvtell_player(NORMAL_T(p), " %-12s", "BSU:");
     while (current)
     {
      P_SHOW_STAFF_PRIV(p, PLAYER_LINK_SAV_GET(current), 13,
                        basic_su, normal_su);
      
      current = PLAYER_LINK_NEXT(current);
     }
     break;
     
   case 4:
     fvtell_player(NORMAL_T(p), " %-12s", "PSU:");
     while (current)
     {
      P_SHOW_STAFF_PRIV(p, PLAYER_LINK_SAV_GET(current), 13,
                        pretend_su, basic_su);
      
      current = PLAYER_LINK_NEXT(current);
     }
     break;
     
   case 3:
     fvtell_player(NORMAL_T(p), " %-12s", "Chan:");
     while (current)
     {
      P_SHOW_STAFF_PRIV(p, PLAYER_LINK_SAV_GET(current), 13,
                        su_channel, basic_su);
      
      current = PLAYER_LINK_NEXT(current);
     }
     break;
     
   case 2:
     fvtell_player(NORMAL_T(p), "%s", "\n Coder/Site SU: ");
     while (current)
     {
      P_SHOW_STAFF_PRIV(p, PLAYER_LINK_SAV_GET(current), 17,
                        coder, banished);
      
      current = PLAYER_LINK_NEXT(current);
     }
     break;
     
   case 1:
     fvtell_player(NORMAL_T(p), "%s", "\n Minister: ");
     while (current)
     {
      P_SHOW_STAFF_PRIV(p, PLAYER_LINK_SAV_GET(current), 12,
                        minister, banished);
      
      current = PLAYER_LINK_NEXT(current);
     }
     break;
     
   default:
     log_assert(FALSE);
     fvtell_player(NORMAL_T(p), "%s", " **** Error has occured.\n");
     return;
  }
  
  if (!done)
    fvtell_player(NORMAL_T(p), "%s", "---\n");
  else
    fvtell_player(NORMAL_T(p), "%s", ".\n");
  
  --count;
 }
 assert(!count);

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void inorder_search_email(player *scan, va_list va)
{
 player *p = va_arg(va, player *);
 pcre *compiled_code = va_arg(va, pcre *);
 pcre_extra *studied_code = va_arg(va, pcre_extra *);
 int *count = va_arg(va, int *);
 
 assert(scan->saved);
 
 if (pcre_exec(compiled_code, studied_code, scan->email, strlen(scan->email),
               0, 0, NULL, 0) >= 0)
 {
  if (!*count)
    ptell_mid(NORMAL_T(p), "Email match", FALSE);
  
  ++*count;
  fvtell_player(NORMAL_T(p),
                " (%d) Player ^S^B%s^s has a matching email address.\n",
                *count, scan->saved->name);
 }
}

static void user_su_email_search(player *p, const char *str)
{
 int count = 0;
 pcre *compiled_code = NULL;
 pcre_extra *studied_code = NULL;
 const char *errptr = NULL;
 int erroffset = 0;

 if (!*str)
   TELL_FORMAT(p, "<pcre-regexp>");

 if (!(compiled_code = pcre_compile(str, 0, &errptr, &erroffset, NULL)))
 {
  fvtell_player(SYSTEM_T(p), " Pcre err: at char %d (%s).\n",
                erroffset, errptr);
  return;
 }

 studied_code = pcre_study(compiled_code, 0, &errptr);

 do_inorder_all_load(inorder_search_email, p, compiled_code, studied_code,
                     &count);

 if (studied_code)
   (*pcre_free)(studied_code);
 (*pcre_free)(compiled_code);

 if (count)
   fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

void cmds_init_admin(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("rename", user_su_rename_player, PARSE_PARAMS, SU);
 CMDS_PRIV(pretend_su);
 
 CMDS_ADD("pstatistics", user_su_player_stats, CONST_CHARS, SU);
 CMDS_PRIV(pretend_su); 
 
 CMDS_ADD("list_residents", user_su_list_residents, CONST_CHARS, SU);
 CMDS_PRIV(pretend_su);

 CMDS_ADD("staff_list", user_su_show_staff_list, NO_CHARS, SU);
 CMDS_PRIV(pretend_su);

 CMDS_ADD("name_from_email", user_su_email_search, CONST_CHARS, SU);
 CMDS_PRIV(senior_su);
}
