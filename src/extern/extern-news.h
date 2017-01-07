#ifndef EXTERN_NEWS_C
#define EXTERN_NEWS_C

#ifdef NEWS_C

# define SET_DEFAULT_GROUP(p, group) do { \
  if ((p)->saved_default_newsgroup == (p)->default_newsgroup) \
    (p)->saved_default_newsgroup = (p)->default_newsgroup = group->id; \
  else \
    (p)->default_newsgroup = group->id; \
 } while (FALSE)

# define GET_DEFAULT_GROUP(p, group) do { \
   if (!(group = news_find_group_id((p)->default_newsgroup)) || \
       (!group->can_read && !PRIV_STAFF((p)->saved))) \
     { \
      if (!(group = news_find_group_name("misc")) || \
          (!group->can_read && !PRIV_STAFF((p)->saved))) \
     { if ((group = news_find_first_group(p))) \
         SET_DEFAULT_GROUP(p, group); \
     else \
     { \
      fvtell_player(NORMAL_T(p), "%s", \
                    " There are no newsgroups at the moment.\n"); \
      return; } } else SET_DEFAULT_GROUP(p, group); } } while (FALSE)
#endif

extern const char *newsgroup_find_group_name(int newsgroup);
extern void init_news(void);
extern int news_check_new_arrived(player_tree_node *, player *,
                                  twinkle_info *, int, time_t);

/* user functions... */
extern void user_news_list_articles(player *, const char *);
extern void user_news_list_articles_all(player *, const char *);
extern void user_news_list_newsgroups(player *, const char *);

#if 0
extern void user_news_post_article(player *, const char *);
extern void user_news_anon_post_article(player *, const char *);
extern void user_news_group_post_article(player *, const char *);
extern void user_news_group_anon_post_article(player *, const char *);

extern void user_news_followup_article(player *, const char *);
extern void user_news_anon_followup_article(player *, const char *);
extern void user_news_group_followup_article(player *, const char *);
extern void user_news_group_anon_followup_article(player *, const char *);

extern void user_news_remove_article(player *, const char *);
extern void user_news_group_remove_article(player *, const char *);

extern void user_news_read_article(player *, const char *);
extern void user_news_group_read_article(player *, const char *);

extern void user_news_reply_article(player *, const char *);
extern void user_news_anon_reply_article(player *, const char *);
extern void user_news_group_reply_article(player *, const char *);
extern void user_news_group_anon_reply_article(player *, const char *);

extern void user_su_news_group_add(player *, const char *);
extern void user_news_group_show_flags(player *, const char *);
extern void user_su_news_group_set_flags(player *, const char *);
extern void user_su_news_group_delete(player *, const char *);

extern void user_news_show_info(player *);
extern void user_news_set_default_group(player *, const char *);
extern void user_news_set_tmp_group(player *, const char *);
extern void user_toggle_news_inform(player *, const char *);
#endif

extern void cmds_init_news(void);

#endif
