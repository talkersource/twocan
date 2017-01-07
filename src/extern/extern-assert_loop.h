#ifndef EXTERN_ASSERT_LOOP_H
#define EXTERN_ASSERT_LOOP_H

#ifdef NDEBUG
/* These should only be done for _simple_ asserts, and ones which you have
 * a backout plan for */
# define log_assert(x)  do { \
 if (!(x)) vwlog("error", " ** error: (%d) %d %s\n", \
                 errno, __LINE__, __FILE__); } while (FALSE)
# define assert_log(x)  do { \
 if (!(x)) vwlog("error", " ** error: (%d) %d %s\n", \
                 errno, __LINE__, __FILE__); } while (FALSE)
#else
# define log_assert(x) assert(x)
# define assert_log(x) assert(x)
#endif

#ifdef USE_ASSERT_LOOP
# ifdef NDEBUG
#  define assert(x) ((void) 0)
# else
extern void assert_loop(const char *, const char *, int, const char *);
#  define assert(x) do { \
 if (!(x)) \
  assert_loop(#x, __FILE__, __LINE__, __PRETTY_FUNCTION__); } while (FALSE)
# endif
#endif

#endif
