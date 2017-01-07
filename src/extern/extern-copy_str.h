#ifndef EXTERN_COPY_STR_H
#define EXTERN_COPY_STR_H

#define COPY_STR_LEN(to, from, len) do { \
 if (len) memcpy(to, from, len); \
 to[len] = 0; \
 } while (FALSE)

#define CONST_COPY_STR_LEN(to, from) memcpy(to, from, sizeof(from))

#define COPY_STR(to, from, max_len) do { \
 int local_copy_str_tmp1 = (max_len); \
 int local_copy_str_tmp2 = strnlen(from, local_copy_str_tmp1 - 1); \
 assert(local_copy_str_tmp1 > 0); \
 COPY_STR_LEN(to, from, local_copy_str_tmp2); \
 } while (FALSE)

#define CONST_COPY_STR(to, from, max_len) do { \
 int local_copy_str_tmp1 = (max_len); \
 int local_copy_str_tmp2 = CONST_STRLEN(from); \
 assert(local_copy_str_tmp1 > 0); \
 --local_copy_str_tmp1; \
 if (local_copy_str_tmp1 < local_copy_str_tmp2) \
   local_copy_str_tmp2 = local_copy_str_tmp1; \
 COPY_STR_LEN(to, from, local_copy_str_tmp2); \
 } while (FALSE)

#define COPY_STR_BUILD_FUNC_ISITS(to, from, aname, max_len, max_out_len) do { \
 COPY_STR(to, from, max_len); \
 \
 if (!(to)[0]) { \
   fvtell_player(NORMAL_T(p), " You blank %s %s" aname "%s string.\n", \
                 "your", "^S^B", "^s"); break; } \
 \
 fvtell_player(NORMAL_T(p), \
               " You change %s %s" aname "%s string to:\n", \
               "your", "^S^B", "^s"); \
 \
 if ((max_out_len) && !p->see_raw_twinkles) \
   fvtell_player(NORMAL_T(p), " %s%s%.*s\n", \
                 p->saved->name, isits1(to), (max_out_len), isits2(to)); \
 else \
   fvtell_player(NORMAL_T(p), " %s%s%s\n", \
                 p->saved->name, isits1(to), isits2(to)); \
 } while (FALSE)

#define COPY_STR_BUILD_FUNC_ISITS_RAW(to, from, aname, max_len) do { \
 COPY_STR(to, from, max_len); \
 \
 if (!(to)[0]) { \
   fvtell_player(NORMAL_T(p), " You blank %s %s" aname "%s string.\n", \
                 "your", "^S^B", "^s"); break; } \
 \
 fvtell_player(NORMAL_T(p), \
               " You change %s %s" aname "%s string to:\n", \
               "your", "^S^B", "^s"); \
 \
 fvtell_player(NORMAL_FT(RAW_OUTPUT, p), "%s%s%s\n", \
               p->saved->name, isits1(to), isits2(to)); \
 } while (FALSE)

#define COPY_STR_BUILD_FUNC_RAW(to, from, name, max_len) do { \
 COPY_STR(to, from, max_len); \
 \
 if (!(to)[0]) { \
   fvtell_player(NORMAL_T(p), " You blank %s %s" name "%s string.\n", \
                 "your", "^S^B", "^s"); break; } \
 \
 fvtell_player(NORMAL_T(p), \
               " You change %s %s" name "%s string to:\n", "your", \
               "^S^B", "^s"); \
 fvtell_player(NORMAL_FT(RAW_OUTPUT, p), " %s\n", (to)); \
 } while (FALSE)

#define COPY_STR_BUILD_FUNC_NORM(to, from, name, max_len, max_out_len) do { \
 COPY_STR(to, from, max_len); \
 \
 if (!(to)[0]) { \
   fvtell_player(NORMAL_T(p), " You blank %s %s" name "%s string.\n", \
                 "your", "^S^B", "^s"); break; } \
 \
 fvtell_player(NORMAL_T(p), \
               " You change %s %s" name "%s string to:\n", \
               "your", "^S^B", "^s"); \
 if (max_out_len) \
   fvtell_player(NORMAL_T(p), " %.*s\n", (max_out_len), to); \
 else \
   fvtell_player(NORMAL_T(p), " %s\n", to); \
 } while (FALSE)

#define COPY_STR_BUILD_FUNC_TALKER_RAW(to, from, name, max_len) do { \
 COPY_STR(to, from, max_len); \
 \
 if (!(to)[0]) { \
   fvtell_player(NORMAL_T(p), " You blank %s %s" name "%s string.\n", \
                 "the", "^S^B", "^s"); break; } \
 \
 fvtell_player(NORMAL_T(p), \
               " You change %s %s" name "%s string to:\n", "the", \
               "^S^B", "^s"); \
 fvtell_player(NORMAL_FT(RAW_OUTPUT, p), " %s\n", (to)); \
 } while (FALSE)

#define COPY_STR_BUILD_FUNC_TALKER_ISITS(to, from, aname, max_len, max_out_len) do { \
 COPY_STR(to, from, max_len); \
 \
 if (!(to)[0]) { \
   fvtell_player(NORMAL_T(p), " You blank %s %s" aname "%s string.\n", \
                 "the", "^S^B", "^s"); break; } \
 \
 fvtell_player(NORMAL_T(p), \
               " You change %s %s" aname "%s string to:\n", \
               "the", "^S^B", "^s"); \
 \
 if ((max_out_len) && !p->see_raw_twinkles) \
   fvtell_player(NORMAL_T(p), " %s%s%.*s\n", \
                 p->saved->name, isits1(to), (max_out_len), isits2(to)); \
 else \
   fvtell_player(NORMAL_T(p), " %s%s%s\n", \
                 p->saved->name, isits1(to), isits2(to)); \
 } while (FALSE)

#ifdef TALKER_MAIN_H
extern void cmds_init_copy_str(void);
#endif

#endif
