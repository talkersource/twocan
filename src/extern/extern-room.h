#ifndef EXTERN_ROOM_H
#define EXTERN_ROOM_H

/* NOTE: when more than one person can get an event on a room...
 * will need to change */
#define ROOM_EVENT_OWNER_INFORM(from, msg) do { \
 player *to = from->location->owner->player_ptr; \
 if (to && to->flag_see_room_events && (from->location != to->location)) { \
    fvtell_player(INFO_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, from->saved, to), \
                  " %s %s just %s (%s.%s)\n", \
                  "$If( ==(1(N) 2($R-Set-Ign_prefix))" \
                  " t($F-Name_full) f($F-Name))", \
                  (to->gender == GENDER_PLURAL) ? "have" : "has", \
                  msg, from->location->owner->name, from->location->id); \
 } } while (FALSE) 
 

/* global functions ... */

extern player_linked_list *do_cronorder_room(int (*) (player *, va_list),
                                             room *, ...);
extern player_linked_list *do_inorder_room(int (*) (player *, va_list),
                                           room *, ...);

extern player_linked_list *vtell_room_wall(room *, player *, const char *, ...)
    __attribute__ ((__format__ (printf, 3, 4)));
extern player_linked_list *vtell_room_says(room *, player *, const char *, ...)
    __attribute__ ((__format__ (printf, 3, 4)));
extern player_linked_list *vtell_room_movement(room *, player *,
                                               const char *, ...)
    __attribute__ ((__format__ (printf, 3, 4)));

extern void init_rooms(void);

extern int room_load_open(player_tree_node *, file_io *);
    
extern void room_load(room *, file_io *);
extern room *room_load_find(player *, const char *, unsigned int);
extern void room_load_saved(player_tree_node *);
extern void room_save(player_tree_node *);
extern void room_del(room *);

extern int room_can_see_location(player *, player *);
extern int room_can_enter(player *, room *, int);

extern int room_player_transfer(player *, room *, int);
extern int room_user_player_transfer(player *, const char *, int);
extern int room_player_rand_main_transfer(player *p, int);

extern room *room_find_home(player_tree_node *);

extern void room_enter_jail(player *, int);

extern player_tree_node *room_random_player(room *r);
extern void room_player_del(player *, room *);

/* user functions.... */
extern void user_room_check_all(player *, const char *);
extern void user_room_check_flags(player *);
extern void user_room_check_autos(player *);

extern void user_follow_check(player *);

extern void user_configure_room_main(player *p, parameter_holder *params);
extern void user_configure_room_connect_msg(player *, const char *);
extern void user_configure_room_disconnect_msg(player *, const char *);


extern void cmds_init_room(void);

#endif
