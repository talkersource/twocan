#ifndef EXTERN_SAFEMALLOC_H
#define EXTERN_SAFEMALLOC_H

#define SHUTDOWN_MEM_ERR() \
 shutdown_error("out of memory: %s, %d.\n", __FILE__, __LINE__)
#define LOG_MEM_ERR() \
 vwlog("error", "out of memory: %s, %d.\n", __FILE__, __LINE__)
#define P_MEM_ERR(x) do { log_assert(FALSE); \
  fvtell_player(SYSTEM_T(x), " ** Error: non memory %s:%d.\n", \
                __FILE__, __LINE__); LOG_MEM_ERR(); } while (FALSE)


#define TMP_MALLOC_PARAMS unsigned int, const char *, unsigned int

extern void *safemalloc(size_t, TMP_MALLOC_PARAMS);
extern void *safecalloc(size_t, size_t, TMP_MALLOC_PARAMS);
extern void *saferealloc(void *, size_t, TMP_MALLOC_PARAMS);
extern void safefree(void *, TMP_MALLOC_PARAMS);


#define XMALLOC(x, X) \
 safemalloc(x, MALLOC_TYPE_ ## X, __FILE__, __LINE__)
#define XCALLOC(x, y, X) \
 safecalloc(x, y, MALLOC_TYPE_ ## X, __FILE__, __LINE__)
#define XREALLOC(x, y, X) \
 saferealloc(x, y, MALLOC_TYPE_ ## X, __FILE__, __LINE__)
#define XFREE(x, X) \
 safefree(x, MALLOC_TYPE_ ## X, __FILE__, __LINE__)

#define MALLOC(x) XMALLOC(x, UNDEFINED)
#define CALLOC(x, y) XCALLOC(x, y, UNDEFINED)
#define REALLOC(x, y) XREALLOC(x, y, UNDEFINED)
#define FREE(x) XFREE(x, UNDEFINED)


/* now the functions that may or may not be used */
#ifdef EXTRA_MALLOC_WRAPPER
# ifdef USE_POINTER_COMPARES
extern int malloc_valid(const void *, size_t, TMP_MALLOC_PARAMS);
#  define MALLOC_VALID(x, y, X) \
 malloc_valid(x, y, MALLOC_TYPE_ ## X, __FILE__, __LINE__)
#  define PTR_LESS(x, y) ((unsigned long int)x < (unsigned long int)y)
#  define PTR_EQUAL(x, y) ((unsigned long int)x == (unsigned long int)y)
#  define PTR_MORE(x, y) ((unsigned long int)x > (unsigned long int)y)
#  define PTR_DIFF(x, y) ((unsigned long int)x - (unsigned long int)y)
# else
#  define MALLOC_VALID(x, y, z) (x ? TRUE : FALSE)
#  ifndef NWARNS
#   warning "Malloc valid reduced in strength"
#  endif
# endif
extern void malloc_no_members(TMP_MALLOC_PARAMS);
# define MALLOC_NO_MEMBERS(x) malloc_no_members(x, __FILE__, __LINE__)
#else
# define MALLOC_NO_MEMBERS(x) (TRUE)
# define MALLOC_VALID(x, y, z) (x ? TRUE : FALSE)
#endif


#undef TMP_MALLOC_PARAMS /* used above above, ONLY */

#ifdef TALKER_MAIN_H
/* user command */
extern void cmds_init_safemalloc(void);
#endif

#endif
