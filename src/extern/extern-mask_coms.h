#ifndef EXTERN_MASK_COMS_H
#define EXTERN_MASK_COMS_H

#define MASK_COMS_P(x, str, type) do { \
 if ((x)->mask_coms_type) { \
  assert(RANGE((x)->mask_coms_type, \
         MASK_COMS_TYPE_FIRST, MASK_COMS_TYPE_LAST)); \
  if (difftime(now, (x)->mask_coms_last_timestamp) < configure.mask_coms_mask_timeout) \
   str = mask_coms_str_ ## type [(x)->mask_coms_type]; \
  else (x)->mask_coms_type = 0; } } while (FALSE)

extern const char *mask_coms_str_emote[];
extern const char *mask_coms_str_say[];
extern const char *mask_coms_str_echo[];

extern void cmds_init_mask_coms(void);

extern void user_configure_mask_coms_again_timeout(player *, const char *);
extern void user_configure_mask_coms_mask_timeout(player *, const char *);


#endif
