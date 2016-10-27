/*
   07/06/2015, 07/18/2016

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

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

void get_ram(char *, uint8_t num);
void get_voltage(char *);
void get_mobo(char *);
void get_mobo_temp(char *);
void get_cpu_temp(char *);
void get_battery(char *);
void get_statio(char *, char *);

#if defined(__linux__)
void get_ssd_model(char *, char *);
void get_loadavg(char *);

#if defined(HAVE_SENSORS_SENSORS_H)
void get_fans(char *);
void match_feature(char *, uint8_t num);
#endif /* HAVE_SENSORS_SENSORS_H */

#endif /* __linux__ */


#if defined(__FreeBSD__) || defined(__OpenBSD__)
void get_swapp(char *, uint8_t num);
#endif /* __FreeBSD__ || __OpenBSD__ */

#if defined(__OpenBSD__)
void get_fans(char *);
void match_feature(char *str1, int8_t sens_type, int8_t sens_num);
#endif /* __OpenBSD__ */

#endif /* FUNCTIONS_H_ */
