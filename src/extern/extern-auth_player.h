#ifndef EXTERN_AUTH_PLAYER_H
#define EXTERN_AUTH_PLAYER_H

#ifdef AUTH_PLAYER_C
# define AUTH_PLAYER_NET_MASK_NUM(y, z) ( \
 ((z) == 3) ? ((y) - 24) : ((z) == 2) ? ((y) - 16) : \
 ((z) == 1) ? ((y) - 8) : ((z) == 0) ? ((y) - 0) : 0)
# define AUTH_PLAYER_NET_MASK(x, y, z) ((x)[z] & auth_player_ip_masks[( \
 (AUTH_PLAYER_NET_MASK_NUM(y, z) > 8) ? 8 : \
 (AUTH_PLAYER_NET_MASK_NUM(y, z) < 0) ? 0 : AUTH_PLAYER_NET_MASK_NUM(y, z))])

# define AUTH_PLAYER_NET_MASK_EQ(x, y, z) ( \
 (AUTH_PLAYER_NET_MASK(x, z, 0) == AUTH_PLAYER_NET_MASK(y, z, 0)) && \
 (AUTH_PLAYER_NET_MASK(x, z, 1) == AUTH_PLAYER_NET_MASK(y, z, 1)) && \
 (AUTH_PLAYER_NET_MASK(x, z, 2) == AUTH_PLAYER_NET_MASK(y, z, 2)) && \
 (AUTH_PLAYER_NET_MASK(x, z, 3) == AUTH_PLAYER_NET_MASK(y, z, 3)) )
#endif

extern int auth_player_total_max;
extern int auth_player_logging_on_max;

extern int auth_parse_ip_addr(const char *, unsigned char *, int);
extern int auth_parse_cidr(const char *, unsigned char *, short *);

extern void user_configure_talker_closed_to_newbies(player *, const char *);
extern void user_configure_talker_closed_to_resis(player *, const char *);
    
extern int auth_check_logon(player *);
extern int auth_check_player(player *);

extern void init_auth_player(void);

extern void cmds_init_auth_player(void);

#endif
