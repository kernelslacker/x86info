/*
   08/06/2016

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

#ifndef COMMON_H_
#define COMMON_H_

void exit_with_err(const char *, const char *);
void get_ssd(char *, uint8_t num);
void get_kernel(char *, uint8_t num);
void get_packs(char *);
void get_taim(char *);
void get_uptime(char *);
void check_fan_vals(char *, uint_fast16_t *, uint_fast16_t iterz);
void split_n_index(char *);

#if !defined(HAVE_SENSORS_SENSORS_H) && !defined(__OpenBSD__)
void get_fans(char *);
#endif /* !HAVE_SENSORS_SENSORS_H && !__OpenBSD__ */

#if defined(__linux__)
void get_temp(const char *, char *);
#endif /* __linux__ */

#if defined(__FreeBSD__)
void get_temp(char *, uint_least32_t temp);
#endif /* __FreeBSD__ */

#if defined (HAVE_X11_XLIB_H)
void set_status(const char *);
#endif

#if defined(HAVE_CDIO_CDIO_H) || defined(__linux__)
void get_dvd(char *);
#endif /* HAVE_CDIO_CDIO_H || __linux__ */

#if defined(__FreeBSD__) || defined(__OpenBSD__)
void get_loadavg(char *);
#endif

#endif /* COMMON_H_ */
