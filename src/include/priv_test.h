#ifndef PRIV_TEST_H
#define PRIV_TEST_H

#define PRIV_TEST_NONE 0

#define PRIV_TEST_NEWBIE 1
#define PRIV_TEST_RESIDENT 2
#define PRIV_TEST_SU_CHAN 3

#define PRIV_TEST_SU_NORM 4
#define PRIV_TEST_LWR_ADMIN 5
#define PRIV_TEST_ADMIN 6

#define PRIV_TEST_MINISTER 7
#define PRIV_TEST_SPOD 8
#define PRIV_TEST_CODER 9
#define PRIV_TEST_CMD_MAIL 10
#define PRIV_TEST_CMD_LIST 11
#define PRIV_TEST_CMD_ROOM 12

typedef int (*priv_test_type)(player_tree_node *);
typedef int (*priv_test_list_type)(list_node *);

#endif
