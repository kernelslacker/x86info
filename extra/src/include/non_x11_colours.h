/*
   07/24/2016

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

#ifndef COLOURS_H_
#define COLOURS_H_


#if WITH_COLOURS == 1
#define XBM_ICON(given_icon) "^i("ICONS_DIR"/"given_icon")"
#define YELLOW   "^fg(#f0c674)"
#define BLUE     "^fg(#1793D1)"
#define PINK     "^fg(#b294bb)"

#define NAME_VAL  BLUE STR_SPEC " " PINK STR_SPEC      /* STR1 STR2        */
#define TEMP      YELLOW "%s" XBM_ICON("temp.xbm") " " /* 32C              */
#define FMT_KERN  YELLOW KERN_ORIG                     /* Kernel Version   */
#define FMT_SONG  PINK SONG_ORIG                       /* Song             */

#define CPU_STR    XBM_ICON("cpu.xbm")
#define RAM_STR    XBM_ICON("mem.xbm")
#define SSD_STR    XBM_ICON("diskette.xbm")
#define PKG_STR    XBM_ICON("arch.xbm")
#define VOLT_STR   XBM_ICON("voltage.xbm")
#define FANS_STR   XBM_ICON("fan.xbm")
#define MOBO_STR   XBM_ICON("mobo.xbm")
#define VOL_STR    XBM_ICON("vol.xbm")
#define TIME_STR   XBM_ICON("clock.xbm")
#define MPD_ICON   XBM_ICON("mpd.xbm")
#define NET_STR    XBM_ICON("net.xbm")
#define SPEED_STR  XBM_ICON("net.xbm")
#define STATIO_STR XBM_ICON("statio.xbm")
#define BATT_STR   XBM_ICON("bat.xbm")
#define LOAD_STR   CPU_STR
#define UP_STR     XBM_ICON("uptime.xbm")

/* Only for the weather */
#define OUT_STR    BLUE XBM_ICON("fox.xbm")" "

/* tmux or other program that can spice
 * the output on it's own */
#else

#define PINK ""
#define YELLOW ""
#define NAME_VAL  STR_SPEC " " STR_SPEC               /* STR1 STR2       */
#define TEMP      STR_SPEC "C "                       /* 32C             */
#define FMT_KERN  KERN_ORIG                           /* Kernel Version  */
#define FMT_SONG  SONG_ORIG                           /* Song            */

#endif /* WITH_COLOURS */


#endif /* COLOURS_H_ */
