#define HANGMAN_C
/*
 * EW-too base code addition by PeeKaBoo
 * --------------------------------------------------------------------------
 *
 * Hangman v2.0
 *
 * Copyright (C) 1999 PeeKaBoo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#if 1 /* main.h defines TWOCAN_CODE */
# include "main.h"
#else
# include <stdio.h>
# include <string.h>
# include "include/config.h"
# include "include/player.h"
# include "include/proto.h"
# include <stdlib.h>
# include <ctype.h>
# include <fcntl.h> 
# include <sys/stat.h> 
# include <unistd.h> 
# include <stdio.h> 
# include "include/hangman.h"
#endif

static const char *HangPics[8][10] = 
{
 {
  " " HIGH "|" OFF,
  " " HIGH "|" OFF,
  " " HIGH "|" OFF,
 },
 
 {
  " " HIGH "|    " YELLOW "O" OFF,
  " " HIGH "|" OFF ,
  " " HIGH "|" OFF,
 },
 
 {
  " " HIGH "|    " YELLOW "O" GREEN "_" OFF,
  " " HIGH "|      " YELLOW "'" OFF,
  " " HIGH "|" OFF,
 },
 
 {
  " " HIGH "|   " GREEN "_" YELLOW "O" GREEN "_" OFF,
  " " HIGH "|  " YELLOW "'   '" OFF,
  " " HIGH "|" OFF,
 },
 
 {
  " " HIGH "|   " GREEN "_" YELLOW "O" GREEN "_" OFF,
  " " HIGH "|  " YELLOW "' " GREEN "| " YELLOW "'" OFF,
  " " HIGH "|" OFF,
 },
 
 {
  " " HIGH "|   " GREEN "_" YELLOW "O" GREEN "_" OFF,
  " " HIGH "|  " YELLOW "' " GREEN "| " YELLOW "'" OFF,
  " " HIGH "|   " PURPLE "/" ESCAPE "^" OFF,
 },
 
 {
  " " HIGH "|   " GREEN "_" YELLOW "O" GREEN "_" OFF,
  " " HIGH "|  " YELLOW "' " GREEN "| " YELLOW "'" OFF,
  " " HIGH "|   " PURPLE "/" ESCAPE "^\\" OFF,
 },
 
 {""},
};

static char *hangman_text = NULL;
static size_t hangman_sz = 0;

static char **hangman_words = 0;
static int hangman_numwords = 0;

static int hangman_reload_words(int force)
{
 static time_t hangman_saved_time = 0;
 struct stat buf;
 int num = 0;
 char *word = NULL;
 char *tmp = NULL;
 
 if (!force && !stat(WORDFILE, &buf) &&
     !difftime(buf.st_mtime, hangman_saved_time))
   return (TRUE);
 
 if (hangman_text)
   FREE(hangman_text);
 
 if (!(hangman_text = file2text(WORDFILE, &hangman_sz)))
   return (FALSE);
 
 hangman_numwords = 0;
 
 if (hangman_words)
   FREE(hangman_words);
 
 word = hangman_text;
 while ((tmp = strchr(word, '\n')) != NULL)
 {
  if ((tmp - word) > 0) 
    num++;
  
  word = ++tmp;
 }
 
 if (!(hangman_words = MALLOC(num * sizeof(char *))))
   return (FALSE);
 
 hangman_numwords = num;
 num = 0;
 word = hangman_text;
 hangman_words[num++] = word;
 
 while ((word = strchr(word, '\n')) != NULL)
 {
  *word++ = 0;
  if (num < hangman_numwords)
    hangman_words[num++] = word;
 }
 
 hangman_saved_time = buf.st_mtime;
 return (TRUE);
}

static void hangman_view(player *p, int finished)
{
 int i;
 char *temp;

 ptell_mid(NORMAL_T(p), "hangman", FALSE);
 
 if (finished)
   fvtell_player(NORMAL_T(p), "%s", "Your word was: ");
 else
   fvtell_player(NORMAL_T(p), "%s", "Your word is: ");
 
 for (temp = p->hangman->word; *temp; temp++)
 {
  if ((p->hangman->letters & HANGMAN_MAGIC_TEST(*temp)) || finished)
    fvtell_player(NORMAL_T(p), "%c", *temp);
  else
    fvtell_player(NORMAL_T(p), "%c", '_');
  
  if (!finished)
    fvtell_player(NORMAL_T(p), "%c", ' ');
 }
 
 fvtell_player(NORMAL_T(p), "\n        " HIGH "___\n       " HIGH "/   |\n");
 
 for (i = 0; i < 3; i++)	/* loop through each line of pic */
 {
  fvtell_player(NORMAL_T(p), "     %s\n", HangPics[p->hangman->misses][i]);
 }
 
 fvtell_player(NORMAL_T(p), "    " HIGH "__|___%s\n\n", "^N");
 
 if (finished)
 {
  if (finished == 1)
  {
#ifdef HAVE_MONETRY_SYSTEM
   if (p->hangman->bet)
   {
    fvtell_player(NORMAL_T(p), HIGH " You win %d %s!!%s\n",
                  p->hangman->bet, MONEY, "^N");
   } else 
#endif
     fvtell_player(NORMAL_T(p), HIGH " You win!!%s\n", "^N");
  }
  else if (finished == 2)
  {
#ifdef HAVE_MONETRY_SYSTEM
   if (p->hangman->bet)
   {
    fvtell_player(NORMAL_T(p), HIGH " You lose %d %s!!%s\n",
                  p->hangman->bet, MONEY, "^N");
   }
   else 
#endif
     fvtell_player(NORMAL_T(p), HIGH " You lose!!%s\n", "^N");
  }
  fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
  return;
 }
 
 fvtell_player(NORMAL_T(p), "Letters used: ");
 
 for (i = 97; i <= 122; i++) /* magic loop */
 {
  if (p->hangman->letters & HANGMAN_MAGIC_TEST(i))
    fvtell_player(NORMAL_T(p), "%c ", i);
 }
 
 fvtell_player(NORMAL_T(p), "\n You have %d misses\n", p->hangman->misses);
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

void hangman_clear(player *p)
{
 if (p->hangman)
 {
  if (p->hangman->word)
    FREE(p->hangman->word);
  
  FREE(p->hangman);

  fvtell_player(SYSTEM_T(p),
                " Your " HIGH "%s" OFF " game is cleared as you quit.\n",
                HANGMAN);
 }
 
 p->hangman = NULL;
}

static int hangman_new(player *p, int bet)
{
 int randNum;
 size_t len = 0;
 
 if (!hangman_reload_words(FALSE) || !hangman_numwords)
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " There are no ^S^Bwords^s in the hangman dicionary atm.\n");
  return (FALSE);
 }
 
 if ((randNum = rand()))
   randNum %= hangman_numwords;
 
 if (!(p->hangman = MALLOC(sizeof(hangman_type))))
   goto malloc_err;

 len = strlen(hangman_words[randNum]);
 if (!(p->hangman->word = MALLOC(len + 1)))
   goto malloc_word_err;
 
 COPY_STR_LEN(p->hangman->word, hangman_words[randNum], len);
 
 p->hangman->bet = bet;
 p->hangman->misses = 0;
 p->hangman->letters = 0;

 return (TRUE);
 
 malloc_word_err:
 FREE(p->hangman);
 malloc_err:
 P_MEM_ERR(p);
 return (FALSE);
}

static int hangman_check_win(player *p)
{
 char *temp = NULL;
 int notfound=0;
 
 for (temp = p->hangman->word; *temp; temp++)
 {
  if (!(p->hangman->letters & HANGMAN_MAGIC_TEST(*temp)))
    notfound = 1;
 }
 
 return (!notfound);
}

static void internal_hangman_do(player *p, const char *str)
{
 int found = 0;
 char letter = *str;
 char *temp = NULL;
 
 /* check if they are guessing the entire word */
 if (*str && str[1])
 {
  if (!strcasecmp(str, p->hangman->word))
  {
   hangman_view(p,1);
#ifdef HAVE_MONETRY_SYSTEM
   p->pennies += 2 * p->hangman->bet;
#endif
   hangman_clear(p);
   return;
  }
 }
 else
 {
  letter = tolower(letter);
  if (p->hangman->letters & HANGMAN_MAGIC_TEST(letter))
  {
   fvtell_player(SYSTEM_T(p), HIGH " -=> You already guessed '%c'\n", letter);
   return;
  }
  
  p->hangman->letters |= HANGMAN_MAGIC_TEST(letter);
  
  for (temp = p->hangman->word; *temp; temp++)
  {
   if (tolower((unsigned char) *temp) == letter)
     found = 1;
  }
 }
 
 if (!found)
   p->hangman->misses++;
 
 if (p->hangman->misses > MISSES)
 {
  hangman_view(p, 2);
  hangman_clear(p);
  return;
 }
 
 if (hangman_check_win(p))
 {
  hangman_view(p,1);
#ifdef HAVE_MONETRY_SYSTEM
  p->pennies += 2 * p->hangman->bet;
#endif
  hangman_clear(p);
  return;
 }
 
 hangman_view(p,0);
}

static void user_hangman_play(player *p, const char *str)
{
 int bet = 0;
 
 if (*str && str[1])
 {
  if (!beg_strcasecmp(str, "clear") || !beg_strcasecmp(str, "quit"))
  {
   if (!p->hangman || !p->hangman->word)
   {
    fvtell_player(SYSTEM_T(p), HIGH " -=> You're not playing %s though.\n",
                  HANGMAN);
    return;
   }
   
   fvtell_player(NORMAL_T(p), HIGH " -=> You quit the %s game.\n", HANGMAN);
   hangman_clear(p);
  }
  else if (!beg_strcasecmp(str, "view"))
  {
   if (!p->hangman || !p->hangman->word)
   {
    fvtell_player(SYSTEM_T(p), HIGH " -=> You're not playing %s though.\n",
                  HANGMAN);
    return;
   }
   
   hangman_view(p, 0);
  }
  return;
 }
 
 /* check if a game is already in progress */
 if (p->hangman && p->hangman->word)
 { /* game exits already */
  if (!*str || !isalpha((unsigned char) *str))
    TELL_FORMAT(p, "<letter>|<word>");
  else
    internal_hangman_do(p, str);
  return;
 }
 
 /* We are starting a new game */
 bet = 0;
#ifdef HAVE_MONETRY_SYSTEM
 if (!str || !*str)
 {
  bet = 0;
 }
 else if (!isdigit((unsigned char) *str) )
 {
  TELLPLAYER(p, HIGH " -=> Your wager must be a number!\n");
  return;
 }
 else
 {
  bet = atoi(str);
 }
 
 if (bet < 0)
 {
  fvtell_player(NORMAL_T(p), HIGH " -=> You must bet a positive amount!\n");
  return;
 }
 else if (!p->residency && (bet != 0))
 {
  fvtell_player(NORMAL_T(p),
                HIGH " -=> Sorry, only residents can bet %s on a game.\n",
                MONEY);
  return;
 }
 else if (bet > p->pennies)
 {
  fvtell_player(NORMAL_T(p),
                HIGH " -=> You can't bet what you don't have!\n");
  return;
 }
#endif

 if (!hangman_new(p, bet))
   return;
 
#ifdef HAVE_MONETRY_SYSTEM
 p->pennies -= p->hangman->bet;
#endif
 
 hangman_view(p, 0);
}

static void user_su_hangman_add(player *p, const char *str)
{ /* FIXME: can't dynamically add words for hangman */
 IGNORE_PARAMETER(str);
 fvtell_player(NORMAL_T(p), " Not working atm.\n");
}

#ifndef TWOCAN_CODE
void hangman_version(void)
{
 sprintf(stack, " -=*> Hangman v2.0 (by PeeKaBoo) enabled.\n");
 stack = strchr(stack, 0);
}

#else

static void user_hangman_version(player *p)
{
 fvtell_player(NORMAL_T(p), " -=*> Hangman v2.0 (by PeeKaBoo).\n");
}

void init_hangman(void)
{
 hangman_reload_words(TRUE);
}

void user_configure_game_hangman_use(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, configure.game_hangman_use, TRUE,
                       " The hangman game is %splayable.\n",
                       " The hangman game is %snot playable.\n", TRUE);

 configure_save(FALSE);
}

void cmds_init_hangman(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("hangman", user_hangman_play, CONST_CHARS, GAME);
 CMDS_PRIV(configure_game_hangman);
 CMDS_ADD("hangman_add", user_su_hangman_add, CONST_CHARS, ADMIN);
 CMDS_PRIV(admin);
 CMDS_ADD("hangman_version", user_hangman_version, NO_CHARS, GAME);
 CMDS_PRIV(configure_game_hangman);
}

#endif

