#ifndef NEWS_H


#ifdef NEWS_C
# define NEWS_INDEX_FILE_VERSION 1
# define NEWS_GROUP_FILE_VERSION 2
#endif

/* default timeout on news articles -- can be changed at runtime */
#define NEWS_TIMEOUT (60 * 60 * 24 * 90)


#define NEWSGROUP_DESCRIPTION_SIZE 256

#define NEWS_ARTICLE_CHARS_SZ 5000
#define NEWS_ARTICLE_LINES_SZ 500

#define NEWSGROUP_NAME_SIZE 20
/* no twinkles in the above */

#define NEWS_SUBJECT_SIZE 200


typedef struct news_article
{
 struct news_article *next;
 struct news_article *prev;

 char *body;
 size_t body_size;
 
 char subject[NEWS_SUBJECT_SIZE];
 char name[PLAYER_S_NAME_SZ];
 time_t timestamp;
 int read_count;
 bitflag anonymous : 1;
} news_article;

typedef struct news_group
{
 struct news_group *next_id;
 struct news_group *prev_id;
 struct news_group *next_name;
 struct news_group *prev_name;
 
 struct news_article *start;
 
 char description[NEWSGROUP_DESCRIPTION_SIZE];
 char name[NEWSGROUP_NAME_SIZE];
 time_t c_timestamp; /* creation */
 time_t m_timestamp; /* modified */
 int expire_after;
 int articles;
 int max_articles;
 int id;
 bitflag can_read : 1; /* simple privs for resis reading and posting */
 bitflag can_post : 1;
} news_group;

#endif
