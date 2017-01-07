#define CMDS_LIST_C
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

#define CMDS_INIT_DUMMY_NODE {CMDS_PARAM_CONST_CHARS, {NULL}}, 0, \
 /* privs */ NULL, \
 /* flags */ FALSE, FALSE, FALSE, FALSE, FALSE, FALSE

command_node cmds_dummy_input_to = {"input_to", CMDS_INIT_DUMMY_NODE};
command_node *cmds_last_ran = NULL;

 /* current text of command player used */
const char *current_command = NULL;
/* number of the command being executed, inc. sub commands
 * NOTE: also it MUST be above 0 to start with */
unsigned int current_command_number = 1;
/* current text of sub command used */
const char *current_sub_command = NULL;

command_base cmds_alpha[CMDS_SIZE_ALPHA] =
{
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(),
};
command_base cmds_sub[CMDS_SIZE_SUB] =
{
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
};
/* doesn't have a section name with it... eg. restricted and keyroom
 * although keyroom might be better of as a section ? */
command_base cmds_misc[CMDS_SIZE_MISC] =
{
 CMDS_INIT_BASE(),
};

command_base cmds_section[CMDS_SIZE_SECTION] =
{
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
 CMDS_INIT_BASE(), CMDS_INIT_BASE(), CMDS_INIT_BASE(),
};

const char *cmds_map_sections[CMDS_SIZE_SECTION] =
{
 "Hidden",
 "Social",
 "Communication",
 "Local",
 "Information",
 "Personal settings",
 "Personal/character information",
 "List",
 "System",
 "Miscellaneous",
 "Multi",
 "Games",
 "Spod level",
 "Minister level",
 "SU level",
 "Admin level",

 "",
 "",
 "",
 "",
 "",
 "",
 "",
 
 "Stats sub",
 "Room sub",
 "Check sub",
 "Editor sub",
 "Mail sub",
 "News sub",
 "NewsGroup sub",
 "Draughts/Checkers sub",
 "Intercom sub",
 "Change limit sub",
 "Channels",
 "Configure"
};


static command_base cmds_malloc_cmds = CMDS_INIT_BASE();




static void cmds_setup(void)
{
 /* need to load the size_max variables here, (save them in cmds_sort) */
}

command_node *cmds_find(command_node *base, size_t len, const char *name)
{
 size_t count = 0;

 while (count < len)
 {
  if (!strcmp(base->name, name))
    return (base);
  
  ++base;
  ++count;
 }

 return (NULL);
}

static void internal_cmds_grow_node_list(command_base *base)
{
 if (base->size < base->max_size)
   return;
 assert(base->size == base->max_size);
 
 base->max_size += CMDS_MALLOC_LIST_BLOCK_SZ;
 if (!(base->ptr = XREALLOC(base->ptr, base->max_size * sizeof(command_node *),
                            CMDS_NODE_LIST)))
   SHUTDOWN_MEM_ERR();
}

static void internal_cmds_shrink_node_list(command_base *base)
{
 assert(base->size <= base->max_size);
 base->max_size = base->size;
 if (!base->max_size)
 { /* realloc isn't well defined when you pass 0 as the size */
  if (base->ptr)
  {
   XFREE(base->ptr, CMDS_NODE_LIST);
   base->ptr = NULL;
  }
  return;
 }
 
 if (!(base->ptr = XREALLOC(base->ptr, base->max_size * sizeof(command_node *),
                            CMDS_NODE_LIST)))
   SHUTDOWN_MEM_ERR();
}

command_node *cmds_get_new_node(void)
{
 command_base *base = &cmds_malloc_cmds;

 internal_cmds_grow_node_list(base);

 if (!(base->ptr[base->size] = XMALLOC(sizeof(command_node), CMDS_NODE_OBJS)))
   SHUTDOWN_MEM_ERR();

 return (base->ptr[base->size++]);
}

void cmds_add_section(command_node *current,
                      command_base *base_array, int offset)
{
 command_base *base = base_array + offset;

 assert(offset >= 0);
 assert(offset < CMDS_SIZE_SECTION);

 internal_cmds_grow_node_list(base);
 
 base->ptr[base->size++] = current;
}

void cmds_add_alpha(command_node *current,
                    command_base *base_array, int offset)
{
 command_base *base = base_array + offset;

 assert(offset >= 0);
 assert(offset < CMDS_SIZE_ALPHA);

 internal_cmds_grow_node_list(base);
 
 base->ptr[base->size++] = current;
}

void cmds_add_sub(command_node *current, command_base *base_array, int offset)
{
 command_base *base = base_array + CMDS_SUB_OFFSET(offset);

 assert(CMDS_SUB_OFFSET(offset) >= 0);
 assert(CMDS_SUB_OFFSET(offset) < CMDS_SIZE_SUB);

 internal_cmds_grow_node_list(base);

 base->ptr[base->size++] = current;
}

void cmds_add_misc(command_node *current, command_base *base_array, int offset)
{
 command_base *base = base_array + offset;

 assert(offset >= 0);
 assert(offset < CMDS_SIZE_MISC);

 internal_cmds_grow_node_list(base);
 
 base->ptr[base->size++] = current;
}

static int internal_command_node_cmp(const void *real_one,
                                     const void *real_two)
{
 const command_node *const *const one = real_one;
 const command_node *const *const two = real_two;

#if !(CMDS_DEBUG)
 if (!strcmp((*one)->name, (*two)->name))
   vwlog("error", " Have a cmd called -- %s -- twice.\n", (*one)->name);
#endif
 
 return (strcmp((*one)->name, (*two)->name));
}

void cmds_sort_arrays(void)
{
 int tmp = 0;

 internal_cmds_shrink_node_list(&cmds_malloc_cmds);
 
 while (tmp < CMDS_SIZE_ALPHA)
 {
  internal_cmds_shrink_node_list(&cmds_alpha[tmp]);
  if (cmds_alpha[tmp].size)
    qsort(cmds_alpha[tmp].ptr, cmds_alpha[tmp].size, sizeof(command_node *),
          internal_command_node_cmp);
  ++tmp;
 }
 tmp = 0;
 
 while (tmp < CMDS_SIZE_SUB)
 {
  internal_cmds_shrink_node_list(&cmds_sub[tmp]);
  if (cmds_sub[tmp].size)
    qsort(cmds_sub[tmp].ptr, cmds_sub[tmp].size, sizeof(command_node *),
          internal_command_node_cmp);
  ++tmp;
 }
 tmp = 0;
 
 while (tmp < CMDS_SIZE_MISC)
 {
  internal_cmds_shrink_node_list(&cmds_misc[tmp]);
  if (cmds_misc[tmp].size)
    qsort(cmds_misc[tmp].ptr, cmds_misc[tmp].size, sizeof(command_node *),
          internal_command_node_cmp);
  ++tmp;
 }
 tmp = 0;
 
 while (tmp < CMDS_SIZE_SECTION)
 {
  internal_cmds_shrink_node_list(&cmds_section[tmp]);
  if (cmds_section[tmp].size)
    qsort(cmds_section[tmp].ptr, cmds_section[tmp].size,
          sizeof(command_node *), internal_command_node_cmp);
  ++tmp;
 }
 tmp = 0;
}

static void internal_cmds_init(void);

void init_cmds_list(void)
{
 cmds_setup();

 cmds_init_admin();
 cmds_init_alarm();
 cmds_init_alias();
 cmds_init_angel();
 cmds_init_auth_player();
 cmds_init_autoexec();
 cmds_init_backups();
 cmds_init_blank();
 cmds_init_channels();
 cmds_init_check();
 cmds_init_chlim();
 cmds_init_colourise();
 cmds_init_commands();
 cmds_init_communication();
 cmds_init_configure();
 cmds_init_converse();
 cmds_init_copy_str();
 cmds_init_crazynews();
 cmds_init_dl();
 cmds_init_draughts();
 cmds_init_editor();
 cmds_init_email();
 cmds_init_friend_coms();
 cmds_init_games();
 cmds_init_hangman();
 cmds_init_help();
 cmds_init_idle();
 cmds_init_intercom();
 cmds_init_karma();
 cmds_init_last_command();
 cmds_init_last_logon();
 cmds_init_log();
 cmds_init_logoff();
 cmds_init_list();
 cmds_init_jotd();
 cmds_init_mail();
 cmds_init_marriage();
 cmds_init_mask_coms();
 cmds_init_mode();
 cmds_init_motd();
 cmds_init_msgs();
 cmds_init_multi_base();
 cmds_init_multi_communication();
 cmds_init_news();
 cmds_init_nickname();
 cmds_init_nuke();
 cmds_init_pager();
 cmds_init_passwd();
 cmds_init_privs();
 cmds_init_process_input();
 cmds_init_prompt();
 cmds_init_quit_in();
 cmds_init_room();
 cmds_init_safemalloc();
 cmds_init_session();
 cmds_init_show();
 cmds_init_shutdown();
 cmds_init_socials();
 cmds_init_spodlist();
 cmds_init_sps();
 cmds_init_sumotd();
 cmds_init_super_channel();
 cmds_init_stats();
 cmds_init_stats_files();
 cmds_init_terminal();
 cmds_init_timer_player();
 cmds_init_tip();
 cmds_init_toggle();
 cmds_init_ttt();
 cmds_init_who();
 cmds_init_wotw();
 
 internal_cmds_init();

 cmds_init_local_talker();
 
 cmds_sort_arrays();
}

const char *cmds_do_match(const char *str, command_node *com_entry,
                          int force_full_match)
{
 int complete_match = FALSE;

 assert(str && com_entry);
 
 if (!force_full_match && !com_entry->flag_no_expand)
 {
  if (match_clever(com_entry->name, &str, com_entry->flag_no_clever_expand ?
                   MATCH_CLEVER_FLAG_DEFAULT : MATCH_CLEVER_FLAG_EXPAND))
    return (NULL);
 }
 else if (!force_full_match && beg_strcasecmp(com_entry->name, str))
   return (NULL);
 else
 {
  str += strlen(com_entry->name);
  complete_match = TRUE;
 }
 
 if ((com_entry->flag_no_expand && !complete_match) ||
     (!com_entry->flag_no_expand && isalpha((unsigned char) *str)))
   return (NULL);
 
 if (!com_entry->flag_no_space_needed && *str && (*str != ' '))
   return (NULL);
 
 if (!com_entry->flag_no_beg_space)
   /* gets rid of ALL spaces separating command and arguments */
   str += strspn(str, " ");
 else if ((*str == ' ') && !com_entry->flag_no_space_needed)
   ++str; /* gets rid of ONE space separating command and arguments */

 return (str);
}

int cmds_run_func(cmds_function *cmd,
                  player *p, const char *str, size_t length)
{
 int worked = TRUE;
 
 switch (cmd->type)
 {
  case CMDS_PARAM_NOTHING:
    worked = FALSE;
    break;
  case CMDS_PARAM_CONST_CHARS:
    (*cmd->func.player_and_const_chars) (p, str);
    break;
  case CMDS_PARAM_CHARS_SIZE_T:
    (*cmd->func.player_and_chars_and_length) (p, str, length);
    break;
  case CMDS_PARAM_RET_CHARS_SIZE_T:
    return ((*cmd->func.player_ret_and_chars_and_length) (p, str, length));
    break;
  case CMDS_PARAM_NO_CHARS:
    (*cmd->func.player_only) (p);
    break;
  case CMDS_PARAM_PARSE_PARAMS:
  {
   GET_PARAMETER_DECL_CREATE(params, (1024 * 4));
              
   GET_PARAMETER_DECL_INIT(params, (1024 * 4));
   
   get_parameter_parse(params, &str, GET_PARAMETER_NUMBER_MAX);
   
   (*cmd->func.player_and_parsed_params) (p, params);
  }
  break;
  
  default:
    assert(FALSE);
 }

 return (TRUE);
}

static int internal_cmds_match(player *p,  const char *str, size_t length,
                               int loop_count, command_base *is_sub_command)
{
 command_base *comlist = NULL;
 const char *passed_str = str;
 const char *args = NULL;
 alias_node *alias = NULL;
 size_t cmd_count = 0;
 int command_length = 0;
 alias_search_node alias_search_save;
 
 if (!(p && p->saved))
 {
  log_assert(FALSE);
  fvtell_player(SYSTEM_T(p), "%s",
                " Something went wrong, please logon again"
                " and bug what you did.\n");
  user_logoff(p, NULL);
  return (FALSE);
 }

 if (is_sub_command)
   current_sub_command = NULL;
 else
   current_command = NULL;
  
 alias_ld_so(p, &str, &alias_search_save, FALSE);
 match_command_init_parse:
 if (is_sub_command && (*str == '/'))
 {
  --command_length;
  ++str;
  is_sub_command = NULL;
 }
 
 str += strspn(str, " ");
 if (!*str)
   return (FALSE);

 alias = alias_search_next_command(&alias_search_save, str, &command_length,
                                   !is_sub_command);

 if (!*str)
 {
  fvtell_player(SYSTEM_T(p), "%s", " Empty command.\n");
  return (FALSE);
 }

 cmd_count = 0;
 if (is_sub_command)
   comlist = is_sub_command;
 else
   if (isalpha((unsigned char) *str))
     comlist = &cmds_alpha[ALPHA_LOWER_OFFSET(tolower((unsigned char) *str))];
 
 while (alias || (comlist && (cmd_count < comlist->size)))
 {
  int s_cmp = 1; 
 
  if (!alias || (comlist && (cmd_count < comlist->size) &&
                 (s_cmp = strcmp(comlist->ptr[cmd_count]->name,
                                 alias->command)) < 0))
  {
   if ((*comlist->ptr[cmd_count]->test_can_run)(p->saved) &&
       (args = cmds_do_match(str, comlist->ptr[cmd_count],
                             p->flag_no_cmd_matching)))
   {
    if (comlist->ptr[cmd_count]->flag_disabled)
    {
     fvtell_player(SYSTEM_T(p),
                   " This %scommand is disabled at the moment - sorry.\n",
                   is_sub_command ? "sub " : "");
     return (FALSE);
    }
    else
    {
     int worked = FALSE;
     
     /* FIXME: give all sub commands flag_no_end_space */
     if (!is_sub_command && !comlist->ptr[cmd_count]->flag_no_end_space)
     { /* takes spaces off the end of a line */
      char *space = (p->input_start->input + p->input_start->length - 1);
      
      while (*space == ' ')
        *space-- = 0;
     }
     
     if (is_sub_command)
     {
      if (!current_sub_command)
        current_sub_command = comlist->ptr[cmd_count]->name;
     }
     else
       if (!current_command)
         current_command = comlist->ptr[cmd_count]->name;

     last_command_add(p, comlist->ptr[cmd_count]->name);

     CTRACE(comlist->ptr[cmd_count]->name);
     
     comlist->ptr[cmd_count]->totals++;
     
     cmds_last_ran = comlist->ptr[cmd_count];
     worked = cmds_run_func(&comlist->ptr[cmd_count]->func, p, args,
                            length - (args - passed_str));
     cmds_last_ran = NULL;
     
     if (is_sub_command)
       current_sub_command = NULL;
     else
       current_command = NULL;

     if (worked)
       ++current_command_number;

     text_objs_del_type(TEXT_OBJS_TYPE_PARAMS);
     
     return (worked);
    }
   }
   
   ++cmd_count;
  }
  else
  {
   alias_node *matched_alias = alias;
   int matched_command_length = command_length;
   
   assert(alias);

   if (loop_count <= 0)
   {
    fvtell_player(SYSTEM_T(p), "%s",
                  " You have gone over the maximum alias loop count.\n");
    return (FALSE);
   }
   
   if (alias->flag_no_beg_space)
     while ((alias =
             alias_search_next_command(&alias_search_save, str,
                                       &command_length, !is_sub_command)) &&
            (alias->flag_no_beg_space))
     {
      matched_alias = alias;
      matched_command_length = command_length;
     }
   
   --loop_count;
   str += matched_command_length;
   alias_substitute_command(p, str, matched_alias);
   if (matched_alias->flag_use_name)
   {
    if (is_sub_command)
      current_sub_command = matched_alias->command;
    else
      current_command = matched_alias->command;
   }
   passed_str = str = p->input_start->input;
   length = p->input_start->length;
   
   str += strspn(str, " ");
   if (!*str)
     return (FALSE);

   
   alias_ld_so(p, &str, &alias_search_save,
               ALIAS_IS_SYSTEM(&alias_search_save));
   goto match_command_init_parse;
  }
 }

 command_length = 0;
 args = str;
 while (*args && (*args != ' '))
   ++args;
 command_length = (args - str);
 
 fvtell_player(SYSTEM_FT(RAW_OUTPUT_VARIABLES, p),
               " Cannot find %scommand '^S^B%.*s^s'.\n",
               is_sub_command ? "sub " : "", command_length, str);
 return (FALSE);
}

int cmds_sub_match(player *p, const char *str, size_t length,
                    command_base *comlist)
{
 return (internal_cmds_match(p, str, length, 4, comlist));
}

int cmds_match(player *p, const char *str, size_t length, int loop_count)
{
 return (internal_cmds_match(p, str, length, loop_count, NULL));
}

void user_cmds_show_section(player *p, const char *str)
{
 const char *admin_check = "you";
 int section = -1;
 player *p2 = NULL;
 GET_PARAMETER_DECL_CREATE(params, 1024);
 
 GET_PARAMETER_DECL_INIT(params, 1024);
   
 get_parameter_parse(params, &str, GET_PARAMETER_NUMBER_MAX);
  
 if (p->saved->priv_senior_su)
 {
  if (params->last_param == 2)
  {
   if (!(p2 = player_find_load(p, GET_PARAMETER_STR(params, 1),
                               PLAYER_FIND_SC_INTERN)))
     return;
   
   admin_check = p2->saved->name;
   get_parameter_shift(params, 1);
  }  
 }

 if (!p2)
   p2 = p;

 if (params->last_param == 1)
 {
  lower_case(GET_PARAMETER_STR(params, 1));
  
  switch (*GET_PARAMETER_STR(params, 1))
  {
   case 'a':
     if ((*(GET_PARAMETER_STR(params, 1) + 1) == 'd') &&
         (p2->saved->priv_coder || p2->saved->priv_senior_su))
     {
      if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "administrator"))
        section = CMDS_SECTION_ADMIN;
     }
     else
     {
      unsigned int count = 0;
      unsigned int cmd_array = 0;
      alias_search_node alias_search_save;
      alias_node *alias = NULL;
      
      if (beg_strcmp(GET_PARAMETER_STR(params, 1), "all"))
        break;
      
      alias_ld_so(p2, NULL, &alias_search_save, FALSE);
      alias = alias_search_next_name(&alias_search_save, NULL, NULL);
       
      fvtell_player(NORMAL_T(p), "\n All the commands available "
                    "to %s are -=>\n", admin_check);
       
      while (cmd_array < 26)
      {
       unsigned int cmd_number = 0;
       
       while ((cmd_number < cmds_alpha[cmd_array].size))
       {
        command_node *tmp = cmds_alpha[cmd_array].ptr[cmd_number];
        int s_cmp = 0;
        
        if (!alias || ((s_cmp = strcmp(tmp->name, alias->command)) < 0))
        {
         if ((*tmp->test_can_run)(p->saved))
         {
          fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(count, "%s"),
                        tmp->name);
          ++count;
         }
         ++cmd_number;
        }
        else
        {
         if (!s_cmp)
           ++cmd_number;
         
         fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(count, "^U%s^u"),
                       alias->command);
         ++count;
         alias = alias_search_next_name(&alias_search_save, NULL, NULL);
        }
       }
       
       ++cmd_array;
      }

      while (alias)
      {
       fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(count, "^U%s^u"),
                     alias->command);
       
       ++count;
       alias = alias_search_next_name(&alias_search_save, NULL, NULL);
      }

      assert(count); /* you find me a talker with no commands, and I'll
                      * show you a talker coder who doesn't care */
      
      fvtell_player(NORMAL_T(p), "%s", ".\n");
      pager(p, PAGER_DEFAULT);
      return;
     }
     break;
     
   case 'c':
     if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "channels"))
       section = CMDS_SECTION_CHANNELS;
     else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "check"))
       section = CMDS_SECTION_CHECK;
     else if (p2->saved->priv_admin &&
              !beg_strcmp(GET_PARAMETER_STR(params, 1), "chlim"))
       section = CMDS_SECTION_CHLIM;
     else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "communication"))
       section = CMDS_SECTION_COMMUNICATION;
     else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "configure"))
       section = CMDS_SECTION_CONFIGURE;
     break;
     
   case 'd':
     if (configure.game_draughts_use && p2->saved->priv_base)
       if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "draughts"))     
         section = CMDS_SECTION_DRAUGHTS;
     break;
     
   case 'e':
     if (p2->saved->priv_base)
       if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "editor"))
         section = CMDS_SECTION_EDITOR;
     break;
     
   case 'g':
     if (priv_test_configure_games(p2->saved) && p2->saved->priv_base)
       if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "games"))     
         section = CMDS_SECTION_GAME;
     break;
     
   case 'h':
     if (PRIV_STAFF(p2->saved) &&
         !beg_strcmp(GET_PARAMETER_STR(params, 1), "hidden"))
       section = CMDS_SECTION_HIDDEN;
     break;
     
   case 'i':
     if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "information"))
       section = CMDS_SECTION_INFORMATION;
#ifdef USE_INTERCOM
     else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "intercom"))
       section = CMDS_SECTION_INTERCOM;
#endif
     break;
     
   case 'l':
     if (p2->saved->priv_command_list &&
         !beg_strcmp(GET_PARAMETER_STR(params, 1), "list"))   
       section = CMDS_SECTION_LIST;
     else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "local"))  
       section = CMDS_SECTION_LOCAL;
     break;
     
   case 'm':
     if (*(GET_PARAMETER_STR(params, 1) + 1) == 'u')
     {
      if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "multis"))     
        section = CMDS_SECTION_MULTI;
     }
     else if ((*(GET_PARAMETER_STR(params, 1) + 1) == 'i') &&
              (*(GET_PARAMETER_STR(params, 1) + 2) == 's'))
     {
      if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "miscellaneous"))
        section = CMDS_SECTION_MISC;
     }
     else if (((*(GET_PARAMETER_STR(params, 1) + 1) == 'i') &&
               (*(GET_PARAMETER_STR(params, 1) + 2) == 'n')) &&
              p2->saved->priv_minister)
     {
      if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "minister"))
        section = CMDS_SECTION_MINISTER;
     }
     else if (p2->saved->priv_command_mail &&
               !beg_strcmp(GET_PARAMETER_STR(params, 1), "mail"))
       section = CMDS_SECTION_MAIL;
     break;
     
   case 'n':
     if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "news"))
       section = CMDS_SECTION_NEWS;
     else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "newsgroup"))
       section = CMDS_SECTION_NEWSGROUP;
     break;
     
   case 'p':
     if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "personal_info"))
       section = CMDS_SECTION_PERSONAL_INFO;
     break;
     
   case 'r':
     if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "room"))
       section = CMDS_SECTION_ROOM;
     break;
     
   case 's':
     if (*(GET_PARAMETER_STR(params, 1) + 1) == 'e')
     {
      if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "settings"))     
        section = CMDS_SECTION_SETTINGS;
     }
     else if (*(GET_PARAMETER_STR(params, 1) + 1) == 'o')
     {
      if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "socials"))     
        section = CMDS_SECTION_SOCIAL;
     }
     else if ((*(GET_PARAMETER_STR(params, 1) + 1) == 'p') &&
               p2->saved->priv_spod)
     {
      if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "spod"))   
        section = CMDS_SECTION_SPOD;
     }
     else if ((*(GET_PARAMETER_STR(params, 1) + 1) == 'u') &&
              p2->saved->priv_pretend_su)
     {
      if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "superuser"))
        section = CMDS_SECTION_SU;
     }
     else if (*(GET_PARAMETER_STR(params, 1) + 1) == 'y')
     {
      if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "system"))
        section = CMDS_SECTION_SYSTEM;
     }
     else
     {
      if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "stats"))
        section = CMDS_SECTION_STATS;
     }
     break;
     
   default:
     ; /* don't need to do section = -1; as that is the default */  
  }
 }
 
 if (section == -1)
 {
  fvtell_player(NORMAL_T(p), "\n The types of commands available to %s are"
               " -=>\n   ", admin_check);
  
  if (p2->saved->priv_coder && p2->saved->priv_senior_su)
    fvtell_player(NORMAL_T(p), "%s", "administrator, hidden, ");
  
  fvtell_player(NORMAL_T(p), "all, channels, check, %scommunication%s",
                p->saved->priv_admin ? "chlim, " : "",
                p->saved->priv_admin ? ", configure" : "");

  if (configure.game_draughts_use && p2->saved->priv_base)
    fvtell_player(NORMAL_T(p), "%s", ", draughts");
  
  if (p2->saved->priv_command_mail)
    fvtell_player(NORMAL_T(p), "%s", ", editor");

  if (priv_test_configure_games(p2->saved) && p2->saved->priv_base)
    fvtell_player(NORMAL_T(p), "%s", ", games");
  
  fvtell_player(NORMAL_T(p), "%s", ", information");

#ifdef USE_INTERCOM
  if (p2->saved->priv_base)
    fvtell_player(NORMAL_T(p), "%s", ", intercom");
#endif
      
  if (p2->saved->priv_command_list)
    fvtell_player(NORMAL_T(p), "%s", ", list");

  fvtell_player(NORMAL_T(p), "%s", ", local");
  
  if (p2->saved->priv_command_mail)
    fvtell_player(NORMAL_T(p), "%s", ", mail");
  
  if (p2->saved->priv_minister)
    fvtell_player(NORMAL_T(p), "%s", ", minister");
  
  fvtell_player(NORMAL_T(p), "%s", ", miscellaneous, multis, "
                "news, newsgroup, personal_info, room, settings, socials");
  
  if (p2->saved->priv_spod)
    fvtell_player(NORMAL_T(p), "%s", ", spod");
  
  if (p2->saved->priv_coder || p2->saved->priv_pretend_su)
    fvtell_player(NORMAL_T(p), "%s", ", superuser");
  
  fvtell_player(NORMAL_T(p), "%s",
                ", stats, system.\n\n Type: commands <type> to "
                "see the commands available, (full word not needed).\n");
 }
 else
 {
  command_base *comlist = &cmds_section[section];
  size_t cmd_count = 0;
  alias_search_node alias_search_save;
  alias_node *alias = NULL;
  int count = 0;
  
  fvtell_player(NORMAL_T(p), "\n %s commands available to %s are -=>\n   ",
                cmds_map_sections[section], admin_check);
  
  switch (section)
  {
   case CMDS_SECTION_STATS:
   case CMDS_SECTION_ROOM:
   case CMDS_SECTION_CHECK:
   case CMDS_SECTION_EDITOR:
   case CMDS_SECTION_MAIL:
   case CMDS_SECTION_NEWS:  
   case CMDS_SECTION_NEWSGROUP:
   case CMDS_SECTION_DRAUGHTS:
   case CMDS_SECTION_INTERCOM:
   case CMDS_SECTION_CHLIM:
   case CMDS_SECTION_CHANNELS:
   case CMDS_SECTION_CONFIGURE:
     fvtell_player(NORMAL_T(p), "%s",
                   "\n This is a command or mode with sub commands.\n");
   default:
     break;
  }
  
  alias_ld_so(p2, NULL, &alias_search_save, FALSE);
  alias = alias_search_next_type(&alias_search_save, section);
  
  while ((cmd_count < comlist->size) || alias)
  {
   int s_cmp = -1;
   
   if (!alias || ((cmd_count < comlist->size) &&
                  ((s_cmp = strcmp(comlist->ptr[cmd_count]->name,
                                   alias->command)) < 0)))
   {
    if ((*comlist->ptr[cmd_count]->test_can_run)(p2->saved) &&
        !comlist->ptr[cmd_count]->flag_disabled)
    {
     fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(count, "%s"),
                   comlist->ptr[cmd_count]->name);
     ++count;
    }
    ++cmd_count;
   } 
   else
   {
    fvtell_player(NORMAL_T(p), ADD_COMMA_FRONT(count, "^U%s^u"),
                  alias->command);
    if (!s_cmp)
      ++cmd_count;
    ++count;
    alias = alias_search_next_type(&alias_search_save, section);
   }
  }
  
  fvtell_player(NORMAL_T(p), "%s", ".\n");
 }
}

static void user_cmds_show_matches(player *p, const char *str)
{
 command_base *comlist = NULL;
 alias_node *alias = NULL;
 alias_search_node alias_search_save;
 size_t cmd_count = 0;
 int done = FALSE;
 int first_real_match = TRUE;
 int command_length; 

 str += strspn(str, " ");
 if (!*str)
   TELL_FORMAT(p, "<part of command>");

 alias_ld_so(p, &str, &alias_search_save, FALSE);
 alias = alias_search_next_name(&alias_search_save, str, &command_length);

 if (!*str)
 {
  fvtell_player(SYSTEM_T(p), "%s", " Empty command. (\\'s will be removed)\n");
  return;
 }
 
 if (isalpha((unsigned char) *str))
   comlist = &cmds_alpha[ALPHA_LOWER_OFFSET(tolower((unsigned char) *str))];
 
 while (alias || (comlist && (cmd_count < comlist->size)))
 {
  int s_cmp = 1;

  if (!alias || (comlist && (cmd_count < comlist->size) &&
                 (s_cmp = strcmp(comlist->ptr[cmd_count]->name,
                                 alias->command)) < 0))
  {
   assert(comlist && comlist->ptr[cmd_count]->name);
   
   if ((*comlist->ptr[cmd_count]->test_can_run)(p->saved) &&
       cmds_do_match(str, comlist->ptr[cmd_count], FALSE))
   {
    fvtell_player(NORMAL_T(p), "%s%s%s%s",
                  done ? ", " : " Command matches to: ",
                  first_real_match ? "^S^B" : "",
                  comlist->ptr[cmd_count]->name,
                  first_real_match ? "^s" : "");
    first_real_match = FALSE;
   }
   else   
   {
    ++cmd_count;
    continue;
   }
   
   ++cmd_count;
  }
  else
  {
   assert(alias);

   fvtell_player(NORMAL_T(p), "%s%s%s%s",
                 done ? ", " : " Command matches to: ",
                 first_real_match ? "^S^U^B" : "^S^U",
                 alias->command,
                 "^s");
   first_real_match = FALSE;
   
   alias = alias_search_next_name(&alias_search_save, str, &command_length);
   if (!s_cmp)
     ++cmd_count;
  }

  ++done;
 }

 if (done) /* we have matched to some commands */
 {
  fvtell_player(NORMAL_T(p), "%s", ".\n");
  return;
 }

 fvtell_player(SYSTEM_T(p), " The string -- ^S^B%s^s -- doesn't match "
               "any commands.\n", str);
}

static void user_cmds_disable_command(player *p, const char *str)
{
 command_base *comlist = NULL;
 size_t cmd_count = 0;
 
 if (!*str)
   TELL_FORMAT(p, "<command_to_disable>");
 
 if (isalpha((unsigned char) *str))
   comlist = &cmds_alpha[ALPHA_LOWER_OFFSET(tolower((unsigned char) *str))];
 
 while (comlist && (comlist && (cmd_count < comlist->size)))
 {
  if (cmds_do_match(str, comlist->ptr[cmd_count], p->flag_no_cmd_matching))
  {
   if (!strcmp(comlist->ptr[cmd_count]->name, current_command))
   {
    fvtell_player(SYSTEM_T(p), "%s",
                  " You cannot disable the disable command!\n");
    return;
   }
   
   if (comlist->ptr[cmd_count]->flag_disabled)
   {
    comlist->ptr[cmd_count]->flag_disabled = FALSE;
    fvtell_player(NORMAL_T(p), " Command '^S^B%s^s' Enabled.\n",
                  comlist->ptr[cmd_count]->name);
   }
   else
   {
    comlist->ptr[cmd_count]->flag_disabled = TRUE;
    fvtell_player(NORMAL_T(p), " Command '^S^B%s^s' Disabled.\n",
                  comlist->ptr[cmd_count]->name);
   }
   
   return;
  }
   
   ++cmd_count;
 }

 fvtell_player(SYSTEM_T(p), " Command - ^S^B%s^s - not found.\n", str);
}

static void user_toggle_cmds_matching(player *p, const char *str)
{
 TOGGLE_COMMAND_OFF_ON(p, str, p->flag_no_cmd_matching, TRUE,
                       " You %sdo have command matching disabled.\n",
                       " You %sdo have command matching enabled.\n",
                       TRUE);
}

static void internal_cmds_init(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("commands", user_cmds_show_section, CONST_CHARS, SYSTEM);
 
 CMDS_ADD("commands_matching", user_toggle_cmds_matching, CONST_CHARS,
          SETTINGS);
 CMDS_ADD("match_commands", user_cmds_show_matches, CONST_CHARS, SYSTEM);
 
 CMDS_ADD("disable_command", user_cmds_disable_command, CONST_CHARS, ADMIN);
 CMDS_PRIV(coder_lower_admin);
}
