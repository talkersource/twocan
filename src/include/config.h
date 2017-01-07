#ifndef CONFIG_H
#define CONFIG_H

#define TALKER_CODE_SNAPSHOT "$Name: release-2000-05-29 $"

/* *************************************************************************
 *
 *  These are _defaults_ you can change them using the "configure" command
 * once you have booted, so you don't need to change these (and then
 * upgrading will probably be easier).
 *
 *  Also you may want to look at creating multiple files and just chaning
 * the default configure filename (by either passing --configure=filename
 * or typing a different name at the unixmake prompt and recompiling).
 *  This means that, for instace, you don't have to have the intercom
 * on the debug site ... or mails going to twocan-bug@ ... if you want
 * to test the bug command etc.
 *
 * *************************************************************************
 */

#ifndef CONFIGURE_DEFAULT_file_name
# define CONFIGURE_DEFAULT_file_name "default"
#endif

#define CONFIGURE_DEFAULT_backups_ammount 8

#define CONFIGURE_DEFAULT_channels_main_name "Talker"
/* one letter shortcut for main channel */
#define CONFIGURE_DEFAULT_channels_main_name_1_1 't'
/* two letter shortcut for main channel */
#define CONFIGURE_DEFAULT_channels_main_name_2_1 't'
#define CONFIGURE_DEFAULT_channels_main_name_2_2 'r'

#define CONFIGURE_DEFAULT_channels_players_do_all 64
#define CONFIGURE_DEFAULT_channels_players_join 16 

#define CONFIGURE_DEFAULT_email_extern_bugs \
 "twocan-bug@lists.twocan.org"
#define CONFIGURE_DEFAULT_email_extern_suggest \
 "twocan-suggest@lists.twocan.org"

#define CONFIGURE_DEFAULT_email_from_long "Talker <talker@example.org>"
#define CONFIGURE_DEFAULT_email_from_short "<talker@example.org>"

#define CONFIGURE_DEFAULT_email_sendmail_run FALSE
#define CONFIGURE_DEFAULT_email_sendmail_extern_run TRUE

#define CONFIGURE_DEFAULT_email_to_abuse "abuse@example.com"
#define CONFIGURE_DEFAULT_email_to_admin "admin@example.com"
#define CONFIGURE_DEFAULT_email_to_bugs "bugs@example.com"
#define CONFIGURE_DEFAULT_email_to_suggest "suggest@example.com"
#define CONFIGURE_DEFAULT_email_to_sus "sus@example.com"
#define CONFIGURE_DEFAULT_email_to_up_down "updown@example.com"

#define CONFIGURE_DEFAULT_game_draughts_use TRUE
#define CONFIGURE_DEFAULT_game_hangman_use TRUE
#define CONFIGURE_DEFAULT_game_sps_use TRUE
#define CONFIGURE_DEFAULT_game_ttt_use TRUE

#define CONFIGURE_DEFAULT_last_logon_def_show 16

#define CONFIGURE_DEFAULT_mask_coms_again_timeout MK_MINUTES(5)
#define CONFIGURE_DEFAULT_mask_coms_mask_timeout 32

#define CONFIGURE_DEFAULT_name_abbr_lower "xx"
#define CONFIGURE_DEFAULT_name_abbr_upper "XX"
#define CONFIGURE_DEFAULT_name_ascii_art \
" _        _ _\n" \
"| |_ __ _| | | _____ _ __\n" \
"| __/ _` | | |/ / _ \\ '__|\n" \
"| || (_| | |   <  __/ |\n" \
" \\__\\__,_|_|_|\\_\\___|_|\n"
#define CONFIGURE_DEFAULT_name_long "Talker"
#define CONFIGURE_DEFAULT_name_short "Talk"

/* most of the email addresses aren't valid when you first boot */
#define CONFIGURE_DEFAULT_output_raw_twinkles FALSE

#define CONFIGURE_DEFAULT_player_name_admin "Root"

#define CONFIGURE_DEFAULT_room_connect_msg "appear$F-Gender(pl()def(s)) as a wobbly image that soon solidifies."
#define CONFIGURE_DEFAULT_room_disconnect_msg "return$F-Gender(pl()def(s)) to the lunatic asylum."

#define CONFIGURE_DEFAULT_room_main "system.void"

#define CONFIGURE_DEFAULT_socials_use TRUE
#define CONFIGURE_DEFAULT_socials_xmas_strings FALSE

#define CONFIGURE_DEFAULT_socket_listen_len 16
#define CONFIGURE_DEFAULT_socket_port 0 /* always use INADDR_ANY */

#define CONFIGURE_DEFAULT_sys_nice_value 4

#define CONFIGURE_DEFAULT_talker_closed_to_newbies FALSE
#define CONFIGURE_DEFAULT_talker_closed_to_resis FALSE
#define CONFIGURE_DEFAULT_talker_read_only FALSE
#define CONFIGURE_DEFAULT_talker_verbose FALSE

#define CONFIGURE_DEFAULT_url_access "telnet://example.org:0"
#define CONFIGURE_DEFAULT_url_web "http://www.example.org/"

#endif

