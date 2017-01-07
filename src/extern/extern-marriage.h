#ifndef EXTERN_MARRIAGE_H
#define EXTERN_MARRIAGE_H

#define MARRIAGE_CHECK(p)  do { \
 if (!(p)->flag_married) \
   fvtell_player(NORMAL_T(p), "%s", \
                 " You must be married to use the marriage channel!\n"); \
 } while (FALSE)


extern const char *get_spouse_id(player *, int);
extern player *is_spouse_on_talker(player *);
extern void marriage_update_spouce_name(player *);

extern void cmds_init_marriage(void);

#endif
