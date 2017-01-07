#ifndef EXTERN_ALIAS_H
#define EXTERN_ALIAS_H
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

#ifdef ALIAS_C

#define ALIAS_FLAG_FUNC_ON_OFF(flag, name) do { \
 const char *flag_type = ""; \
 \
 if (!str) \
   return (current->flag_ ## flag); \
 \
 if (TOGGLE_MATCH_ON(str)) \
 { if (!current->flag_ ## flag) { flag_type = "Set flag"; \
                                  current->flag_ ## flag = TRUE; } \
   else flag_type = "Kept flag on"; } \
 else \
   if (TOGGLE_MATCH_OFF(str)) \
   { if (current->flag_ ## flag) { flag_type = "Removed flag"; \
                                   current->flag_ ## flag = FALSE; } \
     else flag_type = "Kept flag off"; } \
   else \
     if (TOGGLE_MATCH_TOGGLE(str)) \
     { flag_type = "Toggled flag"; \
       current->flag_ ## flag = !current->flag_ ## flag; } \
     else { fvtell_player(SYSTEM_T(p), \
                          " The string -- ^S^B%s^s -- isn't a " \
                          "valid setting.\n", str); return (-1); } \
 \
 if (p) fvtell_player(NORMAL_T(p), " %s ^S^B%s^s, on the alias ^S^B%s^s.\n", \
                      flag_type, name, current->command); \
 return (current->flag_ ## flag); } while (FALSE)

#define ALIAS_ADD_STR(str, len) do { \
      if (use_text_objs) \
      { \
       int id = text_objs_add(p->saved, str, len, TEXT_OBJS_TYPE_PARAMS); \
       char numb_buffer[BUF_NUM_TYPE_SZ(int)]; \
       int numb_length = sprintf(numb_buffer, "%d", id); \
       \
       if (INPUT_SPACE_LEFT(in_node, numb_length) > 0) \
         INPUT_COPY(in_node, numb_buffer, numb_length); \
       else \
         goto buffer_too_small; \
      } \
      else \
      { \
       if (INPUT_SPACE_LEFT(in_node, len) > 0) \
         INPUT_COPY(in_node, str, len); \
       else \
         goto buffer_too_small; \
      } \
 } while (FALSE)

#endif

#define ALIAS_IS_SYSTEM(x) ((x)->is_system_alias)

extern void alias_cleanup(player *);

extern void alias_load(player *, file_io *);
extern void alias_save(player *, file_io *);

extern void init_alias(void);

extern void alias_ld_so(player *, const char **, alias_search_node *, int);
extern void alias_lib_ldconfig(player *);

extern alias_node *alias_search_next_name(alias_search_node *,
                                          const char *, int *);
extern alias_node *alias_search_next_command(alias_search_node *,
                                             const char *, int *, int);
extern alias_node *alias_search_next_type(alias_search_node *, unsigned int);

extern void alias_substitute_command(player *, const char *, alias_node *);

extern void cmds_init_alias(void);

#endif
