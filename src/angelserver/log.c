#include "main.h"
struct sys_flag
{
 unsigned int panic : 1;
} sys_flag = {0};
#include "../log.c"
