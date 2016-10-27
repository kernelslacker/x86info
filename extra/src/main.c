/*
   Copyright 02/22/2015, 07/18/2016

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
*/

#include "config.h" /* Auto-generated */
#include "include/headers.h"
#include "prototypes/options.h"

int main(int argc, char *argv[]) {

  if (1 == argc) {
    FPRINTF("%s\n", "Try --help or --usage for more information.");
    return EXIT_FAILURE;
  }
  if (-1 == (sysconf(_SC_CLK_TCK))) {
    FPRINTF("%s\n", "Error: sysconf() failed");
    return EXIT_FAILURE;
  }

  char combined[WHOLE_MAIN_ARR_LEN];
  char *all = combined;
  parse_opts(argc, argv, all);

  if ('\0' != combined[0]) {
#if defined (HAVE_X11_XLIB_H)
    set_status(combined);
#else
    fprintf(stdout, "%s\n", combined);
#endif
  }

  return EXIT_SUCCESS;
}
