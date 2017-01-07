#define NEWS_C
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


static news_group *start_id = NULL;
static news_group *start_name = NULL;
static int number_newsgroups = 0;
static int unique_newsgroup_id = 1;




static news_article *news_find_article(news_group *group, int article)
{
 news_article *scan = NULL;

 if (!group)
   return (NULL);
 
 scan = group->start;

 while (--article && scan)
   scan = scan->next;

 return (scan);
}

static news_article *news_user_find_article(player *p,
                                            news_group *group, int article)
{
 news_article *scan = news_find_article(group, article);

 if (!scan)
   fvtell_player(SYSTEM_T(p), " No news article -- ^S^B%d^s --, in "
                 "the group -- ^S^B%s^s --.\n", article, group->name);

 return (scan);
}

static void news_remove_article(news_group *group, news_article *art)
{
 if (art->prev)
   art->prev->next = art->next;
 else
   group->start = art->next;

 if (art->next)
   art->next->prev = art->prev;

 --group->articles;


 FREE(art->body);
 XFREE(art, NEWS_ARTICLE);
}

static news_article *news_new_article(news_group *group, time_t timestamp)
{
 if (!group->max_articles || (group->articles < group->max_articles))
 {
  news_article *art = XMALLOC(sizeof(news_article), NEWS_ARTICLE);
  news_article *scan = group->start;
  
  if (!art)
    return (NULL);
  
  assert(MALLOC_VALID(group, sizeof(news_group), NEWS_GROUP));
  
  art->body = NULL;
  art->read_count = 0;
  art->anonymous = 0;
  art->name[0] = 0;
  art->subject[0] = 0;
  art->timestamp = timestamp;
  
  ++group->articles;
  
  if (!scan)
  {
   art->prev = NULL;
   art->next = NULL;
   
   group->start = art;
   return (art);
  }
  
  while (scan->next && (difftime(art->timestamp, scan->timestamp) > 0))
    scan = scan->next;
  
  if (difftime(art->timestamp, scan->timestamp) > 0)
  {
   art->prev = scan;
   
   if ((art->next = scan->next))
     art->next->prev = art;
   
   scan->next = art;
  }
  else
  {
   art->next = scan;
   
   if ((art->prev = scan->prev))
     art->prev->next = art;
   else
     group->start = art;
   
   scan->prev = art;
  }
  
  return (art);
 }
 else
 {
  news_article *scan = group->start;
  
  assert(scan);
  while (scan && scan->next)
    scan = scan->next;
  
  news_remove_article(group, scan);
  return (news_new_article(group, timestamp));
 }
}

static news_group *news_find_group_name(const char *group_name)
{
 news_group *scan = start_name;
 int saved_cmp = 0;

 while (scan && ((saved_cmp = strcasecmp(group_name, scan->name)) > 0))
   scan = scan->next_name;
   
 if (scan && !saved_cmp)
   return (scan);
 else
   return (NULL);
}

static news_group *news_user_find_group_name(player *p, const char *group_name,
                                             int verbose)
{
 news_group *scan = start_name;
 int saved_cmp = 0;
 
 while (scan && (saved_cmp = strcasecmp(group_name, scan->name)) > 0) 
   scan = scan->next_name;

 if (scan && !saved_cmp &&
     (scan->can_read || scan->can_post || PRIV_STAFF(p->saved)))
   return (scan);

 if (verbose)
   fvtell_player(NORMAL_T(p),
                 " The group -- ^S^B%s^s -- does not exist.\n", group_name);
 
 return (NULL);
}

static news_group *news_find_group_id(int newsgroup)
{
 news_group *scan = start_id;

 while (scan && (newsgroup > scan->id))
   scan = scan->next_id;

 if (scan && (newsgroup == scan->id))
   return (scan);
 else
   return (NULL);
}

static news_group *news_add_group(const char *name, int id)
{
 news_group *group = XMALLOC(sizeof(news_group), NEWS_GROUP);
 
 if (!group)
   return (NULL);

 assert(!news_find_group_name(name));
 assert(!news_find_group_id(id));
 
 if (start_name && start_id)
 {
  news_group *scan = start_id;

  while (scan->next_id && (id > scan->id))
    scan = scan->next_id;

  if (id > scan->id)
  {
   group->prev_id = scan;
   
   if ((group->next_id = scan->next_id))
     group->next_id->prev_id = group;
   
   scan->next_id = group;
  }
  else
  {
   group->next_id = scan;

   if ((group->prev_id = scan->prev_id))
     group->prev_id->next_id = group;
   else
     start_id = group;

   scan->prev_id = group;
  }

  scan = start_name; /* do it again but for names this time... */
  while (scan->next_name && (strcasecmp(name, scan->name) > 0))
    scan = scan->next_name;

  if (strcasecmp(name, scan->name) > 0)
  {
   group->prev_name = scan;
   
   if ((group->next_name = scan->next_name))
     group->next_name->prev_name = group;
   
   scan->next_name = group;
  }
  else
  {
   group->next_name = scan;

   if ((group->prev_name = scan->prev_name))
     group->prev_name->next_name = group;
   else
     start_name = group;

   scan->prev_name = group;
  }
 }
 else
 {
  assert(!start_name && !start_id);
  
  start_name = start_id = group;
  group->next_id = NULL;
  group->prev_id = NULL;
  group->next_name = NULL;
  group->prev_name = NULL;
 }

 COPY_STR(group->name, name, NEWSGROUP_NAME_SIZE);
 group->description[0] = 0;
 group->c_timestamp = now;
 group->m_timestamp = now;
 group->expire_after = NEWS_TIMEOUT;
 group->id = id;
 group->start = NULL;
 group->articles = 0;
 group->max_articles = 100;
  /* don't let anyone see it untill an admin changes the privs on it */
 group->can_post = FALSE;
 group->can_read = FALSE;
 ++number_newsgroups;
 
 return (group);
}

static void news_remove_group(news_group *group)
{
 while (group->start)
   news_remove_article(group, group->start);
 
 if (group->prev_id)
   group->prev_id->next_id = group->next_id;
 else
   start_id = group->next_id;

 if (group->next_id)
   group->next_id->prev_id = group->prev_id;

 if (group->prev_name)
   group->prev_name->next_name = group->next_name;
 else
   start_name = group->next_name;

 if (group->next_name)
   group->next_name->prev_name = group->prev_name;

 --number_newsgroups;
 
 XFREE(group, NEWS_GROUP);
}

/* so we can tell people what group they are subscribed to as default */
const char *newsgroup_find_group_name(int newsgroup)
{
 news_group *group = news_find_group_id(newsgroup);

 if (group)
   return (group->name);

 return (NULL);
}

static void news_expire_group(news_group *group)
{
 news_article *art = group->start;

 if (!group->expire_after)
   return;

 while (art && (difftime(now, art->timestamp) > group->expire_after))
 {
  news_article *art_next = art->next;
  
  news_remove_article(group, art);
  
  art = art_next;
 }
}

static void news_save_group(news_group *group)
{
 file_io fs;
 char index_path[256 + sizeof("files/newsgroups/%s.tmp")];
 int length = 0;

 news_expire_group(group);
 
 if (configure.talker_read_only)
   return;

 sprintf(index_path, "files/newsgroups/%.*s.tmp%n", 256, group->name, &length);

 if (file_write_open(index_path, NEWS_GROUP_FILE_VERSION, &fs))
 {
  int count = 0;
  news_article *art = group->start; 
  
  file_section_beg("header", &fs);
  file_put_int("articles", group->articles, &fs);
  file_put_int("expire_after", group->expire_after, &fs);
  file_put_time_t("m_timestamp", group->m_timestamp, &fs);
  file_put_int("max_articles", group->max_articles, &fs);
  file_section_end("header", &fs);
  
  file_section_beg("news", &fs);
  
  while (art && (count < group->articles))
  {
   char buffer[60];
  
   sprintf(buffer, "%04d", ++count);
   
   file_section_beg(buffer, &fs);

   file_put_bitflag("anonymous", art->anonymous , &fs);
   file_put_string("body", art->body, art->body_size, &fs);
   file_put_string("name", art->name, 0, &fs);
   file_put_int("read_count", art->read_count, &fs);
   file_put_string("subject", art->subject, 0, &fs);
   file_put_time_t("timestamp", art->timestamp, &fs);
   
   file_section_end(buffer, &fs);

   art = art->next;
  }
  
  file_section_end("news", &fs);
  
  if (file_write_close(&fs))
  {
   char index_path_orig[sizeof(index_path)];
   
   memcpy(index_path_orig, index_path, length);
   index_path_orig[length - CONST_STRLEN(".tmp")] = 0;
   
   rename(index_path, index_path_orig);
  }
 }
}

static void news_read_group(news_group *group)
{
 char index_path[256 + sizeof("files/newsgroups/%.*s")];
 file_io fs;
 
 sprintf(index_path, "files/newsgroups/%.*s", 256, group->name);
 
 if (file_read_open(index_path, &fs))
 {
  int count = 0;
  int group_articles = 0;
  
  file_section_beg("header", &fs);
  group_articles = file_get_int("articles", &fs);
  group->expire_after = file_get_int("expire_after", &fs);
  group->m_timestamp = file_get_time_t("m_timestamp", &fs);
  group->max_articles = file_get_int("max_articles", &fs);
  if (FILE_IO_CREATED(&fs))
    group->max_articles = 100;
  file_section_end("header", &fs);
  
  file_section_beg("news", &fs);
  
  while (count < group_articles)
  {
   char buffer[60];
   news_article tmp_art;
   news_article *art = NULL; 
  
   sprintf(buffer, "%04d", ++count);
   
   file_section_beg(buffer, &fs);

   tmp_art.anonymous = file_get_bitflag("anonymous", &fs);

   if (!FILE_IO_CREATED(&fs))
   {
    if (!(tmp_art.body = file_get_malloc("body", &tmp_art.body_size, &fs)))
      SHUTDOWN_MEM_ERR();
    
    file_get_string("name", tmp_art.name, PLAYER_S_NAME_SZ, &fs);
    tmp_art.read_count = file_get_int("read_count", &fs);
    file_get_string("subject", tmp_art.subject, NEWS_SUBJECT_SIZE, &fs);
    tmp_art.timestamp = file_get_time_t("timestamp", &fs);

    if ((art = news_new_article(group, tmp_art.timestamp)))
    {
     art->anonymous = tmp_art.anonymous;
     art->body = tmp_art.body;
     art->body_size = tmp_art.body_size;
     art->read_count = tmp_art.read_count;
     qstrcpy(art->name, tmp_art.name);
     qstrcpy(art->subject, tmp_art.subject);
    }
    else
      LOG_MEM_ERR();
   }
   
   file_section_end(buffer, &fs);
  }
  assert((count == group->articles) && (count == group_articles));
  
  file_section_end("news", &fs);
  
  file_read_close(&fs);
 }
}

static void news_save_master_index(void)
{
 file_io fs;

 if (configure.talker_read_only)
   return;

 if (file_write_open("files/newsgroups/.index.tmp",
                     NEWS_INDEX_FILE_VERSION, &fs))
 {
  int count = 0;
  news_group *scan = start_name;
  
  file_section_beg("header", &fs);
  file_put_int("number", number_newsgroups, &fs);
  file_put_int("unique_id", unique_newsgroup_id, &fs);
  file_section_end("header", &fs);
  
  file_section_beg("newsgroup", &fs);
  while (scan && (count < number_newsgroups))
  {
   char buffer[60];

   sprintf(buffer, "%04d", ++count);
   
   file_section_beg(buffer, &fs);

   file_put_bitflag("can_post", scan->can_post, &fs);
   file_put_bitflag("can_read", scan->can_read, &fs);
   file_put_string("description", scan->description, 0, &fs);
   file_put_int("id", scan->id, &fs);
   file_put_string("name", scan->name, 0, &fs);
   file_put_time_t("timestamp", scan->c_timestamp, &fs);
   
   file_section_end(buffer, &fs);
   
   scan = scan->next_name;
  }
  assert((count == number_newsgroups) && !scan);
  
  file_section_end("newsgroup", &fs);
  
  if (file_write_close(&fs))
    rename("files/newsgroups/.index.tmp", "files/newsgroups/.index");
 }
}

void init_news(void)
{
 file_io fs;
 
 if (file_read_open("files/newsgroups/.index", &fs))
 {
  int count = 0;
  int tmp_number_newsgroups = 0;
  
  file_section_beg("header", &fs);
  tmp_number_newsgroups = file_get_int("number", &fs);
  unique_newsgroup_id = file_get_int("unique_id", &fs);
  file_section_end("header", &fs);  
  
  file_section_beg("newsgroup", &fs);
  while (count < tmp_number_newsgroups)
  {
   news_group group;
   char buffer[60];
   
   ++count;
   sprintf(buffer, "%04d", count);
   
   file_section_beg(buffer, &fs);

   group.can_post = file_get_bitflag("can_post", &fs);
   if (!FILE_IO_CREATED(&fs))
   {
    news_group *added_group = NULL;    

    group.can_read = file_get_bitflag("can_read", &fs);
    assert(!FILE_IO_CREATED(&fs));
    file_get_string("description",
                    group.description, NEWSGROUP_DESCRIPTION_SIZE, &fs);
    assert(!FILE_IO_CREATED(&fs));
    group.id = file_get_int("id", &fs);
    assert(!FILE_IO_CREATED(&fs));
    file_get_string("name", group.name, NEWSGROUP_NAME_SIZE, &fs);
    assert(!FILE_IO_CREATED(&fs));
    group.c_timestamp = file_get_time_t("timestamp", &fs);
    assert(!FILE_IO_CREATED(&fs));

    added_group = news_add_group(group.name, group.id);
    qstrcpy(added_group->description, group.description);
    added_group->can_post = group.can_post;
    added_group->can_read = group.can_read;
    added_group->c_timestamp = group.c_timestamp;
    
    news_read_group(added_group);
   }
   
   file_section_end(buffer, &fs);
  }
  assert((count == tmp_number_newsgroups) && (count == number_newsgroups));
  
  file_section_end("newsgroup", &fs);
  
  file_read_close(&fs);
 }
 else
 {
  struct stat buf;
  news_group *group = news_add_group("Misc", unique_newsgroup_id++);
  
  CONST_COPY_STR_LEN(group->description,
                     "The default newsgroup for all miscellaneous "
                     "articles.");
  group->can_post = TRUE;
  group->can_read = TRUE;
  
  if (stat("files/newsgroups", &buf) == -1)
    if (mkdir("files/newsgroups", 0700) == -1)
      vwlog("error", "mkdir: %d %s\n", errno, strerror(errno));

  news_save_master_index();
  news_save_group(group);
 }
}

static news_group *news_find_first_group(player *p)
{
 news_group *scan = start_name;

 while (scan && (!scan->can_read && !PRIV_STAFF(p->saved)))
   scan = scan->next_name;
 
 return (scan);
}

static void news_rejoin_func(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", 
               " Re-Entering news mode. Use ^Bhelp news^N for help\n"
               " To run ^Bnormal commands^N type /<command>\n"
               " Use '^Bend^N' to ^Bleave^N.\n");
}

static int news_command(player *p, const char *str, size_t length)
{
 ICTRACE("news_command");

 if (MODE_IN_MODE(p, NEWS))
   MODE_HELPER_COMMAND();
 else
   if (!*str)
   {
    cmds_function tmp_cmd;
    cmds_function tmp_rejoin;
    
    CMDS_FUNC_TYPE_RET_CHARS_SIZE_T(&tmp_cmd, news_command);
    CMDS_FUNC_TYPE_NO_CHARS(&tmp_rejoin, news_rejoin_func);

    if (mode_add(p, "News Mode-> ", MODE_ID_NEWS, 0,
                 &tmp_cmd, &tmp_rejoin, NULL))
      fvtell_player(NORMAL_T(p), "%s", 
                    " Entering news mode. Use ^Bhelp news^N for help\n"
                    " To run ^Bnormal commands^N type /<command>\n"
                    " Use '^Bend^N' to ^Bleave^N.\n");
    else
      fvtell_player(SYSTEM_T(p), 
                    " You cannot enter news mode as you are in too many "
                    "other modes.\n");
    return (TRUE);
   }
 
 return (cmds_sub_match(p, str, length,
                        &cmds_sub[CMDS_SUB_OFFSET(CMDS_SECTION_NEWS)]));
}

static void newsgroup_rejoin_func(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", 
               " Re-Entering newsgroup mode. Use ^Bhelp newsgroup^N for help\n"
               " To run ^Bnormal commands^N type /<command>\n"
               " Use '^Bend^N' to ^Bleave^N.\n");
}

static int newsgroup_command(player *p, const char *str, size_t length)
{
 ICTRACE("newsgroup_command");
 
 if (MODE_IN_MODE(p, NEWSGROUP))
   MODE_HELPER_COMMAND();
 else
   if (!*str)
   {
    cmds_function tmp_cmd;
    cmds_function tmp_rejoin;
    
    CMDS_FUNC_TYPE_RET_CHARS_SIZE_T(&tmp_cmd, newsgroup_command);
    CMDS_FUNC_TYPE_NO_CHARS(&tmp_rejoin, newsgroup_rejoin_func);
    
    if (mode_add(p, "NewsGroup Mode-> ", MODE_ID_NEWSGROUP, 0,
                 &tmp_cmd, &tmp_rejoin, NULL))
      fvtell_player(NORMAL_T(p), "%s", 
                    " Entering newsgroup mode. Use ^Bhelp newsgroup^N "
                    "for help\n"
                    " To run ^Bnormal commands^N type /<command>\n"
                    " Use '^Bend^N' to ^Bleave^N.\n");
    else
      fvtell_player(SYSTEM_T(p), 
                    " You cannot enter newsgroup mode as you are in too many "
                    "other modes.\n");
    return (TRUE);
   }

 return (cmds_sub_match(p, str, length,
                        &cmds_sub[CMDS_SUB_OFFSET(CMDS_SECTION_NEWSGROUP)]));
}

static int newsgroup_command_wrapper(player *p, const char *str, size_t length)
{
 char current_command_buffer[(ALIAS_COMMAND_SZ * 2) + sizeof("%.*s %.*s")];

 sprintf(current_command_buffer, "%.*s %.*s",
         ALIAS_COMMAND_SZ, current_command,
         ALIAS_COMMAND_SZ, current_sub_command);
  
 current_command = current_command_buffer;
 current_sub_command = NULL;
 
 return (newsgroup_command(p, str, length));
}

static void news_view_commands(player *p)
{
 user_cmds_show_section(p, "news");
}

static void newsgroup_view_commands(player *p)
{
 user_cmds_show_section(p, "newsgroup");
}

static void news_exit_mode(player *p)
{
 assert(MODE_IN_MODE(p, NEWS));

 fvtell_player(NORMAL_T(p), "%s", " Leaving news mode.\n");

 mode_del(p);
}

static void newsgroup_exit_mode(player *p)
{
 assert(MODE_IN_MODE(p, NEWSGROUP));
 
 fvtell_player(NORMAL_T(p), "%s", " Leaving newsgroup mode.\n");

 mode_del(p);
}

/* str isn't used so we can have this as the group one as well */
static void internal_news_list(player *p, const char *str, news_group *group,
                               int number_to_show)
{
 news_article *scan = NULL;
 int count = 0;
 int string_length = 32; /* guess and hope... */

 IGNORE_PARAMETER(str);

 if (!group->articles)
 {
  assert(!group->start);
  
  fvtell_player(NORMAL_T(p), " No news articles, in the "
                "group ^S^B%s^s, to view.\n", group->name);
  return;
 }
 
 assert(MALLOC_VALID(group->start, sizeof(news_article), NEWS_ARTICLE));
 
 scan = group->start;

 if (number_to_show && (number_to_show < group->articles))
 {
  while (scan && (count < (group->articles - number_to_show)))
  {
   ++count;

   scan = scan->next;
  }
  assert(scan && (count == (group->articles - number_to_show)));
 }

 if (group->articles == 1)
 {
  char buffer[sizeof("%s: One Article") + NEWSGROUP_NAME_SIZE];

  sprintf(buffer, "%s: One Article", group->name);
          
  ptell_mid(NORMAL_T(p), buffer, TRUE);
 }
 else
 {
  char buffer[sizeof("%s: $Number(%d)-Tostr Articles%s%s%s") +
             NEWSGROUP_NAME_SIZE + (2 * BUF_NUM_TYPE_SZ(int)) +
             sizeof(" (showing ") + 1];
  char ascii_num[BUF_NUM_TYPE_SZ(int)];

  if (number_to_show && (number_to_show < group->articles))
    sprintf(ascii_num, "%d", number_to_show);
  else
    ascii_num[0] = 0;
  
  sprintf(buffer, "%s: $Number(%d)-Tostr Articles%s%s%s", group->name,
          group->articles,
          USE_STRING(ascii_num[0], " (showing "), ascii_num,
          USE_STRING(ascii_num[0], ")"));
  
  ptell_mid(NORMAL_T(p), buffer, FALSE);
 }
 
 while (scan)
 {
  char buffer[BUF_NUM_TYPE_SZ(int) + 2];
  
  sprintf(buffer, "[%d]", ++count);

  /* tripple space so it looks like mail "UA [x]" */
  fvtell_player(NORMAL_T(p), "   %-5s %5lu ", buffer,
                (unsigned long)scan->body_size);

  if (scan->anonymous && p->saved->priv_admin)
    fvtell_player(NORMAL_T(p), "<%s>", scan->name);
  else
    fvtell_player(NORMAL_T(p), "(%s)", scan->anonymous ? "anonymous" :
                  scan->name);
    
  fvtell_player(NORMAL_T(p), ": %.*s\n", string_length, scan->subject);

  scan = scan->next;
 }

 if (number_to_show && (number_to_show < group->articles))
   ptell_mid(NORMAL_T(p), "Use: check_all ...to see all articles", TRUE);
 else
   fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

void user_news_list_articles(player *p, const char *str)
{
 news_group *group = NULL;
 int number_to_show = 15;
 parameter_holder params;

 get_parameter_init(&params);

 if (get_parameter_parse(&params, &str, 3))
   TELL_FORMAT(p, "[group] [number-to-show]");
 
 switch (params.last_param)
 {
  default:
    assert(FALSE);
    
  case 0:
    GET_DEFAULT_GROUP(p, group);
    break;

  case 1:
    if (*(GET_PARAMETER_STR(&params, 1) +
          strspn(GET_PARAMETER_STR(&params, 1), "0123456789")))
    {
     if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR(&params, 1),
                                             TRUE)))
       return;
    }
    else
    {
     GET_DEFAULT_GROUP(p, group);
     number_to_show = atoi(GET_PARAMETER_STR(&params, 1));
    }
    break;
    
  case 2:
    if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR(&params, 1),
                                            TRUE)))
      return;

    if (*(GET_PARAMETER_STR(&params, 2) +
          strspn(GET_PARAMETER_STR(&params, 2), "0123456789")))
      TELL_FORMAT(p, "[group] [number-to-show]");
    else
      number_to_show = atoi(GET_PARAMETER_STR(&params, 2));
 }
 
 internal_news_list(p, str, group, number_to_show);
 pager(p, PAGER_DEFAULT);
}

void user_news_list_articles_all(player *p, const char *str)
{
 news_group *group = NULL; 

 if (!*str)
   GET_DEFAULT_GROUP(p, group);
 else if (!(group = news_user_find_group_name(p, str, TRUE)))
   return;
 
 internal_news_list(p, str, group, 0);
 pager(p, PAGER_DEFAULT);
}

void user_news_list_newsgroups(player *p, const char *str)
{
 news_group *tmp = start_name;
 int longest_group_name = 0;
 int tmp_len = 0;
 unsigned int group_desc_len = 0;
 unsigned int space_taken = 0;
 int use_desc = TRUE;

 IGNORE_PARAMETER(str);
 
 ptell_mid(NORMAL_T(p), "Newsgroups", TRUE);

 while (tmp)
 {
  if ((tmp_len = strlen(tmp->name)) > longest_group_name)
    longest_group_name = tmp_len;
  tmp = tmp->next_name;
 }

 tmp = start_name;
     
 while (tmp)
 {
  char buffer[20];
  
  sprintf(buffer, "[%d]", tmp->articles);

  group_desc_len = strlen(tmp->description);
  space_taken = longest_group_name + 9;
  /* 10 is an arbitrary amount here */
  if (p->term_width < (space_taken + 10))
    use_desc = FALSE;

  if (use_desc)
  {
   if (tmp->can_read || PRIV_STAFF(p->saved))
     fvtell_player(NORMAL_T(p), "%5s %-*s - %.*s%s\n",
                   buffer, longest_group_name, tmp->name,
                   (int)((p->term_width - space_taken) >= group_desc_len ?
                    (p->term_width - space_taken) :
                    (p->term_width - (space_taken + 4))) , tmp->description,
                   ((p->term_width - space_taken) >= group_desc_len ?
                    "" : " ..."));
  }
  else
    if (tmp->can_read || PRIV_STAFF(p->saved))
      fvtell_player(NORMAL_T(p), "%5s %-*s\n",
                    buffer, longest_group_name, tmp->name);
  
  /* for consistancy you want to put expire_news_group(group); here
   * but it would kill you with lots of groups with lots of articles */
  /* FIMXE: have a last expired time_t and do an expire once every day */
  tmp = tmp->next_name;
 }

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

int news_check_new_arrived(player_tree_node *saved, player *p,
                           twinkle_info *twink, int flags, time_t timestamp)
{ /* TODO: show your defualt group first then all the other groups in
   * alpha order ? ... with how many new/total postings ? */
 news_group *tmp = start_name;
 output_node *tmp_news = NULL;
 tmp_output_list_storage tmp_save;
 int count = 0;
 int ret_value = FALSE;
 
 save_tmp_output_list(p, &tmp_save);
 
 while (tmp)
 {
  if ((tmp->can_read || PRIV_STAFF(p->saved)) &&
      (difftime(tmp->c_timestamp, p->saved->logoff_timestamp) > 0))
  {
   fvtell_player(ALL_T(saved, p, twink, flags | OUTPUT_BUFFER_TMP, timestamp),
                 ADD_COMMA_FRONT(count, "%s"), tmp->name);
   ++count;
  }
  
  tmp = tmp->next_name;
 }

 tmp_news = output_list_grab(p);
 load_tmp_output_list(p, &tmp_save);
 
 if (count)
 {
  if (count == 1)
    fvtell_player(ALL_T(saved, p, twink, flags, timestamp), "%s",
                  " You have new news in the group ");  
  else
    fvtell_player(ALL_T(saved, p, twink, flags, timestamp), "%s",
                  " You have new news in the groups ");

  output_list_linkin(p, flags, &tmp_news, INT_MAX);
  fvtell_player(ALL_T(saved, p, twink, flags, timestamp), "%s", ".\n");
  ret_value = TRUE;
 }
 else
   output_list_cleanup(&tmp_news);

 assert(!tmp_news);
 
 save_tmp_output_list(p, &tmp_save);
 count = 0;
 tmp = start_name;
 while (tmp)
 {
  if ((tmp->can_read || PRIV_STAFF(p->saved)) &&
      (difftime(tmp->c_timestamp, p->saved->logoff_timestamp) > 0))
  {
   fvtell_player(ALL_T(saved, p, twink, flags | OUTPUT_BUFFER_TMP, timestamp),
                 ADD_COMMA_FRONT(count, "%s"), tmp->name);
   ++count;
  }
  
  tmp = tmp->next_name;
 }

 tmp_news = output_list_grab(p);
 load_tmp_output_list(p, &tmp_save);
 
 if (count)
 {
  if (count == 1)
    fvtell_player(ALL_T(saved, p, twink, flags, timestamp), "%s",
                  " There is a new news group called ");  
  else
    fvtell_player(ALL_T(saved, p, twink, flags, timestamp), "%s",
                  " There are new news groups called ");

  output_list_linkin(p, flags, &tmp_news, INT_MAX);
  fvtell_player(ALL_T(saved, p, twink, flags, timestamp), "%s", ".\n"); 
  ret_value = TRUE;
 }
 else
   output_list_cleanup(&tmp_news);

 return (ret_value);
}

static int internal_news_inform(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 news_group *group = va_arg(ap, news_group *);
 int do_inform = FALSE;

 if ((scan == p) || !(group->can_read || PRIV_STAFF(scan->saved)))
   return (TRUE);
 
 LIST_SELF_CHECK_FLAG_START(scan, p->saved);
 if (LIST_SELF_CHECK_FLAG_DO(article_inform))
   do_inform = TRUE;
 LIST_SELF_CHECK_FLAG_END();

 if (!do_inform)
   return (TRUE);
 
 fvtell_player(NORMAL_FT(HILIGHT, scan),
               " -=> There is new news, in group %s.\n", group->name);
 
 return (TRUE);
}

static void news_edit_cleanup(player *p)
{
 buffer_news_destroy(p);
}

static void news_edit_quit(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", " Article NOT posted.\n");

 news_edit_cleanup(p);
}

static void news_edit_end(player *p)
{
 news_group *group = news_find_group_id(p->buffers->news_buff->post_group);
 news_article *new_art = NULL;

 assert(MODE_IN_MODE(p, EDIT));
 
 if (!(group = news_find_group_id(p->buffers->news_buff->post_group)))
   fvtell_player(NORMAL_T(p), "%s", " Sorry the newsgroup that you wanted to "
                 "post to has been removed.\n");
 else
 {
  if ((new_art = news_new_article(group, now)))
    new_art->body = edit_malloc_dump(p, &new_art->body_size);
  
  if (new_art && new_art->body)
  {
   qstrcpy(new_art->name, p->buffers->news_buff->name);
   qstrcpy(new_art->subject, p->buffers->news_buff->subject);
   new_art->read_count = 0;
   new_art->anonymous = p->buffers->news_buff->anonymous;
   
   fvtell_player(NORMAL_T(p), "%s", " Article posted....\n");
   
   do_inorder_logged_on(internal_news_inform, p, group);

   group->m_timestamp = now;
   news_save_group(group);
  }
  else
  {
   if (new_art)
     XFREE(new_art, NEWS_ARTICLE);
   
   fvtell_player(NORMAL_T(p), "%s",
                 " Article ^B_NOT_^b posted, due to site problems.\n");  
  }
 }

 news_edit_cleanup(p);
}

static void internal_news_post(player *p, const char *str,
                               news_group *group, int anon)
{
 if (group->can_post || PRIV_STAFF(p->saved))
 {
  int created = buffer_news_create(p);
  
  if (created == BUFFERS_RET_FAILED)
  {
   P_MEM_ERR(p);
   return;
  }
  else if (created == BUFFERS_RET_USED)
  {
   fvtell_player(NORMAL_T(p), "%s", 
                 " Cannot post news whilst using the current command, "
                 "sorry.\n");
   return;
  }
  assert(created == BUFFERS_RET_WORKED);

  assert(p->buffers && p->buffers->news_buff);
  
  p->buffers->news_buff->post_group = group->id;
  p->buffers->news_buff->anonymous = anon;
  
  COPY_STR(p->buffers->news_buff->subject, str, NEWS_SUBJECT_SIZE);
  
  qstrcpy(p->buffers->news_buff->name, p->saved->name);
  fvtell_player(NORMAL_T(p), "%s",
                " Now enter the main body text for the article.\n");
  
  if (edit_start(p, NULL))
  {
   assert(MODE_IN_MODE(p, EDIT));
   
   edit_limit_characters(p, NEWS_ARTICLE_CHARS_SZ);
   edit_limit_lines(p, NEWS_ARTICLE_LINES_SZ);
   
   CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_quit, news_edit_quit);
   CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_end, news_edit_end);
  }
  else
    buffer_news_destroy(p);
 }
 else /* shouldn't happen */
   fvtell_player(NORMAL_T(p), 
                 " Cannot post news to the group %s.\n", group->name);
}

static void user_news_post_article(player *p, const char *str)
{
 news_group *group = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<subject>");

 GET_DEFAULT_GROUP(p, group);
 
 internal_news_post(p, str, group, FALSE);
}

static void user_news_anon_post_article(player *p, const char *str)
{
 news_group *group = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<subject>");

 GET_DEFAULT_GROUP(p, group);
 
 internal_news_post(p, str, group, TRUE);
}

static void user_news_group_post_article(player *p, const char *str)
{
 news_group *group = NULL;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (!get_parameter_parse(&params, &str, 1) || !*str)
   TELL_FORMAT(p, "<group> <subject>");

 if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR((&params), 1),
                                         TRUE)))
   return;

 internal_news_post(p, str, group, FALSE);
}

static void user_news_group_anon_post_article(player *p, const char *str)
{
 news_group *group = NULL;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (!get_parameter_parse(&params, &str, 1) || !*str)
   TELL_FORMAT(p, "<group> <subject>");

 if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR((&params), 1),
                                         TRUE)))
   return;

 internal_news_post(p, str, group, TRUE);
}

static void internal_news_followup(player *p, const char *str,
                                   news_group *group, int anon)
{
 if (group->can_post || PRIV_STAFF(p->saved))
 {
  int created = 0;
  char buffer[1024];
  char *body = buffer;
  news_article *old = news_user_find_article(p, group, atoi(str));
  
  if (!old)
    return;

  if (old->anonymous)
  {
   fvtell_player(NORMAL_T(p), "%s", " You cannot reply to anonymous news.\n");
   return;
  }

  if ((created = buffer_news_create(p)) == BUFFERS_RET_FAILED)
  {
   P_MEM_ERR(p);
   return;
  }
  else if (created == BUFFERS_RET_USED)
  {
   fvtell_player(NORMAL_T(p), "%s", 
                 " Cannot reply to news whilst using the current command, "
                 "sorry.\n");
   return;
  }
  assert(created == BUFFERS_RET_WORKED);
  
  assert(p->buffers && p->buffers->news_buff);
  
  if (!BEG_CONST_STRCMP("Re: ", old->subject))
    sprintf(p->buffers->news_buff->subject, "%.*s",
            NEWS_SUBJECT_SIZE - 1, old->subject);
  else
    sprintf(p->buffers->news_buff->subject, "Re: %.*s",
            (int)(NEWS_SUBJECT_SIZE - 1 - CONST_STRLEN("Re: ")), old->subject);
  
  p->buffers->news_buff->post_group = group->id;
  p->buffers->news_buff->anonymous = anon;
  
  qstrcpy(p->buffers->news_buff->name, p->saved->name);
  
  sprintf(body, "On %s GMT, %s wrote:\n",
          disp_time_std(old->timestamp, 0, TRUE, TRUE), old->name);
  
  if (edit_indent_start(p, body, old->body))
  {
   assert(MODE_IN_MODE(p, EDIT));
   
   edit_limit_characters(p, NEWS_ARTICLE_CHARS_SZ);
   edit_limit_lines(p, NEWS_ARTICLE_LINES_SZ);
   
   CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_quit, news_edit_quit);
   CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_end, news_edit_end);
  }
  else
    buffer_news_destroy(p);
 }
 else /* shouldn't happen */
   fvtell_player(NORMAL_T(p), 
                 " Cannot post news to the group %s.\n", group->name);
}

static void user_news_followup_article(player *p, const char *str)
{
 news_group *group = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<number>");
 
 GET_DEFAULT_GROUP(p, group);
 
 internal_news_followup(p, str, group, FALSE);
}

static void user_news_anon_followup_article(player *p, const char *str)
{
 news_group *group = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<number>");
 
 GET_DEFAULT_GROUP(p, group);
 
 internal_news_followup(p, str, group, TRUE);
}

static void user_news_group_followup_article(player *p, const char *str)
{
 news_group *group = NULL;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (get_parameter_parse(&params, &str, 3) || (params.last_param != 2))
   TELL_FORMAT(p, "<group> <number>");

 if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR((&params), 1),
                                         TRUE)))
   return;

 internal_news_followup(p, GET_PARAMETER_STR((&params), 2), group, FALSE);
}

static void user_news_group_anon_followup_article(player *p, const char *str)
{
 news_group *group = NULL;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (get_parameter_parse(&params, &str, 3) || (params.last_param != 2))
   TELL_FORMAT(p, "<group> <number>");
 
 if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR((&params), 1),
                                         TRUE)))
   return;

 internal_news_followup(p, GET_PARAMETER_STR((&params), 2), group, TRUE);
}

static void internal_news_remove_article(player *p, const char *str,
                                         news_group *group)
{
 news_article *article = news_user_find_article(p, group, atoi(str));
 
 if (!article)
   return;

 if (!p->saved->priv_admin &&
     strcasecmp(article->name, p->saved->lower_name))
 {
  fvtell_player(NORMAL_T(p), "%s", 
                " You can't remove an article that isn't yours.\n");
  return;
 }

 news_remove_article(group, article);
 news_save_group(group);
 
 fvtell_player(NORMAL_T(p), "%s", " Article removed.\n");
}

static void user_news_remove_article(player *p, const char *str)
{
 news_group *group = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<number>");
 
 GET_DEFAULT_GROUP(p, group);

 internal_news_remove_article(p, str, group);
}

static void user_news_group_remove_article(player *p, const char *str)
{
 news_group *group = NULL;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (get_parameter_parse(&params, &str, 3) || (params.last_param != 2))
   TELL_FORMAT(p, "<group> <number>");

 if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR((&params), 1),
                                         TRUE)))
   return;

 internal_news_remove_article(p, GET_PARAMETER_STR((&params), 2), group);
}

static void internal_news_read_article(player *p, const char *str,
                                       news_group *group)
{
 if (group->can_read || PRIV_STAFF(p->saved))
 {
  char lowered_name[PLAYER_S_NAME_SZ];
  player_tree_node *saved = NULL;
  news_article *article = news_user_find_article(p, group, atoi(str));
  twinkle_info info;
  char buf[128];
  
  setup_twinkle_info(&info);
  
  info.returns_limit = UINT_MAX;
  info.allow_fills = TRUE;
  
  if (!article)
    return;

  COPY_STR(lowered_name, article->name, PLAYER_S_NAME_SZ);
  lower_case(lowered_name);

  if (!(saved = player_tree_find_exact(lowered_name)))
    saved = p->saved;
  
  ptell_mid(INFO_T(saved, p), "header", TRUE);
  ++article->read_count;
  fvtell_player(INFO_T(saved, p), "%s", " From:");
 if (!article->anonymous || !strcasecmp(p->saved->lower_name, article->name) ||
     p->saved->priv_admin)
   fvtell_player(INFO_T(saved, p), " %s", article->name);
  if (article->anonymous)
    fvtell_player(INFO_T(saved, p), "%s", " (anonymous)");
  fvtell_player(INFO_T(saved, p), "%s", "\n");


  fvtell_player(INFO_T(saved, p), "Subject: %.*s\n",
                OUT_LENGTH_NEWS_SUBJECT, article->subject);

  fvtell_player(INFO_T(saved, p), "Date: %s\n", 
                DISP_TIME_P_STD(article->timestamp, p));

  /* this isn't really usefull, but is in EW, *sigh*/
  fvtell_player(INFO_T(saved, p), "Article-read: %s\n",
                word_number_base(buf, 128, NULL, article->read_count,
                                 FALSE, word_number_times));
  
  ptell_mid(INFO_T(saved, p), "body", TRUE);

  fvtell_player(ALL_T(saved, p, &info, 0, now), "%.*s",
                OUT_LENGTH_NEWS_POST, article->body);

  fvtell_player(INFO_T(saved, p), "%s", DASH_LEN);

  pager(p, PAGER_DEFAULT);
 }
 else /* shouldn't happen */
   fvtell_player(NORMAL_T(p), 
                 " Cannot read news from the group -- ^S^B%s^s --.\n",
                 group->name);
}

static void user_news_read_article(player *p, const char *str)
{
 news_group *group = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<number>");
 
 GET_DEFAULT_GROUP(p, group);
 
 internal_news_read_article(p, str, group);
}

static void user_news_group_read_article(player *p, const char *str)
{
 news_group *group = NULL;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (get_parameter_parse(&params, &str, 3) || (params.last_param != 2))
   TELL_FORMAT(p, "<group> <number>");

 if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR((&params), 1),
                                         TRUE)))
   return;

 internal_news_read_article(p, GET_PARAMETER_STR((&params), 2), group);
}

static void internal_news_reply_article(player *p, const char *str,
                                        news_group *group, int anon)
{
 news_article *replying_to = NULL;
 player_tree_node *owner = NULL;

 assert(p->saved);
 if (mail_check_mailout_size(p->saved) >= (unsigned int)p->max_mails)
 {
  fvtell_player(NORMAL_T(p), "%s",
                " Sorry, you have reached your mail limit.\n");
  return;
 }
 
 if (!(replying_to = news_user_find_article(p, group, atoi(str))))
   return;

 if (replying_to->anonymous)
 {
  fvtell_player(NORMAL_T(p), "%s", " You cannot reply to anonymous news.\n");
  return;
 }

 if (!(owner = player_find_all(p, replying_to->name,
                               PLAYER_FIND_VERBOSE | PLAYER_FIND_SELF)))
   return;

 mail_reply_text(p,
                 owner, replying_to->timestamp,
                 replying_to->subject, replying_to->body, anon);
}

static void user_news_reply_article(player *p, const char *str)
{
 news_group *group = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<number>");

 GET_DEFAULT_GROUP(p, group);
 
 internal_news_reply_article(p, str, group, FALSE);
}

static void user_news_anon_reply_article(player *p, const char *str)
{
 news_group *group = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<number>");

 GET_DEFAULT_GROUP(p, group);

 internal_news_reply_article(p, str, group, TRUE);
}

static void user_news_group_reply_article(player *p, const char *str)
{
 news_group *group = NULL;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (get_parameter_parse(&params, &str, 3) || (params.last_param != 2))
   TELL_FORMAT(p, "<group> <number>");
 
 if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR((&params), 1),
                                         TRUE)))
   return;
 
 internal_news_reply_article(p, GET_PARAMETER_STR((&params), 2), group, FALSE);
}

static void user_news_group_anon_reply_article(player *p, const char *str)
{
 news_group *group = NULL;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (get_parameter_parse(&params, &str, 3) || (params.last_param != 2))
   TELL_FORMAT(p, "<group> <number>");
 
 if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR((&params), 1),
                                         TRUE)))
   return;
 
 internal_news_reply_article(p, GET_PARAMETER_STR((&params), 2), group, TRUE);
}

static void user_news_show_info(player *p)
{
 news_group *group = NULL;
 
 GET_DEFAULT_GROUP(p, group);

 fvtell_player(NORMAL_T(p),
               " Your default newsgroup is currently set to '^S^B%s^s'.\n",
               group->name);
}

static void user_news_set_default_group(player *p, parameter_holder *params)
{
 news_group *group = NULL;
 
 if (params->last_param != 1)
   TELL_FORMAT(p, "<newsgroup>");

 if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR(params, 1),
                                         TRUE)))
   return;

 fvtell_player(NORMAL_T(p), " Changed default group to %s.\n",
               group->name);
 p->saved_default_newsgroup = p->default_newsgroup = group->id;
}

static void user_news_set_tmp_group(player *p, parameter_holder *params)
{
 news_group *group = NULL;
 
 if (params->last_param != 1)
   TELL_FORMAT(p, "<newsgroup>");

 if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR(params, 1),
                                         TRUE)))
   return;
 
 fvtell_player(NORMAL_T(p), " Changed group to %s.\n", group->name);
 p->default_newsgroup = group->id;
}

static void user_su_news_group_add(player *p, const char *str)
{
 if (!*str)
   TELL_FORMAT(p, "<newsgroup>");
 
 if (!isdigit((unsigned char) *str) &&
     *(str + strspn(str, "0123456789-+_." ALPHABET_LOWER ALPHABET_UPPER)) != 0)
 {
  fvtell_player(NORMAL_T(p), " You can only put letters, numbers,"
                " -, +, . or _ in a newsgroup name, and it must start "
                "with a letter.\n");
  return;
 }

 if (!news_find_group_name(str))
 {
  news_group *group = news_add_group(str, unique_newsgroup_id++);
  
  fvtell_player(NORMAL_T(p), " News group %s added.\n", group->name);

  news_save_master_index();
  news_save_group(group);
 }
 else
   fvtell_player(NORMAL_T(p), " Oi vey that already exists.\n");
}

static void user_news_group_show_flags(player *p, parameter_holder *params)
{
 news_group *group = NULL;
 char news_group_title[NEWSGROUP_NAME_SIZE + 15] = {0};
 char buf[256];
 
 switch (params->last_param)
 {
  case 0:
    GET_DEFAULT_GROUP(p, group);
    break;

  case 1:
    if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR(params, 1),
                                            TRUE)))
      return;
    break;

  default:
    TELL_FORMAT(p, "[newsgroup]");
 }

 sprintf(news_group_title, "Newsgroup: %s", group->name);
 ptell_mid(NORMAL_T(p), news_group_title, FALSE);
 
 fvtell_player(NORMAL_FT(RAW_OUTPUT, p),
               "%s\n", group->description);
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 fvtell_player(NORMAL_T(p), "%-15s %s\n", "Created:",
               DISP_TIME_P_STD(group->c_timestamp, p));
 fvtell_player(NORMAL_T(p), "%-15s %s\n", "Last posted to:",
               DISP_TIME_P_STD(group->m_timestamp, p));
 fvtell_player(NORMAL_T(p), "%-15s %d\n", "Postings:",
               group->articles);
 if (group->max_articles)
   fvtell_player(NORMAL_T(p), "%-15s %d\n", "Max postings:",
                 group->max_articles);
 fvtell_player(NORMAL_T(p), "%-15s %s\n", "Expire after:",
               word_time_long(buf, sizeof(buf),
                              group->expire_after, WORD_TIME_DEFAULT));
 
 if (!group->can_post)
 {
  if (!PRIV_STAFF(p->saved))
    fvtell_player(NORMAL_T(p), "%s", "This group is read only.\n");
  else
    fvtell_player(NORMAL_T(p), "%s", "This group can only be posted to "
                  "by staff.\n");
 }
 
 if (!group->can_read)
   fvtell_player(NORMAL_T(p), "%s",
                 "This group can only be read and seen by staff.\n");

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void user_su_news_group_set_flags(player *p, parameter_holder *params)
{
 news_group *group = NULL;

 switch (params->last_param)
 {
  default:
    TELL_FORMAT(p, "<newsgroup> [flags]");
    
  case 1:
    user_news_group_show_flags(p, params);
    return;

  case 3:
    break;
 }
 
 if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR(params, 1),
                                         TRUE)))
   return;
 
 if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "can_post") ||
     !beg_strcmp(GET_PARAMETER_STR(params, 2), "can post"))
 {
  if (TOGGLE_MATCH_ON(GET_PARAMETER_STR(params, 3)))
    group->can_post = TRUE;
  else
    if (TOGGLE_MATCH_OFF(GET_PARAMETER_STR(params, 3)))
      group->can_post = FALSE;
    else
    {
     fvtell_player(NORMAL_T(p), " Invalid setting for can post, "
                   "use true/yes/on or false/no/off.\n");
     return;
    }
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "can_read") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "can read"))
 {
  if (TOGGLE_MATCH_ON(GET_PARAMETER_STR(params, 3)))
    group->can_read = TRUE;
  else
    if (TOGGLE_MATCH_OFF(GET_PARAMETER_STR(params, 3)))
      group->can_read = FALSE;
    else
    {
     fvtell_player(NORMAL_T(p), " Invalid setting for can read, "
                   "use true/yes/on or false/no/off.\n");
     return;
    }
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "description"))
 {
  int len = GET_PARAMETER_LENGTH(params, 3);
  if (len > (NEWSGROUP_DESCRIPTION_SIZE - 1))
    len = (NEWSGROUP_DESCRIPTION_SIZE - 1);
  COPY_STR_LEN(group->description, GET_PARAMETER_STR(params, 3), len);
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "max_articles") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "max articles"))
 {
  group->max_articles = atoi(GET_PARAMETER_STR(params, 3));
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "expire_time") ||
          !beg_strcmp(GET_PARAMETER_STR(params, 2), "expire time"))
 {
  int wt_err;
  group->expire_after = word_time_parse(GET_PARAMETER_STR(params, 3),
                                        WORD_TIME_PARSE_ERRORS, &wt_err);
  if (wt_err)
  {
   fvtell_player(SYSTEM_T(p), " Expire time in wrong format, expire time is a"
                 " period time option.\n");
   return;
  }
 }
 else
 {
  fvtell_player(NORMAL_T(p), "%s", " The valid options are:\n"
                "  can_post, can_read, description, max articles, "
                "expire time\n");
  return;
 }
 
 news_save_master_index();

 params->last_param = 1; /* hack :*/
 user_news_group_show_flags(p, params);
}

static void user_su_news_group_delete(player *p, parameter_holder *params)
{
 news_group *group = NULL;

 if (params->last_param != 1)
   TELL_FORMAT(p, "<newsgroup>");
 
 if (!(group = news_user_find_group_name(p, GET_PARAMETER_STR(params, 1),
                                         TRUE)))
   return;
 
 fvtell_player(NORMAL_T(p), " News group %s deleted.\n", group->name);  
 news_remove_group(group);
 news_save_master_index();
}

void cmds_init_news(void)
{
 CMDS_BEGIN_DECLS();

#define CMDS_SECTION_SUB CMDS_SECTION_NEWS
 
 CMDS_ADD_SUB("afollowup", user_news_anon_followup_article, CONST_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("apost", user_news_anon_post_article, CONST_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("areply", user_news_anon_reply_article, CONST_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("commands", news_view_commands, NO_CHARS);
 CMDS_ADD_SUB("check", user_news_list_articles, CONST_CHARS);
 CMDS_XTRA_SUB(NEWSGROUP, NEWSGROUP);
 CMDS_ADD_SUB("check_all", user_news_list_articles_all, CONST_CHARS);
 CMDS_XTRA_SUB(NEWSGROUP, NEWSGROUP);
 CMDS_ADD_SUB("end", news_exit_mode, NO_CHARS);
 CMDS_FLAG(no_expand); CMDS_PRIV(mode_news);
 CMDS_ADD_SUB("followup", user_news_followup_article, CONST_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("group", newsgroup_command_wrapper, RET_CHARS_SIZE_T);
 CMDS_ADD_SUB("groups", user_news_list_newsgroups, CONST_CHARS);
 CMDS_ADD_SUB("post", user_news_post_article, CONST_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("read", user_news_read_article, CONST_CHARS);
 CMDS_ADD_SUB("remove", user_news_remove_article, CONST_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("reply", user_news_reply_article, CONST_CHARS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("show_info", user_news_show_info, NO_CHARS);
 CMDS_ADD_SUB("view", user_news_list_articles, CONST_CHARS);
 CMDS_XTRA_SUB(NEWSGROUP, NEWSGROUP);
 CMDS_ADD_SUB("view_all", user_news_list_articles_all, CONST_CHARS);
 CMDS_XTRA_SUB(NEWSGROUP, NEWSGROUP);
 
#undef CMDS_SECTION_SUB
#define CMDS_SECTION_SUB CMDS_SECTION_NEWSGROUP

 CMDS_ADD_SUB("add_group", user_su_news_group_add, CONST_CHARS);
 CMDS_PRIV(normal_su);
 CMDS_ADD_SUB("afollowup", user_news_group_anon_followup_article, CONST_CHARS);
 CMDS_ADD_SUB("apost", user_news_group_anon_post_article, CONST_CHARS);
 CMDS_ADD_SUB("areply", user_news_group_anon_reply_article, CONST_CHARS);
 CMDS_ADD_SUB("commands", newsgroup_view_commands, NO_CHARS);
 CMDS_ADD_SUB("delete_group", user_su_news_group_delete, PARSE_PARAMS);
 CMDS_PRIV(normal_su);
 CMDS_ADD_SUB("end", newsgroup_exit_mode, NO_CHARS);
 CMDS_FLAG(no_expand); CMDS_PRIV(mode_newsgroup);
 CMDS_ADD_SUB("followup", user_news_group_followup_article, CONST_CHARS);
 CMDS_ADD_SUB("list", user_news_list_newsgroups, CONST_CHARS);
 CMDS_ADD_SUB("post", user_news_group_post_article, CONST_CHARS);
 CMDS_ADD_SUB("read", user_news_group_read_article, CONST_CHARS);
 CMDS_ADD_SUB("reply", user_news_group_reply_article, CONST_CHARS);
 CMDS_ADD_SUB("remove", user_news_group_remove_article, CONST_CHARS);
 CMDS_ADD_SUB("set_default", user_news_set_default_group, PARSE_PARAMS);
 CMDS_ADD_SUB("set_info", user_su_news_group_set_flags, PARSE_PARAMS);
 CMDS_PRIV(coder_senior_su);
 CMDS_ADD_SUB("set_temporary", user_news_set_tmp_group, PARSE_PARAMS);
 CMDS_ADD_SUB("show_info", user_news_group_show_flags, PARSE_PARAMS);

#undef CMDS_SECTION_SUB

 CMDS_ADD("news", news_command, RET_CHARS_SIZE_T, MISC);
 CMDS_ADD("newsgroup", newsgroup_command, RET_CHARS_SIZE_T, MISC);
}
