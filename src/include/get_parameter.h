#ifndef GET_PARAMETER_H
#define GET_PARAMETER_H


# define GET_PARAMETER_NUMBER_MAX 64
# define GET_PARAMETER_BUFFER_DEFAULT_SZ 256


# ifdef GET_PARAMETER_C

#define GET_PARAMETER_LAST_CUR(x) \
 (GET_PARAMETER_LAST_STR(x) + GET_PARAMETER_LAST_LENGTH(x))

# define PARSE_TO_NEXT_TOKENS(x) do { \
 size_t length = 0; assert(GET_PARAMETER_BUF_UNUSED(parameters) > 1); \
 if ((length = strcspn(*str, x))) \
 { \
  if (length >= GET_PARAMETER_BUF_UNUSED(parameters)) \
    length = GET_PARAMETER_BUF_UNUSED(parameters) - 1; \
  if (length) { \
   memcpy(GET_PARAMETER_LAST_CUR(parameters), *str, length); \
   GET_PARAMETER_LAST_LENGTH(parameters) += length; \
   *str += length; parameters->buffer_used += length; } } \
 assert(GET_PARAMETER_BUF_UNUSED(parameters)); } while (FALSE)
/* end of function^H^H^H^H^H^H^H^H #define */

# endif

struct get_parameter_internal
{
 char *str;
 int length;
};

typedef struct parameter_holder
{
 unsigned int last_param;
 struct get_parameter_internal params[GET_PARAMETER_NUMBER_MAX];
 size_t buffer_size;
 size_t buffer_used;
 char buffer[GET_PARAMETER_BUFFER_DEFAULT_SZ]; /* at end for struct hack... */
} parameter_holder;

#endif
