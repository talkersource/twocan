#ifndef EXTERN_CHANNELS_H
#define EXTERN_CHANNELS_H

#ifdef CHANNELS_C
# define CHANNELS_NAME_CHECK(x) do { \
  if (*((x) + strspn((x), ALPHABET_LOWER ALPHABET_UPPER \
                     "1!234567&8*9(0)-_=+[]{};:@#~,<.>/? ")) || \
      !isalpha((unsigned char) (x)[0]) || !isalpha((unsigned char) (x)[1]) || \
      (strnlen((x), 3) != 3)) \
  { \
   fvtell_player(SYSTEM_T(p), "%s", \
                 " Channel names can only contain certain " \
                 "characters, must start with two alphabetic characters " \
                 "and must be 3 characters or more.\n"); \
   return; \
  } \
 } while (FALSE)

#endif

#define CHANNELS_VALID_COLOUR_TYPE(x) (((x) > 0) && ((x) <=CHANNELS_COLOUR_SZ))
#define CHANNELS_VALID_SEPERATOR(x) (((x) > 0) && ((x)<=CHANNELS_SEPERATOR_SZ))

#define CHANNELS_NAME_SEP(x) chan_map_sep[((x)->name_sep ? \
 (x)->name_sep : (x)->base->def_name_sep) - 1]
#define CHANNELS_COLOUR_TYPE(x) chan_map_colour[((x)->colour_type ? \
 (x)->colour_type : (x)->base->def_colour_type) - 1]


#ifdef CHANNELS_C

# define CHANNELS_INTERNAL_START(x) do { \
 if (!(base = channels_find_base(x))) { \
   if (!(base = channels_add_base(x))) \
     SHUTDOWN_MEM_ERR()

# define CHANNELS_INTERNAL_END(x) \
 base->flag_no_kill = TRUE; \
 do_inorder_all(internal_channels_init_player, priv_test_ ## x, base->name); \
 changed = TRUE; \
 } } while (FALSE)

#endif

extern const char *chan_map_colour[CHANNELS_COLOUR_SZ];
extern const char *chan_map_sep[CHANNELS_SEPERATOR_SZ][2];

extern channels_base *channels_find_base(const char *);
extern channels_node *channels_find_node(channels_base *, player_tree_node *);
extern channels_base *channels_user_find_base(player *, const char *, int);
extern channels_base *channels_user_find_write_base(player *, const char *);
extern channels_base *channels_user_find_grant_base(player *, const char *);

extern player_linked_list *channels_wall(const char *, unsigned int,
                                         player *, const char *, ...)
    __attribute__ ((__format__ (printf, 4, 5)));

extern channels_node *channels_add_system(const char *, player_tree_node *);
extern void channels_del_system(const char *, player_tree_node *);

extern void channels_timed_save(channels_base *);


extern void init_channels(void);

extern void user_configure_channels_main_name(player *, parameter_holder *);
extern void user_configure_channels_players_do_all(player *, const char *);
extern void user_configure_channels_players_join(player *, const char *);

extern void cmds_init_channels(void);

#endif
