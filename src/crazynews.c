#define CRAZYNEWS_C
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

typedef int (*vararg_hack)(player *);

static int internal_crazynews_check(player *p)
{
 return (p->flag_crazy_news);
}

static int internal_crazybulletin_check(player *p)
{
 return (p->flag_crazy_news_bulletin);
}

/* this actually extracts the email for crazy news */
static void get_an_email(player *p2, va_list va)
{
 FILE *fp = va_arg(va, FILE *);
 int (*func)(player *) = va_arg(va, vararg_hack);

 assert(fp);

 /* do we get the email? if so, write it out (don't care about success) */
 if (!p2->saved->priv_banished && (*func)(p2))
   if (p2->email[0])
     fprintf(fp, "%s\n", p2->email);
}

static void user_crazy_news_get_emails(player *p)
{
 FILE *fp = NULL;
 int fp_err = 0;
 
 if (!(fp = fopen(CRAZYNEWS_EMAILS_FILE, "wb")))
 {
  fvtell_player(NORMAL_T(p), "%s",
                " Could not create the emails file!\n");
  return;
 }
 
 fvtell_player(NORMAL_T(p), "%s", " Getting CrazyNews emails...\n");
 sys_wall(0, "%s",
          "\n -=> Program pausing for administration purposes.\n");
 socket_all_players_output();

 do_inorder_all_load(get_an_email, fp, internal_crazynews_check);

 if ((fp_err = fclose(fp)))
   fvtell_player(NORMAL_T(p),
                 " There has been an error closing the emails file (%d)!\n",
                 fp_err);
 
 sys_wall(0, "%s", " -=> All done, thankyou.\n\n");
 fvtell_player(NORMAL_T(p), " All done, emails file writen to: %s.\n",
               CRAZYNEWS_EMAILS_FILE);
}

static void user_crazy_news_get_bulletin_emails(player *p)
{
 FILE *fp = NULL;
 int fp_err = 0;
 
 if (!(fp = fopen(CRAZYNEWS_BULLETIN_EMAILS_FILE, "wb")))
 {
  fvtell_player(NORMAL_T(p), "%s",
               " Could not create the bulletin emails file (%d)!\n");
  return;
 }
 
 fvtell_player(NORMAL_T(p), "%s", " Getting CrazyNews bulletin emails...\n");
 sys_wall(0, "%s", "\n -=> Program pausing for administration purposes.\n");
 socket_all_players_output();

 do_inorder_all_load(get_an_email, fp, internal_crazybulletin_check);

 if ((fp_err = fclose(fp)))
   fvtell_player(NORMAL_T(p),
                 " There has been an error closing the emails file (%d)!\n",
                 fp_err);
 
 sys_wall(0, "%s", " -=> All done, thankyou.\n\n");
 fvtell_player(NORMAL_T(p), " All done, emails file writen to: %s.\n",
               CRAZYNEWS_BULLETIN_EMAILS_FILE);
}

static void user_toggle_crazy_news(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_crazy_news, TRUE,
                       " You will %srecieve CrazyNews.\n",
                       " You will %snot recieve CrazyNews.\n",
                       TRUE);
}

static void user_toggle_crazy_news_bulletin(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_crazy_news_bulletin, TRUE,
                       " You will %srecieve CrazyNews bulletins.\n",
                       " You will %snot recieve CrazyNews bulletins.\n",
                       TRUE);
}

void cmds_init_crazynews(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("crazynews", user_toggle_crazy_news, CONST_CHARS, SETTINGS);
 CMDS_PRIV(base);
 CMDS_ADD("crazynews_bulletin", user_toggle_crazy_news_bulletin,
          CONST_CHARS, SETTINGS);
 CMDS_PRIV(base);
 CMDS_ADD("crazynews_bulletin_emails", user_crazy_news_get_bulletin_emails,
          NO_CHARS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(coder_admin);
 CMDS_ADD("crazynews_emails", user_crazy_news_get_emails, NO_CHARS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(coder_admin);
}
