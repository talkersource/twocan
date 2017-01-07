#ifndef EXTERN_EDITOR_H
#define EXTERN_EDITOR_H

#define EDIT_SIZE(x) ((x)->characters + ((x)->is_raw ? 0 : (x)->lines))
#define EDIT_BASE(p) ((p)->buffers->current_edit_base)

extern int edit_start(player *, const char *);
extern int edit_indent_start(player *, const char *, const char *);
extern int edit_pager_start(player *p);

extern void edit_limit_lines(player *, unsigned int);
extern void edit_limit_characters(player *, unsigned int);

extern size_t edit_dump(player *, char *, size_t);
extern char *edit_malloc_dump(player *, size_t *);

extern int edit_pager_insert_line(player *, struct iovec *, int);

extern edit_line_node *edit_find_point(edit_base *);

extern void edit_stats(player *);
extern void edit_cleanup_func(player *);

extern void edit_pager_view(player *);
extern void edit_pager_backward_line(player *);
extern void edit_pager_forward_line(player *);
extern void edit_pager_backward_page(player *);
extern void edit_pager_forward_page(player *);
extern void edit_pager_goto_top(player *);
extern void edit_pager_goto_bottom(player *);

extern void edit_goto_line(player *, const char *);

extern void user_toggle_edit_quiet(player *, const char *);


extern void cmds_init_editor(void);

#endif
