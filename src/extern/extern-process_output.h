#ifndef EXTERN_PROCESS_OUTPUT_H
#define EXTERN_PROCESS_OUTPUT_H

#ifdef PROCESS_OUTPUT_C
# define OUT_VAR_DEPRECIATED(from, p, twink, msg) do { \
 if (from && (from->player_ptr == p) && \
    (p->twink_dep_msg_command_number != current_command_number)) \
 { fvtell_player(ALL_T(NULL, p, NULL, 4 | SYSTEM_INFO, timestamp), \
                 "\n The twinkle -- ^S^B%s^s -- is depreciated" \
                 msg ".\n", twink); \
 p->twink_dep_msg_command_number = current_command_number; } } while (FALSE)
 
#endif

/* macros for the msg id strings */
#define SHOW_PERSONAL(x, y) ((x)->flag_show_personal ? (y) : (""))
#define SHOW_SHOUTS(x, y) ((x)->flag_show_shouts ? (y) : (""))
#define SHOW_ROOM(x, y) ((x)->flag_show_room ? (y) : (""))
#define SHOW_AUTOS(x, y) ((x)->flag_show_autos ? (y) : (""))
#define SHOW_SOCIALS(x, y) ((x)->flag_show_socials ? (y) : (""))

#define SHOW_PERSONAL_SOCIAL(x, y, a, b) \
 (((x)->flag_show_personal && (x)->flag_show_socials) ? (y) : \
   (x)->flag_show_personal ? (a) : (x)->flag_show_socials ? (b) : (""))

/* unlike the others in that it has two outputs... */
#define SHOW_ECHO(x, y, z) ((x)->flag_show_echo ? (y) : ("")), \
                           ((x)->flag_see_echo ? (z) : (""))

#define OUTPUT_TYPE(x) ((x)->output_type[(x)->type_ptr])

#define OUTPUT_NODE_NON_WRITABLE(x) ((x)->has_return || (x)->full_buffer)

/* ignores the inital screen stuff... */
#define OUTPUT_NODE_USE(x, y) (!(x)->output_already && \
                               ((x)->has_return || (x)->full_buffer) && \
                               (((x)->length - (x)->output_bit) > 0) && \
                               (difftime((y), (x)->timestamp) >= 0))

#define OUTPUT_NODE_EQUIV(x, y) \
 ((unsigned)(x & (OUTPUT_BUFFER_NON_PRINT | OUTPUT_REPEAT | \
                  OUTPUT_BYTES | OUTPUT_COMPRESS_MARK)) == \
             (y & (OUTPUT_BUFFER_NON_PRINT | OUTPUT_REPEAT | \
                   OUTPUT_BYTES | OUTPUT_COMPRESS_MARK)))

extern void setup_twinkle_info(twinkle_info *);

extern void save_tmp_output_list(player *, tmp_output_list_storage *);
extern void load_tmp_output_list(player *, tmp_output_list_storage *);

extern output_node *output_list_grab(player *);

/* these are all affected by repeat expansion and wrapping */
extern int output_list_length(player *, output_node *, int, int);
extern int output_list_print_length(player *, output_node *, int, int);
extern int output_list_print_indent(player *, output_node *, int, int);
extern int output_list_lines(player *, output_node *, int, int, int);

extern int output_list_into_buffer(player *,output_node *,char *,int);
extern output_node *output_list_copy(player *, output_node *, int, int, int);
extern output_node *output_list_merge(player *, output_node **,
                                      output_node **, int, int);
extern output_node *output_list_join(output_node **, output_node **);

/* these are mainly for the twinkles */
extern long output_list_toint(output_node *);
extern time_t output_list_totime(output_node *);
extern int output_list_cmp(output_node *, output_node *);
extern int output_list_case_cmp(output_node *, output_node *);
extern int output_list_number_cmp(output_node *, output_node *);

extern void output_list_linkin(player *, int, output_node **, int);
extern void output_list_cleanup(output_node **);

extern output_node *output_raw(player *, int, const char *,
			       int, output_node *);
extern output_node *delete_output_raw(int, output_node **, output_node *);

extern void output_for_player_cleanup(player *);
extern int output_for_player(player *);

extern void fvtell_player(player_tree_node *, player *, twinkle_info *,
                          int, time_t, const char *, ...)
    __attribute__ ((__format__ (printf, 6, 7)));

extern void vfvtell_player(player_tree_node *, player *, twinkle_info *,
                           int, time_t, const char *, va_list)
    __attribute__ ((__format__ (printf, 6, 0)));

#endif
