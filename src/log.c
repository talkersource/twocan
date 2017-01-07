#define LOG_C
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

static FILE *log_error = NULL;


static void log_chop_file(const char *filename, size_t min_size,
                          int round_upto_line)
{
 FILE *old_file = fopen(filename, "rb");
 int current_char = 0;

 if (!old_file)
   return;
 
 fseek(old_file, min_size, SEEK_SET);
 
 if (round_upto_line)
   while ((current_char = getc(old_file)) != EOF)
     if (current_char == '\n')
     {
      char buffer[1024];
      FILE *new_file = fopen("logs/chop-file.tmp", "wb");
      
      while (!feof(old_file))
      {
       int length = fread(buffer, sizeof(char), 1024, old_file);
       
       if (length > 0)
         fwrite(buffer, sizeof(char), length, new_file);
      }
      
      fclose(new_file);
      rename("logs/chop-file.tmp", filename);
      break;
     }

 fclose(old_file);
}

FILE *open_wlog(const char *file_name)
{
 char text[256 + sizeof("logs/%s.log")];
 FILE *log_out = NULL;
 char *tmp = disp_time_std(now, 0, TRUE, TRUE);
 size_t length = strlen(tmp);
 
 if (!sys_flag.panic)
 { /* don't log things if we are about to crash, as trace is being dumpped */
  BTRACE("open_wlog");
 }

 if (configure.talker_read_only)
   return (fopen("/dev/null", "ab"));

 if (!strcmp("error", file_name))
 {
  fwrite(tmp, sizeof(char), length, log_error);
  fwrite(" - ", sizeof(char), 3, log_error);
  
  return (log_error);
 }
 
 sprintf(text, "logs/%.*s.log", 256, file_name);
 
 if (TRUE) /* TODO: allow people specify certain files for non rotation */
 {
  struct stat buf;
  
  if (!stat(text, &buf) && (buf.st_size > LOG_SZ))
    log_chop_file(text, LOG_SZ / 4, TRUE);
 }
 
 if ((log_out = fopen(text, "ab")))
 {
  if ((length == fwrite(tmp, sizeof(char), length, log_out)) &&
      (3 == fwrite(" - ", sizeof(char), 3, log_out)))
    return (log_out);

  fclose(log_out);
 }
 
 return (NULL);
}

void close_wlog(FILE *log_out)
{
 fwrite("\n", sizeof(char), 1, log_out);
 if (log_out == log_error)
   fflush(log_out);
 else
   fclose(log_out);
}

/* NOTE - all log strings will automatically have a \n appended if one is
   not present... If you want two \n's in the file, then just do \n\n at the 
   end of your log func. string */
void vwlog(const char *file_name, const char *fmt, ...)
{
 FILE *log_out = open_wlog(file_name);
 VA_R_DECL(r_ap);
 VA_C_DECL(ap);
 
 VA_R_START(r_ap, fmt);

 if (configure.talker_verbose)
 {
  VA_C_START(ap, fmt);
  
  VA_C_COPY(ap, r_ap);
  
  fprintf(stderr, "Logging(\"%s", file_name);
  fprintf(stderr, "%s", "\") = ");
  vfprintf(stderr, fmt, ap);
  if (*(fmt + strlen(fmt) - 1) != '\n')
    putc('\n', stderr);

  fflush(stderr);
  
  VA_C_END(ap);
 }
 
 if (log_out)
 {
  VA_C_START(ap, fmt);
  
  VA_C_COPY(ap, r_ap);
  
  vfprintf(log_out, fmt, ap);
  if (*(fmt + strlen(fmt) - 1) != '\n')
    putc('\n', log_out);

  if (log_out == log_error)
    fflush(log_out);
  else
    fclose(log_out);
  
  VA_C_END(ap);
 } 

 VA_R_END(r_ap);
}

void log_pid(const char *filename, int force)
{
 FILE *fp = NULL;
 
 if (configure.talker_read_only)
   return; /* doesn't matter if multiple versions are running */

 if ((fp = fopen(filename, "rb")))
 {
  char buffer[BUF_NUM_TYPE_SZ(pid_t)];

  if (fgets(buffer, BUF_NUM_TYPE_SZ(pid_t), fp))
  {
   pid_t logged_pid = strtol(buffer, NULL, 10);

   if (force)
     kill(logged_pid, 9);
   else if (!kill(logged_pid, 0)) /* old version of proc is still alive */
   {
    log_assert(FALSE);
    exit (EXIT_FAILURE);
    return;
   }
  }
 }

 /* allow to write PID file even if read_only set, as long as something
  * isn't already running */
 if (!(fp = fopen(filename, "wb")))
 {
  log_assert(FALSE);
  exit (EXIT_FAILURE);
 }

 fprintf(fp, "%d", getpid());

 fclose(fp);
}

#ifdef TALKER_MAIN_H
static int internal_select_log_files(SCANDIR_STRUCT_DIRNET *dir_ent)
{
 const char *end_str = C_strchr(dir_ent->d_name, 0);
 
 if (((end_str - dir_ent->d_name) > 4) && !strcmp(end_str - 4, ".log"))
   return (TRUE);
 else
   return (FALSE);
}

static void user_su_log_view(player *p, const char *str)
{
 if (!( p->saved->priv_admin ||
        p->saved->priv_lower_admin  ||
        p->saved->priv_senior_su ||
        
        (PRIV_STAFF(p->saved) && !strcasecmp("blanked", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("bug", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("duty", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("idleouts", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("make", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("msgs", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("newbies", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("nuke", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("objections", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("rename", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("resies", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("session", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("suggest", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("timer_player", str)) ||
        (PRIV_STAFF(p->saved) && !strcasecmp("warn", str)) ||
        
        (p->saved->priv_minister && !strcasecmp("divorce", str)) ||
        (p->saved->priv_minister && !strcasecmp("marriage", str)) ||
        (p->saved->priv_minister && !strcasecmp("propose", str)) ||
        
        (p->saved->priv_coder && !strcasecmp("address", str)) ||
        (p->saved->priv_coder && !strcasecmp("dump", str)) ||
        (p->saved->priv_coder && !strcasecmp("error", str)) ||
        (p->saved->priv_coder && !strcasecmp("malloc", str)) ||
        (p->saved->priv_coder && !strcasecmp("trace", str)) ||
        
        FALSE))
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You are not allow to look at the log file -- ^S^B");
  fvtell_player(NORMAL_FT(RAW_OUTPUT, p), "%s", str);
  fvtell_player(NORMAL_T(p), "%s", "^s --.\n");
  return;
 }
 
 switch (*str)
 {
  case 0:
    TELL_FORMAT(p, "<log_file_name> | ?");
    
  case '?':
  {
   struct dirent **dir_ent_list = NULL;
   int log_files = scandir("logs/", &dir_ent_list,
                           internal_select_log_files, alphasort);

   if (log_files > 0)
   {
    int count = 0;
    char buffer[sizeof("Found %d log files") + BUF_NUM_TYPE_SZ(int)];
    
    /* *******************************************************************
     * these free's are for malloc's done inside scandir, so we __MUST__
     * use normal free
     * ***************************************************************** */
    sprintf(buffer, "Found %d log files", log_files);
    ptell_mid(NORMAL_T(p), buffer, FALSE);
    while (count < log_files)
    {
     fvtell_player(NORMAL_T(p), "%s\n", dir_ent_list[count]->d_name);
     free(dir_ent_list[count]);
     ++count;
    }
    fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
    
    pager(p, PAGER_DEFAULT);
   }
   else
     if (log_files == -1)
       if (errno == ENOMEM)
         P_MEM_ERR(p);
       else
         fvtell_player(SYSTEM_T(p),
                       " Error: open the log directory: %d %s\n",
                       errno, strerror(errno));
     else
       fvtell_player(NORMAL_T(p), "%s",
                     " There are no log files!\n");
   
   free(dir_ent_list); /* it _should_ either be NULL or something ... if it's
                        * something then it's been malloc'd */
  }
  break;
    
  case '.':
    fvtell_player(SYSTEM_T(p), "%s", " You can't give a file name "
                  "with a -- '^S^B.^s' -- at the start.\n");
    break;
    
  default:
  {
   char file_name[PATH_MAX + sizeof("logs/%.*s.log")];
   char *text = NULL;
   
   sprintf(file_name, "logs/%.*s.log", PATH_MAX, str);
   
   if ((text = file2text(file_name, NULL)))
   {
    ptell_mid(NORMAL_T(p), "Log file", FALSE);
    fvtell_player(NORMAL_WFT(RAW_OUTPUT, p), "%s", text);
    fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
    pager(p, PAGER_USE_FORCE);
    FREE(text);
   }
   else
     fvtell_player(SYSTEM_T(p), " Couldn't find logfile -- ^S^B%s^s --.\n",
                   str);
  }
 }
}

static int internal_log_email_bug(FILE *fp, void *passed_str)
{
 const char *str = passed_str;
 int len = strlen(str);
 int count = 0;
 int ret = 0;
 
 ret += fprintf(fp, "Time: %s (GMT)\n", disp_time_std(now, 0, TRUE, TRUE));
 ret += fprintf(fp, "Version: %s\n", VERSION);
 ret += fprintf(fp, "Package: %s\n", TALKER_CODE_SNAPSHOT);
 
 ret += fprintf(fp, "%s", " ---------- Bug ---------- \n");
 while (count < len)
 {
  ret += fprintf(fp, " %.*s\n", 74, str + count);
  count += 74;
 }
 ret += fprintf(fp, "%s", "\n");

 return (ret);
}

static int internal_log_extern_email_bug(FILE *fp, void *str)
{
 int ret = internal_log_email_bug(fp, str);

 ret += fprintf(fp, "%s\n", "-- ");
 ret += fprintf(fp, "%s\n", configure.email_from_long);
 ret += fprintf(fp, "%s\n", configure.url_access);
 ret += fprintf(fp, "%s\n", configure.url_web);

 return (ret);
}

static void user_bug(player *p, const char *str)
{
 if (!*str)
   TELL_FORMAT(p, "<error>");
 
 fvtell_player(NORMAL_T(p), "%s", " Bug logged, thank you.\n");

 vwlog("bug", "%s: %s", p->saved->name, str);

 channels_wall("log:bug", RAW_OUTPUT | HILIGHT | 3, NULL, " -=> %s: %s",
               p->saved->name, str);
 
 {
  char subj[CONFIGURE_NAME_SZ + sizeof("%s - Bug report from %s (%s)") +
           PLAYER_S_NAME_SZ + 128];

  sprintf(subj, "%s - Bug report from %s (%.*s)",
          configure.name_long, p->saved->name,
          128, disp_time_std(now, 0, TRUE, TRUE));
  
  email_report_local(p, configure.email_to_bugs, subj,
                     internal_log_email_bug, (char *)str, NULL); /* warning */
  if (configure.email_sendmail_extern_run &&
      p->saved->priv_command_extern_bug_suggest)
  {
   email_info ei = EMAIL_INFO_INIT();
   
   ei.to = configure.email_extern_bugs;
   ei.subject = subj;
   ei.func = internal_log_extern_email_bug;
   ei.param = (char *)str; /* warning */
   
   email_generic(&ei);
  }
 }
}

static int internal_log_email_suggestion(FILE *fp, void *passed_str)
{
 const char *str = passed_str;
 int len = strlen(str);
 int count = 0;
 int ret = 0;
 
 ret += fprintf(fp, "Time: %s (GMT)\n", disp_time_std(now, 0, TRUE, TRUE));
 ret += fprintf(fp, "Version: %s\n", VERSION);
 ret += fprintf(fp, "Package: %s\n", TALKER_CODE_SNAPSHOT);
 
 ret += fprintf(fp, "%s", " ---------- Suggestion ---------- \n");
 while (count < len)
 {
  ret += fprintf(fp, " %.*s\n", 74, str + count);
  count += 74;
 }
 ret += fprintf(fp, "%s", "\n");

 return (ret);
}

static int internal_log_extern_email_suggestion(FILE *fp, void *str)
{
 int ret = internal_log_email_suggestion(fp, str);

 ret += fprintf(fp, "%s\n", "-- ");
 ret += fprintf(fp, "%s\n", configure.email_from_long);
 ret += fprintf(fp, "%s\n", configure.url_access);
 ret += fprintf(fp, "%s\n", configure.url_web);

 return (ret);
}

static void user_suggestion(player *p, const char *str)
{
 if (!*str)
   TELL_FORMAT(p, "<suggestion>");
   
 fvtell_player(NORMAL_T(p), "%s", " Suggestion logged, thank you.\n");
 
 vwlog("suggest", "%s: %s", p->saved->name, str);
 
 channels_wall("log:suggest", RAW_OUTPUT | HILIGHT | 3, NULL, " -=> %s: %s",
               p->saved->name, str);

 {
  char subj[CONFIGURE_NAME_SZ + sizeof("%s - Suggestion from %s (%s)") +
           PLAYER_S_NAME_SZ + 128];
  
  sprintf(subj, "%s - Suggestion from %s (%.*s)",
          configure.name_long, p->saved->name,
          128, disp_time_std(now, 0, TRUE, TRUE));
  
  email_report_local(p, configure.email_to_suggest, subj,
                     internal_log_email_suggestion,
                     (char *)str, NULL); /* warning */
  if (configure.email_sendmail_extern_run &&
      p->saved->priv_command_extern_bug_suggest)
  {
   email_info ei = EMAIL_INFO_INIT();
   
   ei.to = configure.email_extern_suggest;
   ei.subject = subj;
   ei.func = internal_log_extern_email_suggestion;
   ei.param = (char *)str; /* warning */

   email_generic(&ei);
  }
 }
}

static void user_su_log_reopen_error(player *p)
{
 FILE *log_out = fopen("logs/error.log", "ab");

 if (!log_out)
   fvtell_player(NORMAL_T(p), "%s",
                 " Error, couldn't reopen error log file.\n");
 else
 {
  fvtell_player(NORMAL_T(p), "%s", " Reopened the error log file.\n");
  fclose(log_error);
  log_error = log_out;
 }
}

static void user_su_log_pgrep(player *p, parameter_holder *params)
{
 pcre *compiled_code = NULL;
 pcre_extra *studied_code = NULL;
 const char *errptr = NULL;
 int erroffset = 0;
 char buf[1024];
 unsigned int count = 1;
 
 if (params->last_param < 2)
   TELL_FORMAT(p, "<regexp> <file(s)> ...");
 
 if (!(compiled_code = pcre_compile(GET_PARAMETER_STR(params, 1),
                                    0, &errptr, &erroffset, NULL)))
 {
  fvtell_player(SYSTEM_T(p), " Pcre err: at char %d (%s).\n",
                erroffset, errptr);
  return;
 }

 studied_code = pcre_study(compiled_code, 0, &errptr);

 count = 1;
 while (count++ < params->last_param)
 {
  int prev_ret = TRUE;
  FILE *io = NULL;
  char file_name[PATH_MAX + sizeof("logs/%.*s.log")];

  if (GET_PARAMETER_STR(params, count)[0] == '.')
  {
   fvtell_player(SYSTEM_T(p), "%s", "** Error: You can't give a file name "
                 "with a -- '^S^B.^s' -- at the start.\n");
   continue;
  }

  sprintf(file_name, "logs/%.*s.log", PATH_MAX,
          GET_PARAMETER_STR(params, count));

  if (!(io = fopen(file_name, "rb")))
  {
   fvtell_player(SYSTEM_T(p), " Couldn't find logfile -- ^S^B%s^s --.\n",
                 GET_PARAMETER_STR(params, count));
   continue;
  }
  
  while (fgets(buf, sizeof(buf), io))
  {
   size_t len = strlen(buf);
   
   if (!prev_ret ||
       (pcre_exec(compiled_code, studied_code, buf, strlen(buf),
                  0, 0, NULL, 0) >= 0))
   {
    if (prev_ret)
      fvtell_player(NORMAL_FT(RAW_OUTPUT, p), "%s:",
                    GET_PARAMETER_STR(params, count));
    if (len)
    {
     fvtell_player(NORMAL_FT(RAW_OUTPUT, p), "%s", buf);
     if (buf[len - 1] == '\n')
       prev_ret = TRUE;
     else
       prev_ret = FALSE;
    }
   }
  }
  fclose(io);
  
  if (!prev_ret)
    fvtell_player(NORMAL_T(p), "%s", "\n");
 }

 if (studied_code)
   (*pcre_free)(studied_code);
 (*pcre_free)(compiled_code);
}

void init_log(void)
{
 if (!(log_error = fopen("logs/error.log", "ab")))
   exit (EXIT_FAILURE);
}

void cmds_init_log(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("vlog", user_su_log_view, CONST_CHARS, SU);
 CMDS_PRIV(spod_minister_coder_normal_su);
 CMDS_ADD("log_reopen_error", user_su_log_reopen_error, NO_CHARS, SU);
 CMDS_PRIV(coder_lower_admin);

 CMDS_ADD("log_pgrep", user_su_log_pgrep, PARSE_PARAMS, SU);
 
 CMDS_ADD("bug", user_bug, CONST_CHARS, SYSTEM);
 CMDS_PRIV(base);
 CMDS_ADD("suggestion", user_suggestion, CONST_CHARS, SYSTEM);
 CMDS_PRIV(base);
}
#endif
