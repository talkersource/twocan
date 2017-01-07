#define EMAIL_C
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

/* File to send emails out via a pretty function */

void email_generic(const email_info *ei)
{
 FILE *fp = NULL;

 assert(ei && ei->to);

 BTRACE("email_generic");
 
 if (! SENDMAIL_PATH [0]) /* no sendmail... no emails :p */
   return;

 if (!ei || !ei->to)
 {
  log_assert(FALSE);
  return;
 }

 if (!configure.email_sendmail_run)
   return;
 
 if (!(fp = popen(SENDMAIL_PATH " -i -t", "w")))
 {
  log_assert(FALSE);
  return;
 }
 
 fprintf(fp, "To: %s\n", ei->to);
 
 if (ei->cc)
   fprintf(fp, "Cc: %s\n", ei->cc);
 
 if (ei->bcc)
   fprintf(fp, "Bcc: %s\n", ei->bcc);

 if (ei->from)
   fprintf(fp, "From: %s\n", ei->from);
 else
   fprintf(fp, "From: %s\n", configure.email_from_long);

 if (ei->reply_to)
   fprintf(fp, "Reply-To: %s\n", ei->reply_to);

 if (ei->subject)
   fprintf(fp, "Subject: %s\n", ei->subject);

 fprintf(fp, "%s", "\n");

 if (ei->func)
   (*ei->func)(fp, ei->param);
 else if (ei->body)
   fprintf(fp, "%s", ei->body);
 
 pclose(fp);
}

void email_report_local(const player *p, const char *email, const char *subj,
                        int (*func)(FILE *, void *), void *param,
                        const char *body)
{
 email_info ei = EMAIL_INFO_INIT();
 char reply_to[(PLAYER_S_EMAIL_SZ * 2) + 2];

 assert(p && email);
 
 ei.to = email;
 
 if (p->email[0])
 {
  ei.from = p->email;
  sprintf(reply_to, "%.*s%c%.*s", PLAYER_S_EMAIL_SZ - 1, p->email, ',',
          PLAYER_S_EMAIL_SZ - 1, email);
 }
 else
  sprintf(reply_to, "%.*s", PLAYER_S_EMAIL_SZ - 1, email);

 ei.reply_to = reply_to;
 
 ei.subject = subj;

 ei.func = func;
 ei.param = param;
 ei.body = body;
 
 email_generic(&ei);
}

static void user_email_test(player *p)
{
#ifndef NDEBUG
 email_info ei = EMAIL_INFO_INIT();

 ei.to = "swiff";
 ei.from = "Wibble <merlin>";
 ei.reply_to = "Me <swiff>";
 ei.cc = "merlin";
 ei.bcc = "root";
 ei.subject = "This is actually a test...";
 ei.body = "Hi.\n\nThis is a test.\nLuv CL\n";

 email_generic(&ei);
 
 fvtell_player(NORMAL_T(p), "%s", "Done.\n");
#else
 fvtell_player(NORMAL_T(p), "%s", "Done nothing.\n");
#endif
}

/* Validates players emails as they set them. Checks for at least one @
   symbol and no spaces */
int email_validate_player(player *p, const char *str)
{
 int bad = 0;
 int count = 0;
 int atcount = 0;
 
 if (!*str)
   bad = 1; /* should be caught on enter by allowing people to not have one */
 
 if (!strcasecmp(str, "help") || !strcasecmp(str, "?"))
 {
  user_help(p, "emails");
  return (TRUE);
 }
 
 /* Scan the string checking for one space or more than one @ */
 for (count = 0; str[count] && !bad; count++)
   switch (str[count])
   {
    case ' ':
      bad = TRUE;
      break;

    case '@':
      atcount++;
      break;

    case '_':
      if (atcount) /* not allowed it in domain names */
        bad = TRUE;
      
    default:
      break;
   }
 
 if (!atcount)
   bad = TRUE;
 
 if (bad)
   fvtell_player(NORMAL_T(p), "%s",
                 " That seems to be an invalid email address, please try "
                 "again.\n Read help emails or talk to an SU if there is a "
                 "problem.\n");
 
 return (bad);
}

void user_configure_email_extern_bugs(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.email_extern_bugs, str,
                                "external bugs address",
                                CONFIGURE_EMAIL_SZ);
 configure_save(FALSE);
}

void user_configure_email_extern_suggest(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.email_extern_suggest, str,
                                "external suggest address",
                                CONFIGURE_EMAIL_SZ);
 configure_save(FALSE);
}

void user_configure_email_from_long(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.email_from_long, str,
                                "long from address", CONFIGURE_EMAIL_SZ);
 configure_save(FALSE);
}

void user_configure_email_from_short(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.email_from_short, str,
                                "short from address", CONFIGURE_EMAIL_SZ);
 configure_save(FALSE);
}

void user_configure_email_sendmail_run(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, configure.email_sendmail_run, TRUE,
                       " Mails (for boot/shutdown and bugs) will %sbe sent via. sendmail.\n",
                       " Mails will %sbe sent via. sendmail.\n", TRUE);

 configure_save(FALSE);
}

void user_configure_email_sendmail_extern_run(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, configure.email_sendmail_extern_run, TRUE,
                       " External mails will %sbe sent.\n",
                       " External emails will not %sbe sent.\n", TRUE);

 configure_save(FALSE);
}

void user_configure_email_to_abuse(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.email_to_abuse, str,
                                "abuse address", CONFIGURE_EMAIL_SZ);
 configure_save(FALSE);
}

void user_configure_email_to_admin(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.email_to_admin, str,
                                "admin address", CONFIGURE_EMAIL_SZ);
 configure_save(FALSE);
}

void user_configure_email_to_bugs(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.email_to_bugs, str,
                                "bugs address", CONFIGURE_EMAIL_SZ);
 configure_save(FALSE);
}

void user_configure_email_to_suggest(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.email_to_suggest, str,
                                "suggest address", CONFIGURE_EMAIL_SZ);
 configure_save(FALSE);
}

void user_configure_email_to_sus(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.email_to_sus, str,
                                "sus address", CONFIGURE_EMAIL_SZ);
 configure_save(FALSE);
}

void user_configure_email_to_up_down(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_TALKER_RAW(configure.email_to_up_down, str,
                                "up/down address", CONFIGURE_EMAIL_SZ);
 configure_save(FALSE);
}

void cmds_init_email(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("email_test", user_email_test, NO_CHARS, ADMIN);
 CMDS_PRIV(coder_admin);
}
