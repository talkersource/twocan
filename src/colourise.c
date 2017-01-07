#define COLOURISE_C
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

output_node *colour_write(player *p, int flags, unsigned int type,
			  output_node *out_node)
{
 switch (type)
 {
  case SAVE_OUTPUT_TYPE:
    if (OUTPUT_TYPE(p))
      terminal_set_normal(p, flags, &out_node);   
    
    if (p->type_ptr != (PLAYER_S_OUTPUT_TYPE_SZ - 1))
      ++p->type_ptr;
    
    OUTPUT_TYPE(p) = 0;
    break;
    
  case RESTORE_OUTPUT_TYPE:
    if (OUTPUT_TYPE(p))
      terminal_set_normal(p, flags, &out_node);

    OUTPUT_TYPE(p) = 0;
    
    if (p->type_ptr)
      --p->type_ptr;
    break;
    
  case CLEAR_OUTPUT:
    if (OUTPUT_TYPE(p))
      terminal_set_normal(p, flags, &out_node);
    
    ++p->type_ptr;
    while (p->type_ptr > 0)
      p->output_type[--p->type_ptr] = 0;
    break;
    
  case FOREGROUND_ONE:
  case FOREGROUND_TWO:
  case FOREGROUND_THREE:
  case FOREGROUND_FOUR:
  case FOREGROUND_FIVE:
  case FOREGROUND_SIX:
  case FOREGROUND_SEVEN:
  case FOREGROUND_EIGHT:
    if ((OUTPUT_TYPE(p) & FOREGROUND_ALL) != type)
    {
     OUTPUT_TYPE(p) &= FOREGROUND_OFF;
     OUTPUT_TYPE(p) |= type;

     terminal_set_foreground_colour(p, type - 1, flags, &out_node);
    }
    return (out_node);
    
  case BACKGROUND_ONE:
  case BACKGROUND_TWO:
  case BACKGROUND_THREE:
  case BACKGROUND_FOUR:
  case BACKGROUND_FIVE:
  case BACKGROUND_SIX:
  case BACKGROUND_SEVEN:
  case BACKGROUND_EIGHT:
    if ((OUTPUT_TYPE(p) & BACKGROUND_ALL) != type)
    {
     OUTPUT_TYPE(p) &= BACKGROUND_OFF;
     OUTPUT_TYPE(p) |= type;
     terminal_set_background_colour(p, (type >> 4) - 1, flags, &out_node);
    }
    return (out_node);
    
  case BOLD_ON:
    if (!(OUTPUT_TYPE(p) & BOLD_ON))
    {
     OUTPUT_TYPE(p) |= BOLD_ON;
     terminal_set_bold(p, flags, &out_node);
    }
    return (out_node);

  case DIM_ON:
    if (!(OUTPUT_TYPE(p) & DIM_ON))
    {
     OUTPUT_TYPE(p) |= DIM_ON;
     terminal_set_dim(p, flags, &out_node);
    }
    return (out_node);

  case FLASHING_ON:
    if (!(OUTPUT_TYPE(p) & FLASHING_ON))
    {
     OUTPUT_TYPE(p) |= FLASHING_ON;
     terminal_set_flash(p, flags, &out_node);
    }
    return (out_node);
    
  case UNDERLINE_ON:
    if (!(OUTPUT_TYPE(p) & UNDERLINE_ON))
    {
     OUTPUT_TYPE(p) |= UNDERLINE_ON;
     terminal_set_underline(p, flags, &out_node);
    }
    return (out_node);
    
  case INVERSE_ON:
    if (!(OUTPUT_TYPE(p) & INVERSE_ON))
    {
     OUTPUT_TYPE(p) |= INVERSE_ON;
     terminal_set_inverse(p, flags, &out_node);
    }
    return (out_node);
    
  case FOREGROUND_OFF:
  case BACKGROUND_OFF:
  case BOLD_OFF:
  case DIM_OFF:
  case FLASHING_OFF:
  case UNDERLINE_OFF:    
  case INVERSE_OFF:
    if ((OUTPUT_TYPE(p) & ~type))
      terminal_set_normal(p, flags, &out_node);
    else
      return (out_node);
    OUTPUT_TYPE(p) &= type;
    break;
    
  case EVERYTHING_OFF:
    if (OUTPUT_TYPE(p))
      OUTPUT_TYPE(p) = 0; /* falls through the reset break */
    else
      return (out_node);
    
  case RESET_OUTPUT: /* used just to switch everything to normal */
    terminal_set_normal(p, flags, &out_node);
    break;
    
  default:
    assert(0);
 }
 
 if (OUTPUT_TYPE(p)) /* switch things on, which should stay on */
 {
  if ((type = (OUTPUT_TYPE(p) & FOREGROUND_ALL)))
    terminal_set_foreground_colour(p, type - 1, flags, &out_node);
  
  if ((type = (OUTPUT_TYPE(p) & BACKGROUND_ALL)))
    terminal_set_background_colour(p, (type >> 4) - 1, flags, &out_node);
  
  if (OUTPUT_TYPE(p) & BOLD_ON)
    terminal_set_bold(p, flags, &out_node);

  if (OUTPUT_TYPE(p) & DIM_ON)
    terminal_set_dim(p, flags, &out_node);

  if (OUTPUT_TYPE(p) & FLASHING_ON)
    terminal_set_flash(p, flags, &out_node);
  
  if (OUTPUT_TYPE(p) & UNDERLINE_ON)
    terminal_set_underline(p, flags, &out_node);
  
  if (OUTPUT_TYPE(p) & INVERSE_ON)
    terminal_set_inverse(p, flags, &out_node);
 }
 
 return (out_node);
}

output_node *colour_load(player *p, int flags, unsigned int colour, 
                         output_node *out_node)
{
 unsigned int tmp = OUTPUT_TYPE(p);
 
 switch (colour)
 {
  case OUTPUT_COLOUR_SAY:
  case OUTPUT_COLOUR_ECHO:
  case OUTPUT_COLOUR_TELL:
  case OUTPUT_COLOUR_TFRIENDS:
  case OUTPUT_COLOUR_TFOF:
  case OUTPUT_COLOUR_MINE:
  case OUTPUT_COLOUR_SHOUTS:
  case OUTPUT_COLOUR_SOCIALS:
  case OUTPUT_COLOUR_SOCIALS_ME:
  case OUTPUT_COLOUR_DRAUGHTS:
  case OUTPUT_COLOUR_RECHO:
  case OUTPUT_COLOUR_MARRIAGE:
  case OUTPUT_COLOUR_CHAN_1:
  case OUTPUT_COLOUR_CHAN_2:
  case OUTPUT_COLOUR_CHAN_3:
  case OUTPUT_COLOUR_CHAN_4:
  case OUTPUT_COLOUR_CHAN_5:
  case OUTPUT_COLOUR_CHAN_6:
  case OUTPUT_COLOUR_CHAN_7:
  case OUTPUT_COLOUR_CHAN_8:
    OUTPUT_TYPE(p) = p->output[colour];
    break;

  case OUTPUT_COLOUR_OLD_SPOD:
  case OUTPUT_COLOUR_OLD_SUS:
  default:
    return (out_node); /* do nothing... */
 }

 if (tmp != OUTPUT_TYPE(p))
   return (colour_write(p, flags, RESET_OUTPUT, out_node));
 else
   return (out_node);
}
 
void colourise_set_defaults(player *p)
{ 
 /* ******************************************
  * one is supposed to be default background
  * eight is supposed to be default foreground */
  
 p->output[0] = (BACKGROUND_ONE | FOREGROUND_EIGHT); /* say */
 p->output[1] = (BACKGROUND_ONE | FOREGROUND_FOUR); /* echo */
 p->output[2] = (BACKGROUND_ONE | FOREGROUND_FIVE | BOLD_ON); /* tell */
 p->output[3] = (BACKGROUND_ONE | FOREGROUND_TWO | BOLD_ON);
 p->output[4] = (BACKGROUND_ONE | FOREGROUND_THREE | BOLD_ON);
 p->output[5] = (BACKGROUND_ONE | FOREGROUND_FOUR | BOLD_ON); /* unused */
 p->output[6] = (BACKGROUND_ONE | FOREGROUND_EIGHT | INVERSE_ON); /* unused */
 p->output[7] = (BACKGROUND_ONE | FOREGROUND_EIGHT);
 
 p->output[8] = (BACKGROUND_ONE | FOREGROUND_SEVEN | BOLD_ON);
 p->output[9] = (BACKGROUND_ONE | FOREGROUND_SEVEN);
 p->output[10] = p->output[0];
 p->output[11] = p->output[2];
 p->output[12] = p->output[2];
 p->output[13] = p->output[2];
 p->output[14] = p->output[2];
 p->output[15] = p->output[2]; /* chan 1 */
 p->output[16] = p->output[5];
 p->output[17] = p->output[6];
 p->output[18] = p->output[2];
 p->output[19] = p->output[2];
 p->output[20] = p->output[2];
 p->output[21] = p->output[2];
 p->output[22] = p->output[2]; /* chan 8 */
}

static const char *colourise_get_name(unsigned int offset)
{
 switch (offset)
 {
  case OUTPUT_COLOUR_SAY:
    return ("say");
  case OUTPUT_COLOUR_ECHO:
    return ("echo");
  case OUTPUT_COLOUR_TELL:
    return ("tell");
  case OUTPUT_COLOUR_TFRIENDS:
    return ("tell friends");
  case OUTPUT_COLOUR_TFOF:
    return ("tell friends of");
    /*  case OUTPUT_COLOUR_SPOD:
        return ("spod channel");
        case OUTPUT_COLOUR_SUS:
        return ("super channel");
    */
  case OUTPUT_COLOUR_MINE:
    return ("mine");
    /* case OUTPUT_COLOUR_CHAN_DEF:
       return ("channels default");
    */
  case OUTPUT_COLOUR_SHOUTS:
    return ("shouts");
  case OUTPUT_COLOUR_SOCIALS:
    return ("socials");
  case OUTPUT_COLOUR_SOCIALS_ME:
    return ("personal socials");
  case OUTPUT_COLOUR_DRAUGHTS:
    return ("draughts");
  case OUTPUT_COLOUR_RECHO:
    return ("recho");
  case OUTPUT_COLOUR_MARRIAGE:
    return ("marriage channel");
  case OUTPUT_COLOUR_CHAN_1:
    return ("channels 1");
  case OUTPUT_COLOUR_CHAN_2:
    return ("channels 2");
  case OUTPUT_COLOUR_CHAN_3:
    return ("channels 3");
  case OUTPUT_COLOUR_CHAN_4:
    return ("channels 4");
  case OUTPUT_COLOUR_CHAN_5:
    return ("channels 5");
  case OUTPUT_COLOUR_CHAN_6:
    return ("channels 6");
  case OUTPUT_COLOUR_CHAN_7:
    return ("channels 7");
  case OUTPUT_COLOUR_CHAN_8:
    return ("channels 8");
    
  default:
    return ("unknown");
 }
}

static void colour_show_table(player *p, int set_now)
{
 int count = 0;
 unsigned int flags = 0;
 char buffer[sizeof("Your $R-Nationality(us(color) def(colour)) types are%s")
            + sizeof(" ^S^Bnow^s")];
 char fore_colour[4];
 char back_colour[4];

 sprintf(buffer, "Your $R-Nationality(us(color) def(colour)) types are%s", 
         set_now ? " ^S^Bnow^s" : "");

 ptell_mid(NORMAL_T(p), buffer, FALSE);
 
 fvtell_player(NORMAL_T(p), "%-18s%-5s%-10s%-9s%-8s%-11s%s\n",
               "mode",
               "bold", "underline", "flashing", "inverse",
	       "foreground", "background");

 while (count < OUTPUT_COLOUR_SZ)
 {
  char buf1[256];
  char buf2[256];
  
  flags = p->output[count];

  switch (count)
  {
   case OUTPUT_COLOUR_OLD_SUS:
   case OUTPUT_COLOUR_OLD_SPOD:
   case OUTPUT_COLOUR_OLD_MAIN_CHAN:
     /* need to add, channels that you don't add here too */
     break;
     
   default:
     sprintf(fore_colour, "^%d", flags & FOREGROUND_ALL);
     sprintf(back_colour, "^9%d", (flags & BACKGROUND_ALL) >> 4);
     
     fvtell_player(NORMAL_T(p), "%-18s%-5s%-10s%-9s%-8s%s%-11s%s%s%s%s\n",
                   colourise_get_name(count),
                   TOGGLE_ON_OFF(flags & BOLD_ON),
                   TOGGLE_ON_OFF(flags & UNDERLINE_ON),
                   TOGGLE_ON_OFF(flags & FLASHING_ON),
                   TOGGLE_ON_OFF(flags & INVERSE_ON),
                   fore_colour,
                   word_number_base(buf1, 256, NULL, flags & FOREGROUND_ALL,
                                    TRUE, word_number_def), "^N",
                   back_colour,
                   word_number_base(buf2, 256, NULL,
                                    (flags & BACKGROUND_ALL) >> 4,
                                    TRUE, word_number_def), "^N");
     break;
  }
  
  ++count;
 }

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void colour_show_single(player *p, unsigned int flags,
                               int set_now, unsigned int offset)
{
 char buffer[1024]; /*cough* big enough *cough*/
 char fore_colour[4];
 char back_colour[4];
 char buf1[256];
 char buf2[256];
 
 sprintf(buffer, "Your $R-Nationality(us(color) def(colour)) type %s is%s", 
         colourise_get_name(offset),
         set_now ? " ^S^Bnow^s" : "");

 ptell_mid(NORMAL_T(p), buffer, FALSE);

 sprintf(fore_colour, "^%d", flags & FOREGROUND_ALL);
 sprintf(back_colour, "^9%d", (flags & BACKGROUND_ALL) >> 4);
 
 fvtell_player(NORMAL_T(p),
               " ^Bbold^b              = %s\n"
               " ^Iinverse^i           = %s\n"
               " ^Uunderline^u         = %s\n"
               " ^Fflashing^f          = %s\n"
               " foreground colour = %s%s%s\n"
               " background colour = %s%s%s\n",
               TOGGLE_ON_OFF(flags & BOLD_ON),
               TOGGLE_ON_OFF(flags & INVERSE_ON),
               TOGGLE_ON_OFF(flags & UNDERLINE_ON),
               TOGGLE_ON_OFF(flags & FLASHING_ON),
               fore_colour,
               word_number_base(buf1, 256, NULL, flags & FOREGROUND_ALL,
                                TRUE, word_number_def), "^N",
               back_colour,
               word_number_base(buf2, 256, NULL,
                                (flags & BACKGROUND_ALL) >> 4,
                                TRUE, word_number_def), "^N");

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void colourise_options(player *p, char *str, unsigned int flags_offset)
{
 char *tmp = str;
 unsigned int flags = 0;
 unsigned int did_something = 0;

 if (!*str)
   tmp = NULL;
 
 while (tmp)
 {
  char *endword = str;
  
  if ((endword = next_parameter(endword, ',')))
    *endword++ = 0;
  
  if (!beg_strcmp(tmp, "bold"))
    flags |= BOLD_ON;
  else if (!beg_strcmp(tmp, "underline"))
    flags |= UNDERLINE_ON;
  else if (!beg_strcmp(tmp, "flashing"))
    flags |= FLASHING_ON;
  else if (!beg_strcmp(tmp, "inverse"))
    flags |= INVERSE_ON;
  else if (!(strcmp(tmp, "nothing") && strcmp(tmp, "off")))
    did_something = 1; /* do nothing */
  else if (!(strcmp(tmp, "fore1") && strcmp(tmp, "f1") && 
             strcmp(tmp, "fore_colour1") && strcmp(tmp, "fore_color1") &&
             strcmp(tmp, "fore colour1") && strcmp(tmp, "fore color1")))
    flags = ((flags & FOREGROUND_OFF) | FOREGROUND_ONE);
  else if (!(strcmp(tmp, "fore2") && strcmp(tmp, "f2") &&
             strcmp(tmp, "fore_colour2") && strcmp(tmp, "fore_color2") &&
             strcmp(tmp, "fore colour2") && strcmp(tmp, "fore color2")))
    flags = ((flags & FOREGROUND_OFF) | FOREGROUND_TWO);
  else if (!(strcmp(tmp, "fore3") && strcmp(tmp, "f3") && 
             strcmp(tmp, "fore_colour3") && strcmp(tmp, "fore_color3") &&
             strcmp(tmp, "fore colour3") && strcmp(tmp, "fore color3")))
    flags = ((flags & FOREGROUND_OFF) | FOREGROUND_THREE);
  else if (!(strcmp(tmp, "fore4") && strcmp(tmp, "f4") &&
             strcmp(tmp, "fore_colour4") && strcmp(tmp, "fore_color4") &&
             strcmp(tmp, "fore colour4") && strcmp(tmp, "fore color4")))
    flags = ((flags & FOREGROUND_OFF) | FOREGROUND_FOUR);
  else if (!(strcmp(tmp, "fore5") && strcmp(tmp, "f5") &&
             strcmp(tmp, "fore_colour5") && strcmp(tmp, "fore_color5") &&
             strcmp(tmp, "fore colour5") && strcmp(tmp, "fore color5")))
    flags = ((flags & FOREGROUND_OFF) | FOREGROUND_FIVE);
  else if (!(strcmp(tmp, "fore6") && strcmp(tmp, "f6") &&
             strcmp(tmp, "fore_colour6") && strcmp(tmp, "fore_color6") &&
             strcmp(tmp, "fore colour6") && strcmp(tmp, "fore color6")))
    flags = ((flags & FOREGROUND_OFF) | FOREGROUND_SIX);
  else if (!(strcmp(tmp, "fore7") && strcmp(tmp, "f7") &&
             strcmp(tmp, "fore_colour7") && strcmp(tmp, "fore_color7") &&
             strcmp(tmp, "fore colour7") && strcmp(tmp, "fore color7")))
    flags = ((flags & FOREGROUND_OFF) | FOREGROUND_SEVEN);
  else if (!(strcmp(tmp, "fore8") && strcmp(tmp, "f8") &&
             strcmp(tmp, "fore_colour8") && strcmp(tmp, "fore_color8") &&
             strcmp(tmp, "fore colour8") && strcmp(tmp, "fore color8")))
    flags = ((flags & FOREGROUND_OFF) | FOREGROUND_EIGHT);
  else if (!(strcmp(tmp, "back1") && strcmp(tmp, "b1") &&
             strcmp(tmp, "back_colour1") && strcmp(tmp, "back_color1") &&
             strcmp(tmp, "back colour1") && strcmp(tmp, "back color1")))
    flags = ((flags & BACKGROUND_OFF) | BACKGROUND_ONE);
  else if (!(strcmp(tmp, "back2") && strcmp(tmp, "b2") &&
             strcmp(tmp, "back_colour2") && strcmp(tmp, "back_color2") &&
             strcmp(tmp, "back colour2") && strcmp(tmp, "back color2")))
    flags = ((flags & BACKGROUND_OFF) | BACKGROUND_TWO);
  else if (!(strcmp(tmp, "back3") && strcmp(tmp, "b3") &&
             strcmp(tmp, "back_colour3") && strcmp(tmp, "back_color3") &&
             strcmp(tmp, "back colour3") && strcmp(tmp, "back color3")))
    flags = ((flags & BACKGROUND_OFF) | BACKGROUND_THREE);
  else if (!(strcmp(tmp, "back4") && strcmp(tmp, "b4") &&
             strcmp(tmp, "back_colour4") && strcmp(tmp, "back_color4") &&
             strcmp(tmp, "back colour4") && strcmp(tmp, "back color4")))
    flags = ((flags & BACKGROUND_OFF) | BACKGROUND_FOUR);
  else if (!(strcmp(tmp, "back5") && strcmp(tmp, "b5") &&
             strcmp(tmp, "back_colour5") && strcmp(tmp, "back_color5") &&
             strcmp(tmp, "back colour5") && strcmp(tmp, "back color5")))
    flags = ((flags & BACKGROUND_OFF) | BACKGROUND_FIVE);
  else if (!(strcmp(tmp, "back6") && strcmp(tmp, "b6") &&
             strcmp(tmp, "back_colour6") && strcmp(tmp, "back_color6") &&
             strcmp(tmp, "back colour6") && strcmp(tmp, "back color6")))
    flags = ((flags & BACKGROUND_OFF) | BACKGROUND_SIX);
  else if (!(strcmp(tmp, "back7") && strcmp(tmp, "b7") &&
             strcmp(tmp, "back_colour7") && strcmp(tmp, "back_color7") &&
             strcmp(tmp, "back colour7") && strcmp(tmp, "back color7")))
    flags = ((flags & BACKGROUND_OFF) | BACKGROUND_SEVEN);
  else if (!(strcmp(tmp, "back8") && strcmp(tmp, "b8") &&
             strcmp(tmp, "back_colour8") && strcmp(tmp, "back_color8") &&
             strcmp(tmp, "back colour8") && strcmp(tmp, "back color8")))
    flags = ((flags & BACKGROUND_OFF) | BACKGROUND_EIGHT);
  else
  {
   fvtell_player(SYSTEM_T(p), " Invalid option -- ^S^B%s^s --.\n", tmp);
   return;
  }
  
  tmp = endword;
 }

 if (did_something || flags)
 {
  if (flags_offset == UINT_MAX)
  {
   for (did_something = 0; did_something < 11; ++did_something)
     p->output[did_something] = flags;
   colour_show_single(p, flags, TRUE, 0);
  }
  else
    colour_show_single(p, p->output[flags_offset] = flags, TRUE, flags_offset);
 }
 else
 {
  if (flags_offset == UINT_MAX)
    /* so they see what all there options are, they havn't set them */
    colour_show_table(p,  FALSE);
  else
    colour_show_single(p, p->output[flags_offset], FALSE, flags_offset);
 }
}

static void user_colourise(player *p, parameter_holder *params)
{
 switch (params->last_param)
 {
  case 1:
  {
   const char *str = "''";
   
   if (!get_parameter_parse(params, &str, 2))
     TELL_FORMAT(p, "<mode> [options_list]");
  }
  /* FALLTHROUGH */
  case 2:
    break;
    
  default:
    TELL_FORMAT(p, "<mode> [options_list]");
 }
 
 lower_case(GET_PARAMETER_STR(params, 1));
 if (params->last_param >= 2)
   lower_case(GET_PARAMETER_STR(params, 2));
 
 if (!strcmp(GET_PARAMETER_STR(params, 1), "default"))
 {
  colourise_set_defaults(p);
  fvtell_player(NORMAL_T(p),"%s",
                " You have set all modes to thier default value.\n");
 }
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "says") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "emotes")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_SAY);
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "echo"))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_ECHO);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "tells") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "remotes")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_TELL);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "tfriends") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "rfriends") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "tell_friends") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "remote_friends") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "tell friends") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "remote friends")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_TFRIENDS);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "tfof") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "rfof") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "tell_friendsof") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "remote_friendsof") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "tell friends of") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "remote friends of")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_TFOF);
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "mine"))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_MINE);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "shouts") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "emote_shouts")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_SHOUTS);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "socials")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_SOCIALS);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "personal_socials") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "personal socials") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "socials_to_me") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "socials to me")))
   colourise_options(p, GET_PARAMETER_STR(params, 2),
                     OUTPUT_COLOUR_SOCIALS_ME);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "draughts") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "checkers")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_DRAUGHTS);
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "recho"))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_RECHO);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "marriage channel") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "marriage_channel")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_MARRIAGE);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "chan_1") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan1") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan 1") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels1") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels_1") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels 1")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_CHAN_1);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "chan_2") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan2") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan 2") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels_2") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels2") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels 2")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_CHAN_2);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "chan_3") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan3") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan 3") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels_3") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels3") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels 3")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_CHAN_3);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "chan_4") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan4") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan 4") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels_4") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels4") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels 4")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_CHAN_4);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "chan_5") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan5") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan 5") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels_5") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels5") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels 5")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_CHAN_5);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "chan_6") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan6") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan 6") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels_6") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels6") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels 6")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_CHAN_6);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "chan_7") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan7") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan 7") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels_7") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels7") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels 7")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_CHAN_7);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "chan_8") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan8") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "chan 8") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels_8") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels8") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "channels 8")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), OUTPUT_COLOUR_CHAN_8);
 else if (!(beg_strcmp(GET_PARAMETER_STR(params, 1), "all") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "list") &&
            beg_strcmp(GET_PARAMETER_STR(params, 1), "everything")))
   colourise_options(p, GET_PARAMETER_STR(params, 2), UINT_MAX);
 else
 {
  fvtell_player(SYSTEM_T(p),
                " The mode '^B%s^N' doesn't exist, try one of:-\n  "
                "all, says, echo, tells, tfriends, tfof, "
                "mine, shouts, socials, marriage_channel, "
                "personal_socials, draughts, channels default and "
                "channels 1 through to 8.\n", 
                GET_PARAMETER_STR(params, 1));
 }
}

static void user_setup_colours(player *p, const char *str)
{
 parameter_holder params;

 if (!p->termcap)
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You need to have set a terminal name, or your client needs "
                "send that info for you (unix telnet does this, where tf "
                "doesn't).\n"
                " -=> Try typing <help terminal>\n");
  return;
 }
  
 if (!p->termcap->Sf)
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You need to use a terminal that will support colour, "
                "or you need to set that you want ANSI colour.\n"
                " -=> Try typing <help ansi_override>\n");
  return;
 }
 
 get_parameter_init(&params);
 if (!get_parameter_parse(&params, &str, 3))
 {
  fvtell_player(NORMAL_T(p), " %s%s\n" " %s%s\n" " %s%s\n" " %s%s\n"
                " %s%s\n" " %s%s\n" " %s%s\n" " %s%s\n",
                current_command, " on",
                current_command, " off",
                current_command, " user wands",
                current_command, " user colour",
                current_command, " user all",
                current_command, " system wands",
                current_command, " system colour",
                current_command, " system only");
  return;
 }

 if (params.last_param > 0)
 {
  if (TOGGLE_MATCH_ON(GET_PARAMETER_STR((&params), 1)))
    WANDS_ON(p);
  else if (TOGGLE_MATCH_OFF(GET_PARAMETER_STR((&params), 1)))
    WANDS_OFF(p);
  else if (!beg_strcmp(GET_PARAMETER_STR((&params), 1), "user") &&
           (params.last_param > 1))
  {
   assert(params.last_param == 2);
   
   if (!(beg_strcmp(GET_PARAMETER_STR((&params), 2), "colour") &&
         beg_strcmp(GET_PARAMETER_STR((&params), 2), "color")))
   {
    WANDS_ON(p);
    SYS_SPECIALS(p);	    
   }
   else if (!beg_strcmp(GET_PARAMETER_STR((&params), 2), "wands"))
   {
    WANDS_ON(p);
    SYS_COLOUR(p);	    
   }
   else if (!beg_strcmp(GET_PARAMETER_STR((&params), 2), "all"))
     WANDS_ON(p);
  }
  else if (!beg_strcmp(GET_PARAMETER_STR((&params), 1), "system") &&
           (params.last_param > 1))
  {
   assert(params.last_param == 2);
   
   if (!(beg_strcmp(GET_PARAMETER_STR((&params), 2), "colour") &&
         beg_strcmp(GET_PARAMETER_STR((&params), 2), "color")))
   {
    WANDS_ON(p);
    SYS_COLOUR(p);	    
   }
   else if (!beg_strcmp(GET_PARAMETER_STR((&params), 2), "wands"))
   {
    WANDS_ON(p);
    SYS_SPECIALS(p);	    
   }
   else if (!beg_strcmp(GET_PARAMETER_STR((&params), 2), "only"))
   {
    WANDS_ON(p);
    SYS_COLOUR(p);
    SYS_SPECIALS(p);
   }
  }
  else if (!beg_strcmp(GET_PARAMETER_STR((&params), 1), "help"))
  {
   fvtell_player(NORMAL_T(p), " %s%s\n" " %s%s\n" " %s%s\n" " %s%s\n"
                 " %s%s\n" " %s%s\n" " %s%s\n" " %s%s\n",
                 current_command, " on",
                 current_command, " off",
                 current_command, " user wands",
                 current_command, " user colour",
                 current_command, " user all",
                 current_command, " system wands",
                 current_command, " system colour",
                 current_command, " system only");
   return;
  }
 }

 if (p->flag_just_normal_hilight)
   fvtell_player(NORMAL_T(p), "%s",
                 " You will only get the normal hilighting.\n");
 else
   if (p->flag_no_colour_from_others)
   {
    if (p->flag_no_specials_from_others)
      fvtell_player(NORMAL_T(p), "%s",
                    " Other people will not be able to use wands or "
                    "colours to you.\n");
    else
      fvtell_player(NORMAL_T(p), "%s",
                    " Other people will only be able to use wands to you.\n");
   }
   else
   {
    if (p->flag_no_specials_from_others)
      fvtell_player(NORMAL_T(p), "%s",
                    " Other people will only be able to use "
                    "colours to you.\n");
    else
      fvtell_player(NORMAL_T(p), "%s", " You will get wands and colours, "
                    "from other people.\n");
   }
}

void cmds_init_colourise(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("user_colourise", user_colourise, PARSE_PARAMS, SETTINGS);

 CMDS_ADD("hilight", user_setup_colours, CONST_CHARS, SETTINGS);
}
