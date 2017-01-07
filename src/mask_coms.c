#define MASK_COMS_C
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

/* Affects say/emote/echo/shout/emote_shout -- socials ? */
const char *mask_coms_str_say[] =
{
 " ** ERROR ** ",
 "Moo!!",
 "Woof!!",
 "Ribbit!!",
 "Bahh!!",
 "Hiss!!",
 "Oink!!",
 "Beep!!",
};

const char *mask_coms_str_emote[] =
{
 " ** ERROR ** ",
 "looks like gladys!!",
 "is a Fluke!!",
 "looks green!!",
 "looks white and fluffy!!",
 "doesn't have any legs!!",
 "is canadian bacon!!",
 "talks to RS232!!",
};

const char *mask_coms_str_echo[] =
{
 " ** ERROR ** ",
 "A cow goes... Moo!!",
 "You look like a nice big bone!!",
 "A frog jumps!!",
 "A sheep looks around!!",
 "A Snake, Apple, Eve ... Party!!",
 "The leader of the farm looks!!",
 "$Rot13($Talker-Name gets compiled)!!",
};

static int mask_coms_choose_type(const char *str)
{
 if (!beg_strcasecmp(str, "cow"))
   return (MASK_COMS_TYPE_COW);
 else if (!beg_strcasecmp(str, "dog"))
   return (MASK_COMS_TYPE_DOG);
 else if (!beg_strcasecmp(str, "frog"))
   return (MASK_COMS_TYPE_FROG);
 else if (!beg_strcasecmp(str, "sheep"))
   return (MASK_COMS_TYPE_SHEEP);
 else if (!beg_strcasecmp(str, "snake"))
   return (MASK_COMS_TYPE_SNAKE);
 else if (!beg_strcasecmp(str, "pig"))
   return (MASK_COMS_TYPE_PIG);
 else if (!beg_strcasecmp(str, "robot"))
   return (MASK_COMS_TYPE_ROBOT);
 
 return (0);
}

static int internal_mask_coms_player_find_msg(int type, player *p,
                                              const char *name,
                                              const char *player_name)
{
 IGNORE_PARAMETER(name && player_name);
 
 switch (type)
 {
  case PLAYER_FIND_MSG_TYPE_NO_SELF:
    fvtell_player(SYSTEM_T(p), "%s", " You can't mask ^S^Byour own^s coms!\n");
    return (TRUE);
    
  default:
    break;
 }
 
 return (FALSE);
}

static void user_mask_coms(player *p, parameter_holder *params)
{
 int time_needed = MK_HOURS(6);
 int count = 0;
 int mask_type = get_random_num(MASK_COMS_TYPE_FIRST, MASK_COMS_TYPE_LAST);
 int pf_offset = 0;
 player *p2 = NULL;
 
 switch (params->last_param)
 {
  case 1:
  case 2:
    break;
    
  default:
    TELL_FORMAT(p, "<player> " MASK_COMS_FMT);
 }
 
 if (p->flag_mask_coms_block)
 {
  fvtell_player(SYSTEM_T(p), " You are blocking mask coms commands, so you "
                "cannot mask other people's coms .\n");
  return;
 }

 count = real_total_logon_time(p->saved);
 if (count)
   count /= MK_HOURS(1);

 if (count > 6)
 {
  if (count > 96)
    time_needed = MK_MINUTES(30);
  else if (count > 48)
    time_needed = MK_HOURS(1);
  else if (count > 24)
    time_needed = MK_HOURS(2);
  else if (count > 12)
    time_needed = MK_HOURS(3);
 }
 
 if (difftime(now, p->mask_coms_hit_timestamp) < time_needed)
 {
  char buf[256];
  
  fvtell_player(SYSTEM_T(p),
                " You have masked the coms of someone too recently, "
                "so you'll have to wait ^S^B%s^s.\n",
                word_time_long(buf, sizeof(buf), time_needed -
                               difftime(now, p->mask_coms_hit_timestamp),
                               WORD_TIME_DEFAULT));
  return;  
 }
 
 pf_offset = player_find_msg_add(internal_mask_coms_player_find_msg);
 log_assert(pf_offset);
 if (!(p2 = player_find_on(p, GET_PARAMETER_STR(params, 1),
                           PLAYER_FIND_SC_EXTERN_ALL & ~PLAYER_FIND_SELF)))
 {
  player_find_msg_del(pf_offset); 
  return;
 }
 player_find_msg_del(pf_offset);
 
 if (params->last_param == 2)
   if (!(mask_type = mask_coms_choose_type(GET_PARAMETER_STR(params, 2))))
   {
    fvtell_player(SYSTEM_T(p),
                  " That is not a valid mask type -- ^S^B%s^s --.\n",
                  GET_PARAMETER_STR(params, 2));
    return;
   }

 if (p2->flag_mask_coms_block)
 {
  fvtell_player(SYSTEM_T(p),
                " The player -- ^S^B%s^s -- is blocking masked coms.\n",
                p2->saved->name);
  return;
 }

 if (difftime(now, p2->mask_coms_last_timestamp) > configure.mask_coms_mask_timeout)
   p2->mask_coms_type = 0;
 
 else if (difftime(now, p2->mask_coms_last_timestamp) <
          configure.mask_coms_again_timeout)
 {
  char buf[256];
  
  fvtell_player(SYSTEM_T(p),
                " The player -- ^S^B%s^s -- has been masked too recently, "
                "so you'll have to wait ^S^B%s^s.\n",
                p2->saved->name,
                word_time_long(buf, sizeof(buf),
                               configure.mask_coms_again_timeout -
                               difftime(now, p2->mask_coms_last_timestamp),
                               WORD_TIME_DEFAULT));
  return;
 }
 
 p->mask_coms_hit_timestamp = now;
 
 p2->mask_coms_last_timestamp = now;
 p2->mask_coms_type = mask_type;

 fvtell_player(NORMAL_T(p), " You have just masked the coms of %s.\n",
               p2->saved->name);

 fvtell_player(NORMAL_T(p2), " %s%s %s just masked your coms.\n",
               (p->gender == GENDER_PLURAL) ? " The" : "",
               p->saved->name, (p->gender == GENDER_PLURAL) ? "have" : "has");
}

static void user_mask_coms_block(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_mask_coms_block, TRUE,
                       " You are %sblocking mask coms commands.\n",
                       " You are %snot blocking mask coms commands.\n",
                       TRUE);
}

void user_configure_mask_coms_again_timeout(player *p, const char *str)
{
 USER_CONFIGURE_TIME_FUNC(mask_coms_again_timeout,
                          "Mask coms", "again timeout", 1, INT_MAX);
}

void user_configure_mask_coms_mask_timeout(player *p, const char *str)
{
 USER_CONFIGURE_TIME_FUNC(mask_coms_mask_timeout,
                          "Mask coms", "mask timeout", 1, INT_MAX);
}

void cmds_init_mask_coms(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("mask_coms", user_mask_coms, PARSE_PARAMS, SYSTEM);
 CMDS_ADD("block_mask_coms", user_mask_coms_block, CONST_CHARS, SETTINGS);
}
