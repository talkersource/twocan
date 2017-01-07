#ifndef EXTERN_PROCESS_INPUT_H
#define EXTERN_PROCESS_INPUT_H

#ifdef PROCESS_INPUT_C
#define INPUT_ASSURE_NEXT(x) do { \
 if (length < (x)) { \
  BTRACE("input_parse_telnet_options"); \
  p->input_doing_iac = length; \
  assert(p->input_doing_iac < 4); \
  if ((*in_node)->length > (INPUT_BUFFER_SZ - length)) \
    (*in_node)->length = (INPUT_BUFFER_SZ - length); \
  memcpy((*in_node)->input + (*in_node)->length, passed_input, length); \
  BTRACE("input_parse_telnet_options"); \
  return (0); } } while (FALSE)
#endif   

#define INPUT_SPACE_LEFT(x, y) ((INPUT_BUFFER_SZ - (y)) - (x)->length)
#define INPUT_ADD(x, y) do { \
 if (INPUT_SPACE_LEFT(x, 1) > 0) { (x)->input[(x)->length++] = (y); } \
 } while (FALSE)
#define INPUT_COPY(x, y, z) do { \
 if (INPUT_SPACE_LEFT(x, z) > 0) { memcpy((x)->input + (x)->length, y, z); \
 (x)->length += (z); } } while (FALSE)
#define INPUT_TERMINATE(x) do { \
 assert((x)->length <= (INPUT_BUFFER_SZ - 1)); \
 (x)->input[(x)->length] = 0; } while (FALSE)

extern input_node *input_add(player *, input_node *);
extern void input_del(player *, input_node *);

extern void input_del_all_comp(player *);

extern input_node *input_find_current(player *);

/* telnet command mode options... */
extern void telopt_ask_passwd_mode_on(player *);
extern void telopt_ask_passwd_mode_off(player *);
extern void telopt_ask_echo_local(player *);
extern void telopt_ask_echo_remote(player *);
extern void telopt_ask_end_or_record(player *);
extern void telopt_ask_not_end_or_record(player *);
extern void telopt_ask_chars_mode(player *);
extern void telopt_ask_line_mode(player *);
extern void telopt_ask_term_type(player *);
extern void telopt_ask_term_sizes(player *);
extern void telopt_ask_compress_do(player *);
extern void telopt_ask_no_compress(player *);
extern void telopt_ask_compress_start(player *);
extern void telopt_ask_compress_stop(player *);

extern void input_process(player *, input_node *, const char *, size_t);

extern void cmds_init_process_input(void);

#endif
