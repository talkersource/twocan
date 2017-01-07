#ifndef EXTERN_TOOLS_H
#define EXTERN_TOOLS_H
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

/* This file also contains useful macros! */

/* are the saved bits of the player structure available... */
#define P_IS_AVL(x) ((x)->player_ptr && (x)->player_ptr->loaded_player)
/* is the person actually logged on */
#define P_IS_ON(x) ((x)->player_ptr && (x)->player_ptr->is_fully_on)
/* is the person actually logged on */
#define P_IS_ON_P(x, p) (((p) == (x)->player_ptr) && (p)->is_fully_on)

#define P_SHOW_FLAG(p, p2_or_p2_saved, flags, x, y) do { \
 if (p2_or_p2_saved->flag_ ## x) { \
  fvtell_player(NORMAL_WFT(flags, p), "%s%s", done ? ", " : "", y); \
  done = TRUE; } } while (FALSE)

#define SWAP_TYPE(x, y, type) do { \
 type internal_local_tmp = (x); \
 (x) = (y); \
 (y) = internal_local_tmp; \
 } while (FALSE)

#define BUF_NUM_TYPE_SZ(x) ((sizeof(x) * CHAR_BIT) + 1)

#define C_RET1_P2(func, type, x, y)    ((const type *) (func (x, y)))
#define N_RET1_P2(func, type, x, y)    (func ((type *)x, y))
#define C_RET1_P3(func, type, x, y, z) ((const type *) (func (x, y, z)))
#define N_RET1_P3(func, type, x, y, z) (func ((type *)x, y, z))

#define C_strchr(x, y)     C_RET1_P2(strchr, char, x, y)
#define N_strchr(x, y)     N_RET1_P2(strchr, char, x, y)
#define C_strrchr(x, y)    C_RET1_P2(strrchr, char, x, y)
#define N_strrchr(x, y)    N_RET1_P2(strrchr, char, x, y)
#define C_memchr(x, y, z)  C_RET1_P3(memchr, void, x, y, z)
#define N_memchr(x, y, z)  N_RET1_P3(memchr, void, x, y, z)
#define C_strnchr(x, y, z) C_RET1_P3(strnchr, char, x, y, z)
#define N_strnchr(x, y, z) N_RET1_P3(strnchr, char, x, y, z)
#define C_strstr(x, y)     C_RET1_P2(strstr, char, x, y)
#define N_strstr(x, y)     N_RET1_P2(strstr, char, x, y)

#define CHOOSE_CONST_OFFSET(map) ((map)[(rand() % \
 ((sizeof(map) / sizeof((map)[0])) - 1))])

#define BUILD_FILE_ALPHA_HASH(x) do { \
 struct stat buf; \
 \
 if (stat(x, &buf) == -1) \
 { \
  int first = 0; \
  \
  if (mkdir(x, 0700) == -1) \
    shutdown_error("mkdir: %d %s.\n", errno, strerror(errno)); \
  \
  while (first < 26) \
  { \
   char buffer[sizeof(x "/%c/%c")]; \
   int second = 0; \
   \
   sprintf(buffer, x "/%c", ALPHA_LOWER_CHAR(first)); \
   \
   if (mkdir(buffer, 0700)) \
     shutdown_error("mkdir: %d %s.\n", errno, strerror(errno)); \
   \
   while (second < 26) \
   { \
    sprintf(buffer, x "/%c/%c", \
            ALPHA_LOWER_CHAR(first), ALPHA_LOWER_CHAR(second)); \
    \
    if (mkdir(buffer, 0700) == -1) \
      shutdown_error("mkdir: %d %s.\n", errno, strerror(errno)); \
    ++second; \
   } \
   ++first; \
  } \
 } } while (FALSE)

#ifndef NDEBUG
# define DEBUG_PRINT(x, y) do { \
  if (x) fprintf(stderr, "%d %s -- %s\n", __LINE__, __FILE__, y); \
 } while (FALSE)
# define DEBUG_START(x) do { \
   if (x) { fprintf(stderr, "%d %s -- ", __LINE__, __FILE__)
# define DEBUG_END() \
            fprintf(stderr, " -- %d\n", __LINE__); } \
   } while (FALSE)
#else
# define DEBUG_PRINT(x, y) IGNORE_PARAMETER((x) && (y))
# define DEBUG_START(x) do { if (0) { IGNORE_PARAMETER(x)
# define DEBUG_END() } } while (FALSE)
#endif

#ifdef NWARNS
# define IGNORE_PARAMETER(x)
#else
# define IGNORE_PARAMETER(x) while ((x) && FALSE) break
#endif

#define DO_BUILD_ORDER_FMT(func, from, last_p, fmt, start, test, args) do { \
 VA_R_DECL(r_ap); \
 player_linked_list *scan = start; \
 \
 VA_R_START(r_ap, last_p); \
 while (scan) \
 { \
  player_linked_list *scan_next = PLAYER_LINK_NEXT(scan); \
  \
  assert((scan_next && scan_next->has_prev) ? \
         (PLAYER_LINK_PREV(scan_next) == scan) : TRUE); \
  assert((scan->has_prev && PLAYER_LINK_PREV(scan)) ? \
         (PLAYER_LINK_NEXT(PLAYER_LINK_PREV(scan)) == scan) : TRUE); \
  \
  if (PLAYER_LINK_SAV_GET(scan) && PLAYER_LINK_GET(scan) && \
      P_IS_ON_P(PLAYER_LINK_SAV_GET(scan), PLAYER_LINK_GET(scan)) && (test)) \
  { \
   VA_C_DECL(ap); \
   int carry_on = FALSE; \
   \
   VA_C_START(ap, last_p); \
   \
   VA_C_COPY(ap, r_ap); \
   \
   carry_on = (*func) args; \
   \
   VA_C_END(ap); \
   \
   if (!carry_on) { VA_R_END(r_ap); return (scan); } \
  } \
  \
  scan = scan_next; \
 } \
 VA_R_END(r_ap); \
 \
 return (NULL); \
 \
} while (FALSE)

#define DO_BUILD_ORDER_ON_ALL(func, last_p, start, test, args) do { \
 player_linked_list *scan = start; \
 VA_R_DECL(r_ap); \
 \
 VA_R_START(r_ap, last_p); \
 while (scan) \
 { \
  player_linked_list *scan_next = PLAYER_LINK_NEXT(scan); \
  \
  assert((scan_next && scan_next->has_prev) ? \
         (PLAYER_LINK_PREV(scan_next) == scan) : TRUE); \
  assert((scan->has_prev && PLAYER_LINK_PREV(scan)) ? \
         (PLAYER_LINK_NEXT(PLAYER_LINK_PREV(scan)) == scan) : TRUE); \
  \
  if (test) \
  { \
   VA_C_DECL(ap); \
   int carry_on = FALSE; \
   \
   VA_C_START(ap, last_p); \
   \
   VA_C_COPY(ap, r_ap); \
   \
   carry_on = (*func) args; \
   \
   VA_C_END(ap); \
   \
   if (!carry_on) { VA_R_END(r_ap); return (scan); } \
  } \
  \
  scan = scan_next; \
 } \
 VA_R_END(r_ap); \
 \
 return (NULL); \
 \
} while (FALSE)

#define DO_BUILD_ORDER_ON(func, last_p, start, test, args) \
 DO_BUILD_ORDER_ON_ALL(func, last_p, start, \
  (PLAYER_LINK_SAV_GET(scan) && PLAYER_LINK_GET(scan) && \
   P_IS_ON_P(PLAYER_LINK_SAV_GET(scan), PLAYER_LINK_GET(scan)) && (test)),args)

#define DO_BUILD_ORDER_ALL(func, last_p, start, test, args) do { \
 player_linked_list *scan = start; \
 VA_R_DECL(r_ap); \
 \
 VA_R_START(r_ap, last_p); \
 while (scan) \
 { \
  player_linked_list *scan_next = PLAYER_LINK_NEXT(scan); \
  \
  assert((scan_next && scan_next->has_prev) ? \
         (PLAYER_LINK_PREV(scan_next) == scan) : TRUE); \
  assert((scan->has_prev && PLAYER_LINK_PREV(scan)) ? \
         (PLAYER_LINK_NEXT(PLAYER_LINK_PREV(scan)) == scan) : TRUE); \
  \
  if (test) \
  { \
   VA_C_DECL(ap); \
   int carry_on = FALSE; \
   \
   VA_C_START(ap, last_p); \
   \
   VA_C_COPY(ap, r_ap); \
   \
   carry_on = (*func) args; \
   \
   VA_C_END(ap); \
   \
   if (!carry_on) { VA_R_END(r_ap); return (scan); } \
  } \
  \
  scan = scan_next; \
 } \
 VA_R_END(r_ap); \
 \
 return (NULL); \
 \
} while (FALSE)

#define DO_ORDER_TEST(type_after, t, func_true, func_false, params) \
 ((t) ? do_ ## func_true ## order_ ## type_after : \
   do_ ## func_false ## order_ ## type_after)  params

/* time defines */
#define MK_TIME_MINUTE (60)
#define MK_TIME_HOUR (MK_TIME_MINUTE * 60)
#define MK_TIME_DAY (MK_TIME_HOUR * 24)
#define MK_TIME_WEEK (MK_TIME_DAY * 7)
#define MK_TIME_YEAR (MK_TIME_WEEK * 52) /* we'll pretend this is correct */

/* useful time macros */
#define MK_MINUTES(n) (MK_TIME_MINUTE * (n))
#define MK_HOURS(n) (MK_TIME_HOUR * (n))
#define MK_DAYS(n) (MK_TIME_DAY * (n))
#define MK_WEEKS(n) (MK_TIME_WEEK * (n))
#define MK_YEARS(n) (MK_TIME_YEAR * (n))

/* useful string constants */
#define ALPHABET_LOWER "abcdefghijklmnopqrstuvwxyz"
#define ALPHABET_UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#ifdef NOT_HAVE_ASCII
# define ALPHA_LOWER_OFFSET(x) (strchr(ALPHABET_LOWER, \
                                       tolower((unsigned char) (x))) - \
                                ALPHABET_LOWER)
# define ALPHA_UPPER_OFFSET(x) (strchr(ALPHABET_UPPER, \
                                       toupper((unsigned char) (x))) - \
                                ALPHABET_UPPER)
# define ALPHA_LOWER_CHAR(x) (ALPHABET_LOWER[x])
# define ALPHA_UPPER_CHAR(x) (ALPHABET_UPPER[x])
#else /* using ascii */
# define ALPHA_LOWER_OFFSET(x) (tolower(((unsigned char) (x)) - 'a'))
   
# define ALPHA_UPPER_OFFSET(x) (toupper(((unsigned char) (x)) - 'A'))
# define ALPHA_LOWER_CHAR(x) ((x) + 'a')
# define ALPHA_UPPER_CHAR(x) ((x) + 'A')
#endif

/* given a char cnovert it to an int, number */
#define TONUMB(x) ((int) ((x) - '0'))

#define CONST_STRLEN(x) (sizeof(x) - 1)

/* can be changed to use players int */
# define DEF_TERM_WIDTH 80

/* checks x then returns either nothing or the string you pass.
   Used in large tell_players etc when working out loads of strings */
#define USE_STRING(x, y) ((x) ? (y) : (""))

/* MUST be a constant format string */
#define ADD_COMMA_FRONT(x, y) "%s"y, ((x) ? ", " : "")

/* a line for use finger etc... same length as a pstr_len func return */
#define DASH_LEN "$Line_fill(-)\n"
   
#define TELL_FORMAT_NO_RETURN(p, y) \
      fvtell_player(SYSTEM_T(p), " Format: %s%s%s " y "\n", \
                    USE_STRING(current_command, current_command), \
                    USE_STRING(current_command && current_sub_command, " "), \
                    USE_STRING(current_sub_command, current_sub_command))

#define TELL_FORMAT(p, y) do { \
        TELL_FORMAT_NO_RETURN(p, y); return; } while (FALSE)

#define CHECK_DUTY(p) do { \
 if ((p)->flag_tmp_su_channel_block) \
 { fvtell_player(SYSTEM_T(p), " You are currently ^S^Boff^s duty.\n" \
                " The command ^S^B%s%s%s^s needs su level privilages.\n", \
                USE_STRING(current_command, current_command), \
                USE_STRING(current_command && current_sub_command, " "), \
                USE_STRING(current_sub_command, current_sub_command)); \
 return; } } while (FALSE)

/* can the user tell something to everyone? (takes player pointer) */
#define CAN_GOTO_EVERYONE(x) (!((x)->flag_block_tells || \
				(x)->flag_block_shout || \
				(x)->saved->flag_jail)

/* does a range check, inclusive */
#define RANGE(x, min, max) (((x) >= (min)) && ((x) <= (max)))

/* for fvtell_player, system, normal and other (flagged) tells */

#ifndef PROCESS_OUTPUT_C
/*system forced tell, therefore 4 wrap, from 0*/
/*now that takes a variable pointer...*/
# define SYSTEM_T(p) NULL, (p), NULL, 4 | SYSTEM_INFO, now
/*system flagged tells, takes some flags*/
/*now that takes a variable pointer...*/
# define SYSTEM_FT(f, p) NULL, (p), NULL, 4 | SYSTEM_INFO | (f), now
# define SYSTEM_WFT(f, p) NULL, (p), NULL, SYSTEM_INFO | (f), now

/*player has requested info, therefore its FROM them*/
/*now that takes a variable pointer...*/
# define NORMAL_T(p) (p)->saved, (p), NULL, 3, now
/*normal flagged tells, takes some flags*/
/*now that takes a variable pointer...*/
# define NORMAL_FT(f, p) (p)->saved, (p), NULL, (3 | (f)), now
# define NORMAL_WFT(f, p) (p)->saved, (p), NULL, (f), now

/*talking from someone, to someone else, therefore from p to param*/
# define TALK_T(p, p2) (p), (p2), NULL, 3, now
# define TALK_IT(info, p, p2) (p), (p2), info, 3, now
# define TALK_TP(p2) TALK_T(p->saved, p2)
# define TALK_FT(f, p, p2) (p), (p2), NULL, 3 | (f), now
# define TALK_WFT(f, p, p2) (p), (p2), NULL, (f), now
# define TALK_FTP(f, p2) TALK_FT(f, p->saved, p2)

/*sending info from someone to someone else, 0 wrap! */
# define INFO_T(p, p2) (p), (p2), NULL, 0, now
# define INFO_TP(p2) INFO_T(p->saved, p2)
# define INFO_FT(f, p, p2) (p), (p2), NULL, (f), now
# define INFO_FTP(f, p2) INFO_FT(f, p->saved, p2)

# define LOGON_T(p) ALL_T((p)->saved, p, NULL, SYSTEM_INFO | 3, 0)
# define LOGON_FT(f, p) ALL_T((p)->saved, p, NULL, (f) | SYSTEM_INFO | 3, 0)
# define LOGON_WFT(f, p) ALL_T((p)->saved, p, NULL, SYSTEM_INFO | (f), 0)
                              
# define BYTES_T(p) (p)->saved, (p), NULL, SYSTEM_INFO | OUTPUT_BYTES, now
# define BYTES_FT(f, p) (p)->saved, (p), NULL, SYSTEM_INFO | OUTPUT_BYTES | (f), now

# define PRIO_BYTES_T(p) (p)->saved, (p), NULL, OUTPUT_PRIORITY | OUTPUT_BYTES, now
# define PRIO_BYTES_FT(f, p) (p)->saved, (p), NULL, OUTPUT_PRIORITY | OUTPUT_BYTES | (f), now
#endif
                              
/*this is for a tell player that does lots of stuff, but should be used so
  we can add extra parameters in at a later date, and so on*/
#define ALL_T(p, p2, i, f, t) (p), (p2), (i), (f), (t)

extern int get_random_num(int, int);
#define qstrcpy(x, y) stpcpy(x, y)

extern void lower_case(char *);
extern void upper_case(char *);
extern char *next_parameter(char *, char);
extern char *next_parameter_no_seperators(char *, char);
extern int beg_strcmp(const char *, const char *);
extern int beg_strcasecmp(const char *, const char *);
/* this is a guess, you might want to change it */
#ifdef HAVE_STRNCMP
# define BEG_CONST_STRCMP(x, y) (strncmp(x, y, CONST_STRLEN(x)))
#else
# define BEG_CONST_STRCMP(x, y) beg_strcmp(x, y)
#endif
#ifdef HAVE_STRNCASECMP
# define BEG_CONST_STRCASECMP(x, y) (strncasecmp(x, y, CONST_STRLEN(x)))
#else
# define BEG_CONST_STRCASECMP(x, y) beg_strcasecmp(x, y)
#endif
extern int skip_atoi(const char **);
extern long int skip_atol(const char **);
extern char *skip_chars(char *, int);
extern int number_of_flags(int) __attribute__ ((const));
extern unsigned int highest_powerof_2(unsigned int) __attribute__ ((const));
extern unsigned long timeval_diff_time(struct timeval *, struct timeval *);
extern void timeval_add_useconds(struct timeval *, unsigned long );
extern char *file2text(const char *, size_t *);

#ifdef TALKER_MAIN_H
extern void ptell_left(player_tree_node *, player *, twinkle_info *,
                       int, time_t, const char *, int);
extern void ptell_mid(player_tree_node *, player *, twinkle_info *,
                      int, time_t, const char *, int);
extern const char *gender_choose_str(int, const char *, const char *,
                                     const char *, const char *)
    __attribute__ ((const));
extern const char *isits1(const char *);
extern const char *isits2(const char *);
extern const char *get_nationality(player *, int);

extern player_linked_list *do_cronorder_logged_on(int (*) (player *, va_list),
                                                  ...);
extern player_linked_list *do_inorder_logged_on(int (*) (player *, va_list),
                                                ...);

extern player_linked_list *do_order_misc_on(int (*) (player_linked_list *,
                                                     va_list),
                                            player_linked_list *, ...);
extern player_linked_list *do_order_misc_on_all(int (*) (player_linked_list *,
                                                         va_list),
                                                player_linked_list *, ...);
extern player_linked_list *do_order_misc_all(int (*) (player_linked_list *,
                                                      va_list),
                                             player_linked_list *, ...);

extern player_tree_node *find_player_loggedon_count(int, int);
extern player_tree_node *find_player_room_count(int, room *, int);
extern int count_sus_on_or_off_duty(int);
extern int real_total_logon_time(player_tree_node *);
extern int real_age(player *);
extern player_linked_list *sys_wall(unsigned int, const char *, ...)
    __attribute__ ((__format__ (printf, 2, 3)));
extern const char *say_ask_exclaim_me(player *, const char *, size_t);
extern const char *say_ask_exclaim_group(player *, const char *, size_t);

#endif

#endif
