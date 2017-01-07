#define WORD_TIME_C
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


const char *word_time_short(char *buf, size_t len,
                            unsigned long atime, unsigned int flags)
{
 int years = 0;
 int weeks = 0;
 int days = 0;
 int hours = 0;
 int minutes = 0;
 int seconds = 0;
 unsigned int done_before = 0;
 size_t count = 0;
 
 assert(!(flags & WORD_TIME_NOT_USED));
 
 if (!atime)
   return ("no time at all");
 
 if (WORD_TIME_TIME_TEST(YEARS, years))
 {
  if (WORD_TIME_OVERFLOW_TEST(CONST_STRLEN("x")))
    goto buf_overflow;
  
  ++done_before;
  count += sprintf(buf + count, "%dy", years);
 }

 if (WORD_TIME_TIME_TEST(WEEKS, weeks))
 {
  if (WORD_TIME_OVERFLOW_TEST(done_before ?
                              CONST_STRLEN(", x") : CONST_STRLEN("x")))
    goto buf_overflow;
  
  if (done_before)
  {
   qstrcpy(buf + count, ", ");
   count += 2;
  }
  ++done_before;
  
  if (weeks && (WORD_TIME_MOD_WEEKS & flags))
    weeks %= 52;
  
  count += sprintf(buf + count, "%dw", weeks);
 }

 if (WORD_TIME_TIME_TEST(DAYS, days))
 {
  if (WORD_TIME_OVERFLOW_TEST(done_before ?
                              CONST_STRLEN(", x") : CONST_STRLEN("x")))
    goto buf_overflow;
  
  if (done_before)
  {
   qstrcpy(buf + count, ", ");
   count += 2;
  }
  ++done_before;
  
  if (days && (WORD_TIME_MOD_DAYS & flags))
    days %= 7;
  
  count += sprintf(buf + count, "%dd", days);
 }
 
 if (WORD_TIME_TIME_TEST(HOURS, hours))
 {
  if (WORD_TIME_OVERFLOW_TEST(done_before ?
                              CONST_STRLEN(", x") : CONST_STRLEN("x")))
    goto buf_overflow;
  
  if (done_before)
  {
   qstrcpy(buf + count, ", ");
   count += 2;
  }
  ++done_before;
  
  if (hours && (WORD_TIME_MOD_HOURS & flags))
    hours %= 24;
  
  count += sprintf(buf + count, "%dh", hours);
 }
 
 if (WORD_TIME_TIME_TEST(MINUTES, minutes))
 {
  if (WORD_TIME_OVERFLOW_TEST(done_before ?
                              CONST_STRLEN(", x") : CONST_STRLEN("x")))
    goto buf_overflow;
  
  if (done_before)
  {
   qstrcpy(buf + count, ", ");
   count += 2;
  }
  ++done_before;
  
  if (minutes && (WORD_TIME_MOD_MINUTES & flags))
    minutes %= 60;
  
  count += sprintf(buf + count, "%dm", minutes);
 }

 if (WORD_TIME_TIME_TEST(SECONDS, seconds))
 {
  if (WORD_TIME_OVERFLOW_TEST(done_before ?
                              CONST_STRLEN(", x") : CONST_STRLEN("x")))
    goto buf_overflow;
  
  if (done_before)
  {
   qstrcpy(buf + count, ", ");
   count += 2;
  }
  ++done_before;

  if (seconds && (WORD_TIME_MOD_SECONDS & flags))
    seconds %= 60;
  
  count += sprintf(buf + count, "%ds", seconds);
 }
 
 buf[len - 1] = 0;
 return (buf);

 buf_overflow:

 return (" ** error: word time ** ");
}

const char *word_time_long(char *buf, size_t len,
                           unsigned long atime, unsigned int flags)
{
 int years = 0;
 int weeks = 0;
 int days = 0;
 int hours = 0;
 int minutes = 0;
 int seconds = 0;
 unsigned int done_before = 0;
 size_t count = 0;

 assert(!(flags & WORD_TIME_NOT_USED));
 
 if (!atime)
   return ("no time at all");

 if (WORD_TIME_TIME_TEST(YEARS, years))
 {
  if (WORD_TIME_OVERFLOW_TEST(CONST_STRLEN(" years")))
    goto buf_overflow;

  ++done_before;
  count += sprintf(buf + count, "%d year%s", years, (years != 1) ? "s" : "" );
 }
 
 if (WORD_TIME_TIME_TEST(WEEKS, weeks))
 {
  if (WORD_TIME_OVERFLOW_TEST((done_before ? CONST_STRLEN(", ") : 0) +
                              CONST_STRLEN(" weeks")))
    goto buf_overflow;
  
  if (done_before)
  {
   qstrcpy(buf + count, ", ");
   count += 2;
  }
  ++done_before;
  
  if (weeks && (WORD_TIME_MOD_WEEKS & flags))
    weeks %= 52;

  count += sprintf(buf + count, "%d week%s", weeks, (weeks != 1) ? "s" : "" );
 }

 if (WORD_TIME_TIME_TEST(DAYS, days))
 {
  if (WORD_TIME_OVERFLOW_TEST((done_before ? CONST_STRLEN(", ") : 0) +
                              CONST_STRLEN(" days")))
    goto buf_overflow;
  
  if (done_before)
  {
   qstrcpy(buf + count, ", ");
   count += 2;
  }
  ++done_before;
  
  if (days && (WORD_TIME_MOD_DAYS & flags))
    days %= 7;

  count += sprintf(buf + count, "%d day%s", days, (days != 1) ? "s" : "" );
 }

 if (WORD_TIME_TIME_TEST(HOURS, hours))
 {
  if (WORD_TIME_OVERFLOW_TEST((done_before ? CONST_STRLEN(", ") : 0) +
                              CONST_STRLEN(" hours")))
    goto buf_overflow;
  
  if (done_before)
  {
   qstrcpy(buf + count, ", ");
   count += 2;
  }
  ++done_before;
  
  if (hours && (WORD_TIME_MOD_HOURS & flags))
    hours %= 24;

  count += sprintf(buf + count, "%d hour%s", hours, (hours != 1) ? "s" : "");
 }
 
 if (WORD_TIME_TIME_TEST(MINUTES, minutes))
 {
  if (WORD_TIME_OVERFLOW_TEST((done_before ?
                               (!atime ? CONST_STRLEN(" and ") :
                                CONST_STRLEN(", ")) : 0) +
                              CONST_STRLEN(" minutes")))
    goto buf_overflow;
  
  if (done_before)
  {
   if (!atime)
   {
    qstrcpy(buf + count, " and ");
    count += CONST_STRLEN(" and ");
   }
   else
   {
    qstrcpy(buf + count, ", ");
    count += CONST_STRLEN(", ");
   }
  }
  ++done_before;

  if (minutes && (WORD_TIME_MOD_MINUTES & flags))
    minutes %= 60;

  count += sprintf(buf + count, "%d minute%s",
                   minutes, (minutes != 1) ? "s" : "");
 }
 
 if (WORD_TIME_TIME_TEST(SECONDS, seconds))
 {
  if (WORD_TIME_OVERFLOW_TEST((done_before ? CONST_STRLEN(" and ") : 0) +
                              CONST_STRLEN(" seconds")))
    goto buf_overflow;
  
  if (done_before)
  {
   qstrcpy(buf + count, " and ");
   count += CONST_STRLEN(" and ");
  }

  if (seconds && (WORD_TIME_MOD_SECONDS & flags))
    seconds %= 60;
  
  count += sprintf(buf + count, "%d second%s",
                   seconds, (seconds != 1) ? "s" : "");
 }

 buf[len - 1] = 0;
 return (buf);

 buf_overflow:

 return (" ** error: word time ** ");
}

unsigned long word_time_parse(const char *tmp, int flags, int *error)
{
 unsigned long total_time = 0;
 int hack_num = 7;
 int dummy;

 flags |= WORD_TIME_PARSE_DEF_SECONDS;
 
 if (!error)
   error = &dummy;
 
 while (*tmp)
 {
  unsigned long number = 0;
  int test_char = 0;
  
  tmp += strspn(tmp, " ");
  
  if (!isdigit((unsigned char) *tmp))
  {
   *error = TRUE;
   
   if (flags & WORD_TIME_PARSE_ERRORS)
     return (0);
   else
     return (total_time);
  }
  
  number = skip_atoi(&tmp);

  test_char = tolower((unsigned char) *tmp);
  
  if (PARSE_DEF('y', YEARS))
  {
   CHECK_HACK_NUM(6);
   total_time += MK_YEARS(number);
  }
  else if (PARSE_DEF('w', WEEKS))
  {
   CHECK_HACK_NUM(5);
   total_time += MK_WEEKS(number);
  }
  else if (PARSE_DEF('d', DAYS))
  {
   CHECK_HACK_NUM(4);
   total_time += MK_DAYS(number);
  }
  else if (PARSE_DEF('h', HOURS))
  {
   CHECK_HACK_NUM(3);
   total_time += MK_HOURS(number);
  }
  else if (PARSE_DEF('m', MINUTES))
  {
   CHECK_HACK_NUM(2);
   total_time += MK_MINUTES(number);
  }
  else if (PARSE_DEF('s', SECONDS))
  {
   CHECK_HACK_NUM(1);
   total_time += number;
  }
  else
  {
   *error = TRUE;
   if (flags & WORD_TIME_PARSE_ERRORS)
     return (0);
   else
     return (total_time);
  }

  if (!test_char || (test_char == ' '))
    break;
  
  ++tmp;
 }

 *error = FALSE;
 
 return (total_time);
}
