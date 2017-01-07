#define DISPLAY_TIME_C
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


/* these are for twinkles... just get things one at a time */
char *disp_time_ampm_string(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%p", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_second_number(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%S", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_minute_number(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%M", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_hour_number_24(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%H", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_hour_number_12(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%I", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_day_name(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%A", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_day_name_short(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%A", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_week_day_number(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%w", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_month_name(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%B", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_month_name_short(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%b", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_month_number(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%m", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_month_day_number(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%d", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_week_number(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%U", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_year_number(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%Y", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

char *disp_time_year_day_number(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%j", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

/* END of single times... */

char *disp_time_cmp_string(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 /* NOTE: %m = (01-12) OR (gmtime()->tm_mon + 1) */
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%Y/%m.%d-%H:%M+%S", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
   
 return (tmp);
}

time_t disp_time_string_cmp(const char *tmp)
{
 struct tm *the_time = gmtime(&now);
 
 while (*tmp)
 {
  int number = 0;
  
  tmp += strspn(tmp, " ");
  
  if (!isdigit((unsigned char) *tmp))
    return (mktime(the_time));
  
  number = skip_atoi(&tmp);
  
  switch (*tmp)
  {
   case '/': /* year */
     the_time->tm_year = number - 1900; /* years since 1900 */
     break;
   case '.': /* month */
     the_time->tm_mon = number - 1; /* goes from 0 .. 11 */
     break;
   case '-': /* day of month */
     the_time->tm_mday = number;
     break;
   case ':': /* hour */
     the_time->tm_hour = number;
     break;
   case '+': /* minute */
     the_time->tm_min = number;
     break;
     
   case ' ':
     /* FALLTHROUGH */
   case 0:
     the_time->tm_sec = number;
     continue;
     
   default:
     return (-1);
  }
  
  ++tmp;
 }

 return (mktime(the_time));
}

char *disp_time_file_name(time_t atime)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 
 strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%H%M%S,%d%m%Y", gmtime(&atime));
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
 
 return (tmp);
}

time_t disp_time_add(time_t atime, int seconds)
{
#ifdef USE_QUICK_ADD_TIME
 atime += seconds;
#else
 struct tm *tm = gmtime(&atime);

 if (!seconds)
   return (atime);

 tm->tm_sec += seconds; /* is valid upto 12*60*60 --
                           which is all we'll ever give it */
 atime = mktime(tm);
#endif

  assert(abs(seconds) <= (12 * 60 * 60)); /* we only alter for timezones... */

 return (atime);
}

time_t disp_time_create(int year, int month, int day,
                        int hour, int minute, int second)
{
 time_t atime = now;
 struct tm *tm = gmtime(&atime);

 assert(year >= 1900);
 assert((month > 0) && (month < 13));
 assert((day > 0) && (day < 32)); /* obviously this doesn't always work */
 
 assert((hour >= 0) && (hour < 24));
 assert((minute >= 0) && (minute < 60));
 assert((second >= 0) && (second < 60));
 
 tm->tm_year = year - 1900;
 tm->tm_mon = month - 1;
 tm->tm_mday = day;
 tm->tm_hour = hour;
 tm->tm_min = minute;
 tm->tm_sec = second;
 
 atime = mktime(tm);

 return (atime);
}

char *disp_time_hour_min(time_t atime, int offset, int use_24_clock)
{
 char *tmp = DISPLAY_TIME_GET_STR();

 atime = disp_time_add(atime, offset);
 
 if (use_24_clock)
   strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%H:%M", gmtime(&atime));
 else
   strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%I:%M%p", gmtime(&atime));
 
 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
 
 return (tmp);
}

char *disp_time_std(time_t passed_time, int offset,
                    int use_24_clock, int override)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 time_t atime = disp_time_add(passed_time, offset);
 int diff_secs = abs(difftime(now, passed_time));
 
 if (use_24_clock)
 {
  if (override || (diff_secs > MK_HOURS(8)))
    if (override || (diff_secs > MK_DAYS(3)))
      if (override || (diff_secs > MK_WEEKS(13)))
        strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%a, %d %b %Y %H:%M:%S",
                 gmtime(&atime));
      else
        strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%a, %d %b %H:%M:%S", gmtime(&atime));
    else
      strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%a, %H:%M:%S", gmtime(&atime));
  else
    strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%H:%M:%S", gmtime(&atime));

  tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
 }
 else
 {
  int num = 0;
  
  if (override || (diff_secs > MK_HOURS(8)))
    if (override || (diff_secs > MK_DAYS(3)))
      if (override || (diff_secs > MK_WEEKS(13)))
        num = strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%a, %d %b %Y %I:%M:%S %p",
                       gmtime(&atime));
      else
        num = strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%a, %d %b %I:%M:%S %p",
                       gmtime(&atime));
    else
      num = strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%a, %I:%M:%S %p", gmtime(&atime));
  else
    num = strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%I:%M:%S %p", gmtime(&atime));

  tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
  
  if ((num > 0) && (num < (DISPLAY_TIME_STR_SZ - 1)) && (tmp[num - 3] == ' '))
    lower_case(tmp + num - 3); /* lower_case AM/PM */
 }

 return (tmp);
}

char *disp_date_std(time_t passed_time, int offset, int override)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 time_t atime = disp_time_add(passed_time, offset);
 int diff_secs = abs(difftime(now, passed_time));

 if (override || (diff_secs > MK_DAYS(3)))
   if (override || (diff_secs > MK_WEEKS(13)))
     strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%a, %d %B %Y",
              gmtime(&atime));
   else
     strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%a, %d %B", gmtime(&atime));
 else
   strftime(tmp, DISPLAY_TIME_STR_SZ - 1, "%a", gmtime(&atime));

 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
 
 return (tmp);
}

char *disp_date_birthday_string(time_t atime, int show_year)
{
 char *tmp = DISPLAY_TIME_GET_STR();
 struct tm *birth_tm = gmtime(&atime);
 
 if ((birth_tm->tm_mday > 10) && (birth_tm->tm_mday < 20))
   strftime(tmp, DISPLAY_TIME_STR_SZ - 1,
	    (show_year ? ("%dth of %B %Y") : ("%dth of %B")), birth_tm);
 else
   switch (birth_tm->tm_mday % 10)
   {
    case 1:
      strftime(tmp, DISPLAY_TIME_STR_SZ - 1,
               (show_year ? ("%dst of %B %Y") : ("%dst of %B")), birth_tm);
      break;
      
    case 2:
      strftime(tmp, DISPLAY_TIME_STR_SZ - 1,
               (show_year ? ("%dnd of %B %Y") : ("%dnd of %B")), birth_tm);
      break;
      
    case 3:
      strftime(tmp, DISPLAY_TIME_STR_SZ - 1,
               (show_year ? ("%drd of %B %Y") : ("%drd of %B")), birth_tm);
      break;
      
    default:
      strftime(tmp, DISPLAY_TIME_STR_SZ - 1,
               (show_year ? ("%dth of %B %Y") : ("%dth of %B")), birth_tm);
   }

 tmp[DISPLAY_TIME_STR_SZ - 1] = 0;
 
 return (tmp);
}

const char *disp_time_filename(time_t atime,
                               const char *start, const char *end)
{
 char *buf = DISPLAY_TIME_GET_STR();
 size_t len = 0;
 size_t tmp = 0;

 len = sprintf(buf, "%.*s", DISPLAY_TIME_STR_SZ, start);
 
 tmp = strftime(buf + len, DISPLAY_TIME_STR_SZ - len,
                "%Y-%m-%d_%H-%M-%S", gmtime(&atime));
 buf[DISPLAY_TIME_STR_SZ - 1] = 0;

 if ((tmp > 0) && ((tmp + len) < DISPLAY_TIME_STR_SZ))
   sprintf(buf + tmp + len, "%.*s",
           (int) (DISPLAY_TIME_STR_SZ - (len + tmp)), end);
 
 return (buf);
}
