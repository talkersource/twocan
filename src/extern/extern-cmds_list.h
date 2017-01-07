#ifndef EXTERN_CMDS_LIST_H
#define EXTERN_CMDS_LIST_H
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


#if !(HELP_DEBUG)
# define CMDS_HELP_SEARCH(x, y)
#else
# define CMDS_HELP_SEARCH(x, y) do { if (!help_exists(x)) \
 fprintf(stderr, " Help not found for " y ": %s\n", (x)); \
 } while (FALSE)
#endif

#define CMDS_BEGIN_DECLS() command_node *cmds_current = NULL

#define CMDS_ADD_GENERIC(cmd_name, function, section) do { \
 cmds_current = cmds_get_new_node(); \
 cmds_current->name = cmd_name; \
 cmds_add_section(cmds_current, cmds_section, section); \
 cmds_current->totals = 0; \
 cmds_current->flag_no_space_needed = FALSE; \
 cmds_current->flag_no_expand = FALSE; \
 cmds_current->flag_no_clever_expand = FALSE; \
 cmds_current->flag_no_beg_space = FALSE; \
 cmds_current->flag_no_end_space = FALSE; \
 cmds_current->flag_disabled = FALSE; \
 cmds_current->test_can_run = priv_test_none; \
 } while(FALSE)

/* if you do the below using a switch statment you get lots of errors about
 * incompatable ptrs *even though the swicth will mean it will never happen
 * so we create cpp macros on the fly... */
#define CMDS_FUNC_TYPE_NOTHING(cmds_func, function) do { \
 (cmds_func)->type = CMDS_PARAM_NOTHING; \
 assert(!function); \
 (cmds_func)->func.player_and_const_chars = function; \
 } while(FALSE)

#define CMDS_FUNC_TYPE_CONST_CHARS(cmds_func, function) do { \
 (cmds_func)->type = CMDS_PARAM_CONST_CHARS; \
 assert(function); \
 (cmds_func)->func.player_and_const_chars = function; \
 } while(FALSE)

#define CMDS_FUNC_TYPE_CHARS_SIZE_T(cmds_func, function) do { \
 (cmds_func)->type = CMDS_PARAM_CHARS_SIZE_T; \
 assert(function); \
 (cmds_func)->func.player_and_chars_and_length = function; \
 } while(FALSE)

#define CMDS_FUNC_TYPE_RET_CHARS_SIZE_T(cmds_func, function) do { \
 (cmds_func)->type = CMDS_PARAM_RET_CHARS_SIZE_T; \
 assert(function); \
 (cmds_func)->func.player_ret_and_chars_and_length = function; \
 } while(FALSE)

#define CMDS_FUNC_TYPE_NO_CHARS(cmds_func, function) do { \
 (cmds_func)->type = CMDS_PARAM_NO_CHARS; \
 assert(function); \
 (cmds_func)->func.player_only = function; \
 } while(FALSE)

#define CMDS_FUNC_TYPE_PARSE_PARAMS(cmds_func, function) do { \
 (cmds_func)->type = CMDS_PARAM_PARSE_PARAMS; \
 assert(function); \
 (cmds_func)->func.player_and_parsed_params = function; \
 } while(FALSE)

#define CMDS_ADD(name, cmd_func, func_type, section) do { \
 CMDS_ADD_GENERIC(name, cmd_func, CMDS_SECTION_ ## section); \
 CMDS_HELP_SEARCH(name, "command"); \
 cmds_add_alpha(cmds_current, cmds_alpha, ALPHA_LOWER_OFFSET(*name)); \
 CMDS_FUNC_TYPE_ ## func_type ((&cmds_current->func), cmd_func); \
 } while (FALSE)
 
#define CMDS_ADD_SUB(name, cmd_func, func_type) do { \
 CMDS_ADD_GENERIC(name, cmd_func, CMDS_SECTION_SUB); \
 cmds_add_sub(cmds_current, cmds_sub, CMDS_SECTION_SUB); \
 CMDS_FUNC_TYPE_ ## func_type ((&cmds_current->func), cmd_func); \
 } while (FALSE)

/* explicit version of above... so you can have hidden sub commands */
#define CMDS_ADD_XSUB(name, cmd_func, func_type, section_cmd, section) do { \
 CMDS_ADD_GENERIC(name, cmd_func, CMDS_SECTION_ ## section); \
 cmds_add_sub(cmds_current, cmds_sub, CMDS_SECTION_ ## section_cmd); \
 CMDS_FUNC_TYPE_ ## func_type ((&cmds_current->func), cmd_func); \
 } while (FALSE)

#define CMDS_XTRA_SUB(section_cmd, section) do { \
 assert(cmds_current); \
 cmds_add_section(cmds_current, cmds_section, CMDS_SECTION_ ## section); \
 cmds_add_sub(cmds_current, cmds_sub, CMDS_SECTION_ ## section_cmd); \
 } while (FALSE)

#define CMDS_ADD_MISC(name, cmd_func, func_type, misc_cmd) do { \
 CMDS_ADD_GENERIC(name, cmd_func, CMDS_SECTION_HIDDEN); \
 cmds_add_misc(cmds_current, cmds_misc, CMDS_MISC_ ## misc_cmd); \
 CMDS_FUNC_TYPE_ ## func_type ((&cmds_current->func), cmd_func); \
 } while (FALSE) 

#define CMDS_OVERRIDE(base, name, cmd_func, func_type) do { \
  command_node *cmds_tmp = cmds_find((base)->ptr, (base)->size, name); \
  if (cmds_tmp) CMDS_FUNC_TYPE_ ## func_type ((&cmds_tmp->func), cmd_func); \
 } while (FALSE)

#define CMDS_XTRA_MISC(section) do { \
 assert(cmds_current); \
 cmds_add_misc(cmds_current, cmds_misc, CMDS_MISC_ ## section); \
 } while (FALSE)

 
#define CMDS_FLAG(flag) do { \
 assert(cmds_current && !(cmds_current->flag_ ## flag)); \
 cmds_current->flag_ ## flag = TRUE; \
 } while (FALSE)
 
#define CMDS_PRIV(priv_func) do { \
 assert(cmds_current && (cmds_current->test_can_run == priv_test_none)); \
 cmds_current->test_can_run = priv_test_ ## priv_func; \
 } while (FALSE)

#define CMDS_XTRA_SECTION(section) do { \
  assert(cmds_current); \
  cmds_add_section(cmds_current, cmds_section, CMDS_SECTION_ ## section); \
 } while (FALSE)

#define CMDS_SUB_OFFSET(x) ((x) - CMDS_SUB_SECTION_START)


extern command_node cmds_dummy_input_to;
extern command_node *cmds_last_ran;

extern command_base cmds_alpha[CMDS_SIZE_ALPHA];
extern command_base cmds_sub[CMDS_SIZE_SUB];
extern command_base cmds_misc[CMDS_SIZE_MISC];

extern command_base cmds_section[CMDS_SIZE_SECTION];
extern const char *cmds_map_sections[CMDS_SIZE_SECTION];

/* text pointer for full name of current command */
extern const char *current_command;
/* text pointer for full name of current sub command */
extern const char *current_sub_command;
/* number of the command being executed, inc. sub commands */
extern unsigned int current_command_number;


extern command_node *cmds_get_new_node(void);
extern command_node *cmds_find(command_node *base, size_t len,
                               const char *name);

extern void cmds_add_section(command_node *, command_base *, int);
extern void cmds_add_alpha(command_node *, command_base *, int);
extern void cmds_add_sub(command_node *, command_base *, int);
extern void cmds_add_misc(command_node *, command_base *, int);
    
extern void cmds_sort_arrays(void);
extern void init_cmds_list(void);

extern int cmds_run_func(cmds_function *, player *, const char *, size_t);
extern const char *cmds_do_match(const char *str, command_node *, int);
extern int cmds_test_cando(player *, command_node *);

extern void user_cmds_show_section(player *, const char *);

extern int cmds_sub_match(player *, const char *, size_t, command_base *);
extern int cmds_match(player *, const char *, size_t, int);

#endif
