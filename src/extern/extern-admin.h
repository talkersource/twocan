#ifndef EXTERN_ADMIN_H
#define EXTERN_ADMIN_H

#define P_SHOW_PRIV(p, p2_saved, flags, x, y) do { \
 if (p2_saved->priv_ ## x) { \
  fvtell_player(NORMAL_WFT(flags, p), "%s%s", done ? ", " : "", y); \
  done = TRUE; \
 } } while (FALSE)

#define P_SHOW_STAFF_PRIV(p, p2_saved, flags, x, y) do { \
 if (p2_saved->priv_ ## x && !p2_saved->priv_ ## y) { \
  fvtell_player(NORMAL_WFT(flags, p), "%s%s", done ? ", " : "", \
                p2_saved->name); \
  done = TRUE; } } while (FALSE)

extern void cmds_init_admin(void);

#endif
