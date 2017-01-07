#ifndef EDITOR_H
#define EDITOR_H

typedef struct edit_line_node
{
 struct edit_line_node *next;
 struct edit_line_node *prev;

 char *line;
 size_t length;
} edit_line_node;

typedef struct edit_base
{
 struct edit_line_node *lines_start;
 struct edit_line_node *lines_end;

 unsigned int lines;
 unsigned int words;
 unsigned int characters;

 unsigned int current_line;
 edit_line_node *cached_current_line;
 
 /* public below here... */

 unsigned int max_lines;
 unsigned int max_characters;
 
 cmds_function cmd_quit;
 cmds_function cmd_end;

 bitflag is_raw : 1;
} edit_base;

#endif
