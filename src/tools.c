#define TOOLS_C
/*
 *  Copyright (C) 1999 James Antill, John Tobin
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
 * email: james@twocan.org, john@twocan.org
 */
#include "main.h"

/* gets a decent reandom number from lbound to ubound, inclusive */
int get_random_num(int lbound, int ubound)
{
 int i = 0;
 int j = 0;
 int num = 0;

 i = RAND_MAX / ((ubound - lbound) + 1);
 i *= ((ubound - lbound) + 1);
 
 while ((j = rand()) >= i)
   continue;
 
 num = (j % i);

 return ((num % ((ubound - lbound) + 1)) + lbound);
}

/* sees if all of str1 is in str2, but str2 does not need to end there */
int beg_strcmp(const char *str1, const char *str2)
{ /* FIXME: is this a problem with non-ASCII archs ? */
 int cmp_sve = -1; /* saved compare value */
 
 if (!(*str1 || *str2))
   return (0);

 while (*str1 && !(cmp_sve = (*str1 - *str2)))
 {
  str1++;
  str2++;
 }

 return (cmp_sve);
}

/* non case sensitive of above */
/* sees if all of str1 is in str2, but str2 does not need to end there */
int beg_strcasecmp(const char *passed_str1, const char *passed_str2)
{
 unsigned const char *str1 = (unsigned const char *) passed_str1;
 unsigned const char *str2 = (unsigned const char *) passed_str2;
 int cmp_sve = -1; /* saved compare value */
 
 if (!(*str1 || *str2))
   return (0);

 while (*str1 && !(cmp_sve = (tolower(*str1) - tolower(*str2))))
 {
  ++str1;
  ++str2;
 }

 return (cmp_sve);
}

void lower_case(char *passed_str)
{
 unsigned char *str = (unsigned char *) passed_str;
 
 assert(str);
 
 while (*str)
 {
  *str = tolower(*str);
  ++str;
 }
}

void upper_case(char *passed_str)
{
 unsigned char *str = (unsigned char *) passed_str;
 
 assert(str);
 
 while (*str)
 {
  *str = toupper(*str);
  ++str;
 }
}

/* search for param and terminate */
char *next_parameter(char *str, char seperator)
{
 if ((str = N_strchr(str, seperator)))
   while (*(str + 1) == seperator)
     *str++ = 0;
 
 return (str);
}

/* like next_parameter but terminates ALL seperators */
char *next_parameter_no_seperators(char *str, char seperator)
{
 if ((str = N_strchr(str, seperator)))
   while (*str == seperator)
     *str++ = 0;
 
 return (str);
}

int skip_atoi(const char **input)
{
 int tmp = 0;
 
 while (isdigit((unsigned char) **input))
   tmp = (tmp * 10) + TONUMB(*((*input)++));
 
 return (tmp);
}

long int skip_atol(const char **input)
{
 long int tmp = 0;
 
 while (isdigit((unsigned char) **input))
   tmp = (tmp * 10) + TONUMB(*((*input)++));
 
 return (tmp);
}

char *skip_chars(char *str, int the_char)
{
 while (*str && (*str == the_char))
   ++str;

 return (str);
}

/* counts the number of flags set on an int */
int number_of_flags(int int_to_check)
{
 int count = 0;

 while (int_to_check)
 {
  int tmp = (int_to_check & -int_to_check);
  
  ++count;
  int_to_check &= ~tmp;
 }

 return (count);
}

/* gives the number of the highest bit set */
unsigned int highest_powerof_2(unsigned int numb)
{
 /* should be 16 for linux */
 unsigned int count = CHAR_BIT * (sizeof(unsigned int) >> 1);
 unsigned int after = 0;
   
 if (!numb)
   return (0); /* NOTE: the numbers 0 and 1 will return the same result */

 after = (numb >> count);
 while (after != 1)
 {
  if (after) /* count is too big */
    ++count;
  else
    --count;

  after = (numb >> count);
 }

 return (count);
}

/* NOTE: at 4000 seconds (66 mins) we just return ULONG_MAX...
 * obviously only works one way */
unsigned long timeval_diff_time(struct timeval *end, struct timeval *start)
{
 unsigned long sched_useconds = end->tv_usec;
 
 assert(start->tv_usec < 1000000); /* less than 1 second */
 assert(end->tv_usec < 1000000);
 
 if (end->tv_sec != start->tv_sec)
 {
  int sched_seconds = (end->tv_sec - start->tv_sec - 1);
  assert(sched_seconds >= 0);
  if ((sched_seconds > 4000) || (sched_seconds < 0))
    return (ULONG_MAX);
  else
    sched_useconds += ((1000000 * sched_seconds) + (1000000 - start->tv_usec));
 }
 else
   sched_useconds -= start->tv_usec;

 return (sched_useconds);
}

void timeval_add_useconds(struct timeval *tv, unsigned long useconds)
{
 assert(tv && useconds);

 if (!useconds)
   return;
 
 tv->tv_sec += useconds / 1000000;
 tv->tv_usec += useconds % 1000000;
 
 if (tv->tv_usec)
 {
  tv->tv_sec += (tv->tv_usec / 1000000);
  tv->tv_usec %= 1000000;
 }
}

char *file2text(const char *filename, size_t *len)
{
 struct stat file_info;
 int fd = 0;
 char *text = NULL;
 size_t dummy;

 if (!len)
   len = &dummy;
 
 if ((stat(filename, &file_info) == -1) || !S_ISREG(file_info.st_mode) ||
     ((fd = open(filename, O_RDONLY)) == -1))
   return (NULL);
 
 *len = file_info.st_size;
 text = MALLOC(*len + sizeof(char));
 
 if ((size_t)read(fd, text, *len) != *len)
 {
  log_assert(FALSE);
  close(fd);
  return (NULL);
 }
 
 close(fd);
 
 text[*len] = 0;
 return (text);
}

#ifdef TALKER_MAIN_H
void ptell_left(player_tree_node *from, player *p, twinkle_info *info,
                int flags, time_t timestamp, const char *msg, int change_first)
{
 int term_size = (p->term_width ? p->term_width : 79) - 8;
 output_node *buffer_msg = NULL;
 twinkle_info local_info;
 tmp_output_list_storage tmp_save;

 if (!info)
 {
  info = &local_info;
  setup_twinkle_info(info);
 }

 save_tmp_output_list(p, &tmp_save);
 if (p->see_raw_twinkles)
   term_size = INT_MAX;
 
 fvtell_player(ALL_T(from, p, info, flags | OUTPUT_BUFFER_TMP, timestamp),
               "%.*s^N", term_size, msg);
 buffer_msg = output_list_grab(p);
 load_tmp_output_list(p, &tmp_save);
 /* should be no returns in it -- wrapping not done on buffer_tmp stuff */
 assert(!output_list_lines(p, buffer_msg, 1, flags, INT_MAX));
 
 fvtell_player(ALL_T(from, p, info, flags, timestamp), "%c%s",
               (info->used_twinkles && change_first) ? '$' : '-', "- ");
 
 output_list_linkin(p, flags, &buffer_msg, INT_MAX);
  
 fvtell_player(ALL_T(from, p, info,
                     (flags | OVERRIDE_RAW_OUTPUT_VARIABLES) &
                     ~RAW_OUTPUT_VARIABLES,
                     timestamp), " %s", DASH_LEN);
}

void ptell_mid(player_tree_node *from, player *p, twinkle_info *info,
               int flags, time_t timestamp, const char *msg, int change_first)
{
 int term_size = (p->term_width ? p->term_width : 79) - 8;
 int msg_size = 0;
 output_node *buffer_msg = NULL;
 twinkle_info local_info;
 tmp_output_list_storage tmp_save;

 if (!info)
 {
  info = &local_info;
  setup_twinkle_info(info);
 }

 if (p->see_raw_twinkles)
   term_size = INT_MAX;
 
 save_tmp_output_list(p, &tmp_save);
 fvtell_player(ALL_T(from, p, info, flags | OUTPUT_BUFFER_TMP, timestamp),
               "%.*s^N", term_size, msg);
 buffer_msg = output_list_grab(p);
 load_tmp_output_list(p, &tmp_save);
 /* should be no returns in it */
 assert(!output_list_lines(p, buffer_msg, 1, flags, INT_MAX));
 
 msg_size = output_list_print_length(p, buffer_msg, flags, INT_MAX);
 
 if (p->see_raw_twinkles && (term_size < msg_size))
 { /* this isn't guaranteed to be nice */
  fvtell_player(ALL_T(from, p, info, flags, timestamp), "%s", "-- ");

  output_list_linkin(p, flags, &buffer_msg, INT_MAX);
  
  fvtell_player(ALL_T(from, p, info,
                      (flags | OVERRIDE_RAW_OUTPUT_VARIABLES) &
                      ~RAW_OUTPUT_VARIABLES,
                      timestamp), " %s", DASH_LEN);
 }
 else
 {
  output_node *buffer_lines = NULL;
  output_node *buffer_all = NULL;

  save_tmp_output_list(p, &tmp_save);
  fvtell_player(ALL_T(from, p, info,
                      (flags | OVERRIDE_RAW_OUTPUT_VARIABLES |
                       OUTPUT_BUFFER_TMP) & ~ RAW_OUTPUT_VARIABLES,
                      timestamp),
		"%c%s", (info->used_twinkles && change_first) ? '$' : '-',
                DASH_LEN);
  buffer_lines = output_list_grab(p);
  load_tmp_output_list(p, &tmp_save);
  assert(!p->column);
  
  buffer_all = output_list_merge(p, &buffer_lines, &buffer_msg, TRUE, 57);
  output_list_cleanup(&buffer_msg);
  output_list_cleanup(&buffer_lines);
  output_list_linkin(p, flags, &buffer_all, INT_MAX);
 }
}

const char *gender_choose_str(int the_gender,
                              const char *male, const char *female,
                              const char *plural, const char *other)
{
 switch (the_gender)
 {
  case GENDER_MALE:   return (male);
  case GENDER_FEMALE: return (female);    
  case GENDER_PLURAL: return (plural);
  case GENDER_OTHER:  return (other);
  
  default:
    assert(FALSE);
    return ("** ERROR **");
 }
}

const char *isits1(const char *str)
{
 assert(str);
 
 /* Is the first letter alphanumeric? If so, return with a space */
 if ((*str == '$') || (*str == '^') || isalnum((unsigned char) *str))
   return (" ");

 /* Is the first char a \ and the second not alphanumeric?
    If so, make the '\' into a ' ' and return nothing */
 if ((*str == '\\') && !isalnum((unsigned char) *(str + 1)))
   return (" ");

 return ("");
}

const char *isits2(const char *str)
{
 assert(str);
 
 /* Is the first letter alphanumeric? If so, return with a space */
 if ((*str == '$') || (*str == '^') || isalnum((unsigned char) *str))
   return (str);

 /* Is the first char a \ and the second not alphanumeric?
    If so, make the '\' into a ' ' and return nothing */
 if ((*str == '\\') && !isalnum((unsigned char) *(str + 1)))
   return (str + 1);

 return (str);
}

const char *get_nationality(player *p, int caps)
{
 switch (p->nationality)
 {
  case NATIONALITY_VOID:
    if (caps)
      return ("Void");
    else
      return ("oid");
    
  case NATIONALITY_BRITISH:
    if (caps)
      return ("British");
    else
      return ("british");
    
  case NATIONALITY_AMERICAN:
    if (caps)
      return ("American");
    else
      return ("american");
    
  case NATIONALITY_CANADIAN:
    if (caps)
      return ("Canadian");
    else
      return ("canadian");
    
  case NATIONALITY_AUSTRALIAN:
    if (caps)
      return ("Australian");
    else
      return ("australian");
    
  case NATIONALITY_OTHER:
    if (caps)
      return ("Other");
    else
      return ("other");
    
  default:
    assert(FALSE);
 }
 return ("**** ERROR ****");
}

player_linked_list *do_cronorder_logged_on(int (*func) (player *, va_list),
                                           ...)
{
 DO_BUILD_ORDER_ON(func, func, player_list_cron_start(), TRUE,
                   (PLAYER_LINK_GET(scan), ap));
}

player_linked_list *do_inorder_logged_on(int (*func) (player *, va_list), ...)
{
 DO_BUILD_ORDER_ON(func, func, player_list_alpha_start(), TRUE,
                   (PLAYER_LINK_GET(scan), ap));
}

player_linked_list *do_order_misc_on(int (*func) (player_linked_list *,
                                                  va_list),
                                     player_linked_list *offset, ...)
{
 DO_BUILD_ORDER_ON(func, offset, offset, TRUE, (scan, ap));
}

player_linked_list *do_order_misc_on_all(int (*func) (player_linked_list *,
                                                      va_list),
                                         player_linked_list *offset, ...)
{
 DO_BUILD_ORDER_ON_ALL(func, offset, offset, TRUE, (scan, ap));
}

player_linked_list *do_order_misc_all(int (*func) (player_linked_list *,
                                                   va_list),
                                      player_linked_list *offset, ...)
{
 DO_BUILD_ORDER_ALL(func, offset, offset, TRUE, (scan, ap));
}

static int internal_find_player_count_p(player *scan, va_list ap)
{
 int *count = va_arg(ap, int *);
 player_tree_node **ret_value = va_arg(ap, player_tree_node **);
 
 if (*count < 1)
   *ret_value = scan->saved;
 else
 {
  --*count;
  return (TRUE);
 }
 
 return (FALSE);
}

static int internal_find_player_count(player_linked_list *passed_scan,
                                      va_list ap)
{
 player *scan = PLAYER_LINK_GET(passed_scan);

 return (internal_find_player_count_p(scan, ap));
}

player_tree_node *find_player_loggedon_count(int check_do_inorder, int count)
{
 static struct timeval timestamp = {0, 0};
 static int last_count = 0;
 static int last_order = -1;
 static player_linked_list *cache_value = NULL;
 player_tree_node *ret_value = NULL;

 if (count <= 0)
   return (NULL);

 --count;
 
 if ((count >= last_count) &&
     timercmp(&timestamp, &last_entered_left, ==) && cache_value &&
     (check_do_inorder == last_order))
 {
  int use_count = count;
  
  timestamp = last_entered_left;
  
  if ((use_count -= last_count))
    cache_value = do_order_misc_on(internal_find_player_count, cache_value,
                                   &use_count, &ret_value);
  else
    ret_value = PLAYER_LINK_SAV_GET(cache_value);
  
  last_count = count;
 }
 else
 {
  timestamp = last_entered_left;
  last_count = count;
  last_order = check_do_inorder;
  cache_value = DO_ORDER_TEST(logged_on, check_do_inorder, in, cron,
                              (internal_find_player_count_p,
                               &count, &ret_value));
 }
 
 return (ret_value);
}

player_tree_node *find_player_room_count(int check_do_inorder,
                                         room *the_room, int count)
{
 static struct timeval timestamp = {0, 0};
 static int last_count = 0;
 static int last_order = -1;
 static player_linked_list *cache_value = NULL;
 player_tree_node *ret_value = NULL;

 if (count <= 0)
   return (NULL);

 --count;
 
 if ((count >= last_count) &&
     timercmp(&timestamp, &the_room->last_entered_left, ==) && cache_value &&
     (last_order == check_do_inorder))
 {
  int use_count = count;
  
  timestamp = the_room->last_entered_left;
  
  if ((use_count -= last_count))
    cache_value = do_order_misc_on(internal_find_player_count, cache_value,
                                   &use_count, &ret_value);
  else
    ret_value = PLAYER_LINK_SAV_GET(cache_value);
  
  last_count = count;
 }
 else
 {
  timestamp = the_room->last_entered_left;
  last_count = count;
  last_order = check_do_inorder;
  cache_value = DO_ORDER_TEST(room, check_do_inorder, in, cron,
                              (internal_find_player_count_p, the_room,
                               &count, &ret_value));
 }
 
 return (ret_value);
}

static int inorder_check_su_for_duty(player *check, va_list ap)
{
 int *count_of_sus = va_arg(ap, int *);
 int on_or_off = va_arg(ap, int);

 if (PRIV_STAFF(check->saved))
 {
  if (on_or_off)
  { /* checking for on */
   if (!check->flag_tmp_su_channel_block && !check->flag_tmp_su_channel_off)
     (*count_of_sus)++;
  }
  else
  { /* checking for off */
   if (check->flag_tmp_su_channel_block || check->flag_tmp_su_channel_off)
     (*count_of_sus)++;
  }
 }
 
 return (TRUE);
}

/* counts the sus on or off duity - on duty if on_or_off == TRUE */
int count_sus_on_or_off_duty(int on_or_off)
{
 int count_of_sus = 0;

 do_inorder_logged_on(inorder_check_su_for_duty, &count_of_sus, on_or_off);

 return (count_of_sus);
}

int real_total_logon_time(player_tree_node *entry)
{
 int total_logon_time = entry->total_logon - entry->total_idle_logon;

 if (P_IS_ON(entry))
 {
  int idle_time = difftime(now, entry->player_ptr->last_command_timestamp);
  
  total_logon_time += difftime(now, entry->player_ptr->logon_timestamp);
  total_logon_time -= entry->player_ptr->idle_logon;
  if (idle_time > IDLE_TIME_PERMITTED)
    total_logon_time -= idle_time;
 }

 if (total_logon_time < 0)
   return (0);
 
 return (total_logon_time);
}

int real_age(player *p)
{
 if (p->birthday && p->flag_use_birthday_as_age)
 {
  time_t time_t_tmp = p->birthday;
  struct tm *birthday_tm = gmtime(&time_t_tmp);
  int birth_year = birthday_tm->tm_year;
  int birth_yday = birthday_tm->tm_yday;
  struct tm *now_tm = gmtime(&now);
  int years = (now_tm->tm_year - birth_year) - (now_tm->tm_yday < birth_yday);

  return (years);
 }
 
 return (p->age);
}

static int internal_sys_wall(player *scan, unsigned int flags, const char *fmt,
                             va_list ap)
{ 
 vfvtell_player(ALL_T(NULL, scan, NULL, 3 | OVERRIDE_RAW_OUTPUT_VARIABLES |
                      SYSTEM_INFO | flags, 0), fmt, ap);
 
 return (TRUE);
}

player_linked_list *sys_wall(unsigned int flags, const char *fmt, ...)
{
 DO_BUILD_ORDER_FMT(internal_sys_wall, NULL, fmt, fmt,
                    player_list_alpha_start(), (TRUE),
                    (PLAYER_LINK_GET(scan), flags, fmt, ap));
}

const char *say_ask_exclaim_me(player *p, const char *str, size_t length)
{
 IGNORE_PARAMETER(p);
 
 if (!length)
   ++length;

 switch (str[length - 1])
 {
  case '?':
    return ("ask");
    
  case '!':
    return ("exclaim");
    
  default:
    break;
 }

 return ("say");
}

const char *say_ask_exclaim_group(player *p, const char *str, size_t length)
{
 if (p->gender == GENDER_PLURAL)
   return (say_ask_exclaim_me(p, str, length));
 
 if (!length)
   ++length;
 
 switch (str[length - 1])
 {
  case '?':
    return ("asks");
    
  case '!':
    return ("exclaims");
    
  default:
    break;
 }
 
 return ("says");
}

#endif
