#define WORD_NUMBER_C
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

/* prints numbers as words */
static size_t internal_word_number(const char **words, char *str,
                                   size_t passed_len,
                                   long num, int capitalise)
{
 long nummod10 = 0;
 long numdiv100 = 0;
 long numdiv1000 = 0;
 long numdiv1000000 = 0;
 long numdiv1000000000 = 0;
 size_t length = 0;
 char *tmp = str;
 ssize_t len = passed_len;
 
 if (passed_len < 4)
 {
  *tmp = 0;
  return (0);
 }
 --len; /* for the zero */

 if (passed_len > LONG_MAX)
   goto output_full;
 
 if (num < 0)
 {
  tmp += sprintf(tmp, "%.*s ", (int)len, words[WORD_NUMBER_NEGATIVE]);
  if (capitalise)
  {
   *str = toupper((unsigned char) *str);
   capitalise = 0;
  }

  num = -num;
  assert(num == abs(num));
 }
 if (num < 100)
   goto small_numbers_only;

 WORD_NUMBER_DO(1000000000, WORD_NUMBER_BILLION);
 WORD_NUMBER_DO(1000000, WORD_NUMBER_MILLION);
 WORD_NUMBER_DO(1000, WORD_NUMBER_THOUSAND);

 if ((numdiv100 = (num / 100)))
 { /* because of thousand being before this will only be 1-9 */
  length = sprintf(tmp, "%.*s",
                   (int)(len - (tmp - str)), words[numdiv100]);

  if (capitalise)
  {
   capitalise = 0;
   *tmp = toupper((unsigned char) *tmp);
  }
  tmp += length;

  if ((len - (tmp - str)) <= 1)
    goto output_full;

  tmp += sprintf(tmp, " %.*s", (int)(len - (tmp - str) - 1),
                   words[WORD_NUMBER_HUNDRED]);
  
  if ((len - (tmp - str)) <= 0)
    goto output_full;

  num -= (numdiv100 * 100);
  if (!num)
    return (tmp - str);
  else if ((len - (tmp - str)) <= (int)CONST_STRLEN(" and "))
    goto output_full;
  
  tmp = qstrcpy(tmp, " and ");
 }

 small_numbers_only:
 /* it is now a numb from 0-99 */
 if (num > 20)
   if ((nummod10 = (num % 10)))
   {
    length = sprintf(tmp, "%.*s", (int)(len - (tmp - str)), 
                     words[18 + (num / 10)]);
    
    if ((len - ((tmp + length) - str)) <= 1)
      goto output_full;
    
    length += sprintf(tmp + length, " %.*s",
                      (int)(len - ((tmp + length) - str) - 1), 
                      words[nummod10]);
   }
   else
     length = sprintf(tmp, "%.*s", (int)(len - (tmp - str)),
                      words[18 + (num / 10)]);
 else
   length = sprintf(tmp, "%.*s", (int)(len - (tmp - str)),
                    words[num]);

 if ((len - ((tmp + length) - str)) <= 0)
   goto output_full;
 
 if (capitalise)
   *tmp = toupper((unsigned char) *tmp);
 
 tmp += length;
 
 return (tmp - str);

 output_full:
 {
  assert((len - (tmp - str)) >= 0);
  assert((tmp - str) > 3);
  
  tmp[0] = 0;
  tmp[-1] = '.';
  tmp[-2] = '.';
  tmp[-3] = '.';
  
  return (tmp - str);
 }
}

size_t word_number_def(char *str, size_t len, long num, int capitalise)
{
 const char *words[WORD_NUMBER_WORDS_SZ] =
 {
  "zero",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine",
  "ten",
  "eleven",
  "twelve",
  "thirteen",
  "fourteen",
  "fifteen",
  "sixteen",
  "seventeen",
  "eighteen",
  "nineteen",
  "twenty",
  "thirty",
  "forty",
  "fifty",
  "sixty",
  "seventy",
  "eighty",
  "ninety",
  "hundred",
  "thousand",
  "million",
  "billion",
  "negative",
 };

 return (internal_word_number(words, str, len, num, capitalise));
}

size_t word_number_times(char *str, size_t len, long num, int capitalise)
{
 if (len < 8)
 {
  *str = 0;
  return (0);
 }
 
 switch (num)
 {
  case 1:
    CONST_COPY_STR_LEN(str, "once");
    return (CONST_STRLEN("once"));
      
  case 2:
    CONST_COPY_STR_LEN(str, "twice");
    return (CONST_STRLEN("twice"));

  default:
  {
   size_t ret = word_number_def(str, len - CONST_STRLEN(" times"),
                                num, capitalise);
   if (*str && (str[ret - 1] != '.'))
   {
    memcpy(str + ret, " times", CONST_STRLEN(" times") + 1);
    return (ret + CONST_STRLEN(" times"));
   }
   else
     return (ret);
  }
 }

 assert(FALSE);
 *str = 0;
 return (0);
}

size_t word_number_th(char *str, size_t len, long num, int capitalise)
{
 const char *words[] =
 {
  "no", /* ! */
  "first",
  "second",
  "third",
  "fourth",
  "fifth",
  "sixth",
  "seventh",
  "eigth",
  "ninth",
  "tenth",
  "eleventh",
  "twelfth",
  "thirteenth",
  "fourteenth",
  "fifteenth",
  "sixteenth",
  "seventeenth",
  "eigthteenth",
  "nineteenth",
  "twentieth",
  "thirtieth",
  "fourtieth",
  "fiftieth",
  "sixtieth",
  "seventieth",
  "eightieth",
  "ninetieth",
  "hundredth",
  "thousandth",
  "millionth",
  "billionth",
  "negative",
 };
 

 return (internal_word_number(words, str, len, num, capitalise));
}

extern char *word_number_base(char *buf, size_t len, size_t *ret_len,
                              long num, int capitalise,
                              size_t (*func)(char *, size_t, long, int))
{
 size_t dummy_ret_len;

 if (!ret_len)
   ret_len = &dummy_ret_len;
 
 *ret_len = (*func)(buf, len, num, capitalise);
 
 return (buf);
}
