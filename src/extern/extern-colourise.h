#ifndef EXTERN_COLOURISE_H
#define EXTERN_COLOURISE_H

extern output_node *colour_write(player *, int, unsigned int, output_node *);
extern output_node *colour_load(player *, int, unsigned int, output_node *);
extern void colourise_set_defaults(player *);

extern void cmds_init_colourise(void);

#endif
