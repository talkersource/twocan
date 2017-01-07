#define AUTOEXEC_C
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


void autoexec_commands(player *p)
{
 int count = 32;
 int autoexec_copy = p->autoexec;

 BTRACE("autoexec_comands");
 
 if (autoexec_copy)
   for (; count--; autoexec_copy >>= 1)
     if (autoexec_copy & 1)
     {
      switch (count)
      {
       case 31:
       {
        parameter_holder params;
        const char *tmp = "";
        
        get_parameter_init(&params);
        get_parameter_parse(&params, &tmp, 1);
        user_mail_check(p, &params);
       }
       break;
         
       case 30:
         user_news_list_articles(p, ""); /* news check */
         break;
         
       case 29:
         user_finger(p, p->saved->lower_name); /* finger yourself */
         break;
         
       case 28:
         user_friend_who(p, ""); /* see friends */
         break;
         
       case 27:
         user_examine(p, p->saved->lower_name); /* examine yourself */
         break;
         
       case 26:
         user_motd(p); /* see motd */
         break;
         
       case 25:
         user_short_who(p, "");	/* see all ppl on */
         break;
         
       case 24:
         user_lsu(p, ""); /* see all sus on */
         break;
         
       case 23: /* run autoexec_start */
       {
        input_node *in_node = input_add(p, p->input_start);

        if (!in_node)
          return;
        
        INPUT_COPY(in_node, "autoexec_start", CONST_STRLEN("autoexec_start"));
        INPUT_TERMINATE(in_node);
        in_node->ready = TRUE;
       }
       break;
         
         /* fill in here as you please */
         
       case 1:
         if (PRIV_STAFF(p->saved))
           user_configure_talker_closed_to_newbies(p, "on");
         else
           p->autoexec &= ~(OPEN_TO_NEWBIES | GO_OFF_LSU_LIST);
         break;
         
       case 0:
         if (PRIV_STAFF(p->saved))
           /* go off the lsu list */
           user_su_toggle_off_lsu(p, "on");
         else
           p->autoexec &= ~(OPEN_TO_NEWBIES | GO_OFF_LSU_LIST);
         break;
         
       default:
         /* do nothing */;
      }
     }
}

static void user_autoexec_show_command(player *p)
{
 int count = 32;
 int autoexec_copy;
 int cmd_count = 0;
 
 if ((autoexec_copy = p->autoexec))
   for (; count--; autoexec_copy >>= 1)
     if (autoexec_copy & 1)
     {
      const char *command_name = NULL;
      
      switch (count)
      {
       case 31:
         command_name = "mailcheck";
         break;
         
       case 30:
         command_name = "newscheck";
         break;
         
       case 29:
         command_name = "finger";
         break;
         
       case 28:
         command_name = "fwho";
         break;
         
       case 27:
         command_name = "examine";
         break;
         
       case 26:
         command_name = "motd";
         break;
         
       case 25:
         command_name = "swho";
         break;
         
       case 24:
         command_name = "lsu";
         break;

       case 23:
         command_name = "USER ALIAS";
         break;
         
         /* fill in here as you please */
         
       case 1:
         command_name = "open";
         break;
         
       case 0:
         command_name = "offlsu";
         break;
         
       default:
         /* do nothing */;
      }
      
      if (!cmd_count)
        ptell_mid(NORMAL_T(p), "Autoexec", FALSE);
      
      ++cmd_count;
      
      fvtell_player(NORMAL_T(p), "%s\n", command_name);
     }
 
 if (cmd_count)
   fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 else
   fvtell_player(NORMAL_T(p), " You won't auto execute any "
                 "commands when you login.\n");
}

static void user_autoexec_toggle_command(player *p, const char *str)
{
 int change = 0;
 int correct = 1;
 const char *command_is = "";
 const char *the_command = "";
 
 switch(*str)
 {
 case 'e':
  change = EXAMINE_YOURSELF;
  command_is = "examine";
  break;

 case 'f':
  if (*(str + 1) != 'w')
  {
   change = FINGER_YOURSELF;
   command_is = "finger";
  }
  else
  {
   change = SEE_YOUR_FRIENDS;
   command_is = "fwho";
  }
  break;
    
 case 'l':
  change = SEE_ALL_SUS_ON;
  command_is = "lsu";
  break;
  
 case 'm':
  if (*(str + 1) != 'o')
  {
   change = MAIL_CHECKER;
   command_is = "mailcheck";
  }
  else
  {
   change = SEE_THE_MOTD;
   command_is = "motd";
  }
  break;

 case 'n':
   if ((p->saved->priv_basic_su || p->saved->priv_coder) &&
       (*(str + 3) == 'b'))
  {
   change = OPEN_TO_NEWBIES;
   command_is = "open";
  }
  else
  {
   change = NEWS_CHECKER;
   command_is = "newscheck";
  }
  break;

 case 'o':
  if (*(str + 1) != 'p')
  {
   if (p->saved->priv_basic_su || p->saved->priv_coder)
   {
    change = GO_OFF_LSU_LIST;
    command_is = "offlsu";
   }
  }
  else
    if (p->saved->priv_basic_su || p->saved->priv_coder)
    {
     change = OPEN_TO_NEWBIES;
     command_is = "open";
    }
  break;

 case 'U':
   change = AUTOEXEC_USER_DEFINED;
   command_is = "USER DEFINED";
   break;
   
 case 's':
   change = SEE_ALL_PPL_ON;
   command_is = "swho";
   break;
  
 default:
  ;
  
 }

 the_command = command_is;
 
 if (change)
 {
  while (*str && *command_is && correct)
    if (*str++ != *command_is++)
      correct = 0;
  if (correct)
  {
   if (p->autoexec & change)
     fvtell_player(NORMAL_T(p),
		   " You will not execute the command %s when you logon.\n",
		   the_command);
   else
     fvtell_player(NORMAL_T(p), 
		   " You will execute the command %s when you logon.\n",
		   the_command);
   p->autoexec ^= change;
  }
  else
    fvtell_player(NORMAL_T(p), "%s",
                  " That is not a valid autoexec command.\n");  
 }
 else
   fvtell_player(NORMAL_T(p), "%s",
                 " That is not a valid autoexec command.\n");
  
}

void cmds_init_autoexec(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("autoexec", user_autoexec_toggle_command, CONST_CHARS, SYSTEM);
 CMDS_PRIV(base);
 CMDS_ADD("show_autoexec", user_autoexec_show_command, NO_CHARS, INFORMATION);
 CMDS_PRIV(base);
}
