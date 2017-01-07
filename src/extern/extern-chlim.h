#ifndef EXTERN_CHLIM_H
#define EXTERN_CHLIM_H

#ifdef CHLIM_C

# define CHLIM_BUILD(limit_name, limit_type, do_test) do { \
 long new_size = 0; \
 player *p2 = NULL; \
 \
 if (params->last_param != 2) \
   TELL_FORMAT(p, "<player> <limit>"); \
 \
 if (*(GET_PARAMETER_STR(params, 2) + \
       strspn(GET_PARAMETER_STR(params, 2), "0123456789"))) \
 { \
  fvtell_player(SYSTEM_T(p), "%s", \
                " Limits must consist of numbers only.\n"); \
  TELL_FORMAT(p, "<player> <limit>"); \
 } \
 \
 new_size = strtol(GET_PARAMETER_STR(params, 2), NULL, 10); \
 \
 if (new_size < 0) \
 { \
  fvtell_player(NORMAL_T(p), "%s", \
                " Limits for " limit_name " must be _possitive_.\n"); \
  return; \
 } \
 \
 if (!(p2 = player_find_load(p, GET_PARAMETER_STR(params, 1), \
       PLAYER_FIND_SC_SU))) \
   return; \
 \
 if (do_test && (priv_test_check(p->saved, p2->saved) < 0)) \
 { \
   fvtell_player(SYSTEM_T(p), \
                 " The player -- ^S^B%s^s -- has enough " \
                 "privilages that you cannot change their -- ^S^B" \
                 limit_name " limit^s -- them.\n", \
                 p2->saved->name); \
  \
  if (!p2->loaded_player) \
    fvtell_player(SYSTEM_T(p2), \
                  " -=> %s%s tried to change your " limit_name " limit.\n", \
                  gender_choose_str(p->gender, "", "", "The ", "The "), \
                  p->saved->name); \
  return; \
 } \
 \
 p2-> limit_type = new_size; \
 \
 fvtell_player(SYSTEM_T(p), \
               " Changed the limit of ^S^B" limit_name "^s for the player " \
               "'^S^B%s^s' to ^S^B%ld^s.\n", \
               p2->saved->name, new_size); \
  \
 if (p2->loaded_player) \
   p2->saved->flag_tmp_player_needs_saving = TRUE; \
 else \
   fvtell_player(SYSTEM_T(p2), \
                 " %s has changed your ^S^B" limit_name "^s limit to %ld.\n", \
                 p->saved->name, new_size); \
 \
 } while (FALSE)
#endif

extern void cmds_init_chlim(void);
    
#endif
