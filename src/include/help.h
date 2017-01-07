#ifndef HELP_H
#define HELP_H

#ifndef HELP_DEBUG
# ifndef NDEBUG
#  define HELP_DEBUG 1
# else
#  define HELP_DEBUG 0
# endif
#endif

#ifdef HELP_C

/* used when parsing the help fiels... */
typedef struct help_options
{
 const char *primary_name;

 const char *keywords;
 
 const char *header;
 const char *footer;

 const char *body;

 bitflag always_header : 1;
 bitflag always_footer : 1;
 bitflag override_last : 1;
 bitflag expand : 1;
 bitflag expand_special : 1;
 bitflag telnet_client : 1;
 bitflag allow_find : 1;
} help_options;

typedef struct help_node
{
 struct help_node *next;
 
 const char *search_name;
 
 const char *primary_name;

 const char *keywords;

 const char *header;
 const char *footer;

 const char *body;

 bitflag always_header : 1;
 bitflag always_footer : 1;
 bitflag override_last : 1;
 bitflag expand : 1;
 bitflag expand_special : 1;
 bitflag telnet_client : 1;
 bitflag allow_find : 1;
} help_node;

typedef struct help_base
{
 char *malloc_start;
 help_node *start;

 struct msg_file msg_help;
 
 int (*test_can_see)(player_tree_node *);
} help_base;

/* because of msg_tmp can't have a different mount point between...
 * files/msgs and files/help */
#define HELP_INIT_SECTION(file, priv_func) {NULL, NULL, \
 MSG_INIT(file), priv_test_ ## priv_func}

#endif

#endif
