#ifndef EXTERN_TOGGLE_H
#define EXTERN_TOGGLE_H


#define TOGGLE_ON_OFF(x) ((x) ? ("^S^Bon^s") : ("off"))
#define TOGGLE_YES_NO(x) ((x) ? ("^S^Byes^s") : ("no"))
#define TOGGLE_TRUE_FALSE(x) ((x) ? ("^S^Btrue^s") : ("false"))

/* for toggle commands, see set_nationality */
#define TOGGLE_CHANGED(x, y) (((x) != (y)) ? "^S^Bnow^s " : "")

/* don't use lower_case so we can pass (const char *) as str */

#define TOGGLE_MATCH_ON(str) (!(beg_strcasecmp(str, "on") && \
 beg_strcasecmp(str, "yes") && beg_strcasecmp(str, "true") && \
 beg_strcasecmp(str, "start") && beg_strcasecmp(str, "do")))

#define TOGGLE_MATCH_OFF(str) (!(beg_strcasecmp(str, "off") && \
 beg_strcasecmp(str, "no") && beg_strcasecmp(str, "false") && \
 beg_strcasecmp(str, "stop") && beg_strcasecmp(str, "dont")))

#define TOGGLE_MATCH_TOGGLE(str) (!(beg_strcasecmp(str, "toggle") && \
 beg_strcasecmp(str, "change") && beg_strcasecmp(str, "other")))

/* p and str are global varibales... ON will turn it ON */
#define TOGGLE_COMMAND_ON_OFF(p, str, x, y, msg1, msg2, do_msg) do { \
 int pre_flag = (x); \
 \
 if (TOGGLE_MATCH_ON(str)) \
   (x) |= (y); \
 else { \
   if (TOGGLE_MATCH_OFF(str)) \
     (x) &= ~(y); \
   else { \
   if (TOGGLE_MATCH_TOGGLE(str)) \
     (x) ^= y; \
   else { \
     if (*str) \
       TELL_FORMAT(p, "[on|off|toggle]"); } } } \
\
 if (do_msg) { if ((x) & (y)) \
   fvtell_player(NORMAL_FT(SYSTEM_INFO, p), msg1, TOGGLE_CHANGED(pre_flag, x)); \
 else \
   fvtell_player(NORMAL_FT(SYSTEM_INFO, p), msg2, TOGGLE_CHANGED(pre_flag, x)); } \
 } while (FALSE)

/* p and str are global varibales... ON will turn it OFF */
#define TOGGLE_COMMAND_OFF_ON(p, str, x, y, msg1, msg2, do_msg) do { \
 int pre_flag = (x); \
 \
 if (TOGGLE_MATCH_ON(str)) \
   (x) &= ~(y); \
 else { \
   if (TOGGLE_MATCH_OFF(str)) \
     (x) |= (y); \
   else { \
    if (TOGGLE_MATCH_TOGGLE(str)) \
     (x) ^= y; \
    else { \
     if (*str) \
       TELL_FORMAT(p, "[on|off|toggle]"); } } } \
\
 if (do_msg) { if ((x) & (y)) \
   fvtell_player(NORMAL_T(p), msg1, TOGGLE_CHANGED(pre_flag, x)); \
 else \
   fvtell_player(NORMAL_T(p), msg2, TOGGLE_CHANGED(pre_flag, x)); } \
 } while (FALSE)

extern void cmds_init_toggle(void);

#endif
