#define HELP_C
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

const char *help_primary_name = NULL;
const char *help_search_name = NULL;
player *help_shown_by = NULL;

#define HELP_ARRAY_SZ 10
help_base help_array[HELP_ARRAY_SZ] = 
{
 HELP_INIT_SECTION("files/help/rules", none),
 HELP_INIT_SECTION("files/help/general", none),
 HELP_INIT_SECTION("files/help/residents", base),
 HELP_INIT_SECTION("files/help/spod", spod),
 HELP_INIT_SECTION("files/help/minister", minister),
 /* hardly worth a whole bsu so let BSU see SU help */
 HELP_INIT_SECTION("files/help/su", basic_su),
 HELP_INIT_SECTION("files/help/senior_su", senior_su),
 HELP_INIT_SECTION("files/help/lower_admin", lower_admin),
 HELP_INIT_SECTION("files/help/admin", admin),
 HELP_INIT_SECTION("files/help/formats", admin),
};

static int help_match(const char *str, help_node *current, int expand)
{
 assert(current);

 assert(*current->search_name);
  
 if (current->expand && expand)
 {
  int ret = match_clever(current->search_name, &str,
                         MATCH_CLEVER_FLAG_ALLOW_SPACE |
                         (current->expand_special ? MATCH_CLEVER_FLAG_EXPAND :
                          MATCH_CLEVER_FLAG_DEFAULT));
  
  if (*str)
    return (-1);
  else
    return (ret);
 }
 else
   return (strcmp(current->search_name, str));
}

static help_node *help_find(const char *str, help_node *begginning,
                            int expand)
{
 help_node *current = begginning; 
 int result = -1;
 
 while (current && ((result = help_match(str, current, expand)) < 0))
   /* if it has been found don't increment */
   current = current->next;
 
 if (result)
   return (NULL);
 else
   return (current);
}

#if HELP_DEBUG
/* does an exact match for cmds searching */
int help_exists(const char *str)
{
 unsigned int count = 0;
 
 while (count < HELP_ARRAY_SZ)
 {  
  if (help_find(str, (help_array + count)->start, FALSE))
    return (TRUE); /* this one matches completely */
  
  ++count;
 }

 return (FALSE);
}
#endif

static void help_destroy_section(int count)
{
 help_node *current = 0;
 help_node *next = 0;

 assert(count < HELP_ARRAY_SZ);
 
 FREE((help_array + count)->malloc_start);
 (help_array + count)->malloc_start = NULL;

 next = (help_array + count)->start;
 while ((current = next))
 {
  next = current->next;
  XFREE(current, HELP_NODE);
 }
    
 (help_array + count)->start = NULL;
}

void help_destroy_all(void)
{
 int count = 0;
 help_node *current = 0;
 help_node *next = 0;
 
 while (count < HELP_ARRAY_SZ)
 {
  FREE((help_array + count)->malloc_start);
  (help_array + count)->malloc_start = 0; /* free text */

  /* free help nodes */
  next = (help_array + count)->start;
  while ((current = next))
  {
   next = current->next;
   FREE(current);
  }
    
  (help_array + count)->start = 0;
  ++count;
 }
}

static void help_add(int count, help_options *opts, const char *name)
{
 help_node *put_in = NULL;
 help_node *current = NULL;
 help_node *last = NULL;

 assert(count < HELP_ARRAY_SZ);

 current = (help_array + count)->start;

 assert(opts->header && opts->footer && opts->body &&
        opts->primary_name && name);
  
 while (current && (strcmp(current->search_name, name) < 0))
 {
  last = current;
  current = current->next;
 }
 
 if (!(put_in = XMALLOC(sizeof(help_node), HELP_NODE)))
   SHUTDOWN_MEM_ERR();

 put_in->body = opts->body;
 put_in->primary_name = opts->primary_name;
 put_in->search_name = name;

 put_in->keywords = opts->keywords;
 put_in->header = opts->header;
 put_in->footer = opts->footer;
 put_in->always_header = opts->always_header;
 put_in->always_footer = opts->always_footer;
 put_in->override_last = opts->override_last;
 put_in->expand = opts->expand;
 put_in->expand_special = opts->expand_special;
 put_in->telnet_client = opts->telnet_client;
 put_in->allow_find = opts->allow_find;
 
 if (current || last)
 {
  if (last)
  { /* not on head case */
   put_in->next = last->next;
   last->next = put_in;
  }
  else
  { /* head case */
   put_in->next = current;
   (help_array + count)->start = put_in;
  }
 }
 else
 {
  assert(!(help_array + count)->start);
  
  (help_array + count)->start = put_in;
  put_in->next = 0;
 }
}

static void help_parse_file(int count)
{
 help_options opts = {NULL, NULL, NULL, NULL, NULL,
                      FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
 char *scan = NULL;
 char *scan_next = NULL;

 assert(count < HELP_ARRAY_SZ);
 
 scan_next = (help_array + count)->msg_help.text;
 while ((scan = scan_next))
 {
  if ((scan_next = N_strchr(scan, '\n')))
    *scan_next++ = 0;

  if ((*scan == '#') || !*(scan + strspn(scan, "\t\v \r")))
  {
   continue;
  }
  else if (!BEG_CONST_STRCASECMP("expand-type: ", scan))
  {
   scan += CONST_STRLEN("expand-type: ");
   
   if (!BEG_CONST_STRCMP("expand_special", scan))
     opts.expand = opts.expand_special = TRUE;   
   else if (!BEG_CONST_STRCMP("expand", scan))
   {
    opts.expand = TRUE;
    opts.expand_special = FALSE;
   }
   else if (!BEG_CONST_STRCMP("exact", scan))
     opts.expand = opts.expand_special = FALSE;
   else
     vwlog("error", "Parse error in help file %s, Expand-type: %s\n",
           (help_array + count)->msg_help.file_name, scan);
  }
  else if (!BEG_CONST_STRCASECMP("telnet-client: ", scan))
  {
   scan += CONST_STRLEN("telnet-client: ");
   if (TOGGLE_MATCH_ON(scan))
     opts.telnet_client = TRUE;
   else if (TOGGLE_MATCH_OFF(scan))
     opts.telnet_client = FALSE;
   else
     vwlog("error", "Parse error in help file %s, Telnet-Client: %s\n",
           (help_array + count)->msg_help.file_name, scan);
  }
  else if (!BEG_CONST_STRCASECMP("allow-find: ", scan))
  {
   scan += CONST_STRLEN("allow-find: ");
   if (TOGGLE_MATCH_ON(scan))
     opts.allow_find = TRUE;
   else if (TOGGLE_MATCH_OFF(scan))
     opts.allow_find = FALSE;
   else
     vwlog("error", "Parse error in help file %s, Allow-Find: %s\n",
           (help_array + count)->msg_help.file_name, scan);
  }
  else if (!BEG_CONST_STRCASECMP("header: ", scan))
  {
   scan += CONST_STRLEN("header: ");
   opts.header = scan;
  }
  else if (!BEG_CONST_STRCASECMP("footer: ", scan))
  {
   scan += CONST_STRLEN("footer: ");
   opts.footer = scan;
  }
  else if (!BEG_CONST_STRCASECMP("always-header: ", scan))
  {
   scan += CONST_STRLEN("always-header: ");
   if (TOGGLE_MATCH_ON(scan))
     opts.always_header = TRUE;
   else if (TOGGLE_MATCH_OFF(scan))
     opts.always_header = FALSE;
   else
     vwlog("error", "Parse error in help file %s, Always-Header: %s\n",
           (help_array + count)->msg_help.file_name, scan);
  }
  else if (!BEG_CONST_STRCASECMP("always-footer: ", scan))
  {
   scan += CONST_STRLEN("always-footer: ");
   if (TOGGLE_MATCH_ON(scan))
     opts.always_footer = TRUE;
   else if (TOGGLE_MATCH_OFF(scan))
     opts.always_footer = FALSE;
   else
     vwlog("error", "Parse error in help file %s, Always-Footer: %s\n",
           (help_array + count)->msg_help.file_name, scan);
  }
  else if (!BEG_CONST_STRCASECMP("override-last: ", scan))
  {
   scan += CONST_STRLEN("override-last: ");
   if (TOGGLE_MATCH_ON(scan))
     opts.override_last = TRUE;
   else if (TOGGLE_MATCH_OFF(scan))
     opts.override_last = FALSE;
   else
     vwlog("error", "Parse error in help file %s, Override-Last: %s\n",
           (help_array + count)->msg_help.file_name, scan);
  }
  else if (!BEG_CONST_STRCASECMP("primary-name: ", scan))
  {
   scan += CONST_STRLEN("primary-name: ");

   if (opts.primary_name || !opts.body)
     vwlog("error", "Parse error in help file %s, Primary-Name: %s\n",
           (help_array + count)->msg_help.file_name, scan);
   else
   {
    opts.primary_name = scan;
    help_add(count, &opts, scan);
   }
  }
  else if (!BEG_CONST_STRCASECMP("other-name: ", scan))
  {
   scan += CONST_STRLEN("other-name: ");
   if (!opts.primary_name)
     vwlog("error", "Parse error in help file %s, Primary-Name: %s\n",
           (help_array + count)->msg_help.file_name, scan);
   else
     help_add(count, &opts, scan);
  }
  else if (!BEG_CONST_STRCASECMP("keywords: ", scan))
  {
   scan += CONST_STRLEN("keywords: ");
   if (!opts.body)
     vwlog("error", "Parse error in help file %s, Keywords: %s\n",
           (help_array + count)->msg_help.file_name, scan);
   else
     opts.keywords = scan;
  }
  else if (!BEG_CONST_STRCASECMP("body: ", scan))
  {
   scan += CONST_STRLEN("body: ");

   opts.primary_name = opts.keywords = NULL;
   opts.body = scan_next;

   while (scan_next)
   {
    if (!beg_strcmp(scan, scan_next))
    {
     char *tmp = scan_next + strlen(scan);
     tmp += strspn(tmp, " \t\v");
     
     if ((*tmp == '\n') || (*tmp == '\r'))
       break;
    }
    
    scan_next = N_strchr(scan_next, '\n');
    scan_next += strspn(scan_next, "\n\r");
   }
   
   if (!(scan_next && opts.header && opts.footer))
   {
    vwlog("error", "Parse error in help file %s, Body: %s\n",
          (help_array + count)->msg_help.file_name, scan);
    opts.body = NULL;
   }
   else
   {
    *scan_next++ = 0;
    scan_next = N_strchr(scan_next, '\n');
   }
  }
  else
  {
   vwlog("error", "Parse error in help file %s, %s\n",
         (help_array + count)->msg_help.file_name, scan);
  }  
 }
}

void init_help(void)
{
 int count = 0;

 while (count < HELP_ARRAY_SZ)
 {
  switch (MSG_RELOAD_FILE(&(help_array + count)->msg_help))
  {
   case MSG_FILE_ERROR:
     break;
     
   case MSG_FILE_CACHED:
     break;
     
   case MSG_FILE_READ:
     if ((help_array + count)->malloc_start)
      /* called when you want to reload help files */
       help_destroy_section(count);
     
     help_parse_file(count);
     break;
     
   default:
     assert(FALSE); 
  }
  ++count;
 }
}

void user_help(player *p, const char *str)
{
 int count = 0;

 assert(!help_primary_name);
 assert(!help_search_name);
 assert(!help_shown_by);
 
 if (!*str)
 {
  if (p->saved->priv_base)
    user_help(p, "general");
  else
    user_help(p, "newbie");
  return;
 }

 while (count < HELP_ARRAY_SZ)
 {
  help_node *help_file = NULL;
  help_base *current = (help_array + count);
  
  if ((*current->test_can_see)(p->saved) &&
      (help_file = help_find(str, current->start, TRUE)) &&
      help_file->allow_find)
  {
   if (!strcasecmp(str, help_file->search_name))
   {
    help_search_name = help_file->search_name;
    break; /* this one matches completely */
   }
   
   if (!help_search_name)
     help_search_name = help_file->search_name;
  }
  
  ++count;
 }

 if (help_search_name)
 {
  int done = FALSE;
  help_node *last = NULL;
  
  count = 0;
  while (count < HELP_ARRAY_SZ)
  {  
   help_node *help_file = NULL;
   help_base *current = (help_array + count);
   
   if ((*current->test_can_see)(p->saved) &&
       (help_file = help_find(help_search_name, current->start, FALSE)))
   {
    help_primary_name = help_file->primary_name;

    if (!done || help_file->always_header)
      fvtell_player(NORMAL_WFT(0, p), "%s\n", help_file->header);
    done = TRUE;
    
    fvtell_player(NORMAL_WFT(0, p), "%s", help_file->body);

    if (!last || help_file->override_last)
    {
     if (last && last->always_footer)
       fvtell_player(NORMAL_WFT(0, p), "%s\n", last->footer);
     last = help_file;
    }
   }
   
   ++count;
  }

  if (last && !last->always_footer)
    fvtell_player(NORMAL_WFT(0, p), "%s\n", last->footer);
  pager(p, PAGER_DEFAULT);
  assert(!help_shown_by);
  help_primary_name = NULL;
  help_search_name = NULL;
 }
 else
   fvtell_player(NORMAL_T(p), 
                 " Help on the subject -- ^S^B%s^s -- is not "
                 "available.\n", str);
}

static void user_su_showhelp(player *p, const char *str)
{
 player *p2 = NULL;
 int count = 0; 
 parameter_holder params;
 
 get_parameter_init(&params);
 
 assert(!help_primary_name);
 assert(!help_search_name);
 assert(!help_shown_by);

 if (!get_parameter_parse(&params, &str, 1) || !*str)
   TELL_FORMAT(p, "<player> <help-file>");

 CHECK_DUTY(p);
 
 if (!(p2 = player_find_on(p, GET_PARAMETER_STR(&params, 1),
                           PLAYER_FIND_SC_SU_ALL & ~PLAYER_FIND_SELF)))
   return;
 
 while (count < HELP_ARRAY_SZ)
 {  
  help_node *help_file = NULL;
  help_base *current = (help_array + count);
  
  if ((*current->test_can_see)(p->saved) &&
      (help_file = help_find(str, current->start, TRUE)) &&
      help_file->allow_find)
  {
   if (!strcasecmp(str, help_file->search_name))
   {
    help_search_name = help_file->search_name;
    break; /* this one matches completely */
   }
   
   if (!help_search_name)
     help_search_name = help_file->search_name;
  }
  
  ++count;
 }
 
 if (help_search_name)
 {
  int done = FALSE;
  help_node *last = NULL;
  
  help_shown_by = p;

  fvtell_player(NORMAL_T(p2), " ^B%s suggests you read this...^N\n", 
                p->saved->name);
  
  count = 0;
  while (count < HELP_ARRAY_SZ)
  {
   help_node *help_file = NULL;
   help_base *current = (help_array + count);
   
   if ((*current->test_can_see)(p->saved) &&
       (help_file = help_find(help_search_name, current->start, FALSE)))
   {
    help_primary_name = help_file->primary_name;

    if (!done || help_file->always_header)
      fvtell_player(NORMAL_WFT(0, p2), "%s", help_file->header);
    done = TRUE;
    
    fvtell_player(NORMAL_WFT(0, p2), "%s", help_file->body);

    if (!last || help_file->override_last)
    {
     if (last && last->always_footer)
       fvtell_player(NORMAL_WFT(0, p2), "%s", last->footer);
     last = help_file;
    }
   }
   
   ++count;
  }

  channels_wall("staff", 3, p2, " -=> %s shows 'help %s' to %s.",
                p->saved->name, help_primary_name, p2->saved->name);

  if (last && !last->always_footer)
    fvtell_player(NORMAL_WFT(0, p2), "%s", last->footer);
  pager(p2, PAGER_DEFAULT);

  help_primary_name = NULL;
  help_search_name = NULL;
  help_shown_by = NULL;
 }
 else
   fvtell_player(NORMAL_T(p), " Help on the subject -- ^S^B%s^s -- is not "
                 "available.\n", str);
}

static void user_help_list_topics(player *p, const char *str)
{
 int help_priv;
 int count = 0;
 help_node *current;
 char buffer[1024];
 
 if (!*str)
   TELL_FORMAT(p, "<number>");
 
 help_priv = atoi(str);

 if ((help_priv >= HELP_ARRAY_SZ) || (help_priv < 0))
 {
  fvtell_player(NORMAL_T(p), " Number not valid try another between, "
                "-- ^S^B%d^s -- and -- ^S^B%d^s --.\n", 0, HELP_ARRAY_SZ - 1);
  return;
 }

 current = (help_array + help_priv)->start;
 
 sprintf(buffer, "Help for %s", (help_array + help_priv)->msg_help.file_name);
 ptell_mid(NORMAL_T(p), buffer, FALSE);
 
 while (current)
 {
  fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(count, "%s\n"),
                current->search_name);
  
  current = current->next;
  ++count;
 }
 
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 pager(p, PAGER_DEFAULT);
}

static void user_help_list_keyword(player *p, const char *str)
{
 int count = 0;
 int matches = 0;
 
 while (count < HELP_ARRAY_SZ)
 {
  help_base *base = (help_array + count);
  
  if ((*base->test_can_see)(p->saved))
  {
   help_node *node = base->start;

   while (node)
   {
    const char *keys = node->keywords;
    parameter_holder real_params;
    parameter_holder *params = &real_params;

    if (!keys)
    {
     node = node->next;
     continue;
    }
    
    get_parameter_init(params);

    get_parameter_parse(params, &keys, GET_PARAMETER_NUMBER_MAX);

    while (params->last_param)
    {
     if (!beg_strcasecmp(str, GET_PARAMETER_STR(params, 1)))
     {
      if (!matches++)
      {
       char buffer[sizeof("keyword: %.*s ...") + 10];
       int tmp = strnlen(str, 11);

       if (tmp == 11)
         sprintf(buffer, "keyword: %.*s ...", 10, str);
       else
         sprintf(buffer, "keyword: %s", str);

       ptell_mid(NORMAL_FT(RAW_OUTPUT, p), buffer, FALSE);
      }

      /* FIXME: better UI */
      fvtell_player(NORMAL_T(p), "%s: %s\n",
                    node->search_name, node->primary_name);
     }

     get_parameter_shift(params, 1);
    }
    
    node = node->next;
   }
  }

  ++count;
 }

 if (!matches)
 {
  fvtell_player(SYSTEM_T(p), "%s", " The string -- ^S^B");
  fvtell_player(SYSTEM_FT(RAW_OUTPUT, p), "%s", str);
  fvtell_player(SYSTEM_T(p), "%s", "^s -- doesn't match any keywords "
                "for the help files.\n");
 }
 else
   fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

void cmds_init_help(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("help", user_help, CONST_CHARS, INFORMATION);
 CMDS_XTRA_SECTION(SYSTEM);
 CMDS_XTRA_SUB(DRAUGHTS, DRAUGHTS); CMDS_XTRA_SUB(EDITOR, EDITOR);
 CMDS_XTRA_SUB(ROOM, ROOM); CMDS_XTRA_SUB(CHECK, CHECK);
 CMDS_XTRA_SUB(NEWS, NEWS); CMDS_XTRA_SUB(NEWSGROUP, NEWSGROUP);
 CMDS_XTRA_SUB(MAIL, MAIL);
 CMDS_XTRA_MISC(RESTRICTED);
 CMDS_ADD("man", user_help, CONST_CHARS, INFORMATION);
 CMDS_XTRA_SECTION(SYSTEM);
 CMDS_XTRA_SUB(DRAUGHTS, DRAUGHTS); CMDS_XTRA_SUB(EDITOR, EDITOR);
 CMDS_XTRA_SUB(ROOM, ROOM); CMDS_XTRA_SUB(CHECK, CHECK);
 CMDS_XTRA_SUB(NEWS, NEWS); CMDS_XTRA_SUB(NEWSGROUP, NEWSGROUP);
 CMDS_XTRA_SUB(MAIL, MAIL);
 CMDS_XTRA_MISC(RESTRICTED);
 CMDS_ADD("showhelp", user_su_showhelp, CONST_CHARS, SU);
 CMDS_PRIV(pretend_su);
 CMDS_ADD("help_topics", user_help_list_topics, CONST_CHARS, INFORMATION);
 CMDS_PRIV(spod);
 CMDS_ADD("help_keyword", user_help_list_keyword, CONST_CHARS, INFORMATION);
}
