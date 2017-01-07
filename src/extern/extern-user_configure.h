#ifndef EXTERN_CONFIGURE_H
#define EXTERN_CONFIGURE_H

#ifdef CONFIGURE_C

# define CONF_FILE_get_STR(x, y, yd, z) do { \
 file_get_string(x, y, z, io_configure); \
 if (FILE_IO_CREATED(io_configure)) \
   CONST_COPY_STR(y, yd, z); \
 } while (FALSE)
# define CONF_FILE_put_STR(x, y, yd, z) \
 file_put_string(x, y, 0, io_configure)
# define CONF_FILE_OP_STR(x, y, z) \
 CONF_FILE_ ## x ## _STR(#y, configure. y, CONFIGURE_DEFAULT_ ## y, z)

# define CONF_FILE_get_MAL(x, y, yd) do { \
 y = file_get_malloc(x, NULL, io_configure); \
 if (FILE_IO_CREATED(io_configure)) { \
   size_t len = CONST_STRLEN(yd); \
   y = MALLOC(len + 1); \
   if (!y) SHUTDOWN_MEM_ERR(); \
   CONST_COPY_STR_LEN(y, yd); } \
 else { \
   if (!y) SHUTDOWN_MEM_ERR(); \
 } } while (FALSE)
# define CONF_FILE_put_MAL(x, y, yd) \
 file_put_string(x, y, 0, io_configure)
# define CONF_FILE_OP_MAL(x, y) \
 CONF_FILE_ ## x ## _MAL(#y, configure. y, CONFIGURE_DEFAULT_ ## y)

# define CONF_FILE_get_INT(x, y, yd) do { \
 y = file_get_int(x, io_configure); \
 if (FILE_IO_CREATED(io_configure)) y = yd; \
 } while (FALSE)
# define CONF_FILE_put_INT(x, y, yd) \
 file_put_int(x, y, io_configure)
# define CONF_FILE_OP_INT(x, y) \
 CONF_FILE_ ## x ## _INT(#y, configure. y, CONFIGURE_DEFAULT_ ## y)

# define CONF_FILE_get_BIT(x, y, yd) do { \
 y = file_get_bitflag(x, io_configure); \
 if (FILE_IO_CREATED(io_configure)) y = yd; \
 } while (FALSE)
# define CONF_FILE_put_BIT(x, y, yd) \
 file_put_bitflag(x, y, io_configure)
# define CONF_FILE_OP_BIT(x, y) \
 CONF_FILE_ ## x ## _BIT(#y, configure. y, CONFIGURE_DEFAULT_ ## y)

# define CONF_CMD(x, y) CMDS_ADD_SUB(#x, user_configure_ ## x, y)
     
#endif


#define CONFIGURE_INIT_MAIN() {CONFIGURE_DEFAULT_file_name, \
 0, \
 "", 0, 0, 0, 0, 0, \
 "", "", \
 "", "", \
 FALSE, FALSE, \
 "", "", "", "", "", "", \
 FALSE, FALSE, FALSE, FALSE, \
 0, \
 0, 0, \
 "", "", NULL, "", "", \
 FALSE, \
 "", \
 FALSE, \
 "", "", "", \
 FALSE, FALSE, \
 NULL, NULL, 0, FALSE, \
 0, \
 FALSE, FALSE, FALSE, FALSE, \
 "", "", \
 }

#define USER_CONFIGURE_INT_FUNC(x, y, z, min, max) do { \
 long num = 0; \
 \
 if (!*str) TELL_FORMAT(p, "<number>"); \
 \
 num = strtol(str, NULL, 0); \
 \
 if (configure.x == num) { \
 fvtell_player(NORMAL_T(p), " " y " ^S^B" z "^s %sset to '^S^B%ld^s'.\n", \
               "already", num); \
 return; } \
 \
 if ((num > (max)) || (num < (min))) { \
  fvtell_player(SYSTEM_T(p), " " y " ^S^B" z "^s %sset to '^S^B%ld^s'.\n", \
               "cannot be", num); \
  return; } \
 \
 fvtell_player(NORMAL_T(p), " " y " ^S^B" z "^s %sset to '^S^B%ld^s'.\n", \
               "", num); \
 \
 configure.x = num; \
 \
 configure_save(FALSE); \
 } while (FALSE)

#define USER_CONFIGURE_TIME_FUNC(x, y, z, min, max) do { \
 int num = 0; \
 int wt_err = 0; \
 \
 if (!*str) TELL_FORMAT(p, "<time>"); \
 \
 num = word_time_parse(str, WORD_TIME_PARSE_ERRORS, &wt_err); \
 if (wt_err) TELL_FORMAT(p, "<time>"); \
 \
 if (configure.x == num) { \
 fvtell_player(NORMAL_T(p), " " y " ^S^B" z "^s %sset to '^S^B%d^s'.\n", \
               "already", num); \
 return; } \
 \
 if ((num > (max)) || (num < (min))) { \
  fvtell_player(SYSTEM_T(p), " " y " ^S^B" z "^s %sset to '^S^B%d^s'.\n", \
               "cannot be", num); \
  return; } \
 \
 fvtell_player(NORMAL_T(p), " " y " ^S^B" z "^s %sset to '^S^B%d^s'.\n", \
               "", num); \
 \
 configure.x = num; \
 \
 configure_save(FALSE); \
 } while (FALSE)

extern configure_base configure;
     
extern configure_interface_node *configure_add_interface(int, int);
extern void configure_del_interface(configure_interface_node *);

extern void configure_save(int);

extern void init_configure(void);

extern void cmds_init_configure(void);

#endif
