#define ALIAS_C
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


static alias_lib_node *alias_lib_start = NULL;
static int alias_lib_num = 0;
static int alias_lib_num_must_use = 0;

static time_t alias_lib_last_del;

static int alias_flag_func_no_beg_space(player *p, alias_node *current,
                                        const char *str)
{ ALIAS_FLAG_FUNC_ON_OFF(no_beg_space, "No space"); }
static int alias_flag_func_disabled(player *p, alias_node *current,
                                    const char *str)
{ ALIAS_FLAG_FUNC_ON_OFF(disabled, "Disabled"); }
static int alias_flag_func_clever_match(player *p, alias_node *current,
                                        const char *str)
{ ALIAS_FLAG_FUNC_ON_OFF(clever_match, "Clever match"); }
static int alias_flag_func_expand_match(player *p, alias_node *current,
                                        const char *str)
{ ALIAS_FLAG_FUNC_ON_OFF(expand_match, "Expand match"); }
static int alias_flag_func_hidden(player *p, alias_node *current,
                                  const char *str)
{ ALIAS_FLAG_FUNC_ON_OFF(hidden, "Hidden"); }
static int alias_flag_func_use_name(player *p, alias_node *current,
                                    const char *str)
{ ALIAS_FLAG_FUNC_ON_OFF(use_name, "Use name"); }
static int alias_flag_func_in_sub_command(player *p, alias_node *current,
                                          const char *str)
{ ALIAS_FLAG_FUNC_ON_OFF(in_sub_command, "Sub alias"); }
static int alias_flag_func_in_sub_command_only(player *p, alias_node *current,
                                               const char *str)
{ ALIAS_FLAG_FUNC_ON_OFF(in_sub_command_only, "Sub alias only"); }
static int alias_flag_func_public(player *p, alias_node *current,
                                  const char *str)
{ ALIAS_FLAG_FUNC_ON_OFF(public, "Public"); }
static int alias_flag_func_comment(player *p, alias_node *current,
                                   const char *str)
{
 if (!str)
   return (current->comment != NULL);

 if (!*str)
 {
  if (current->comment)
    FREE(current->comment);
  current->comment = NULL;

  if (p)
    fvtell_player(NORMAL_FT(RAW_OUTPUT_VARIABLES, p),
                  " Set ^S^BComment^s to be ^S^Bblank^s, "
                  "on the alias ^S^B%s^s.\n", current->command);
  return (FALSE);
 }
 else
 {
  size_t len = strnlen(str, ALIAS_COMMENT_SZ - 1);

  if (!(current->comment = MALLOC(len + 1)))
  {
   if (p)
     fvtell_player(NORMAL_FT(RAW_OUTPUT_VARIABLES, p),
                   " ** Error: couldn't set that comment at this time.\n");
   return (FALSE);
  }
  
  memcpy(current->comment, str, len);
  current->comment[len] = 0;

  if (p)
    fvtell_player(NORMAL_FT(RAW_OUTPUT_VARIABLES, p),
                  " Set ^S^Bcomment^s to '^B%s^N', on the alias ^S^B%s^s.\n",
                  current->comment, current->command);
 }

 return (TRUE);
}

static int alias_flag_func_command_type(player *p, alias_node *current,
                                        const char *str)
{
 if (!str)
   return (current->command_type);
 
 str += strspn(str, " ");
 if (*str == '=')
 {
  ++str;
  str += strspn(str, " ");
 }
 
 if (!beg_strcmp(str, "none"))
 {
  current->command_type = ALIAS_COMMAND_TYPE_NONE;
  return (FALSE);
 }
 else if (!beg_strcmp(str, "socials"))
   current->command_type = ALIAS_COMMAND_TYPE_SOCIALS;
 else if (!beg_strcmp(str, "communication") || !beg_strcmp(str, "comms") ||
          !beg_strcmp(str, "coms"))
   current->command_type = ALIAS_COMMAND_TYPE_COMMUNICATION;
 else if (!beg_strcmp(str, "local") || !beg_strcmp(str, "room"))
   current->command_type = ALIAS_COMMAND_TYPE_LOCAL;
 else if (!beg_strcmp(str, "information"))
   current->command_type = ALIAS_COMMAND_TYPE_INFO;
 else if (!beg_strcmp(str, "settings"))
   current->command_type = ALIAS_COMMAND_TYPE_SETTINGS;
 else if (!(beg_strcmp(str, "pinformation") &&
            beg_strcmp(str, "personal information")))
   current->command_type = ALIAS_COMMAND_TYPE_PERSONAL_INFO;
 else if (!beg_strcmp(str, "list"))
   current->command_type = ALIAS_COMMAND_TYPE_LIST;
 else if (!beg_strcmp(str, "system"))
   current->command_type = ALIAS_COMMAND_TYPE_SYSTEM;
 else if (!beg_strcmp(str, "miscellaneous"))
   current->command_type = ALIAS_COMMAND_TYPE_MISC;
 else if (!beg_strcmp(str, "multis"))
   current->command_type = ALIAS_COMMAND_TYPE_MULTIS;
 else if (!beg_strcmp(str, "games"))
   current->command_type = ALIAS_COMMAND_TYPE_GAMES;
 else if (p->saved->priv_spod && !beg_strcmp(str, "spods"))
   current->command_type = ALIAS_COMMAND_TYPE_SPOD;
 else if ((p->saved->priv_minister || p->saved->priv_admin) &&
          !beg_strcmp(str, "minister"))
   current->command_type = ALIAS_COMMAND_TYPE_MINISTER;
 else if (PRIV_STAFF(p->saved) && !beg_strcmp(str, "sus"))
   current->command_type = ALIAS_COMMAND_TYPE_SU;
 else if (p->saved->priv_admin && !beg_strcmp(str, "admin"))
   current->command_type = ALIAS_COMMAND_TYPE_ADMIN;
 else
 {
  if (p)
    fvtell_player(SYSTEM_T(p), "%s",
                  " That is not a valid alias type, setting to type none.\n");
  current->command_type = ALIAS_COMMAND_TYPE_NONE;
  return (FALSE);
 }

 fvtell_player(NORMAL_T(p), " Set ^S^Btype^s to "
               "'^S^B%s^s', on the alias ^S^B%s^s.\n",
               cmds_map_sections[current->command_type], current->command);

 return (TRUE);
}

static struct
{
 const char *name;
 int (*func)(player *, alias_node *, const char *);
 bitflag dup : 1;
} alias_flags[] =
{
 {"clever match", alias_flag_func_clever_match, FALSE},
 {"clever_match", alias_flag_func_clever_match, TRUE},
 {"comment", alias_flag_func_comment, TRUE},
 {"disabled", alias_flag_func_disabled, FALSE},
 {"expand match", alias_flag_func_expand_match, FALSE},
 {"expand_match", alias_flag_func_expand_match, TRUE},
 {"hidden", alias_flag_func_hidden, FALSE},
 {"hide", alias_flag_func_hidden, TRUE},
 {"no space", alias_flag_func_no_beg_space, FALSE},
 {"no_space", alias_flag_func_no_beg_space, TRUE},
 {"public", alias_flag_func_public, FALSE},
 {"sub alias", alias_flag_func_in_sub_command, FALSE},
 {"sub_alias", alias_flag_func_in_sub_command, TRUE},
 {"sub alias only", alias_flag_func_in_sub_command_only, FALSE},
 {"sub_alias_only", alias_flag_func_in_sub_command_only, TRUE},
 {"type", alias_flag_func_command_type, TRUE},
 {"use name", alias_flag_func_use_name, FALSE},
 {"use_name", alias_flag_func_use_name, TRUE},
 {NULL, NULL, TRUE}
};

static int alias_insert(alias_node **head, alias_node *new_node)
{
 alias_node *insert = *head;

 if (insert)
 {
  while (insert->next && (strcmp(new_node->command, insert->command) > 0))
    insert = insert->next;
  
  if (!strcmp(new_node->command, insert->command))
    return (FALSE);
  
  if (strcmp(new_node->command, insert->command) > 0)
  {
   if ((new_node->next = insert->next))
     new_node->next->prev = new_node;
   
   new_node->prev = insert;
   insert->next = new_node;
  }
  else
  {
   assert(strcmp(new_node->command, insert->command) < 0);
   
   if ((new_node->prev = insert->prev))
     new_node->prev->next = new_node;
   else
     *head = new_node;
   
   new_node->next = insert;
   insert->prev = new_node;
  }
 }
 else
 {
  *head = new_node;
  new_node->next = NULL;
  new_node->prev = NULL;
 }

 return (TRUE);
}


static alias_node *alias_new_insert(alias_node **head, const char *command,
                                    const char *str)
{
 alias_node *new_node = NULL;
 char *saved_str = NULL;
 size_t len = 0;
 
 if (!(new_node = XMALLOC(sizeof(alias_node), ALIAS_NODE)))
   return (NULL);

 len = strnlen(str, ALIAS_STR_SZ - 1);
 if (!(saved_str = MALLOC(len + 1)))
 {
  XFREE(new_node, ALIAS_NODE);
  return (NULL);
 }

 sprintf(new_node->command, "%.*s", ALIAS_COMMAND_SZ - 1, command);

 if (!alias_insert(head, new_node))
 {
  FREE(saved_str);
  XFREE(new_node, ALIAS_NODE);
  
  return (NULL);
 }
 
 new_node->str = saved_str;
 memcpy(saved_str, str, len + 1);
 new_node->str[len] = 0;
 
 new_node->comment = NULL;
 new_node->command_type = ALIAS_COMMAND_TYPE_NONE;
 
 new_node->flag_no_beg_space = FALSE;
 new_node->flag_disabled = FALSE;
 new_node->flag_clever_match = FALSE;
 new_node->flag_expand_match = FALSE;
 new_node->flag_hidden = FALSE;
 new_node->flag_use_name = FALSE;
 new_node->flag_in_sub_command = FALSE;
 new_node->flag_in_sub_command_only = FALSE;
 new_node->flag_public = FALSE;
 new_node->flag_prived = FALSE;
 
 return (new_node);
}

static alias_node *alias_find(alias_node *scan, const char *command)
{
 while (scan && (strcmp(command, scan->command) > 0))
   scan = scan->next;

 if (scan && !strcmp(command, scan->command))
   return (scan);

 return (NULL);
}

static alias_node *alias_remove(alias_node **head, const char *command)
{
 alias_node *tmp = alias_find(*head, command);

 if (tmp)
 {
  if (tmp->next)
    tmp->next->prev = tmp->prev;

  if (tmp->prev)
    tmp->prev->next = tmp->next;
  else
    *head = tmp->next;
 }

 return (tmp);
}

static int alias_destroy(alias_node **head, const char *command)
{
 alias_node *tmp = alias_remove(head, command);

 if (tmp)
 {
  FREE(tmp->str);
  XFREE(tmp, ALIAS_NODE);
  
  return (TRUE);
 }

 return (FALSE);
}

static alias_lib_node *alias_lib_create(alias_lib_node *prev)
{
 alias_lib_node *lib = NULL;
 
 if (!(lib = XMALLOC(sizeof(alias_lib_node), ALIAS_LIB_NODE)))
   SHUTDOWN_MEM_ERR();

 ++alias_lib_num;
 
 if (!prev)
 {
  lib->next =  alias_lib_start;
  alias_lib_start = lib;
 }
 else
 {
  lib->next = prev->next;
  prev->next = lib;
 }
 
 lib->num = 0;
 lib->head = NULL;

 lib->name[0] = 0;
 lib->priv_type = 0;

 lib->player_lib = FALSE;

 return (lib);
}

static alias_lib_node **alias_lib_find(const char *name)
{
 alias_lib_node **scan = &alias_lib_start;

 while (*scan)
 {
  if (!strcmp((*scan)->name, name))
    return (scan);
  
  scan = &(*scan)->next;
 }

 return (NULL);
}

static alias_lib_node *alias_lib_user_find(player *p, const char *name,
                                           int writable)
{
 alias_lib_node **tmp = alias_lib_find(name);

 if (!tmp)
 {
  fvtell_player(SYSTEM_T(p), " Alias library -- ^S^B%s^s -- doesn't "
                "exist.\n", name);
  return (NULL);
 }

 if ((*tmp)->player_lib && p->saved->priv_lib_maintainer &&
     !strcmp(p->saved->lower_name, name))
 {
  if (writable && (*tmp)->priv_type &&
      !priv_test_int(p->saved, (*tmp)->priv_type))
    fvtell_player(SYSTEM_T(p), " Your library is only available to people "
                  "who have privs that you ^S^Bdo not^s.\n");

  return (*tmp);
 }

 if (writable && !p->saved->priv_coder)
 {
  fvtell_player(SYSTEM_T(p), " Alias library -- ^S^B%s^s -- isn't "
                "available to you.\n", name);
  return (NULL);
 }
 
 if ((*tmp)->priv_type && !priv_test_int(p->saved, (*tmp)->priv_type))
 {
  fvtell_player(SYSTEM_T(p), " Alias library -- ^S^B%s^s -- isn't "
                "available to you.\n", name);
  return (NULL);
 }

 return (*tmp);
}

static void internal_alias_destroy_list(alias_node *scan)
{
 while (scan)
 {
  alias_node *scan_next = scan->next; 

  if (scan->comment)
    FREE(scan->comment);
  
  FREE(scan->str);
  XFREE(scan, ALIAS_NODE);
  
  scan = scan_next;
 }
}

static int alias_lib_destroy(const char *name)
{
 alias_lib_node **lib = alias_lib_find(name);
 alias_lib_node *tmp = NULL;
 
 if (!lib)
   return (FALSE);

 tmp = *lib;
 
 internal_alias_destroy_list(tmp->head);
 
 alias_lib_last_del = now;

 *lib = tmp->next;

 if (tmp->must_use)
   --alias_lib_num_must_use;

 XFREE(tmp, ALIAS_LIB_NODE);

 --alias_lib_num;

 return (TRUE);
}

static void internal_alias_load(alias_node **head, int number_of_aliases,
                                file_io *io_player)
{
 int count = 0;
 
 file_section_beg("list", io_player);
 
 while (count < number_of_aliases)
 {
  alias_node *tmp = NULL;
  char command[ALIAS_COMMAND_SZ];
  char *comment = NULL;
  char str[ALIAS_STR_SZ];
  char buffer[BUF_NUM_TYPE_SZ(int)];
  short command_type = 0;
  int flag_clever_match,
    flag_disabled,
    flag_expand_match,
    flag_hidden,
    flag_no_beg_space,
    flag_prived,
    flag_public,
    flag_in_sub_command,
    flag_in_sub_command_only,
    flag_use_name;

  sprintf(buffer, "%04d", ++count);

  file_section_beg(buffer, io_player);

  file_get_string("command", command, ALIAS_COMMAND_SZ, io_player);

  command_type = file_get_short("command_type", io_player);
  if (file_get_bitflag("comment_do", io_player))
  {
   comment = file_get_malloc("comment_str", NULL, io_player);
   assert(strlen(comment) < ALIAS_COMMENT_SZ);
  }

  file_section_beg("flags", io_player);
  flag_clever_match = file_get_bitflag("clever_match", io_player);
  flag_disabled = file_get_bitflag("disabled", io_player);
  flag_expand_match = file_get_bitflag("expand_match", io_player);
  flag_hidden = file_get_bitflag("hidden", io_player);
  flag_no_beg_space = file_get_bitflag("no_beg_space", io_player);
  flag_prived = file_get_bitflag("prived", io_player);
  flag_public = file_get_bitflag("public", io_player);
  flag_in_sub_command = file_get_bitflag("sub_command", io_player);
  flag_in_sub_command_only = file_get_bitflag("sub_command_only", io_player);
  flag_use_name = file_get_bitflag("use_name", io_player);
  file_section_end("flags", io_player);
  
  file_get_string("string", str, ALIAS_STR_SZ, io_player);
  
  if (!(tmp = alias_new_insert(head, command, str)))
    SHUTDOWN_MEM_ERR();

  tmp->command_type = command_type;

  tmp->comment = comment;

  tmp->flag_clever_match = flag_clever_match;
  tmp->flag_disabled = flag_disabled;
  tmp->flag_expand_match = flag_expand_match;
  tmp->flag_hidden = flag_hidden;
  tmp->flag_no_beg_space = flag_no_beg_space;
  tmp->flag_prived = flag_prived;
  tmp->flag_public = flag_public;
  tmp->flag_in_sub_command = flag_in_sub_command;
  tmp->flag_in_sub_command_only = flag_in_sub_command_only;
  tmp->flag_use_name = flag_use_name;
  
  file_section_end(buffer, io_player);
 }
 
 file_section_end("list", io_player);
}

static void internal_alias_load_subscriptions(player *p, file_io *io_player)
{
 int count = 0;
 
 file_section_beg("subs", io_player);

 while (count < (ALIAS_LIB_LD_SO_SZ - 1))
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];

  sprintf(buffer, "%04d", ++count);

  file_section_beg(buffer, io_player);
  if (!file_get_bitflag("is_used", io_player))
  {
   file_section_end(buffer, io_player);
   break;
  }
  
  if (!(p->alias_lib_saved_list[count - 1] = file_get_malloc("name",
                                                             NULL, io_player)))
    SHUTDOWN_MEM_ERR();
  
  file_section_end(buffer, io_player);
 }
 
 file_section_end("subs", io_player);
}

void alias_load(player *p, file_io *io_player)
{ 
 file_section_beg("header", io_player);

 p->max_aliases = file_get_int("max_aliases", io_player);
 p->number_of_aliases = file_get_int("number_of_aliases", io_player);
 
 file_section_end("header", io_player);

 internal_alias_load(&p->aliases_start, p->number_of_aliases, io_player);
 internal_alias_load_subscriptions(p, io_player);
}

static int internal_alias_save(alias_node *scan, file_io *io_player)
{
 int count = 0;
 
 file_section_beg("list", io_player);
 
 while (scan)
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];

  sprintf(buffer, "%04d", ++count);

  file_section_beg(buffer, io_player);

  file_put_string("command", scan->command, 0, io_player);
  file_put_short("command_type", scan->command_type, io_player);
  
  if (scan->comment)
  {
   file_put_bitflag("comment_do", TRUE, io_player);
   file_put_string("comment_str", scan->comment, 0, io_player);
  }
  else
    file_put_bitflag("comment_do", FALSE, io_player);
  
  file_section_beg("flags", io_player);
  file_put_bitflag("clever_match", scan->flag_clever_match, io_player);
  file_put_bitflag("disabled", scan->flag_disabled, io_player);
  file_put_bitflag("expand_match", scan->flag_expand_match, io_player);
  file_put_bitflag("hidden", scan->flag_hidden, io_player);
  file_put_bitflag("no_beg_space", scan->flag_no_beg_space, io_player);
  file_put_bitflag("prived", scan->flag_prived, io_player);
  file_put_bitflag("public", scan->flag_public, io_player);
  file_put_bitflag("sub_command", scan->flag_in_sub_command, io_player);
  file_put_bitflag("sub_command_only", scan->flag_in_sub_command_only,
                   io_player);
  file_put_bitflag("use_name", scan->flag_use_name, io_player);
  file_section_end("flags", io_player);
  
  file_put_string("string", scan->str, 0, io_player);
  
  file_section_end(buffer, io_player);
  
  scan = scan->next;
 }
 
 file_section_end("list", io_player);

 return (count);
}

static void internal_alias_save_subscriptions(player *p, file_io *io_player)
{
 int count = 0;
 
 file_section_beg("subs", io_player);

 while ((count < (ALIAS_LIB_LD_SO_SZ - 1)) && p->alias_lib_saved_list[count])
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];

  sprintf(buffer, "%04d", ++count);

  file_section_beg(buffer, io_player);
  
  file_put_bitflag("is_used", TRUE, io_player);
  
  file_put_string("name", p->alias_lib_saved_list[count - 1], 0, io_player);
  
  file_section_end(buffer, io_player);
 }
 
 file_section_end("subs", io_player);
}

void alias_save(player *p, file_io *io_player)
{
 int count = 0;

 file_section_beg("header", io_player);

 file_put_int("max_aliases", p->max_aliases, io_player);
 file_put_int("number_of_aliases", p->number_of_aliases, io_player);
 
 file_section_end("header", io_player);

 count = internal_alias_save(p->aliases_start, io_player);

 if (p->number_of_aliases != count)
   log_assert(FALSE);
 
 p->number_of_aliases = count;

 internal_alias_save_subscriptions(p, io_player);
}

static void alias_lib_load(void)
{
 file_io index_io_sys;
 file_io real_io_sys;
 file_io *io_sys = &real_io_sys;
 int lib_count = 0;
 alias_lib_node *lib = NULL;
 int local_alias_lib_num = 0;
 
 alias_lib_last_del = now;
 
 if (!file_read_open("files/sys/aliases/index", &index_io_sys))
   return;

 file_section_beg("header", &index_io_sys);
 local_alias_lib_num = file_get_int("number_of_libraries", &index_io_sys);
 file_section_end("header", &index_io_sys);

 file_section_beg("libraries", &index_io_sys);

 while (lib_count < local_alias_lib_num)
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];
  char file_name[sizeof("files/sys/aliases/players/") + ALIAS_LIB_NAME_SZ];
  
  sprintf(buffer, "%04d", ++lib_count);

  file_section_beg(buffer, &index_io_sys);

  lib = alias_lib_create(lib);
  
  file_get_string("name", lib->name, ALIAS_LIB_NAME_SZ, &index_io_sys);
  lib->player_lib = file_get_bitflag("player_lib", &index_io_sys);
  
  sprintf(file_name, "files/sys/aliases/%s/%s",
          lib->player_lib ? "players" : "misc", lib->name);
  
  if (file_read_open(file_name, io_sys))
  {
   alias_node *scan = NULL;
   
   file_section_beg("header", io_sys);
   
   if ((lib->must_use = file_get_bitflag("must_use", io_sys)))
     ++alias_lib_num_must_use;
   
   lib->needs_help = file_get_bitflag("needs_help", io_sys);
   lib->num = file_get_int("number_of_aliases", io_sys);
   lib->priv_type = file_get_int("priv_type", io_sys);
   
   file_section_end("header", io_sys);

   internal_alias_load(&lib->head, lib->num, io_sys);
   scan = lib->head;
   while (scan)
   {
    scan->flag_public = TRUE;
    if (lib->needs_help)
    {
     CMDS_HELP_SEARCH(scan->command, "sys alias");
    }
    scan = scan->next;
   }
   
   file_read_close(io_sys);
  }
  
  file_section_end(buffer, &index_io_sys);
 }
 assert((lib_count == local_alias_lib_num) && (lib_count == alias_lib_num));
   
 file_section_end("libraries", &index_io_sys);

 file_read_close(&index_io_sys);
}

static void alias_lib_index_save(void)
{
 file_io index_io_sys;
 alias_lib_node *scan = alias_lib_start;
 int lib_count = 0;
 
 if (configure.talker_read_only)
   return;
 
 if (!file_write_open("files/sys/aliases/index.tmp",
                      ALIAS_LIB_INDEX_FILE_VERSION, &index_io_sys))
 {
  log_assert(FALSE);
  return;
 }

 file_section_beg("header", &index_io_sys);
 file_put_int("number_of_libraries", alias_lib_num, &index_io_sys);
 file_section_end("header", &index_io_sys);

 file_section_beg("libraries", &index_io_sys);

 while (scan)
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];
  sprintf(buffer, "%04d", ++lib_count);
  
  file_section_beg(buffer, &index_io_sys);

  file_put_string("name", scan->name, 0, &index_io_sys);
  file_put_bitflag("player_lib", scan->player_lib, &index_io_sys);

  file_section_end(buffer, &index_io_sys);

  scan = scan->next;
 }
 assert(!scan && (lib_count == alias_lib_num));
 
 file_section_end("libraries", &index_io_sys);

 if (file_write_close(&index_io_sys))
   rename("files/sys/aliases/index.tmp", "files/sys/aliases/index");
}

static void alias_lib_save(alias_lib_node *scan)
{
 file_io real_io_sys;
 file_io *io_sys = &real_io_sys;
 char file_name[sizeof("files/sys/aliases/players/.tmp") + ALIAS_LIB_NAME_SZ];
 char real_name[sizeof("files/sys/aliases/players/") + ALIAS_LIB_NAME_SZ];
 int sz = 0;
 int count = 0;
 
 if (configure.talker_read_only)
   return;

 sprintf(file_name, "files/sys/aliases/%s/%s%n.tmp",
         scan->player_lib ? "players" : "misc", scan->name, &sz);

 if (!file_write_open(file_name, ALIAS_LIB_FILE_VERSION, io_sys))
 {
  log_assert(FALSE);
  return;
 }
 memcpy(real_name, file_name, sz);
 real_name[sz] = 0;

 file_section_beg("header", io_sys);

 file_put_bitflag("must_use", scan->must_use, io_sys);
 file_put_bitflag("needs_help", scan->needs_help, io_sys);
 file_put_int("number_of_aliases", scan->num, io_sys);
 file_put_int("priv_type", scan->priv_type, io_sys);
 
 file_section_end("header", io_sys);

 count = internal_alias_save(scan->head, io_sys);
 assert(scan->num == count);

 if (file_write_close(io_sys))
   rename(file_name, real_name);
}

void alias_cleanup(player *p)
{
 int count = 0;
 
 internal_alias_destroy_list(p->aliases_start);

 while ((count < (ALIAS_LIB_LD_SO_SZ - 1)) && p->alias_lib_saved_list[count])
 {
  FREE(p->alias_lib_saved_list[count]);
  ++count;
 }
 p->alias_lib_saved_list[0] = NULL;
 
 p->aliases_start = NULL; 
 p->flag_tmp_dont_save_after_this = TRUE;
}

static int alias_get_number(const char **str)
{
 const char *scan = *str;
 int number = -1;
 
 if (*scan == '{')
 {
  ++scan;

  if (isdigit((unsigned char) *scan) &&
      (*(scan + strspn(scan, "0123456789")) == '}'))
  {
   number = skip_atoi(&scan);
   assert(*scan == '}');
   ++scan;
  }
  else
    number = -1;
  }
  else if (isdigit((unsigned char) *scan))
    number = skip_atoi(&scan);

 if (number >= 0)
 {
  *str = scan;
  return (number);
 }

 return (-1);
}

static input_node *internal_substitute_alias_command(player *p,
                                                     alias_node *a_node,
                                                     const char *param_input)
{
 const char *scan = a_node->str;
 const char *raw_str = NULL;
 int raw_length = 0;
 input_node *in_node = input_add(p, p->input_start);
 parameter_holder real_parameters;
 parameter_holder *parameters = &real_parameters;

 assert(p->input_start);
 if (!in_node)
 {
  log_assert(FALSE);
  user_logoff(p, NULL);
  return (NULL);
 }
 in_node->comp_generated = TRUE;

 get_parameter_init(parameters);
 
 while (*scan && (INPUT_SPACE_LEFT(in_node, 1) > 0))
 {
  int param_number = 0;
  
  if (*scan == '%')
  {
   const char *old_scan = scan;
   int use_text_objs = FALSE;
   
  parse_type:
   
   ++scan;
   switch (*scan)
   {
   case 'n': /* return ... ohhh multiple commands */
   case ';':
     ++scan;

     INPUT_TERMINATE(in_node);
     in_node->ready = TRUE;
     if (!(in_node = input_add(p, in_node)))
     {
      log_assert(FALSE);
      user_logoff(p, NULL);
      return (NULL);
     }
     in_node->comp_generated = TRUE;
     
      /* skip spaces... as we can't have them at the begining anyway */
     scan += strspn(scan, " ");
     continue;
     
   case '!': /* raw text from now on */
      if (!raw_str)
      {
       if (*param_input == ' ')
         raw_str = param_input + 1;
       else
         raw_str = param_input;
       raw_length = strlen(raw_str);

       param_input = raw_str + raw_length;
       assert(!*param_input);
      }

      ALIAS_ADD_STR(raw_str, raw_length);
            
      ++scan;
      continue;
      
    case '%':
      INPUT_ADD(in_node, '%');
      ++scan;
      continue;

    case '#':
    {
     char numb_buffer[BUF_NUM_TYPE_SZ(int)];
     int numb_length = 0;

     get_parameter_parse(parameters, &param_input, GET_PARAMETER_NUMBER_MAX);
     
     numb_length = sprintf(numb_buffer, "%d", parameters->last_param);

     if (INPUT_SPACE_LEFT(in_node, numb_length) > 0)
       INPUT_COPY(in_node, numb_buffer, numb_length);
     else
       goto buffer_too_small;
     ++scan;
    }
    continue;
     
    case '$':
    {
     int from_end = 0;
     
     ++scan;
     
     get_parameter_parse(parameters, &param_input, GET_PARAMETER_NUMBER_MAX);
     
     if (*scan == '-')
     {
      ++scan;
      if ((from_end = alias_get_number(&scan)) == -1)
      {
       from_end = 0;
       --scan;
      }
     }
     
     param_number = parameters->last_param - from_end;
    }
    break;
      
    case '@':
    {
     int from_end  = -1;
     unsigned int tmp = 0;
     
     ++scan;
     
     get_parameter_parse(parameters, &param_input, GET_PARAMETER_NUMBER_MAX);
     
     if (*scan == '-')
     {
      ++scan;
      if ((from_end = alias_get_number(&scan)) == -1)
      {
       from_end = 0;
       --scan;
      }
     }

     tmp = from_end + 1;
     
     while (tmp <= parameters->last_param)
     {
      const char *parameter_str = GET_PARAMETER_STR(parameters, tmp);
      
      assert(parameter_str);
      
      if (GET_PARAMETER_LENGTH(parameters, tmp))
        /* -2 below for the %@ thing ... can still overflow though */
      {
       int extra_chars = 0;
       
       if (INPUT_SPACE_LEFT(in_node,
                            GET_PARAMETER_LENGTH(parameters, tmp) + 3) <= 0)
         goto buffer_too_small; /* speed hack */
       
       if (tmp > (unsigned int)(from_end + 1))
       {
        INPUT_ADD(in_node, ' ');
        ++extra_chars;
       }
       
       INPUT_ADD(in_node, '"');
       ++extra_chars;
       
       while (*parameter_str)
       {
        if (*parameter_str == '"')
        {
         if (INPUT_SPACE_LEFT(in_node,
                              GET_PARAMETER_LENGTH(parameters, tmp) + 5) <= 0)
         {
          in_node->length -= extra_chars;
          goto buffer_too_small;
         }

         INPUT_ADD(in_node, '"');
         INPUT_ADD(in_node, '\'');
         INPUT_ADD(in_node, *parameter_str);
         INPUT_ADD(in_node, '\'');
         INPUT_ADD(in_node, '"');
         extra_chars += 4;
        }
        else
        {
         if (INPUT_SPACE_LEFT(in_node,
                              GET_PARAMETER_LENGTH(parameters, tmp) + 1) <= 0)
         {
          in_node->length -= extra_chars;
          goto buffer_too_small;
         }
           
         INPUT_ADD(in_node, *parameter_str);
        }

        ++parameter_str;
        ++extra_chars;
       }
       
       if (INPUT_SPACE_LEFT(in_node,
                            GET_PARAMETER_LENGTH(parameters, tmp) + 1) <= 0)
       {
        in_node->length -= extra_chars;
        goto buffer_too_small;
       }
       
       INPUT_ADD(in_node, '"');
      }
      ++tmp;
     }
    }
    continue;

    case '*':
    {
     int from_end = 0;
     unsigned int tmp = 0;
     
     ++scan;
     
     get_parameter_parse(parameters, &param_input, GET_PARAMETER_NUMBER_MAX);
     
     if (*scan == '-')
     {
      ++scan;
      if ((from_end = alias_get_number(&scan)) == -1)
      {
       from_end = 0;
       --scan;
      }
     }     
     
     tmp = from_end + 1;
     
     while (tmp <= parameters->last_param)
     {
      assert(GET_PARAMETER_STR(parameters, tmp));
      
      if (GET_PARAMETER_LENGTH(parameters, tmp))
      {
       if (INPUT_SPACE_LEFT(in_node, 1) <= 0)
         goto buffer_too_small;
       if (tmp > (unsigned int)(from_end + 1))
         INPUT_ADD(in_node, ' ');

       ALIAS_ADD_STR(GET_PARAMETER_STR(parameters, tmp),
                     GET_PARAMETER_LENGTH(parameters, tmp));
      }
      
      ++tmp;
     }
    }
    continue;

    case 'T':
      use_text_objs = TRUE;
      goto parse_type;
    
    case '{':
    default:
      param_number = alias_get_number(&scan);
   }
   
   if ((param_number >= 0)  && (param_number <= GET_PARAMETER_NUMBER_MAX))
   {
    if (param_number)
    {
     if (!get_parameter_parse(parameters, &param_input, param_number))
       continue; /* skip it */

     assert(GET_PARAMETER_STR(parameters, param_number));
     if (GET_PARAMETER_LENGTH(parameters, param_number))
     {
      ALIAS_ADD_STR(GET_PARAMETER_STR(parameters, param_number),
                    GET_PARAMETER_LENGTH(parameters, param_number));
     }
    }
    else
    {
     const char *str = current_command ? current_command : a_node->command;
     int tmp = strlen(str);

     ALIAS_ADD_STR(str, tmp);
    }
    
    continue;
   }

   scan = old_scan;
  }

  INPUT_ADD(in_node, *scan);  
  ++scan;
 }

 buffer_too_small: /* used inside the switch for %@ %* %1 etc... */
 input_del(p, p->input_start); /* can't think of a better place... */
 
 INPUT_TERMINATE(in_node);
 in_node->ready = TRUE;
 
 return (in_node);
}

void alias_ld_so(player *p, const char **name,
                 alias_search_node *save, int do_skip_system)
{
 alias_lib_node **scan = NULL;
 alias_node **lib = save->sys_list;
 int skip_player = FALSE;
 int skip_system = FALSE;
 unsigned int count = 0;

 assert(save && p);

 if (difftime(alias_lib_last_del, p->alias_lib_ld_cache_timestamp) >= 0)
   alias_lib_ldconfig(p);

 scan = p->alias_lib_ldconfig_list;
 
 if (name && (**name == '\\'))
 {
  save->player_current = NULL;
  ++*name;
 
  if (**name == '\\')
  {
   skip_player = TRUE;
   if (do_skip_system)
     skip_system = TRUE;
   ++*name;
  }
 }
 else
   save->player_current = p->aliases_start;
 
 while ((count < ALIAS_LIB_LD_SO_SZ) && *scan)
 {
  if ((*scan)->must_use)
  {
   if (!skip_system)
     if (!(*scan)->priv_type || priv_test_int(p->saved, (*scan)->priv_type))
       lib[count++] = (*scan)->head;
  }
  else if (!skip_player)
    if (!(*scan)->priv_type || priv_test_int(p->saved, (*scan)->priv_type))
      lib[count++] = (*scan)->head;
  
  ++scan;
 }
 save->num = count;
 assert(count <= ALIAS_LIB_LD_SO_SZ);
}

static void internal_alias_lib_subscription_del(player *p, unsigned int count)
{
 assert(p->alias_lib_saved_list[count]);

 FREE(p->alias_lib_saved_list[count]);
 
 while ((count < (ALIAS_LIB_LD_SO_SZ - 1)) &&
        p->alias_lib_saved_list[count + 1])
 {
  p->alias_lib_saved_list[count] = p->alias_lib_saved_list[count + 1];
  ++count;
 }

 p->alias_lib_saved_list[count] = NULL;
}

void alias_lib_ldconfig(player *p)
{
 char **scan = p->alias_lib_saved_list;
 alias_lib_node **lib = p->alias_lib_ldconfig_list;
 alias_lib_node *sys_lib = alias_lib_start;
 unsigned int count = 0;
 
 p->alias_lib_ld_cache_timestamp = now;

 while (((scan - p->alias_lib_saved_list) < ALIAS_LIB_LD_SO_SZ) && *scan)
 {
  alias_lib_node **tmp = alias_lib_find(*scan);
  
  if (tmp && !(*tmp)->must_use &&
      (((*tmp)->player_lib && p->saved->priv_lib_maintainer &&
        !strcmp(p->saved->lower_name, (*tmp)->name)) ||
       !(*tmp)->priv_type || priv_test_int(p->saved, (*tmp)->priv_type)))
  {
   lib[count++] = *tmp;
   
   ++scan;
  }
  else
  {
   fvtell_player(SYSTEM_T(p), " Auto deleting your subscription to alias "
                 "library -- ^S^B%s^s -- as you don't have the privs "
                 "to access it.\n", (*tmp)->name);
   internal_alias_lib_subscription_del(p, scan - p->alias_lib_saved_list);
  }
 }

 if (count > (unsigned int)(ALIAS_LIB_LD_SO_SZ - alias_lib_num_must_use))
   count = (ALIAS_LIB_LD_SO_SZ - alias_lib_num_must_use);
 
 while (sys_lib)
 {
  if (sys_lib->must_use)
    lib[count++] = sys_lib;
  
  sys_lib = sys_lib->next;
 }
 assert(count <= ALIAS_LIB_LD_SO_SZ);
}

static alias_node *internal_alias_search_next(alias_search_node *save)
{
 alias_node **sys_first = save->sys_list;
 alias_node **sys_last = save->sys_list;
 alias_node *ret = NULL;
 int sav = 0;
 unsigned int count = 0;

 save->is_system_alias = FALSE;
 
 while (save->player_current && save->player_current->flag_disabled)
   save->player_current = save->player_current->next;

 while (count < save->num)
 {
  while (sys_first[0] && sys_first[0]->flag_disabled)
    *sys_first = sys_first[0]->next;
  
  if (sys_first[0])
    break;
  ++sys_first;
  ++count;
 }
 
 if (count >= save->num)
 {
  assert(count == save->num);
  if ((ret = save->player_current))
    save->player_current = save->player_current->next;

  return (ret);
 }

 sys_last = sys_first;
 while (++count < save->num)
 {
  ++sys_last;
  
  while (sys_last[0] && sys_last[0]->flag_disabled)
    *sys_last = sys_last[0]->next;
  
  if (sys_last[0])  
    if (strcmp(sys_first[0]->command, sys_last[0]->command) > 0)
      sys_first = sys_last;
 }
 
 assert(sys_first[0]);
 if (!save->player_current ||
     ((sav = strcmp(sys_first[0]->command,
                    save->player_current->command)) < 0))
 {
  ret = sys_first[0];
  
  *sys_first = sys_first[0]->next;
  save->is_system_alias = TRUE;
 }
 else
 {
  if (!sav)
    *sys_first = sys_first[0]->next;
  
  ret = save->player_current;
  save->player_current = save->player_current->next;
 }

 count = 0;
 sys_first = save->sys_list;
 while (++count < save->num)
 {
  if (sys_first[0])
    if (!strcmp(sys_first[0]->command, ret->command))
      *sys_first = sys_first[0]->next;
  
  ++count;
  ++sys_first;
 }
 
 return (ret);
}

alias_node *alias_search_next_name(alias_search_node *save,
                                   const char *name, int *len)
{
 alias_node *curr = NULL;
 
 while ((curr = internal_alias_search_next(save)))
 {
  const char *tmp = name;
  int s_cmp = 1;
  
  if (!name)
    s_cmp = 0; /* search through alphabeticaly */
  else
    if (curr->flag_clever_match)
      s_cmp = match_clever(curr->command, &tmp, MATCH_CLEVER_FLAG_EXPAND);
    else
    {
     if ((s_cmp = beg_strcasecmp(curr->command, name)) > 0)
       s_cmp = -1;
    }
  
  if (s_cmp > 0)
    return (NULL);
  else if (!s_cmp)
  {
   if (!name)
   {
    assert(!len);
    break;
   }
   
   if (curr->flag_clever_match)
     *len = (tmp - name);
   else
     *len = strlen(curr->command);
   
   if (!(name[*len] && ((name[*len] != ' ') && !curr->flag_no_beg_space)))
     break;
  }
 }
 
 return (curr);
}

alias_node *alias_search_next_command(alias_search_node *save,
                                      const char *name, int *len,
                                      int is_main_command)
{
 alias_node *curr = NULL;
 
 while ((curr = alias_search_next_name(save, name, len)))
   if (is_main_command ?
       !curr->flag_in_sub_command_only :
       (curr->flag_in_sub_command_only || curr->flag_in_sub_command))
     return (curr);
 
 return (NULL);
}

alias_node *alias_search_next_type(alias_search_node *save, unsigned int type)
{
 alias_node *curr = NULL;
 
 while ((curr = alias_search_next_name(save, NULL, NULL)))
   if (curr->command_type == type)
     return (curr);

 return (NULL);
}

void alias_substitute_command(player *p, const char *str, alias_node *alias)
{
 assert(alias);

 str += strspn(str, " ");
  
 internal_substitute_alias_command(p, alias, str);
}

/* start of user commands... */

#define ALIAS_FLAG_SZ ((sizeof(alias_flags) / sizeof(alias_flags[0])) - 1)

static int alias_flags_get_offset(const char *str)
{
 int count = 0;

 while (alias_flags[count].name)
 {
  int save_cmp = 0;
  
  assert(alias_flags[count].func);
  if (!(save_cmp = beg_strcmp(str, alias_flags[count].name)))
    return (count);
  else if (save_cmp < 0)
    return (-1);

  ++count;
 }

 return (-1);
}

static void alias_flag_change_pre(player *p, parameter_holder *params,
                                  alias_flag_offset *flag_offsets,
                                  size_t *flag_offset_count)
{
 char *flags = NULL;

 if ((params->last_param != 2) || !*GET_PARAMETER_STR(params, 1))
   TELL_FORMAT(p, "<flag(s)> [on|off|toggle]");

 assert(*GET_PARAMETER_STR(params, 2));
 lower_case(GET_PARAMETER_STR(params, 1));

 flags = GET_PARAMETER_STR(params, 1);
 while (flags)
 {
  int flag_offset = -1;
  char *end_comma = next_parameter(flags, ',');
  char *data = NULL;
  
  if (end_comma)
    *end_comma++ = 0;

  if ((data = strchr(flags, '=')))
  {
   char *tmp = data;

   *data++ = 0;
   --tmp;
   while (*tmp == ' ')
     *tmp-- = 0;
  }
  
  if ((flag_offset = alias_flags_get_offset(flags)) == -1)
  {
   *flag_offset_count = 0;
   fvtell_player(NORMAL_T(p),
                 " Bad alias flag -- ^S^B%s^s -- no changes made.\n", flags);
   return;
  }

  flag_offsets[*flag_offset_count].data = data;
  flag_offsets[*flag_offset_count].type = flag_offset;
  ++*flag_offset_count;

  if (*flag_offset_count >= ALIAS_FLAG_SZ)
  {
   *flag_offset_count = 0;
   fvtell_player(SYSTEM_T(p), "%s",
                 " Too many alias flags, no changes made.\n");
   return;
  }
  
  flags = end_comma;
 }
}

static void internal_alias_set_flags(player *p, parameter_holder *params,
                                     alias_node *head)
{
 alias_flag_offset flag_offsets[ALIAS_FLAG_SZ];
 size_t flag_offset_count = 0;
 alias_node *tmp = NULL; 
 const char *cun_toggle = "toggle";
 const char *cun_on = "on";
 const char *cun_off = "off";
 
 switch (params->last_param)
 {
  case 2:
  {
   switch (*GET_PARAMETER_STR(params, 2))
   {
    case '=':
    case '+':
      ++GET_PARAMETER_STR(params, 2);
      --GET_PARAMETER_LENGTH(params, 2);
      get_parameter_parse(params, &cun_on, 3);
      break;
    case '-':
      ++GET_PARAMETER_STR(params, 2);
      --GET_PARAMETER_LENGTH(params, 2);
      get_parameter_parse(params, &cun_off, 3);
      break;
    case '~':
      ++GET_PARAMETER_STR(params, 2);
      --GET_PARAMETER_LENGTH(params, 2);
      
    default:
      get_parameter_parse(params, &cun_toggle, 3);
   }
   lower_case(GET_PARAMETER_STR(params, 2));
  }
  /* FALLTHROUGH */
  case 3:
    /* FALLTHROUGH */
  case 1:
   break;

  default:
    TELL_FORMAT(p, "<alias-name> [alias-flags] [on|off|toggle]");
 }

 if (!(tmp = alias_find(head, GET_PARAMETER_STR(params, 1))))
 {
  fvtell_player(NORMAL_T(p), " Alias command ^S^B%s^s doesn't exist.\n",
                GET_PARAMETER_STR(params, 1));
  return;
 }
 
 if (params->last_param != 1)
 {
  int ret = 1;
  size_t count = 0;
  
  get_parameter_shift(params, 1);
  alias_flag_change_pre(p, params, flag_offsets, &flag_offset_count);
  if (!flag_offset_count)
    return;
  
  while ((ret != -1) && (count < flag_offset_count))
  {
   int (*func)(player *, alias_node *, const char *);
   char *data = flag_offsets[count].data;
   
   func = alias_flags[flag_offsets[count++].type].func;
   if (!data)
     data = GET_PARAMETER_STR(params, 2);
   
   ret = (*func)(p, tmp, data);
  }
  if (ret == -1)
    return;
  assert(count == flag_offset_count);
 }
 else
 {
  char buffer[sizeof("Alias command ^S^B%s^s") + ALIAS_COMMAND_SZ];
  
  sprintf(buffer, "Alias command ^S^B%s^s", tmp->command);
  ptell_mid(NORMAL_T(p), buffer, FALSE);

  fvtell_player(NORMAL_WFT(RAW_OUTPUT | 7, p), " Code: %s\n", tmp->str);
  
  if (tmp->flag_public)
    fvtell_player(NORMAL_T(p), "%s", " Alias is publicly viewable.\n");
  if (tmp->flag_clever_match)
    fvtell_player(NORMAL_T(p), "%s", " Alias has clever expantion enabled.\n");
  if (tmp->flag_disabled)
    fvtell_player(NORMAL_T(p), "%s", " Command disabled.\n");
  if (((p->aliases_start == head) || PRIV_STAFF(p->saved)) && tmp->flag_hidden)
    fvtell_player(NORMAL_T(p), "%s", " Alias is hidden from normal view.\n");
  if (tmp->flag_no_beg_space)
    fvtell_player(NORMAL_T(p), "%s", " No space needed after command.\n");
  if (tmp->flag_use_name)
    fvtell_player(NORMAL_T(p), "%s",
                  " The alias name will set the command name.\n");
  if (tmp->flag_in_sub_command || tmp->flag_in_sub_command_only)
    fvtell_player(NORMAL_T(p), "   Alias works %son sub commands.\n",
                  tmp->flag_in_sub_command_only ? "^S^BONLY^s " : "");
  
  if (tmp->command_type)
    fvtell_player(NORMAL_T(p),
                  " The alias command type is set to ^S^B%s^s.\n",
                  cmds_map_sections[tmp->command_type]);
  if (tmp->comment)
    fvtell_player(NORMAL_T(p), " The alias command comment is "
                  "set to ^S^B%s^s.\n", tmp->comment);
  fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 }
}

static void user_alias_set_flags(player *p, parameter_holder *params)
{
 internal_alias_set_flags(p, params, p->aliases_start);
}

static void user_alias_lib_set_flags(player *p, parameter_holder *params)
{
 alias_lib_node *lib = NULL;

 if (!params->last_param)
   TELL_FORMAT(p, "<lib> <alias> [flags] [on|off|toggle]");

 if (!(lib = alias_lib_user_find(p, GET_PARAMETER_STR(params, 1),
                                 params->last_param != 2)))
   return;
 get_parameter_shift(params, 1);
 
 internal_alias_set_flags(p, params, lib->head);
 alias_lib_save(lib);
}

static void internal_alias_show_all(player *p, const char *str,
                                    player *p2,
                                    alias_node *scan,
                                    int number_of_aliases,
                                    int max_aliases,
                                    int show_non_public,
                                    int lib_msgs)
{
 int do_all = FALSE;
 tmp_output_list_storage tmp_save;
 output_node *catchment = NULL;
 char buffer[sizeof("Used %d of %d aliases") + (BUF_NUM_TYPE_SZ(int) * 2)];
 
 if (!scan)
 {
  if (lib_msgs)
    fvtell_player(NORMAL_T(p), " No aliases currently defined.\n");
  else
    fvtell_player(NORMAL_T(p), " The player -- ^S^B%s^s -- doesn't have any "
                  "public aliases.\n", p2->saved->name);
  return;
 }

 if (PRIV_STAFF(p->saved) && !BEG_CONST_STRCMP("all", str))
 {
  str += CONST_STRLEN("all");
  str += strspn(str, " ");
  do_all = TRUE;
 }

 save_tmp_output_list(p, &tmp_save);

 if (p->aliases_start == scan)
   sprintf(buffer, "Used %d of %d aliases", number_of_aliases, max_aliases);
 else
   sprintf(buffer, "Used %d aliases", number_of_aliases);
 
#if ALIAS_COMMAND_SZ + 3 >= WRAPPING_SPACES
# error "Wee for hilights and stuff ~Nevyn ."
#endif
 
 if (*str && (scan = alias_find(scan, str)))
 {
  fvtell_player(ALL_T(p->saved, p, NULL,
                      OUTPUT_BUFFER_TMP | RAW_OUTPUT, now),
                "%-*s %s %s\n",
                ALIAS_COMMAND_SZ, scan->command,
                scan->flag_disabled ? "." :
                (scan->flag_public ? "*" : "-"),
                scan->str);  
 }
 else
 {
  while (scan)
  {
   if ((!scan->flag_hidden || do_all) &&
       (scan->flag_public || show_non_public))
     fvtell_player(ALL_T(p->saved, p, NULL,
                         OUTPUT_BUFFER_TMP | RAW_OUTPUT, now),
                   "%-*s %s %s\n",
                   ALIAS_COMMAND_SZ, scan->command,
                   scan->flag_disabled ? "." :
                   (scan->flag_public ? "*" : "-"),
                   scan->str);
   
   scan = scan->next;
  }
 }

 catchment = output_list_grab(p);
 load_tmp_output_list(p, &tmp_save);

 if (!catchment)
 {
  if (*str)
  {
   fvtell_player(NORMAL_T(p), " The alias -- ^S^B%s^s -- doesn't exist.\n",
                 str);
  }
  else
    if (show_non_public)
    {
     assert(!do_all);
     fvtell_player(NORMAL_T(p), " No unhidden aliases currently defined.\n");
    }
    else if (lib_msgs)
      fvtell_player(NORMAL_T(p), " No aliases currently defined.\n");
    else
      fvtell_player(NORMAL_T(p), " Sorry that player doesn't have any "
                    "public aliases.\n");
  return;
 }

 ptell_mid(NORMAL_T(p), buffer, FALSE);

 if (1 || p->alias_list_code) /* FIXME : */
   fvtell_player(NORMAL_T(p), "%-*s %s Command(s) to be executed\n\n",
                 ALIAS_COMMAND_SZ, " - Alias -", "-");
 else
   fvtell_player(NORMAL_T(p), "%-*s %s Comment for alias\n\n",
                 ALIAS_COMMAND_SZ, " - Alias -", "-");
  
 output_list_linkin(p, (ALIAS_COMMAND_SZ + 3), &catchment, INT_MAX);

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);

 pager(p, PAGER_DEFAULT);
}

static void user_alias_show_all(player *p, parameter_holder *params)
{
 player *p2 = p;
 int show_non_public = FALSE; 

 switch (params->last_param)
 {
  case 1:
  case 2:
    p2 = player_find_load(p, GET_PARAMETER_STR(params, 1),
                          PLAYER_FIND_SC_EXTERN_ALL);
    
    if (p2)
    {
     if (PRIV_STAFF(p->saved) || (p == p2))
       show_non_public = TRUE;
     get_parameter_shift(params, 1);
    }
    else
    {
     p2 = p;
     show_non_public = TRUE;
    }
    break;

  case 0:
    show_non_public = TRUE;
    break;

  default:
    TELL_FORMAT(p, "[player] [alias_name]");
 }

 if (params->last_param)
   internal_alias_show_all(p, GET_PARAMETER_STR(params, 1), p2,
                           p2->aliases_start, p2->number_of_aliases,
                           p2->max_aliases, show_non_public, FALSE);
 else
   internal_alias_show_all(p, "", p2,
                           p2->aliases_start, p2->number_of_aliases,
                           p2->max_aliases, show_non_public, FALSE);
}

static void user_alias_lib_show_all(player *p, parameter_holder *params)
{
 const char *tmp = "";
 alias_node *sys_list = NULL;
 int sys_num = 0;
 
 switch (params->last_param)
 {
  case 2:
    tmp = GET_PARAMETER_STR(params, 2);
  case 1:
  {
   alias_lib_node *lib = alias_lib_user_find(p, GET_PARAMETER_STR(params, 1),
                                             FALSE);
   
   if (!lib)
     return;
   
   get_parameter_shift(params, 1);
   sys_list = lib->head;
   sys_num = lib->num;
  }
  break;
  
  default:
    TELL_FORMAT(p, "<lib> [alias_name]");
 }

 internal_alias_show_all(p, tmp, p, sys_list, sys_num,
                         ALIAS_MAX_SYSTEM_ALIASES, TRUE, TRUE);
}

static void internal_user_alias_add(player *p, parameter_holder *params,
                                    const char *str, alias_node **head,
                                    int *number_of_aliases,
                                    int max_aliases)
{
 if (*GET_PARAMETER_STR(params, 1) == '\\')
 {
  fvtell_player(NORMAL_T(p), "%s",
                " Aliases cannot start with a '^S^B\\^s'.\n");
  return;
 }

 if ((*number_of_aliases + 1) > max_aliases)
 {
  fvtell_player(NORMAL_T(p), 
                " You have reached your alias limit of"
                " ^S^B%d^s.\n",
                max_aliases);
  return;
 }
 
 if (GET_PARAMETER_LENGTH(params, 1) >= ALIAS_COMMAND_SZ)
 {
  int longer_tmp = GET_PARAMETER_LENGTH(params, 1) - (ALIAS_COMMAND_SZ - 1);
  fvtell_player(SYSTEM_T(p), " Alias command is %d character%s longer than "
                "the maximum length allowed.\n",
                longer_tmp, (longer_tmp > 1 ? "s" : ""));
  return;
 }

 if (strchr(GET_PARAMETER_STR(params, 1), ' '))
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " Alias commands cannot have a space in them.\n");
  return;
 }
 
 if (alias_new_insert(head, GET_PARAMETER_STR(params, 1), str))
 {
  fvtell_player(NORMAL_T(p), " Alias command ^S^B%s^s added.\n",
                GET_PARAMETER_STR(params, 1));
  ++*number_of_aliases;
 }
 else
   fvtell_player(NORMAL_T(p),
                 " Alias command -- ^S^B%s^s -- already exists.\n",
                 GET_PARAMETER_STR(params, 1));
}

static void user_alias_add(player *p, const char *str)
{
 parameter_holder params;
 
 get_parameter_init(&params);

 if (!*str)
 {
  user_alias_show_all(p, &params);
  return;
 }
 
 if (!get_parameter_parse(&params, &str, 1) ||
     !GET_PARAMETER_LENGTH(&params, 1) || !*str)
 {
  if (alias_find(p->aliases_start, str))
  {    
    user_alias_show_all(p, &params);
    return;
  }
  else
    TELL_FORMAT(p, "<alias-name> <alias-string>");
 }

 internal_user_alias_add(p, &params, str, &p->aliases_start,
                         &p->number_of_aliases, p->max_aliases);
}

static void user_alias_lib_add(player *p, const char *str)
{
 alias_lib_node *lib = NULL;
 parameter_holder params;

 get_parameter_init(&params);

 if (!get_parameter_parse(&params, &str, 1))
   TELL_FORMAT(p, "<lib> <alias> <command(s)>");
 
 if (!(lib = alias_lib_user_find(p, GET_PARAMETER_STR(&params, 1), TRUE)))
   return;

 get_parameter_shift(&params, 1);
 
 if (!*str)
 {
  user_alias_lib_show_all(p, &params);
  return;
 }

 if (!get_parameter_parse(&params, &str, 1) ||
     !GET_PARAMETER_LENGTH(&params, 1) || !*str)
   TELL_FORMAT(p, "<lib> <alias> <command(s)>");
 
 internal_user_alias_add(p, &params, str, &lib->head, &lib->num,
                         ALIAS_MAX_SYSTEM_ALIASES);

 alias_lib_save(lib);
}

static void user_alias_copy(player *p, parameter_holder *params)
{
 player *alias_from = NULL;
 alias_node *alias_copy = NULL;
 alias_node *new_alias = NULL;
 
 if (params->last_param != 2)
   TELL_FORMAT(p, "<lib=name | player> <alias-name>"); 

 if ((p->number_of_aliases + 1) > p->max_aliases)
 {
  fvtell_player(NORMAL_T(p), 
                " You have reached your alias limit of"
                " ^S^B%d^s.\n",
                p->max_aliases);
  return;
 } 

 if (BEG_CONST_STRCMP("lib=", GET_PARAMETER_STR(params, 1)) &&
     !(alias_from = player_find_load(p, GET_PARAMETER_STR(params, 1),
                                     PLAYER_FIND_SC_EXTERN)))
   return;
 
 if (alias_from)
 {
  if (!(alias_copy = alias_find(alias_from->aliases_start,
                                GET_PARAMETER_STR(params, 2))) ||
      !alias_copy->flag_public)
  {
   fvtell_player(SYSTEM_T(p), " Sorry person ^B%s^b doesn't have a public "
                 "alias called ^B%s^b.\n",
                 alias_from->saved->name, GET_PARAMETER_STR(params, 2));
   return;
  }
 }
 else
 {
  alias_lib_node *lib = alias_lib_user_find(p, GET_PARAMETER_STR(params, 1) +
                                             CONST_STRLEN("lib="), FALSE);
  
  if (!lib)
    return;

  if (!(alias_copy = alias_find(lib->head, GET_PARAMETER_STR(params, 2))))
  {
   fvtell_player(SYSTEM_T(p), " The alias library -- ^S^B%s^s -- doesn't "
                 "have an alias called -- ^S^B%s^s.\n",
                 GET_PARAMETER_STR(params, 1) + CONST_STRLEN("lib="),
                 GET_PARAMETER_STR(params, 2));
   return;
  }
 }
 
 if (!(new_alias = alias_new_insert(&p->aliases_start,
                                    alias_copy->command, alias_copy->str)))
 {
  fvtell_player(NORMAL_T(p), " Alias command -- ^S^B%s^s -- already exists.\n",
                alias_copy->command);
  return;
 }
 
 fvtell_player(NORMAL_T(p), " Alias command ^S^B%s^s added.\n",
               alias_copy->command);
 
 new_alias->flag_no_beg_space = alias_copy->flag_no_beg_space;
 new_alias->flag_disabled = alias_copy->flag_disabled;
 new_alias->flag_clever_match = alias_copy->flag_clever_match;
 new_alias->flag_expand_match = alias_copy->flag_expand_match;
 new_alias->flag_hidden = alias_copy->flag_hidden;
 new_alias->flag_use_name = alias_copy->flag_use_name;
 new_alias->flag_in_sub_command = alias_copy->flag_in_sub_command;
 new_alias->flag_in_sub_command_only = alias_copy->flag_in_sub_command_only;
 new_alias->flag_no_beg_space = alias_copy->flag_public;
 
 assert(!alias_copy->flag_prived);
 new_alias->flag_prived = FALSE;
 ++p->number_of_aliases;
}

static void internal_alias_del(player *p, parameter_holder *params,
                               alias_node **head, int *number_of_aliases)
{ 
 if (params->last_param != 1)
   TELL_FORMAT(p, "<alias-name>");

 if (*GET_PARAMETER_STR(params, 1) == '\\')
 {
  fvtell_player(NORMAL_T(p), "%s",
                " Aliases cannot start with a '^S^B\\^s'.\n");
  return;
 }
 
 if (alias_destroy(head, GET_PARAMETER_STR(params, 1)))
 {
  --*number_of_aliases;
  fvtell_player(NORMAL_T(p), " Alias command ^S^B%s^s removed.\n",
                GET_PARAMETER_STR(params, 1));
 }
 else
   fvtell_player(NORMAL_T(p), " Alias command -- ^S^B%s^s -- doesn't exist.\n",
                 GET_PARAMETER_STR(params, 1));
}

static void user_alias_del(player *p, parameter_holder *params)
{
 internal_alias_del(p, params, &p->aliases_start, &p->number_of_aliases);
}

static void user_alias_lib_del(player *p, parameter_holder *params)
{
 alias_lib_node *lib = NULL;
 
 if (params->last_param != 2)
   TELL_FORMAT(p, "<lib> <alias>");

 if (!(lib = alias_lib_user_find(p, GET_PARAMETER_STR(params, 1), TRUE)))
   return;
 get_parameter_shift(params, 1);
 
 internal_alias_del(p, params, &lib->head, &lib->num);
 alias_lib_save(lib);
}

static void internal_alias_rename(player *p, parameter_holder *params,
                                  alias_node **head)
{
 alias_node *tmp = NULL;
 
 if (params->last_param != 2)
   TELL_FORMAT(p, "<old-alias> <new-alias>");
 
 if (!(tmp = alias_remove(head, GET_PARAMETER_STR(params, 1))))
 {
  fvtell_player(SYSTEM_T(p), " There is no alias called -- ^S^B%s^s.\n",
                GET_PARAMETER_STR(params, 1));
  return;
 }

 sprintf(tmp->command, "%.*s", ALIAS_COMMAND_SZ - 1,
         GET_PARAMETER_STR(params, 2));

 if (alias_insert(head, tmp))
 {
  fvtell_player(NORMAL_T(p),
                " Changed alias name from %s to: ^S^B%s^s\n",
                GET_PARAMETER_STR(params, 1), GET_PARAMETER_STR(params, 2));
 }
 else
 {
  int q_debug = 0;
  
  sprintf(tmp->command, "%.*s", ALIAS_COMMAND_SZ - 1,
          GET_PARAMETER_STR(params, 1));
  q_debug = alias_insert(head, tmp);
  assert(q_debug);
  fvtell_player(SYSTEM_T(p), " Cannot rename alias to ^S^B%s^s as an alias "
                "already exists with that name.\n",
                GET_PARAMETER_STR(params, 2));
 }
}

static void user_alias_rename(player *p, parameter_holder *params)
{
 internal_alias_rename(p, params, &p->aliases_start); 
}

static void user_alias_lib_rename(player *p, parameter_holder *params)
{
 alias_lib_node *lib = NULL;
 
 if (!params->last_param)
   TELL_FORMAT(p, "<lib> <old-alias> <new-alias>");
 
 if (!(lib = alias_lib_user_find(p, GET_PARAMETER_STR(params, 1), TRUE)))
   return;
 get_parameter_shift(params, 1);

 internal_alias_rename(p, params, &lib->head);
 alias_lib_save(lib);
}

static void user_su_alias_lib_add(player *p, parameter_holder *params)
{
 alias_lib_node **tmp = NULL;
 alias_lib_node *curr = NULL;
 
 if (params->last_param != 5)
   TELL_FORMAT(p, "<library> <priv_type> <needs_help> <player_lib> <must_use>");

 if (GET_PARAMETER_LENGTH(params, 1) >= ALIAS_LIB_NAME_SZ)
 {
  fvtell_player(NORMAL_T(p), " Alias library names can only be upto ^S^B%d^s "
                "characters in length.\n", ALIAS_LIB_NAME_SZ - 1);
  return;
 }
 
 if ((tmp = alias_lib_find(GET_PARAMETER_STR(params, 1))))
 {
  curr = *tmp;
  fvtell_player(NORMAL_T(p), " There is already an alias library "
                "called -- ^S^B%s^s -- altering the attributes.\n",
                GET_PARAMETER_STR(params, 1));
  if (curr->must_use)
    --alias_lib_num_must_use;
 }
 else
 {
  if (!(curr = alias_lib_create(NULL)))
  {
   P_MEM_ERR(p);
   return;
  }

  memcpy(curr->name, GET_PARAMETER_STR(params, 1),
         GET_PARAMETER_LENGTH(params, 1));
  curr->name[GET_PARAMETER_LENGTH(params, 1)] = 0;
 }
 
 curr->priv_type = priv_test_parse_int(GET_PARAMETER_STR(params, 2));

 curr->needs_help = TOGGLE_MATCH_ON(GET_PARAMETER_STR(params, 3));
 curr->player_lib = TOGGLE_MATCH_ON(GET_PARAMETER_STR(params, 4));
 if ((curr->must_use = TOGGLE_MATCH_ON(GET_PARAMETER_STR(params, 5))))
 {
  ++alias_lib_num_must_use;
  fvtell_player(NORMAL_T(p), " Now have -- ^S^B%d^s -- must use libraries and "
                "players can only have -- ^S^B%d^s -- libraries.\n",
                alias_lib_num_must_use, ALIAS_LIB_LD_SO_SZ);
 }

 if (curr->player_lib && curr->must_use)
   fvtell_player(NORMAL_T(p), " The alias library -- ^S^B%s^s -- is a player "
                 "library, ^S^B_but_^s it has must use on (this is bad).\n",
                 GET_PARAMETER_STR(params, 1));

 if (curr->player_lib)
 {
  player_tree_node *sp = NULL;
  
  if (!(sp = player_find_all(p, curr->name, PLAYER_FIND_SC_INTERN)))
  {
   fvtell_player(NORMAL_T(p), " The alias library -- ^S^B%s^s -- is a player "
                 "library, ^S^B_but_^s there is no player by that name.\n",
                 GET_PARAMETER_STR(params, 1));
  }
  else
  {
   if (curr->must_use)
   {
    fvtell_player(NORMAL_T(p), " The alias library -- ^S^B%s^s -- is a player "
                  "library, ^S^B_but_^s it has must use on (this is bad).\n",
                  GET_PARAMETER_STR(params, 1));
   }
   
   if (curr->needs_help)
   {
    fvtell_player(NORMAL_T(p), " The alias library -- ^S^B%s^s -- is a player "
                  "library, ^S^B_but_^s it has needs help on.\n",
                  GET_PARAMETER_STR(params, 1));
   }
   
   if (curr->priv_type && !priv_test_int(sp, curr->priv_type))
   {
    fvtell_player(NORMAL_T(p), " The alias library -- ^S^B%s^s -- is a player "
                  "library, ^S^B_but_^s that player doesn't have the "
                  "privs required to use it.\n",
                  GET_PARAMETER_STR(params, 1));
   }
  }
 }
 
 fvtell_player(NORMAL_T(p), " The alias library ^S^B%s^s now exists.\n",
               GET_PARAMETER_STR(params, 1));
 
 alias_lib_index_save();
 alias_lib_save(curr);
}

static void user_su_alias_lib_del(player *p, const char *str)
{
 if (!*str)
   TELL_FORMAT(p, "<library>");

 if (!alias_lib_destroy(str))
   fvtell_player(SYSTEM_T(p), " Alias library -- ^S^B%s^s -- doesn't "
                 "exist.\n", str);
 else
 {
  fvtell_player(NORMAL_T(p), " Alias library ^S^B%s^s deleted.\n", str);
  alias_lib_index_save();
 }
}

static void user_su_alias_lib_info(player *p, const char *str)
{
 alias_lib_node *lib = NULL;
 char buffer[sizeof("Alias library %s") + ALIAS_LIB_NAME_SZ];
 
 if (!*str)
   TELL_FORMAT(p, "<lib>");

 if (!(lib = alias_lib_user_find(p, str, TRUE)))
   return;

 sprintf(buffer, "Alias library %s", lib->name);
 ptell_mid(NORMAL_T(p), buffer, FALSE);
 
 fvtell_player(NORMAL_T(p), "Privs needed: ^S^B%s^s\n",
               priv_test_names[lib->priv_type]);
 fvtell_player(NORMAL_T(p), "Needs help file: %s\n",
               TOGGLE_YES_NO(lib->needs_help));
 fvtell_player(NORMAL_T(p), "Player lib: %s\n",
               TOGGLE_YES_NO(lib->player_lib));
 fvtell_player(NORMAL_T(p), "Must use: %s\n",
               TOGGLE_YES_NO(lib->must_use));
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void user_alias_lib_subscribe(player *p, parameter_holder *params)
{
 alias_lib_node *tmp = NULL;
 unsigned int count = 0;
 
 switch (params->last_param)
 {
  default:
    TELL_FORMAT(p, "[library]");
    
  case 0:
  {
   alias_lib_node *scan = alias_lib_start;
   int found_lib = FALSE;
   
   while (!found_lib)
   {
    if (!scan)
    {
     fvtell_player(NORMAL_T(p), " There are no alias libraries "
                   "available at present.\n");
     return;
    }
    else if (!scan->priv_type || p->saved->priv_coder ||
             priv_test_int(p->saved, scan->priv_type))
      found_lib = TRUE;
    
    scan = scan->next;
   }
   
   scan = alias_lib_start;
   ptell_mid(NORMAL_T(p), "Global alias libraries", FALSE);
   while (scan)
   {
    if (!scan->priv_type || p->saved->priv_coder ||
        priv_test_int(p->saved, scan->priv_type))
    {
     ++count;
     fvtell_player(NORMAL_T(p), "%s % 2d. %s (%d)\n",
                   scan->must_use ? "*" : " ", count, scan->name, scan->num);
    }
    scan = scan->next;
   }
   fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
  }
  return;
  
  case 1:
    break;
 }

 lower_case(GET_PARAMETER_STR(params, 1));
 if (!(tmp = alias_lib_user_find(p, GET_PARAMETER_STR(params, 1), FALSE)))
   return;
 assert(GET_PARAMETER_LENGTH(params, 1) < ALIAS_LIB_NAME_SZ);

 assert(!p->alias_lib_saved_list[ALIAS_LIB_LD_SO_SZ - 1]);

 if (tmp->must_use)
 {
  fvtell_player(SYSTEM_T(p), " The alias library -- ^S^B%s^s -- is a must use "
                "library and so can't be subscribed to.\n", tmp->name);
  return;
 }
 
 if (p->alias_lib_saved_list[ALIAS_LIB_LD_SO_SZ - 1])
 {
  fvtell_player(SYSTEM_T(p), " You are already subscribed to "
                "as many libraries as you are allowed to be.\n");
  return;
 }

 while (p->alias_lib_saved_list[count])
 {
  if (!strcmp(p->alias_lib_saved_list[count], tmp->name))
  {
   fvtell_player(SYSTEM_T(p), " You are already subscribed to "
                 "the alias library -- ^S^B%s^s --.\n", tmp->name);
   return;   
  }
  
  ++count;
 }
 
 assert(count < ALIAS_LIB_LD_SO_SZ);

 p->alias_lib_saved_list[count] = MALLOC(GET_PARAMETER_LENGTH(params, 1) + 1);
 if (!(p->alias_lib_saved_list[count]))
 {
  P_MEM_ERR(p);
  return;
 }

 COPY_STR_LEN(p->alias_lib_saved_list[count], tmp->name,
              GET_PARAMETER_LENGTH(params, 1));
 
 fvtell_player(NORMAL_T(p), " You are now subscribed to the alias "
               "library ^S^B%s^s.\n", GET_PARAMETER_STR(params, 1));
 alias_lib_ldconfig(p);
}

static void user_alias_lib_unsubscribe(player *p, parameter_holder *params)
{
 unsigned int count = 0;

 switch (params->last_param)
 {
  default:
    TELL_FORMAT(p, "<library>");
    
  case 0:
    if (!p->alias_lib_saved_list[0])
    {
     fvtell_player(NORMAL_T(p), " You have not subscribbed to "
                   "any alias libraries.\n");
     return;
    }

    ptell_mid(NORMAL_T(p), "Your alias libraries", FALSE);
    while ((count < ALIAS_LIB_LD_SO_SZ) &&
           p->alias_lib_saved_list[count])
    {
     fvtell_player(NORMAL_T(p), "% 2d. %s\n", count + 1,
                   p->alias_lib_saved_list[count]);
     ++count;
    }
    fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
    return;

  case 1:
    break;
 }

 lower_case(GET_PARAMETER_STR(params, 1));

 while ((count < ALIAS_LIB_LD_SO_SZ) &&
        p->alias_lib_saved_list[count])
 {
  if (!strcmp(p->alias_lib_saved_list[count], GET_PARAMETER_STR(params, 1)))
    break;
  ++count;
 }
 if ((count >= ALIAS_LIB_LD_SO_SZ) ||
     !p->alias_lib_saved_list[count])
 {
  assert((count == ALIAS_LIB_LD_SO_SZ) || !p->alias_lib_saved_list[count]);
  
  fvtell_player(SYSTEM_T(p), " You are not subscribed to an alias "
                "library called  -- ^S^B%s^s --.\n",
                GET_PARAMETER_STR(params, 1));
  return;
 }

 internal_alias_lib_subscription_del(p, count);
 
 fvtell_player(NORMAL_T(p), " You are now unsubscribed from the alias "
               "library ^S^B%s^s.\n", GET_PARAMETER_STR(params, 1));
 alias_lib_ldconfig(p);
}

static void user_alias_realias(player *p, const char *str)
{
 size_t len = 0;
 char *tmp = NULL;
 alias_node *alias_realias = NULL;
 parameter_holder params;
 
 get_parameter_init(&params);

 if (!get_parameter_parse(&params, &str, 1) || !*str)
   TELL_FORMAT(p, "<alias-name> <alias-string>");

 if (!(alias_realias = alias_find(p->aliases_start,
                                  GET_PARAMETER_STR(&params, 1))))
 {
  internal_user_alias_add(p, &params, str, &p->aliases_start,
                          &p->number_of_aliases, p->max_aliases);
  return;
 }
 
 len = strnlen(str, ALIAS_STR_SZ - 1);

 if (!(tmp = REALLOC(alias_realias->str, len + 1)))
 {
  P_MEM_ERR(p);
  return;
 }
 alias_realias->str = tmp;
 
 COPY_STR_LEN(alias_realias->str, str, len);

 fvtell_player(NORMAL_T(p), " Alias command ^S^B%s^s realiased.\n",
               GET_PARAMETER_STR(&params, 1));
}

static void user_keep_going_toggle(player *p, const char *str)
{ /* keep executing computer generated input */
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_input_keep_going, TRUE,
                       " Your commands will %skeep on executing.\n",
                       " Your commands will %snot keep "
                       "on executing.\n", TRUE);
}

static void user_parse_execute(player *p, const char *str)
{ /* we can always make it as the input for aliases is a commands_todo_buffer,
   * hence they can only use (MAX - 1) of the buffers */
 input_node *in_node = input_add(p, p->input_start);
 int length = 0;
 output_node *catchment = NULL;
 tmp_output_list_storage tmp_save;
 
 save_tmp_output_list(p, &tmp_save);
 fvtell_player(ALL_T(p->saved, p, NULL, OUTPUT_BUFFER_TMP, now),
               "%.*s", INPUT_BUFFER_SZ - 1, str);
 catchment = output_list_grab(p);
 load_tmp_output_list(p, &tmp_save);

 length = output_list_into_buffer(p, catchment, in_node->input,
                                  INPUT_BUFFER_SZ - 1);
 output_list_cleanup(&catchment);
 
 assert(length < INPUT_BUFFER_SZ);
 in_node->length = length;
 
 INPUT_TERMINATE(in_node);
 in_node->ready = TRUE;
 in_node->comp_generated = TRUE;
 
 assert(!C_strchr(in_node->input, '\n'));
}

void init_alias(void)
{
 alias_lib_load();
}

void cmds_init_alias(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("alias", user_alias_add, CONST_CHARS, MISC);
 CMDS_ADD("realias", user_alias_realias, CONST_CHARS, MISC);
 CMDS_ADD("alias_copy", user_alias_copy, PARSE_PARAMS, MISC);
 CMDS_ADD("alias_set_flags", user_alias_set_flags, PARSE_PARAMS, MISC);
 CMDS_ADD("alias_rename", user_alias_rename, PARSE_PARAMS, MISC);
 CMDS_ADD("alias_show", user_alias_show_all, PARSE_PARAMS, MISC);

 /* 2 namespaces ... Argghhh */
 CMDS_ADD("alias_library_add", user_su_alias_lib_add, PARSE_PARAMS, ADMIN);
 CMDS_PRIV(coder_admin);

 CMDS_ADD("alias_library_delete", user_su_alias_lib_del, CONST_CHARS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(coder_admin);

 CMDS_ADD("alias_info_library", user_su_alias_lib_info, CONST_CHARS, MISC);
 CMDS_PRIV(lib_maintainer);

 CMDS_ADD("alias_add_library", user_alias_lib_add, CONST_CHARS, MISC);
 CMDS_PRIV(lib_maintainer);
 CMDS_ADD("alias_rename_library", user_alias_lib_rename,
          PARSE_PARAMS, MISC);
 CMDS_PRIV(lib_maintainer);
 CMDS_ADD("alias_set_flags_library", user_alias_lib_set_flags,
          PARSE_PARAMS, MISC);
 CMDS_ADD("alias_show_library", user_alias_lib_show_all, PARSE_PARAMS, MISC);

 CMDS_ADD("alias_subscribe_library", user_alias_lib_subscribe,
          PARSE_PARAMS, MISC);
 CMDS_ADD("alias_unsubscribe_library", user_alias_lib_unsubscribe,
          PARSE_PARAMS, MISC);

 CMDS_ADD("unalias", user_alias_del, PARSE_PARAMS, MISC);
 CMDS_ADD("unalias_library", user_alias_lib_del, PARSE_PARAMS, MISC);
 CMDS_PRIV(lib_maintainer);

 CMDS_ADD("keep_executing", user_keep_going_toggle, CONST_CHARS, SPOD);
 CMDS_PRIV(spod);

 CMDS_ADD("parse_execute", user_parse_execute, CONST_CHARS, MISC);
 CMDS_FLAG(no_beg_space); CMDS_FLAG(no_end_space);
}
