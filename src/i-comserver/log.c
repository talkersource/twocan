struct sys_flag
{
 unsigned int panic : 1;
} sys_flag = {0};
struct configure
{
 unsigned int talker_read_only : 1;
 unsigned int talker_verbose : 1;
} configure = {0, 0};
#include "../log.c"
