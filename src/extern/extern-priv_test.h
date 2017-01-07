#ifndef EXTERN_PRIV_TEST_H
#define EXTERN_PRIV_TEST_H

#ifdef PRIV_TEST_C
# define PRIV_TEST_CHECK(p1, p2, priv) do { \
 if (p1->priv_ ## priv) \
 { if (p2->priv_ ## priv) return (0); else return (1); } \
 else if (p2->priv_ ## priv) return (-1); } while (FALSE)
#endif

#define PRIV_STAFF(sp) ((sp) && ((sp)->priv_su_channel || (sp)->priv_coder || \
 (sp)->priv_basic_su || (sp)->priv_normal_su || (sp)->priv_senior_su || \
 (sp)->priv_lower_admin || (sp)->priv_higher_admin || (sp)->priv_admin))

#define PRIV_SYSTEM_ROOM(sp) ((sp)->priv_system_room && !(sp)->priv_base)

extern const char *priv_test_names[];

extern int priv_test_int(player_tree_node *, int);
extern int priv_test_parse_int(const char *);
extern int priv_test_string(player *, const char *, int);

extern int priv_test_check(player_tree_node *, player_tree_node *);
extern int priv_test_user_check(player_tree_node *, player_tree_node *,
                                const char *, int);


extern int priv_test_none(player_tree_node *);

extern int priv_test_higher_admin(player_tree_node *);
extern int priv_test_admin(player_tree_node *);
extern int priv_test_lower_admin(player_tree_node *);
extern int priv_test_senior_su(player_tree_node *);
extern int priv_test_normal_su(player_tree_node *);
extern int priv_test_coder(player_tree_node *);
extern int priv_test_pretend_su(player_tree_node *);
extern int priv_test_basic_su(player_tree_node *);
extern int priv_test_su_channel(player_tree_node *);
extern int priv_test_spod(player_tree_node *);
extern int priv_test_minister(player_tree_node *);
extern int priv_test_alter_system_room(player_tree_node *);
extern int priv_test_no_timeout(player_tree_node *);
extern int priv_test_base(player_tree_node *);
extern int priv_test_newbie(player_tree_node *);

extern int priv_test_coder_and_admin(player_tree_node *);
extern int priv_test_coder_admin(player_tree_node *);
extern int priv_test_coder_lower_admin(player_tree_node *);
extern int priv_test_coder_senior_su(player_tree_node *);
extern int priv_test_coder_normal_su(player_tree_node *);
extern int priv_test_coder_pretend_su(player_tree_node *);
extern int priv_test_spod_pretend_su(player_tree_node *);
extern int priv_test_spod_minister_coder_normal_su(player_tree_node *);
extern int priv_test_lib_maintainer(player_tree_node *);
extern int priv_test_edit_files(player_tree_node *);

extern int priv_test_command_echo(player_tree_node *);
extern int priv_test_command_jotd_edit(player_tree_node *);
extern int priv_test_command_list(player_tree_node *);
extern int priv_test_command_mail(player_tree_node *);
extern int priv_test_command_extern_bug_suggest(player_tree_node *);
extern int priv_test_command_marriage_channel(player_tree_node *);
extern int priv_test_command_room(player_tree_node *);
extern int priv_test_command_room_link(player_tree_node *);
extern int priv_test_command_room_lock(player_tree_node *);
extern int priv_test_command_room_bolt(player_tree_node *);
extern int priv_test_command_room_alter(player_tree_node *);
extern int priv_test_command_room_grant(player_tree_node *);
extern int priv_test_command_script(player_tree_node *);
extern int priv_test_command_session(player_tree_node *);
extern int priv_test_command_trace(player_tree_node *);
extern int priv_test_command_warn(player_tree_node *);

extern int priv_test_configure_socials(player_tree_node *);
extern int priv_test_configure_game_draughts(player_tree_node *);
extern int priv_test_configure_game_hangman(player_tree_node *);
extern int priv_test_configure_game_sps(player_tree_node *);
extern int priv_test_configure_game_ttt(player_tree_node *);
extern int priv_test_configure_games(player_tree_node *);
    
extern int priv_test_configure_game_draughts_base(player_tree_node *);
extern int priv_test_configure_game_hangman_base(player_tree_node *);
extern int priv_test_configure_game_sps_base(player_tree_node *);
extern int priv_test_configure_game_ttt_base(player_tree_node *);
extern int priv_test_configure_games_base(player_tree_node *);
    
extern int priv_test_mode_mail(player_tree_node *);
extern int priv_test_mode_news(player_tree_node *);
extern int priv_test_mode_newsgroup(player_tree_node *);
extern int priv_test_mode_room(player_tree_node *);
extern int priv_test_mode_channels(player_tree_node *);
extern int priv_test_mode_check(player_tree_node *);
extern int priv_test_mode_chlim(player_tree_node *);
extern int priv_test_mode_draughts(player_tree_node *);
extern int priv_test_mode_stats(player_tree_node *);
extern int priv_test_mode_configure(player_tree_node *);

extern int priv_test_command_room_list(player_tree_node *);

/* list priv_tests ... */
extern int list_self_priv_test_friend(list_node *);
extern int list_self_priv_test_grab(list_node *);

#endif
