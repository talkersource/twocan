#ifndef EXTERN_TRACE_H
#define EXTERN_TRACE_H


#ifdef TRACE_INPUT_TO

# define ICTRACE(x) do_ictrace(x, __FILE__, __LINE__)
# define DUMP_ICTRACE() dump_ictrace()

extern void do_ictrace(const char *, const char *, int);
extern void dump_ictrace(void);

#else

# define ICTRACE(x) IGNORE_PARAMETER(x)
# define DUMP_ICTRACE() IGNORE_PARAMETER(TRUE)

#endif


#ifdef TRACE_TIMED

# define TCTRACE(x) do_tctrace(x, __FILE__, __LINE__)
# define DUMP_TCTRACE() dump_tctrace()

extern void do_tctrace(const char *, const char *, int);
extern void dump_tctrace(void);

#else

# define TCTRACE(x) IGNORE_PARAMETER(x)
# define DUMP_TCTRACE() IGNORE_PARAMETER(TRUE)

#endif


#ifdef TRACE_BASE

# define BTRACE(x) do_btrace(x, __FILE__, __LINE__)
# define DUMP_BTRACE() dump_btrace()

extern void do_btrace(const char *, const char *, int);
extern void dump_btrace(void);

#else

# define BTRACE(x) IGNORE_PARAMETER(x)
# define DUMP_BTRACE() IGNORE_PARAMETER(TRUE)

#endif


#ifdef TRACE_SECTION

# define STRACE(x) do_strace(x, __FILE__, __LINE__)
# define DUMP_STRACE() dump_strace()

extern void do_strace(const char *, const char *, int);
extern void dump_strace(void);

#else

# define STRACE(x) IGNORE_PARAMETER(x)
# define DUMP_STRACE() IGNORE_PARAMETER(TRUE)

#endif


#ifdef TRACE_COMMANDS

# define CTRACE(x) do_ctrace(x, __FILE__, __LINE__)
# define DUMP_CTRACE() dump_ctrace()

extern void do_ctrace(const char *, const char *, int);
extern void dump_ctrace(void);

#else

# define CTRACE(x) IGNORE_PARAMETER(x)
# define DUMP_CTRACE() IGNORE_PARAMETER(TRUE)

#endif


#endif
