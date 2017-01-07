#ifndef EXTERN_WORD_TIME_H
#define EXTERN_WORD_TIME_H

#ifdef WORD_TIME_C

# define WORD_TIME_TIME_TEST(x, y) \
 ((flags & WORD_TIME_ ## x) && \
  ((atime && (y = (atime / WORD_TIME_DIV_ ## x))) || \
   (flags & WORD_TIME_STICK_ ## x)) && \
  ((atime %= WORD_TIME_DIV_ ## x) || TRUE))

# define WORD_TIME_OVERFLOW_TEST(z) \
  (len <= (BUF_NUM_TYPE_SZ(int) + count + z))

# define PARSE_DEF(x, y) (((x) == test_char) || \
 ((!test_char || (test_char == ' ')) && (flags & WORD_TIME_PARSE_DEF_ ## y)))

# define CHECK_HACK_NUM(x) if (hack_num <= x) { \
 *error = TRUE; if (flags & WORD_TIME_PARSE_ERRORS) return (0); \
 return (total_time); } else hack_num = (x)

#endif

extern const char *word_time_short(char *, size_t, unsigned long,
                                   unsigned int);
extern const char *word_time_long(char *, size_t, unsigned long, unsigned int);
extern unsigned long word_time_parse(const char *, int, int *);

#endif
