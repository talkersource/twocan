#ifndef DISPLAY_TIME_H
#define DISPLAY_TIME_H

#ifdef DISPLAY_TIME_C

/* str length has to be at least about 30 for some functions and 8 
  for others */
# define DISPLAY_TIME_ARRAY_SZ 8
# define DISPLAY_TIME_STR_SZ 132

/* we can run a few functions in the same parameter list */
static char time_string[DISPLAY_TIME_ARRAY_SZ][DISPLAY_TIME_STR_SZ];
static int time_str_offset = 0;

# define DISPLAY_TIME_GET_STR() \
 time_string[time_str_offset = ((time_str_offset + 1) % DISPLAY_TIME_ARRAY_SZ)]
                               
#endif

                               
#endif
