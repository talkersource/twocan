#define TIP_C
/*
 *  Copyright (C) 1999 James Antill
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * email: james@twocan.org
 */
#include "main.h"

static tip_base *tip_start = NULL;
unsigned int tip_section_num = 0;
unsigned int tip_num = 0;

static tip_node *tip_add_node(tip_base *section, char *tip, size_t len)
{
 tip_node *new_node = XMALLOC(sizeof(tip_node), TIP_NODE);

 if (!new_node)
   return (NULL);

 ++section->num;
 ++tip_num;
 
 new_node->text = tip;
 new_node->len = len;

 new_node->next = section->start;
 section->start = new_node;
 
 return (new_node);
}

static void tip_del_node(tip_base *section, tip_node **tip)
{
 tip_node *togo = *tip;

 assert(section->num > 0);
 --section->num;
 --tip_num;
 *tip = (*tip)->next;
 FREE(togo->text);
 XFREE(togo, TIP_NODE);
}

static tip_node **tip_find_node(tip_base *section, int count)
{
 tip_node **scan = &section->start;

 while (*scan && (--count > 0))
   scan = &(*scan)->next;

 if (*scan)
   return (scan);

 return (NULL);
}

static tip_base *tip_add_section(char *name, unsigned int priv_type)
{
 tip_base *new_base = XMALLOC(sizeof(tip_base), TIP_BASE);

 if (!new_base)
   return (NULL);

 ++tip_section_num;
 
 new_base->name = name;
 new_base->priv_type = priv_type;
 
 new_base->start = NULL;
 new_base->num = 0;
 
 new_base->next = tip_start;
 new_base->prev = NULL;
 tip_start = new_base;
 
 return (new_base);
}

static void tip_del_section(tip_base *section)
{ 
 --tip_section_num;

 while (section->start)
   tip_del_node(section, &section->start);
 
 FREE(section->name);
 
 if (section->next)
   section->next->prev = section->prev;

 if (section->prev)
   section->prev->next = section->next;
 else
   tip_start = section->next;
 
 XFREE(section, TIP_BASE);
}

static tip_base *tip_find_section(const char *str)
{
 tip_base *scan = tip_start;

 while (scan)
 {
  if (!strcasecmp(scan->name, str))
    return (scan);

  scan = scan->next;
 }

 return (NULL);
}

static tip_base *tip_user_find_section(player *p, const char *str)
{
 tip_base *section = NULL;
 
 if (!(section = tip_find_section(str)))
 {
  fvtell_player(SYSTEM_T(p), " The section -- ^S^B%s^s -- doesn't exist.\n",
                str);
  return (NULL);
 }

 if (!priv_test_int(p->saved, section->priv_type))
 {
  fvtell_player(SYSTEM_T(p), " The section -- ^S^B%s^s -- is only available "
                "to people who have privs that you ^S^Bdo not^s.\n",
                str);
  return (NULL);
 }
 
 return (section);
}

static tip_node *tip_rand_section(tip_base *section)
{
 int count = section->num ? get_random_num(1, section->num) : 0;
 tip_node *scan = section->start;

 if (!count)
   return (NULL);

 while (scan && (--count > 0))
   scan = scan->next;
 assert(scan);

 return (scan);
}

static unsigned int tip_count_valid(player *p)
{
 tip_base *section = tip_start;
 unsigned int count = 0;
 
 while (section)
 {
  if (priv_test_int(p->saved, section->priv_type))
    count += section->num;
  
  section = section->next;
 }

 return (count);
}

static tip_node *tip_rand_global(player *p)
{
 unsigned int valid_num = tip_count_valid(p);
 unsigned int count = valid_num ? get_random_num(1, valid_num) : 0;
 tip_base *section = tip_start;
 tip_node *scan = NULL;

 if (!count)
   return (NULL);
 
 while (section)
 {
  assert(count);
  
  if (priv_test_int(p->saved, section->priv_type))
  {
   if (section->num >= count)
     break;
   
   count -= section->num;
  }
  
  section = section->next;
 }

 assert(section);
 scan = section->start;
 while (scan && (--count > 0))
   scan = scan->next;
 assert(scan);

 return (scan);
}

void tip_logon_show(player *p)
{
 tip_node *tip = tip_rand_global(p);
 
 if (tip)
   fvtell_player(LOGON_WFT(9, p), " -=> Tip: %s.\n", tip->text);
}

static void tip_load(void)
{
 file_io real_io_tip;
 file_io *io_tip = &real_io_tip;
 unsigned int num = 0;
 unsigned int test_total_num = 0;
 unsigned int count = 0;
 
 if (!file_read_open("files/sys/tips", io_tip))
 {
  return;
 }

 file_section_beg("header", io_tip);
 num = file_get_unsigned_int("number_of_sections", io_tip);
 test_total_num = file_get_unsigned_int("number_of_tips", io_tip);
 file_section_end("header", io_tip);

 file_section_beg("sections", io_tip);

 while (count < num)
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];
  char *name = NULL;
  tip_base *base = NULL;
  unsigned int section_num = 0;
  unsigned int section_count = 0;
  unsigned int priv_type = 0;
  
  sprintf(buffer, "%04d", ++count);
  file_section_beg(buffer, io_tip);

  if (!(name = file_get_malloc("name", NULL, io_tip)))
    SHUTDOWN_MEM_ERR();

  section_num = file_get_unsigned_int("number", io_tip);
  
  priv_type = file_get_unsigned_int("priv_type", io_tip);
  
  if (!(base = tip_add_section(name, priv_type)))
    SHUTDOWN_MEM_ERR();

  file_section_beg("tips", io_tip);
  
  while (section_count < section_num)
  {
   char section_buffer[BUF_NUM_TYPE_SZ(int)];
   char *tip = NULL;
   size_t len = 0;
   
   sprintf(section_buffer, "%04d", ++section_count);
   file_section_beg(section_buffer, io_tip);

   if (!(tip = file_get_malloc("text", &len, io_tip)))
     SHUTDOWN_MEM_ERR();
   
   if (!tip_add_node(base, tip, len))
     SHUTDOWN_MEM_ERR();
   
   file_section_end(section_buffer, io_tip);
  }
  assert(section_count == section_num);
  assert(section_num == base->num);
  
  file_section_end("tips", io_tip);
  
  file_section_end(buffer, io_tip);
 }
 assert(count == num);
 assert(test_total_num == tip_num);
 
 file_section_end("sections", io_tip);
 
 file_read_close(io_tip);
}

static void tip_save(void)
{
 file_io real_io_tip;
 file_io *io_tip = &real_io_tip;
 unsigned int count = 0;
 tip_base *scan_base = tip_start;
 
 if (configure.talker_read_only)
   return;
 
 if (!file_write_open("files/sys/tips.tmp", TIP_FILE_VERSION, io_tip))
 {
  log_assert(FALSE);
  return;
 }
 
 file_section_beg("header", io_tip);
 file_put_unsigned_int("number_of_sections", tip_section_num, io_tip);
 file_put_unsigned_int("number_of_tips", tip_num, io_tip);
 file_section_end("header", io_tip);

 file_section_beg("sections", io_tip);

 while (scan_base)
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];
  unsigned int section_count = 0;
  tip_node *scan = scan_base->start;
  
  sprintf(buffer, "%04d", ++count);
  file_section_beg(buffer, io_tip);

  file_put_string("name", scan_base->name, 0, io_tip);
  file_put_unsigned_int("number", scan_base->num, io_tip);

  file_put_unsigned_int("priv_type", scan_base->priv_type, io_tip);

  file_section_beg("tips", io_tip);
  
  while (scan)
  {
   char section_buffer[BUF_NUM_TYPE_SZ(int)];
   
   sprintf(section_buffer, "%04d", ++section_count);
   file_section_beg(section_buffer, io_tip);

   file_put_string("text", scan->text, scan->len, io_tip);
      
   file_section_end(section_buffer, io_tip);

   scan = scan->next;
  }
  assert(section_count == scan_base->num);
  
  file_section_end("tips", io_tip);
  
  file_section_end(buffer, io_tip);

  scan_base = scan_base->next;
 }
 assert(!scan_base && (count == tip_section_num));
 
 file_section_end("sections", io_tip);

 if (file_write_close(io_tip))
   rename("files/sys/tips.tmp", "files/sys/tips");
}

static void user_su_tip_add(player *p, const char *str)
{
 tip_base *section = NULL;
 char *dup_str = NULL;
 size_t len = 0;
 parameter_holder params;

 get_parameter_init(&params);

 if (!get_parameter_parse(&params, &str, 1) || !*str)
   TELL_FORMAT(p, "<section> <tip>");

 if (!(section = tip_user_find_section(p, GET_PARAMETER_STR(&params, 1))))
   return;
 
 if ((len = strnlen(str, TIP_TEXT_SZ)) == TIP_TEXT_SZ)
 {
  fvtell_player(SYSTEM_T(p), " The tips can have a maximum length of ^S^B%d^s "
                "you tip was too big.\n", TIP_TEXT_SZ - 1);
  return;
 }
 
 if (!(dup_str = MALLOC(len + 1)))
 {
  P_MEM_ERR(p);
  return;
 }

 memcpy(dup_str, str, len);
 dup_str[len] = 0;
 
 if (!tip_add_node(section, dup_str, len))
 {
  FREE(dup_str);
  P_MEM_ERR(p);
  return;
 }

 fvtell_player(NORMAL_T(p), " Added tip, in section ^S^B%s^s.\n",
               section->name);
 tip_save();
}

static void user_su_tip_list_sections(player *p, parameter_holder *params)
{
 unsigned int count = 0;
 tip_base *section = NULL;
 tip_node *scan = NULL;
 char buffer[sizeof(" Tips in section %s") + TIP_TEXT_SZ];

 if (params->last_param != 1)
 {
  tip_base *scan_base = tip_start;
  
  fvtell_player(SYSTEM_T(p), " Valid tip sections are: ");
  
  while (scan_base)
  {
   fvtell_player(SYSTEM_T(p), ADD_COMMA_FRONT(scan_base != tip_start, "%s"),
                 scan_base->name);
   scan_base = scan_base->next;
  }
  fvtell_player(SYSTEM_T(p), "\n");
  
  TELL_FORMAT(p, "<section>");
 }

 if (!(section = tip_user_find_section(p, GET_PARAMETER_STR(params, 1))))
   return;

 scan = section->start;
 sprintf(buffer, "Tips in section %s", section->name);
 ptell_mid(NORMAL_T(p), buffer, FALSE);
 fvtell_player(NORMAL_T(p), "Priv_type: ^S^B%s^s\n",
               priv_test_names[section->priv_type]);
 while (scan)
 {
  fvtell_player(NORMAL_WFT(5, p), " ^S^B% 3d^s %s\n", ++count, scan->text);
  scan = scan->next;
 }
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void user_su_tip_del(player *p, parameter_holder *params)
{
 tip_base *section = NULL;
 unsigned int count = 0;
 
 if (params->last_param != 2)
   TELL_FORMAT(p, "<section> <num>");

 if (!(section = tip_user_find_section(p, GET_PARAMETER_STR(params, 1))))
   return;

 count = strtoul(GET_PARAMETER_STR(params, 2), NULL, 0);
 if ((count < 1) || (count > section->num))
 {
  fvtell_player(SYSTEM_T(p), " The section -- ^S^B%s^s -- only has "
                "-- ^S^B%d^s -- tips in it.\n",
                section->name, section->num);
  return;
 }

 tip_del_node(section, tip_find_node(section, count));
 fvtell_player(NORMAL_T(p), " Tip removed, from section ^S^B%s^s.\n",
               section->name);
 tip_save();
}

static void user_tip_show(player *p, const char *str)
{
 tip_base *section = NULL;
 tip_node *tmp = NULL;
 
 if (!*str)
 {
  tip_base *scan = tip_start;
  
  fvtell_player(SYSTEM_T(p), " Valid tip sections are: ");

  while (scan)
  {
   fvtell_player(SYSTEM_T(p), ADD_COMMA_FRONT(scan != tip_start, "%s"),
                 scan->name);
   scan = scan->next;
  }
  fvtell_player(SYSTEM_T(p), "\n");
  
  TELL_FORMAT(p, "<section>");
 }

 if (!(section = tip_user_find_section(p, str)))
   return;

 if (!(tmp = tip_rand_section(section)))
 {
  fvtell_player(SYSTEM_T(p), " The section -- ^S^B%s^s -- doesn't have "
                "any tips in it.\n", section->name);
  return;
 } 

 fvtell_player(NORMAL_WFT(9, p), " -=> Tip: %s.\n", tmp->text);
}

static void user_su_tip_section_add(player *p, parameter_holder *params)
{
 tip_base *section = NULL;
 size_t len = 0;
 char *dup_str = NULL;
 unsigned int priv_type = 0;

 if (params->last_param != 2)
   TELL_FORMAT(p, "<section> <priv_type>");
 
 if (tip_find_section(GET_PARAMETER_STR(params, 1)))
 {
  fvtell_player(SYSTEM_T(p), " The section -- ^S^B%s^s -- already "
                "exists.\n", section->name);
  return;
 }

 len = GET_PARAMETER_LENGTH(params, 1);

 if (!(dup_str = MALLOC(len + 1)))
 {
  P_MEM_ERR(p);
  return;
 }
 
 memcpy(dup_str, GET_PARAMETER_STR(params, 1),
        GET_PARAMETER_LENGTH(params, 1));
 dup_str[GET_PARAMETER_LENGTH(params, 1)] = 0;

 priv_type = priv_test_parse_int(GET_PARAMETER_STR(params, 2));
 
 if (!tip_add_section(dup_str, priv_type))
 {
  FREE(dup_str);
  P_MEM_ERR(p);
  return;
 }

 fvtell_player(NORMAL_T(p), " Added tip section ^S^B%s^s.\n",
               GET_PARAMETER_STR(params, 1));
}

static void user_su_tip_section_del(player *p, const char *str)
{
 tip_base *section = NULL;
 
 if (!(section = tip_user_find_section(p, str)))
   return;

 tip_del_section(section);
 fvtell_player(NORMAL_T(p), " Removed tip section ^S^B%s^s.\n", str);
 tip_save();
}

void init_tip(void)
{
 tip_load();
}

void cmds_init_tip(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("tip_show", user_tip_show, CONST_CHARS, SYSTEM);
 
 CMDS_ADD("tip_section_add", user_su_tip_section_add, PARSE_PARAMS, SYSTEM);
 CMDS_PRIV(normal_su);
 CMDS_ADD("tip_section_del", user_su_tip_section_del, CONST_CHARS, SYSTEM);
 CMDS_PRIV(normal_su);
 
 CMDS_ADD("tip_list", user_su_tip_list_sections, PARSE_PARAMS, SYSTEM);
 CMDS_PRIV(normal_su);
 
 CMDS_ADD("tip_add", user_su_tip_add, CONST_CHARS, SYSTEM);
 CMDS_PRIV(normal_su);
 CMDS_ADD("tip_del", user_su_tip_del, PARSE_PARAMS, SYSTEM);
 CMDS_PRIV(normal_su);
}
