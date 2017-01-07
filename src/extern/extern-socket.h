#ifndef EXTERN_SOCKET_H
#define EXTERN_SOCKET_H

#define INVALID_PLAYER_SOCKET(x) (!(x) || !(x)->io_indicator || \
 (SOCKET_POLL_INDICATOR((x)->io_indicator)->fd == -1))

#define IN_TELOPT_CMD(x, y) ((x)->telopt_do_ ## y)
#define ON_TELOPT_CMD(x, y)  (((x)->telopt_do_ ## y) = TRUE)
#define OFF_TELOPT_CMD(x, y) (((x)->telopt_do_ ## y) = FALSE)

/* global functions... */
extern void socket_copy_fd(player *, player *);

extern ssize_t socket_writev(int *, const struct iovec *, size_t);

extern void init_socket(void);

extern struct pollfd *socket_add(int);
extern void socket_del(struct pollfd *);

extern void socket_update_indicators(int);
extern void socket_update_player(player *);

extern void socket_main_socket(void);

extern void socket_player_input(player *);
extern void socket_player_output(player *);

extern void socket_all_players_output(void);

extern void socket_close_all(void);

extern void socket_interfaces_show(player *, const char *);

extern void user_configure_socket_interfaces(player *, parameter_holder *);
extern void user_configure_socket_listen_len(player *, const char *);
    
#endif

