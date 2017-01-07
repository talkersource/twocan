#ifndef EXTERN_GET_PARAMETER_H
#define EXTERN_GET_PARAMETER_H

#define GET_PARAMETER_STR(x, y) ((x)->params[(y) - 1].str)
#define GET_PARAMETER_LENGTH(x, y) ((x)->params[(y) - 1].length)

#define GET_PARAMETER_LAST_STR(x) (GET_PARAMETER_STR(x, (x)->last_param))
#define GET_PARAMETER_LAST_LENGTH(x) (GET_PARAMETER_LENGTH(x, (x)->last_param))

#define GET_PARAMETER_BUF_PTR(x) ((x)->buffer + (x)->buffer_used)
#define GET_PARAMETER_BUF_UNUSED(x) ((x)->buffer_size - (x)->buffer_used)

/* minor adjustment on the struct hack */
#define GET_PARAMETER_DECL_CREATE(x, y) struct get_parameter_decl_ ## x { \
 parameter_holder x; char buffer[y]; } get_parameter_decl_ ## x; \
 parameter_holder *x = &(get_parameter_decl_ ## x . x)

#define GET_PARAMETER_DECL_INIT(x, y) do { \
 get_parameter_init(x); (x)->buffer_size = y; } while (FALSE)


extern void get_parameter_init(parameter_holder *);
extern parameter_holder *get_parameter_create(size_t);
extern int get_parameter_parse(parameter_holder *, const char **,
                               unsigned int);
extern void get_parameter_shift(parameter_holder *, unsigned int);


#endif
