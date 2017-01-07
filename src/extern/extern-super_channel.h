#ifndef EXTERN_SUPER_CHANNEL_H
#define EXTERN_SUPER_CHANNEL_H

#if 0
#define su_wall(p, str) vsu_wall(p, "%s", str)
#define su_wall_but(p, str) vsu_wall_but(p, "%s", str)

extern player_linked_list *vsu_wall(const char *, ... )
    __attribute__ ((__format__ (printf, 1, 2)));
extern player_linked_list *vsu_wall_but(player *, const char *, ... )
    __attribute__ ((__format__ (printf, 2, 3)));
#endif

extern void user_lsu(player *, const char *);

extern void user_su_toggle_off_lsu(player *, const char *);

extern void cmds_init_super_channel(void);

#endif
