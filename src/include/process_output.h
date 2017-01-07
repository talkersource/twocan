#ifndef PROCESS_OUTPUT_H
#define PROCESS_OUTPUT_H

#ifdef PROCESS_OUTPUT_C
/* local values ... */

# define SHORT_TYPE 1
# define INT_TYPE 2
# define LONG_TYPE 3
/* # define SIZE_T_TYPE 4 */

/* pad with zero */
# define BASE_MASK ((1 << 6) - 1)
# define ZEROPAD (1 << 6)
/* unsigned/signed long */
# define SIGN (1 << 7)
/* show plus */
# define PLUS (1 << 8)
/* space if plus */
# define SPACE (1 << 9)
/* left justified */
# define LEFT (1 << 10)
/* 0x */
# define SPECIAL (1 << 11)
/* use 'ABCDEF' instead of 'abcdef' */
# define LARGE (1 << 12)

# define SKIP_MATCHED_TO(x, name) (*(x) += sizeof(name) - 1)

# define NEED_CONNECTOR(v, w, x, y, name, length, z) \
 if (**(v) != '-') \
 { \
  const char *tmp_msg = "<" name " is a connector>"; \
  return (output_string(w, x, info, y, timestamp, & tmp_msg, INT_MAX, \
          length, z)); \
 } \
 else \
  ++*v

/* used to save space */
# define SKIP_MATCHED_TO_CONNECTOR(v, w, x, y, name, length, z) do \
{ \
 SKIP_MATCHED_TO(v, name); \
 NEED_CONNECTOR(v, w, x, y, name, length, z); \
} while (FALSE)

 /* do this at the top of connectors, if they have to have a value */
# define CHECK_OBJECT(v, x, y, msg, length, z) do \
 if (!(v)) \
 { \
  const char *tmp_msg = msg; \
  return (output_string(0, x, info, y, timestamp, & tmp_msg, INT_MAX, \
          length, z)); \
 } while (FALSE)

# define DECREMENT_LENGTH(x, y) do \
   if ((x) < INT_MAX) (x) -= (y); while (FALSE)

# define TAG_BUFFER_LEN 2048
# define MAX_TAG_SIZE 30
# define MAX_PROB_IMBEDDING 10

# define DEFAULT_TAG_STRING "d", "def", "default"
 
# define CHECK_TAG_STRING_END(input, int_len, out_len) do { \
 if ((out_len) == -1) \
 { \
   const char *def_tag_list[] = {DEFAULT_TAG_STRING, 0}; \
   (out_len) = tag_get_def_name(def_tag_list, input, int_len); \
   assert(out_len >= 0); \
 } } while (FALSE)

   /* uses tmp/last_found/object/tag_buffer/out_buffer as global */
# define TAG_PRIV(priv, tag_list) do { \
 if ((name_len == -1) && (priv) && \
     ((P_IS_AVL(object) && \
      !object->player_ptr->flag_tmp_su_channel_block) || \
      PRIV_STAFF(p->saved))) \
 { tmp = start_tags; \
  name_len = tag_get_name(tag_list, &tmp, tag_list_size); \
 } } while (FALSE)

   /* uses tag_buffer/out_buffer/from/p/flags/tmp/out_node as global */
# define TAG_PRIV_EQ_START() do {} while (FALSE) /* do nothing to start */
# define TAG_PRIV_EQUAL(priv, tag_list) \
 if ((priv) && \
     ((P_IS_AVL(object) && \
       !object->player_ptr->flag_tmp_su_channel_block) || \
      PRIV_STAFF(p->saved))) { tmp = start_tags; \
  if ((name_len = tag_get_name(tag_list, &start_tags, tag_list_size)) >= 0) \
    return (string_variables(from, p, info, flags, timestamp, name_len, \
                             start_tags, input, length, out_node)); \
 } else
# define TAG_PRIV_EQ_END() do {} while (FALSE) /* do nothing at end, NEEDED */

# define MATH_TAG_START(short_name, long_name) \
 const char *tag_list_valid[] = {"1", "2", "l", "left", "r", "right", 0}; \
 output_node *tag_left_side = NULL; \
 output_node *tag_right_side = NULL; \
 const char *start_tags = NULL; \
 int tag_list_size = 0; \
 long number = 0; \
 long left_num = 0; \
 long right_num = 0; \
 \
 if (!BEG_CONST_STRCMP(long_name, *input)) \
   SKIP_MATCHED_TO(input, long_name); \
 else \
   SKIP_MATCHED_TO(input, short_name); \
 \
 if ((tag_list_size = check_tag_string(from, p, info, flags, timestamp, \
                                       input, &start_tags, tag_list_valid, \
                                       long_name, length, &out_node)) < 0) \
     return (out_node); \
 \
 out_node = math_tag_variables(from, p, info, flags, timestamp, \
                               length, long_name, \
                               start_tags, tag_list_size, &tag_left_side, \
                               &tag_right_side, out_node); \
 if (!(tag_left_side && tag_right_side)) \
 { \
  assert(!(tag_left_side || tag_right_side)); \
  return (out_node); \
 }

# define MATH_TAG_END() \
 output_list_cleanup(&tag_left_side); \
 output_list_cleanup(&tag_right_side);
   
/* *******************
 * macro for skipping '_mod' option and '_stick' option in
 * seconds constructor
 */
# define MOD_STICK(A, x, y) do { \
 if (!BEG_CONST_STRCMP("_mod", *(A))){SKIP_MATCHED_TO(A, "_mod"); \
   x ^= WORD_TIME_MOD_ ## y;} \
 if (!BEG_CONST_STRCMP("_stick", *(A))){SKIP_MATCHED_TO(A, "_stick"); \
   x ^= WORD_TIME_STICK_ ## y;} } while (FALSE)

# define OUTPUT_MAIN_TWINKLE_DEPTH 64
/* things like title and decription etc... (include buffers) */
# define OUTPUT_SUB_TWINKLE_DEPTH 4

# define PLAYER_TWINKLE_NAME 0
# define PLAYER_TWINKLE_ALPHA 1
# define PLAYER_TWINKLE_CRON 2
   
# define PLAYER_TWINKLE_DEFAULT PLAYER_TWINKLE_NAME

#endif /*end PROCESS_OUTPUT_C*/

#define WRAPPING_SPACES ((1<<7) - 1) /* number of spaces when line wraps: 127*/
/* same as HIGHLIGHT ... */
#define HIGHLIGHT (1<<7)
#define HILIGHT (1<<7)
/* splits words on any punctuation */
#define SPLIT_ON_PUNCTUATION (1<<8)
/* all specials get processed out, as though they were not there */
#define EAT_ALL_SPECIALS (1<<9)
/* allows you to use the remove_tmp_buffer command etc... so you can have
   lots of preprocessed buffers */
#define OUTPUT_BUFFER_TMP (1<<10)
/* no output variables work */
#define RAW_OUTPUT_VARIABLES (1<<11)
/* no specials or twinkles work */
#define RAW_OUTPUT (RAW_SPECIALS | RAW_OUTPUT_VARIABLES)
/* signifies that a buffer doesn't have any printable chars in it */
#define OUTPUT_BUFFER_NON_PRINT (1<<12)
/* means that twinkles HAVE to be processed, no matter what */
#define OVERRIDE_RAW_OUTPUT_VARIABLES (1<<13)
/* output for this node will be repeated untill the end of the screen,
   then there will be a return */
#define OUTPUT_REPEAT (1<<14)
/* gets through when(p->flags & SYSTEM_INFO_ONLY) */
#define SYSTEM_INFO (1<<15)
/* no specials work */
#define RAW_SPECIALS (1<<16)
/* don't do priv checks on output variables */
#define OUTPUT_VARIABLES_NO_CHECKS (1<<17)
/* special type of output, no wrapping checks etc. done. */
#define OUTPUT_BYTES (1<<18)
/* make the output go down the socket first */
#define OUTPUT_PRIORITY (1<<19)
/* a marker for compress start/stop nodes */
#define OUTPUT_COMPRESS_MARK (1<<20)
/* parse out all \ escapes, Ie. \\ -=> \
                                \( -=> (
                                \) -=> ) */
#define OUTPUT_IN_TAG (1<<21)

/* this is used because a few times the flags and time(0) have been the wrong
   way around */
#define NOT_USED_FLAGS_TELL_PLAYER ((1<<22) | (1<<23) | (1<<24) | (1<<25) | (1<<26) | (1<<27) | (1<<28) | (1<<29) | (1<<30) | (1<<31))

/* passed in printf %n, so needs the below cast */
#define OUTPUT_BYTES_TERMINATE ((int *)0)

#define OUTPUT_VARIABLES_PLAYER_LOADS 2 /* how many player loads are allowed */

#define NORM_LINE_TIMEOUT 2 /* seconds untill a node times out */
#define OUTPUT_LINE_SZ 16 /* max length of each node */
#define MAX_OUTPUT_LIST 256 /* max ammount of pending nodes */

/* If you change this you need to change the configure test */
#define OUTPUT_WRITE_MAX_NODES 1024

#define OUTPUT_VARIABLES_COUNT_SZ 32

typedef struct output_node
{
 struct output_node *next;
 struct output_node *prev;
 unsigned int flags;

 short length;
 short output_bit;

 bitflag full_buffer : 1;
 bitflag has_return : 1;
 bitflag output_already : 1;
  /* there are probably soo many reasons why I shouldn't fix repeat lists
   * like this, but right now I can't think of one */
 bitflag end_repeat_list : 1;
 
 time_t timestamp;
 
 char start[OUTPUT_LINE_SZ];
} output_node;

/* structure for twinkle information */
typedef struct twinkle_info
{
 bitflag used_twinkles : 1; /* output */

 bitflag allow_fills : 1; /* input */
 bitflag output_not_me : 1; /* input */
 
 unsigned int returns_limit; /* input */

 long counter[OUTPUT_VARIABLES_COUNT_SZ]; /* input/output */
} twinkle_info;

#endif
