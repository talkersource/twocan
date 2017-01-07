#ifndef EXTERN_WORD_NUMBER_H
#define EXTERN_WORD_NUMBER_H

#ifdef WORD_NUMBER_C

# define WORD_NUMBER_DO(y, z) do { \
 if ((numdiv ## y = (num / y))) \
 { \
  length = internal_word_number(words, tmp, len - (tmp - str), \
                                numdiv ## y, capitalise); \
  capitalise = 0; tmp += length; \
  \
  if ((len - (tmp - str)) <= 1) \
    goto output_full; \
  \
  tmp += sprintf(tmp, " %.*s", (int)(len - (tmp - str) - 1), words[z]); \
  \
  if ((len - (tmp - str)) <= 0) \
    goto output_full; \
  \
  num -= ((numdiv ## y) * y); \
  if (!num) \
    return (tmp - str); \
  else if ((len - (tmp - str)) <= 1) \
    goto output_full; \
 \
 *tmp++ = ' '; } } while (FALSE)

#endif

extern size_t word_number_def(char *, size_t, long, int);
extern size_t word_number_times(char *, size_t, long, int);
extern size_t word_number_th(char *, size_t, long, int);

extern char *word_number_base(char *, size_t, size_t *, long, int,
                              size_t (*func)(char *, size_t, long, int));

#endif
