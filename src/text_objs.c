#define TEXT_OBJS_C

#include "main.h"

static text_objs_node *text_objs_start = NULL;
static text_objs_node *text_objs_end = NULL;
static int text_objs_id = 1;

static Timer_q_func_node text_objs_timer_node;

static text_objs_node *text_objs_find_id(int id)
{
 text_objs_node *tmp = text_objs_start;
 int is_less_than = FALSE;

 while (tmp)
 {
  if (tmp->id == id)
  {
   tmp->a_timestamp = now;
   return (tmp);
  }
  else if (tmp->id < id)
    is_less_than = TRUE;
  else if (is_less_than)
    return (NULL);
  
  tmp = tmp->next;
 }

 return (NULL);
}

text_objs_node *text_objs_user_find_id(player_tree_node *owner, int id)
{
 text_objs_node *tmp = text_objs_find_id(id);

 if (!tmp)
   return (NULL);

 if (!tmp->owner[0] || !owner)
   return (tmp);

 if (!strcmp(owner->lower_name, tmp->owner))
   return (tmp);

 return (NULL);
}

int text_objs_add(player_tree_node *owner,
                  const char *str, size_t len, int type)
{
 text_objs_node *tmp = XMALLOC(sizeof(text_objs_node), TEXT_OBJS_NODE);
 
 if (!tmp)
   return (0);

 if (!(tmp->str = MALLOC(len + 1)))
 {
  XFREE(tmp, TEXT_OBJS_NODE);
  return (0);
 }
 
 tmp->id = text_objs_id % TEXT_OBJS_ID_MAX;
 ++tmp->id;
 
 COPY_STR_LEN(tmp->str, str, len);
 tmp->len = len;
 if (owner)
   COPY_STR(tmp->owner, owner->lower_name, PLAYER_S_NAME_SZ);
 else
   tmp->owner[0] = 0;
 tmp->type = type;
 tmp->can_del = FALSE;
 tmp->c_timestamp = now;
 tmp->a_timestamp = now;
 
 if (!text_objs_start)
   text_objs_start = tmp;
   
 tmp->next = NULL;
 if (text_objs_end)
   text_objs_end->next = tmp;
 text_objs_end = tmp;
 
 return ((text_objs_id = tmp->id));
}

void text_objs_del_id(int id)
{
 text_objs_node *tmp = text_objs_find_id(id);

 if (tmp)
   tmp->can_del = TRUE;
}

void text_objs_del_type(int type)
{
 text_objs_node *tmp = text_objs_start;

 while (tmp)
 {
  if (tmp->type == type)
    tmp->can_del = TRUE;
  
  tmp = tmp->next;
 }
}

void text_objs_cleanup(int force)
{
 text_objs_node *tmp = text_objs_start;
 
 while (tmp &&
       (tmp->can_del ||
        (difftime(now, tmp->a_timestamp) > TEXT_OBJS_CLEANUP_TIMEOUT_ACCESS) ||
        (difftime(now, tmp->c_timestamp) > TEXT_OBJS_CLEANUP_TIMEOUT_CREATE) ||
        force))
 {
  text_objs_node *tmp_next = tmp->next;

  FREE(tmp->str);
  XFREE(tmp, TEXT_OBJS_NODE);
  
  tmp = tmp_next;
 }
 if (!(text_objs_start = tmp))
   text_objs_end = NULL;
}

static void timed_text_objs_cleanup(int timed_type, void *data)
{
 struct timeval tv;
 
 IGNORE_PARAMETER(data);
 
 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;
 
 if (timed_type == TIMER_Q_TYPE_CALL_RUN_ALL)
   return;

 text_objs_cleanup(FALSE);

 gettimeofday(&tv, NULL);

 TIMER_Q_TIMEVAL_ADD_SECS(&tv, TEXT_OBJS_CLEANUP_TIMEOUT_REDO, 0);

 timer_q_add_static_node(&text_objs_timer_node.s, &timer_queue_global,
                         &text_objs_timer_node.s, &tv,
                         TIMER_Q_FLAG_NODE_FUNC);
 timer_q_cntl_node(&text_objs_timer_node.s, TIMER_Q_CNTL_NODE_SET_FUNC,
                   timed_text_objs_cleanup);
}

void init_text_objs(void)
{
 struct timeval tv;
 
 gettimeofday(&tv, NULL);

 TIMER_Q_TIMEVAL_ADD_SECS(&tv, TEXT_OBJS_CLEANUP_TIMEOUT_INIT, 0);

 timer_q_add_static_node(&text_objs_timer_node.s, &timer_queue_global,
                         &text_objs_timer_node.s, &tv,
                         TIMER_Q_FLAG_NODE_FUNC);
 timer_q_cntl_node(&text_objs_timer_node.s, TIMER_Q_CNTL_NODE_SET_FUNC,
                   timed_text_objs_cleanup);
}
