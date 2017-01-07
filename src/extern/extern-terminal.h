#ifndef EXTERN_TERMINAL_H
#define EXTERN_TERMINAL_H

#ifdef TERMINAL_C
# define TERMINAL_DO_START(x) \
 const char *tmp = (x); \
 int len = strlen(tmp); \
 \
 assert(p->termcap); \
 PC = p->termcap->pc; \
 terminal_output_p = p; \
 terminal_output_flags = flags; \
 terminal_output_out_node = *passed_out_node

# define TERMINAL_DO_END() do { \
 \
 PC = 0; \
 terminal_output_p = NULL; \
 terminal_output_flags = 0; \
 *passed_out_node = terminal_output_out_node; \
 terminal_output_out_node = NULL; \
} while (FALSE)

# if 0
#  define TERMINAL_DO_STR(x) do { \
 TERMINAL_DO_START(x); \
 \
 assert(!strchr(tmp, '%')); \
 terminal_output_out_node = output_raw(p, flags | OUTPUT_BUFFER_NON_PRINT, \
                                       tmp, len, terminal_output_out_node); \
 \
 TERMINAL_DO_END(); \
} while (FALSE)
# endif

# define TERMINAL_DO_STR(x, param1, param2, param3, param4, param5, param6, param7, param8, param9) do { \
 TERMINAL_DO_START(x); \
 \
 terminal_output_out_node = terminal_output_raw(p, \
                                         flags | OUTPUT_BUFFER_NON_PRINT, \
                                         tmp, len, terminal_output_out_node, \
    param1, param2, param3, param4, param5, param6, param7, param8, param9); \
 \
 TERMINAL_DO_END(); \
} while (FALSE)

#endif

extern int terminal_setup(player *, const char *);
extern void terminal_unsetup(terminal_termcap **);

extern int terminal_set_normal(player *, int, output_node **);

extern int terminal_set_bold(player *, int, output_node **);
extern int terminal_set_dim(player *, int, output_node **);
extern int terminal_set_flash(player *, int, output_node **);
extern int terminal_set_underline(player *, int, output_node **);
extern int terminal_set_inverse(player *, int, output_node **);

extern int terminal_set_foreground_colour(player *, int, int, output_node **);
extern int terminal_set_background_colour(player *, int, int, output_node **);

extern int terminal_do_clear_screen(player *, int, output_node **);
extern int terminal_do_clear_eol(player *, int, output_node **);
extern int terminal_do_cursor_up(player *, int, output_node **);

extern int terminal_do_audible_bell(player *, int, output_node **);
extern int terminal_do_visual_bell(player *, int, output_node **);

extern int output_maybe_delete_line(player *);

extern void user_check_terminal(player *, const char *);

extern void cmds_init_terminal(void);

#endif
