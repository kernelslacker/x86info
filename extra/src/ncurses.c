/*
   09/03/2016

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.

  Compile with:
   gcc -std=c99 -D_POSIX_C_SOURCE=200112L -Wall -Wextra -O2 ncurses.c -o pinky_curses -lncurses
  BSD users should type -D_DEFAULT_SOURCE instead.
*/

#include "config.h" /* Auto-generated */

#if WITH_NCURSES != 1
#error "Have you forgot --with-ncurses ?"
#endif /* WITH_NCURSES */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include <locale.h>
#include <unistd.h>
#include <signal.h>

#include <ncurses.h>

#define VLA 1000
#define EIGHT 8
#define WIN95 stdscr

#define RESTORE_CURSOR() \
  echo(); \
  nl(); \
  curs_set(TRUE); \
  endwin();

void unuglify(char *);
void sighandler(int num);
void init_da_handler(void);

volatile sig_atomic_t call_it_quits = 0;

int main(void) {
  init_da_handler();
  if (NULL == (setlocale(LC_ALL, ""))) {
    return EXIT_FAILURE;
  }

  int16_t color_pair = 1, fg = 1, bg = 1, x = 0, z = 0;
  int32_t old_x = 0, old_y = 0, new_y = 0, new_x = 0;
  char buf[VLA];

  initscr();
  noecho();
  nocbreak();
  nodelay(WIN95, TRUE);
  nonl();
  intrflush(WIN95, FALSE);
  curs_set(FALSE);

  if(FALSE == (has_colors())) {
    RESTORE_CURSOR();
    return EXIT_FAILURE;
  }
  start_color();

  for (x = 0; x < EIGHT; x++) {
    for (z = 0; z < EIGHT; z++) {
      init_pair((int16_t)((x * EIGHT) + z), x, z);
    }
  }

  pair_content(color_pair, &fg, &bg);
  getmaxyx(WIN95, old_y, old_x);

  while (1) {
    getmaxyx(WIN95, new_y, new_x);
    if (old_y != new_y || old_x != new_x) {
      old_y = new_y;
      old_x = new_x;
      werase(WIN95);
      wrefresh(WIN95);
    }
    if (NULL != (fgets(buf, VLA, stdin))) {
      unuglify(buf);
    }
    if (1 == call_it_quits) {
      break;
    }
  }

  RESTORE_CURSOR();
  fprintf(stderr, "\n%s\n", "See you later");

  return EXIT_SUCCESS;
}

/* 
 * The signal handler is based on
 * https://www.gnu.org/software/libc/manual/html_node/Advanced-Signal-Handling.html#Advanced-Signal-Handling
*/
void init_da_handler(void) {
  struct sigaction setup_action;
  memset(&setup_action, 0, sizeof(struct sigaction));

  setup_action.sa_handler = &sighandler;
  setup_action.sa_flags = 0;

  if (-1 == (sigaction(SIGINT, &setup_action, NULL))) {
    fprintf(stderr, "%s\n", "sigaction() failed");
    exit(EXIT_FAILURE);
  }
}

/* !!! WARNING !!! */
void sighandler(int num) {
/* ASYNC CODE ONLY */
  (void)num;
  call_it_quits = 1;
}

void unuglify(char *str1) {
  static uint8_t iclr = 0;
  char *ptr = str1;
  static const char discard[] = "\"*;`{}~|";

  wmove(WIN95, 0, 0);
  wclrtoeol(WIN95);

  for (; *ptr; ptr++) {
    if (0 != (iscntrl((unsigned char) *ptr)) ||
     0 == (isprint((unsigned char) *ptr))) {
      continue;
    }
    if ('^' == *ptr)  {
      ++ptr;
      switch((toupper((unsigned char) *ptr))) {
        case 'B':
          iclr = COLOR_BLUE;
          break;
        case 'M':
          iclr = COLOR_MAGENTA;
          break;
        case 'Y':
          iclr = COLOR_YELLOW;
          break;
        default:
          /* Can be cntrl since we advanced with 1 char */
          break;
      }
      wattrset(WIN95,
        COLOR_PAIR((int32_t)((iclr % EIGHT) * EIGHT)) | (int32_t)A_BOLD);
    } else {
      if (NULL == (strchr(discard, *ptr))) {
        waddch(WIN95, (chtype)*ptr);
      }
    }
  }
  wrefresh(WIN95);
}
