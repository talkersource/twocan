#define MATCH_C
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

int match_clever(const char *full_str, const char **str_orig, int flags)
{
 const char *const full_str_start = full_str;
 const char *const str_start = *str_orig;
 const char *str = str_start;
 ptrdiff_t skip_start;
 ptrdiff_t skip_end;
 int skipping = FALSE; /* are we skipping chars now */
 int have_skipped = FALSE; /* have we ever skipped chars, parsing this str */
 
 while (*full_str && *str && ((*str != ' ') ||
                              (flags & MATCH_CLEVER_FLAG_ALLOW_SPACE)))
 {
  unsigned char it = *str;

  if (!(flags & MATCH_CLEVER_FLAG_CASE))
    it = tolower(it);
  
  if (it == *full_str)
  {
   str++;
   full_str++;
   
   if (skipping)
   {
    skipping = FALSE;
    have_skipped = TRUE;
    skip_end = (full_str - full_str_start);
   }
  }
  else
    if (skipping)
      full_str++;
    else
      if ((it == '-') && (flags & MATCH_CLEVER_FLAG_EXPAND))
      {
       skipping = TRUE;
       have_skipped = TRUE;
       str += strspn(str, "-");
       skip_start = (str - str_start);
      }
      else
        if (have_skipped)
        {
         skipping = TRUE;
         str = (str_start + skip_start);
         full_str = (full_str_start + skip_end);
        }
        else
        {
         assert((unsigned char)*full_str - it);
         return ((unsigned char)*full_str - it);
        }
 }
 
 *str_orig = str;
 return (0); /* found a match */
}

