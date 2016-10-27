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
*/

#ifndef NCURSES_COLOURS_H_
#define NCURSES_COLOURS_H_


#if WITH_COLOURS == 1
#define PINK "^M"
#define BLUE "^B"
#define YELLOW "^Y"

#else
#define PINK ""
#define BLUE ""
#define YELLOW ""

#endif /* WITH_COLOURS */

#define NAME_VAL  BLUE STR_SPEC " " PINK STR_SPEC      /* STR1 STR2       */
#define TEMP      YELLOW STR_SPEC "C "                 /* 32C             */
#define FMT_KERN  YELLOW KERN_ORIG                     /* Kernel Version  */
#define FMT_SONG  PINK SONG_ORIG                       /* Song            */


#endif /* NCURSES_COLOURS_H_ */
