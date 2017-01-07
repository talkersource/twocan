#ifndef TRACE_H
#define TRACE_H

/* only seen in the trace.c file */
#ifdef TRACE_C

#define USING_TRACE defined(TRACE_INPUT_TO) || defined(TRACE_TIMED) || defined(TRACE_BASE) || defined(TRACE_SECTION) || defined(TRACE_COMMANDS)

#if USING_TRACE

typedef struct trace_slots
{
 unsigned int number;
 time_t timestamp;
 const char *info;

 const char *filename;
 int line_number;
} trace_slots;

#ifdef TRACE_INPUT_TO
# define MAX_TRACE_INPUT_TO 4
#endif

#ifdef TRACE_TIMED
# define MAX_TRACE_TIMED 2
#endif

#ifdef TRACE_BASE
# define MAX_TRACE_BASE 64
#endif

#ifdef TRACE_SECTION
# define MAX_TRACE_SECTION 6
#endif

#ifdef TRACE_COMMANDS
# define MAX_TRACE_COMMANDS 8
#endif

#endif /* using trace */
#endif /* if in trace.c */

#endif
