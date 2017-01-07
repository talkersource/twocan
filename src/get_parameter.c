#define GET_PARAMETER_C
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

void get_parameter_init(parameter_holder *parameters)
{
 unsigned int scan = 0;
 
 while (scan < GET_PARAMETER_NUMBER_MAX)
   parameters->params[scan++].length = 0;
 
 parameters->last_param = 0;
 parameters->buffer_used = 0;
 parameters->buffer[0] = 0;
 parameters->buffer_size = GET_PARAMETER_BUFFER_DEFAULT_SZ;
}

parameter_holder *get_parameter_create(size_t buffer_add)
{
 parameter_holder *tmp = MALLOC(sizeof(parameter_holder) + buffer_add);

 if (!tmp)
   return (NULL);

 get_parameter_init(tmp);

 tmp->buffer_size = GET_PARAMETER_BUFFER_DEFAULT_SZ + buffer_add;

 return (tmp);
}

int get_parameter_parse(parameter_holder *parameters,
                        const char **str, unsigned int upto_param)
{
 assert(parameters && str);
 
 if ((upto_param > GET_PARAMETER_NUMBER_MAX) || (upto_param < 1))
   return (FALSE);
 
 while (parameters->last_param < upto_param)
 {
  *str += strspn(*str, " ");
  if (!**str || (GET_PARAMETER_BUF_UNUSED(parameters) < 1))
    return (FALSE);

  assert(parameters->last_param < GET_PARAMETER_NUMBER_MAX);
  ++parameters->last_param;
  
  assert(!*(GET_PARAMETER_BUF_PTR(parameters)));
  GET_PARAMETER_LAST_STR(parameters) = GET_PARAMETER_BUF_PTR(parameters);
  
  while (**str && (**str != ' ') && (GET_PARAMETER_BUF_UNUSED(parameters) > 1))
    switch (**str)
    {
     case '\'':
       ++*str;
       PARSE_TO_NEXT_TOKENS("\'");
       /* skip the ' too, if it's there */
       if (**str)
         ++*str; 
       break;
       
     case '\"':
       ++*str;
       PARSE_TO_NEXT_TOKENS("\"");
       /* skip the " too, if it's there */
       if (**str)
         ++*str; 
       break;
       
     default:
       PARSE_TO_NEXT_TOKENS(" \'\"");
    }

  assert(GET_PARAMETER_BUF_PTR(parameters) ==
         GET_PARAMETER_LAST_CUR(parameters));

  assert(GET_PARAMETER_BUF_UNUSED(parameters));

  *GET_PARAMETER_LAST_CUR(parameters) = 0;
  ++parameters->buffer_used; /* skip the 0 */
  
  if (GET_PARAMETER_BUF_UNUSED(parameters))
    *(GET_PARAMETER_LAST_CUR(parameters) + 1) = 0;
 }

 *str += strspn(*str, " ");
 
 return (TRUE);
}

void get_parameter_shift(parameter_holder *parameters, unsigned int shift_num)
{
 unsigned int count = 0;
 
 assert(shift_num);
 
 if (shift_num > parameters->last_param)
   shift_num = parameters->last_param;

 parameters->last_param -= shift_num;
 
 while (count < parameters->last_param)
 {
  parameters->params[count] = parameters->params[count + shift_num];
  ++count;
 }
 assert(count == parameters->last_param);
 
 count = 0;
 while (count < shift_num)
 {
  parameters->params[parameters->last_param + count].length = 0;
  ++count;
 }
}
