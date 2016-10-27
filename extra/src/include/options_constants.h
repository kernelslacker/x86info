/*
   07/29/2015, 07/18/2016

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

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#define VLA 200
#define WHOLE_MAIN_ARR_LEN VLA*16
#define GLUE(x, z, ...) (x+=snprintf(x, WHOLE_MAIN_ARR_LEN, z, __VA_ARGS__))

#if defined(__FreeBSD__)
/* The clock ticks in FreeBSD are 133 */
#define TICKZ 100
#else
#define TICKZ sysconf(_SC_CLK_TCK)
#endif

#include "options_generator.h"

#define STR_SPEC "%s"
#define SONG_ORIG STR_SPEC " "
#define KERN_ORIG STR_SPEC " "

#ifndef WITH_NCURSES
#define WITH_NCURSES 6
#endif
#if defined (HAVE_X11_XLIB_H) || WITH_NCURSES == 1 || WITH_COLOURS == 0

#define CPU_STR    "CPU"
#define RAM_STR    "RAM"
#define SSD_STR    "SSD"
#define PKG_STR    "Pkgs"
#define VOLT_STR   "Voltage"
#define FANS_STR   "Fans/RPM"
#define MOBO_STR   "Mobo"
#define VOL_STR    "Volume"
#define TIME_STR   "Time"
#define NET_STR    "Used Net"
#define STATIO_STR "Statio"
#define SPEED_STR  "Speed"
#define BATT_STR   "BATT"
#define UP_STR     "Up"
#define LOAD_STR   "Load/avg"
#define OUT_STR    "Out "

#endif /* HAVE_X11_XLIB_H || WITH_NCURSES || WITH_COLOURS */


#if defined (HAVE_X11_XLIB_H)
#include "x11_colours.h"

#elif WITH_NCURSES == 1
#include "ncurses_colours.h"

#else
#include "non_x11_colours.h"

#endif /* HAVE_X11_XLIB_H */


/* options.c format constants */
#define FMT           NAME_VAL"%% "                       /* STR1 10%             */
#define FMT_TIME      NAME_VAL                            /* Time 10:00 PM        */
#define FMT_MOBO      FMT_TIME" "                         /* VEND NAME            */
#define FMT_CPU       FMT_TIME"%% "                       /* CPU 10%              */
#define FMT_CORES     FMT_TIME                            /* CPU varying          */
#define FMT_RAM       FMT                                 /* RAM 10%              */
#define FMT_RAM2      FMT_TIME" "                         /* RAM 10MB             */
#define FMT_SSD       FMT                                 /* SSD 10%              */
#define FMT_SSD2      FMT_RAM2                            /* SSD 10MB             */
#define FMT_PKGS      FMT_TIME" "                         /* Pkgs 123             */
#define FMT_VOLT      FMT_PKGS                            /* Voltage 1 2 3 4      */
#define FMT_FANS      FMT_TIME                            /* Fans varying         */
#define FMT_VOL       FMT                                 /* Volume 10%           */
#define FMT_NET       FMT_PKGS                            /* Down 123 Up 123      */
#define FMT_STATIO    FMT_NET                             /* Read 123 Written 123 */
#define FMT_CPUSPEED  PINK STR_SPEC " "                   /* 1234 MHz             */
#define FMT_TEMP      TEMP                                /* 32C                  */
#define FMT_BATT      FMT                                 /* BATT 10%             */
#define FMT_UP        FMT_TIME" "                         /* Up 10 min            */
#define FMT_LOAD      FMT_TIME" "                         /* Load/avg 0.01 0.01 0.01       */


#endif /* CONSTANTS_H_ */
