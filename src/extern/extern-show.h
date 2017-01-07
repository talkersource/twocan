#ifndef EXTERN_SHOW_H
#define EXTERN_SHOW_H
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

#ifdef SHOW_C
#define SHOW_FLAG_FUNC_ON_OFF(flag, name) do { \
 const char *flag_type = ""; \
 \
 if (!str) \
   return (p->flag_show_ ## flag); \
 \
 if (TOGGLE_MATCH_ON(str)) \
 { flag_type = "Set flag"; p->flag_show_ ## flag = TRUE; } \
 else \
   if (TOGGLE_MATCH_OFF(str)) \
   { flag_type = "Removed flag"; p->flag_show_ ## flag = FALSE; } \
   else \
     if (TOGGLE_MATCH_TOGGLE(str)) \
     { flag_type = "Toggled flag"; \
       p->flag_show_ ## flag = !p->flag_show_ ## flag; } \
     else { TELL_FORMAT_NO_RETURN(p, "<flag(s)> [on|off|toggle]"); \
            return (-1); } \
 \
 if (p) fvtell_player(NORMAL_T(p), " %s ^S^B%s^s, on show types.\n", \
                      flag_type, name); \
 return (p->flag_show_ ## flag); } while (FALSE)

#endif

extern void cmds_init_show(void);

#endif
