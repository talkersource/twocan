#define COMMUNICATION_C
/*
 *  Copyright (C) 1999 James Antill, John Tobin
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
 * email: james@twocan.org, john@twocan.org
 */
#include "main.h"


static int internal_user_emote(player *scan, va_list va)
{
 player *p = va_arg(va, player *);
 char *str = va_arg(va, char *);
 int *count = va_arg(va, int *);
 
 if (p != scan)
 {
  LIST_COMS_CHECK_FLAG_START(scan, p->saved);
  if (LIST_COMS_CHECK_FLAG_DO(says))
    return (TRUE);
  LIST_COMS_CHECK_FLAG_END();

  ++*count;
  
  fvtell_player(TALK_TP(scan), "%s%s%s%s%.*s%s\n", USER_COLOUR_SAY,
                SHOW_ROOM(scan, "- "),
                "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                " t($F-Name_full) f($F-Name))",
                isits1(str), OUT_LENGTH_COMMUNICATION, isits2(str), "^N");
 }
 
 return (TRUE);
}

static void user_emote(player *p, const char *str)
{
 int count = 0;
 
 if (!*str)
   TELL_FORMAT(p, "<message>");

 MASK_COMS_P(p, str, emote);
 
 do_inorder_room(internal_user_emote, p->location, p, str, &count);

 if (count)
 {
  twinkle_info info;
  
  setup_twinkle_info(&info);

  info.output_not_me = TRUE;
  
  fvtell_player(TALK_IT(&info, p->saved, p),
                "%s You emote '%s%s%.*s%s' to the room.%s\n",
                USER_COLOUR_MINE,
                "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                " t($F-Name_full) f($F-Name))",
                isits1(str), OUT_LENGTH_COMMUNICATION, isits2(str),
                USER_COLOUR_MINE, "^N");
 }
 else
   fvtell_player(SYSTEM_T(p), " No-one is listening to your ^S^Bemotes^s.\n");
}

static int internal_user_echo(player *scan, va_list va)
{
 player *p = va_arg(va, player *);
 const char *str = va_arg(va, const char *);
 const char *name_buffer = va_arg(va, const char *);
 int *count = va_arg(va, int *);
 
 if (p == scan)
   return (TRUE);
 
 LIST_COMS_CHECK_FLAG_START(scan, p->saved);
 if (LIST_COMS_CHECK_FLAG_DO(echos))
   return (TRUE);
 LIST_COMS_CHECK_FLAG_END();
 
 ++*count;
 
 fvtell_player(TALK_TP(scan), "%s%s%s%.*s%s\n",
               USER_COLOUR_ECHO, SHOW_ECHO(scan, "+ ", name_buffer),
               OUT_LENGTH_COMMUNICATION, str, "^N");

 return (TRUE);
}

static void user_echo(player *p, const char *str)
{
 char name_buffer[PLAYER_S_NAME_SZ + CONST_STRLEN("[] ")];
 int count = 0;
 
 if (!*str)
   TELL_FORMAT(p, "<message>");

 sprintf(name_buffer, "%c%.*s%c ", '[',
         PLAYER_S_NAME_SZ - 1, p->saved->name, ']');

 MASK_COMS_P(p, str, echo);
 
 do_inorder_room(internal_user_echo, p->location, p, str, name_buffer, &count);

 if (count)
   fvtell_player(TALK_TP(p),
                 "%s You echo: %.*s%s\n",
                 USER_COLOUR_MINE, OUT_LENGTH_COMMUNICATION, str, "^N");
 else
   fvtell_player(SYSTEM_T(p), "%s",
                 " No-one is listening to your ^S^Bechos^s.\n");
}

static int internal_user_say(player *scan, va_list va)
{
 player *p = va_arg(va, player *);
 const char *str = va_arg(va, const char *);
 const char *mid = va_arg(va, const char *);
 int *count = va_arg(va, int *);
 
 if (p == scan)
   return (TRUE);
 
 LIST_COMS_CHECK_FLAG_START(scan, p->saved);
 if (LIST_COMS_CHECK_FLAG_DO(says))
   return (TRUE);
 LIST_COMS_CHECK_FLAG_END();
 
 ++*count;
 
 fvtell_player(TALK_TP(scan), "%s%s%s %s '%.*s%s'.%s\n",
               USER_COLOUR_SAY, SHOW_ROOM(scan, "- "),
               "$If( ==(1(N) 2($R-Set-Ign_prefix))"
               " t($F-Name_full) f($F-Name))",
               mid, OUT_LENGTH_COMMUNICATION, str,
               USER_COLOUR_SAY, "^N");

 return (TRUE);
}

void user_say(player *p, const char *str, size_t length)
{
 const char *mid = NULL;
 int count = 0;
 
 if (!*str)
   TELL_FORMAT(p, "<message>");
 
 mid = say_ask_exclaim_group(p, str, length);

 MASK_COMS_P(p, str, say);
 
 do_inorder_room(internal_user_say, p->location, p, str, mid, &count);

 if (count)
 {
  mid = say_ask_exclaim_me(p, str, length);
  
  fvtell_player(TALK_TP(p), "%s You %s '%.*s%s'%s\n", 
                USER_COLOUR_MINE, mid,
                OUT_LENGTH_COMMUNICATION, str, USER_COLOUR_MINE, "^N");
 }
 else
   fvtell_player(SYSTEM_T(p), "%s",
                 " No-one is listening to your ^S^Bsays^s.\n");
}

static const char *whisper_ask_exclaim_me(player *p, const char *str,
                                          size_t length)
{
 IGNORE_PARAMETER(p); /* symentry is a wonderful thing... */

 if (!length)
   ++length;
 
 switch (str[length - 1])
 {
  case '?':
   return ("ask in a whisper");
   
  case '!':
   return ("exclaim in a whisper");
   
  default:
    break;
 }

 return ("whisper");
}

static const char *whisper_ask_exclaim_group(player *p, const char *str,
                                          size_t length)
{
 if (p->gender == GENDER_PLURAL)
   return (tell_ask_exclaim_me(p, str, length));

 if (!length)
   ++length;
 
 switch (str[length - 1])
 {
  case '?':
    return ("asks in a whisper");
    
  case '!':
    return ("exclaims in a whisper");
    
  default:
    break;
 }
 
 return ("whispers");
}

static int internal_whisper_player_find_msg(int type, player *p,
                                            const char *name,
                                            const char *player_name)
{
 IGNORE_PARAMETER(name && player_name);
 
 switch (type)
 {
  case PLAYER_FIND_MSG_TYPE_NO_SELF:
    fvtell_player(SYSTEM_T(p), "%s",
                  " You can't whisper to ^S^Byourself^s!\n");
    return (TRUE);
    
  default:
    break;
 }
 
 return (FALSE);
}

static int internal_coms_whisper_room(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 player *p2 = va_arg(ap, player *);
 const char *str = va_arg(ap, const char *);
 size_t len = va_arg(ap, size_t);
 int *count = va_arg(ap, int *);
 
 if (scan == p2)
 {
  LIST_COMS_CHECK_FLAG_START(scan, p->saved);
  if (LIST_COMS_CHECK_FLAG_DO(tells))
  {
   fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking tells "
                 "from you.\n", p2->saved->name);
   return (TRUE);
  }
  LIST_COMS_CHECK_FLAG_END();

  ++*count;
  
  fvtell_player(TALK_TP(scan), "%s%s%s %s '%.*s%s' to %s.%s\n",
                USER_COLOUR_SOCIALS, SHOW_PERSONAL(scan, "> "),
                "$If( ==(1(N) 2($R-Set-Ign_prefix))"
                " t($F-Name_full) f($F-Name))",
                whisper_ask_exclaim_group(p, str, len),
                OUT_LENGTH_COMMUNICATION, str,
                USER_COLOUR_SOCIALS, "you", "^N");
  check_receive_state(p->saved, scan);
 }
 else if (scan != p)
 {
  LIST_COMS_CHECK_FLAG_START(scan, p->saved);
  if (LIST_COMS_CHECK_FLAG_DO(says))
    return (TRUE);
  LIST_COMS_CHECK_FLAG_END();

  ++*count;
  
  fvtell_player(TALK_TP(scan),
                "%s%s%s %s something to %s.%s\n",
                USER_COLOUR_SAY, SHOW_ROOM(scan, "- "),
                "$If( ==(1(N) 2($R-Set-Ign_prefix))"
                " t($F-Name_full) f($F-Name))",
                whisper_ask_exclaim_group(p, str, len), p2->saved->name,
                "^N");
 }

 return (TRUE);
}

static void user_whisper(player *p, const char *str, size_t len)
{
 parameter_holder real_params;
 parameter_holder *params = &real_params;
 player *p2 = NULL;
 int count = 0;
 int pf_offset = 0;
 
 get_parameter_init(params);
  
 if (!get_parameter_parse(params, &str, 1) || !*str)
   TELL_FORMAT(p, "<person(s)> <msg>");

 pf_offset = player_find_msg_add(internal_whisper_player_find_msg);
 log_assert(pf_offset);
 
 if (!(p2 = player_find_local(p, GET_PARAMETER_STR(params, 1),
                              PLAYER_FIND_SC_COMS)))
 {
  player_find_msg_del(pf_offset);
  return;
 }

 player_find_msg_del(pf_offset);
 
 len -= GET_PARAMETER_LENGTH(params, 1);
  
 do_inorder_room(internal_coms_whisper_room, p->location, p, p2,
                 str, len, &count);

 if (count)
   fvtell_player(TALK_TP(p),
                 "%s You %s '%.*s%s' to %s.%s\n",
                 USER_COLOUR_MINE, whisper_ask_exclaim_me(p, str, len),
                 OUT_LENGTH_COMMUNICATION, str,
                 USER_COLOUR_MINE, p2->saved->name, "^N");
 else
   fvtell_player(SYSTEM_T(p), " No-one is listening to your "
                 "^S^Bwhispers^s.\n");
}

static int internal_exclude_player_find_msg(int type, player *p,
                                            const char *name,
                                            const char *player_name)
{
 IGNORE_PARAMETER(name && player_name);
 
 switch (type)
 {
  case PLAYER_FIND_MSG_TYPE_NO_SELF:
    fvtell_player(SYSTEM_T(p), "%s", " You can't exclude ^S^Byourself^s from"
                  " conversation!\n");
    return (TRUE);
    
  default:
    break;
 }
 
 return (FALSE);
}

static int internal_coms_exclude_room(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 player *p2 = va_arg(ap, player *);
 const char *mid = va_arg(ap, const char *);
 const char *str = va_arg(ap, const char *);
 int *count = va_arg(ap, int *);
 
 if (scan == p)
   return (TRUE);
 
 if (scan == p2)
 {
  LIST_COMS_CHECK_FLAG_START(scan, p->saved);
  if (LIST_COMS_CHECK_FLAG_DO(says))
  {
   fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking says "
                 "from you.\n", p2->saved->name);
   return (TRUE);
  }
  LIST_COMS_CHECK_FLAG_END();
  
  fvtell_player(TALK_TP(scan), "%s%s%s tells something to everyone "
                "but you%s\n",
                USER_COLOUR_SAY,
                SHOW_ROOM(scan, "- "),
                "$If( ==(1(N) 2($R-Set-Ign_prefix))"
                " t($F-Name_full) f($F-Name))",
                "^N");
 }
 else
 {
  LIST_COMS_CHECK_FLAG_START(scan, p->saved);
  if (LIST_COMS_CHECK_FLAG_DO(tells))
    return (TRUE);
  LIST_COMS_CHECK_FLAG_END();

  ++*count;
  
  fvtell_player(TALK_TP(scan), "%s%s%s %s everyone but %s '%.*s%s'%s\n",
                USER_COLOUR_SAY,
                SHOW_ROOM(scan, "- "),
                "$If( ==(1(N) 2($R-Set-Ign_prefix))"
                " t($F-Name_full) f($F-Name))",
                mid, p2->saved->name,
                OUT_LENGTH_COMMUNICATION, str, USER_COLOUR_SAY, "^N");

  check_receive_state(p->saved, scan);
 }

 return (TRUE);
}

static void user_exclude(player *p, const char *str, size_t len)
{
 parameter_holder real_params;
 parameter_holder *params = &real_params;
 player *p2 = NULL;
 int count = 0;
 int pf_offset = 0;

 get_parameter_init(params);

 if (!get_parameter_parse(params, &str, 1) || !*str)
   TELL_FORMAT(p, "<person(s)> <msg>");

 pf_offset = player_find_msg_add(internal_exclude_player_find_msg);
 log_assert(pf_offset);

 if (!(p2 = player_find_local(p, GET_PARAMETER_STR(params, 1),
                              PLAYER_FIND_SC_COMS)))
 {
  player_find_msg_del(pf_offset);
  return;
 }

 player_find_msg_del(pf_offset);

 len -= GET_PARAMETER_LENGTH(params, 1);
  
 do_inorder_room(internal_coms_exclude_room, p->location, p, p2,
                 tell_ask_exclaim_group(p, str, len), str, &count);

 if (TRUE) /* FIXME: if (count) -- doesn't work v. well */
   fvtell_player(TALK_TP(p),
                 "%s You %s everyone but %s '%.*s%s'%s\n",
                 USER_COLOUR_MINE, tell_ask_exclaim_me(p, str, len),
                 p2->saved->name,
                 OUT_LENGTH_COMMUNICATION, str, USER_COLOUR_MINE, "^N");
 else
   fvtell_player(SYSTEM_T(p), " No-one is listening to your "
                 "^S^Bexcludes^s.\n");
}

void cmds_init_communication(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("echo", user_echo, CONST_CHARS, COMMUNICATION);
 CMDS_PRIV(command_echo);
 CMDS_ADD("emote", user_emote, CONST_CHARS, COMMUNICATION);
 CMDS_XTRA_MISC(RESTRICTED);
 CMDS_ADD("exclude", user_exclude, CHARS_SIZE_T, COMMUNICATION);
 CMDS_ADD("say", user_say, CHARS_SIZE_T, COMMUNICATION);
 CMDS_XTRA_MISC(RESTRICTED);
 CMDS_ADD("whisper", user_whisper, CHARS_SIZE_T, SOCIAL);
 CMDS_XTRA_MISC(RESTRICTED);
}
