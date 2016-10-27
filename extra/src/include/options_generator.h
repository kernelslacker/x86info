/*
   08/12/2016

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

#ifndef GENERATOR_H_
#define GENERATOR_H_

/* Employ the preprocessor */
#define GET_N_FMT(func, ...) \
  get_##func(func); \
  GLUE(__VA_ARGS__);

/* Remember vanilla ice ? */
#define SLEEP_SLEEP_BABY(x) \
  tc.tv_nsec = x; \
  if (-1 == (nanosleep(&tc, NULL))) { \
    printf("%s\n", "Error: nanosleep() failed"); \
    return EXIT_FAILURE; \
  }

#define GET_NET_N_FMT(arg1, arg2, arg3, ...) \
  get_net(arg1, arg2, arg3); \
  if (2 == arg3)  { \
    SLEEP_SLEEP_BABY(850000000L); \
    get_net(arg1, arg2, arg3); \
  } \
  GLUE(__VA_ARGS__);

/* Let the platform ticks decide how much
 * time is needed to sleep
 * 100 in linux and openbsd, 133 in freebsd */
#define NEW_CPU_LABEL(lbl, declareVar, useVar, ...) \
  case lbl: \
  { \
    declareVar; \
    get_##useVar(useVar); \
    SLEEP_SLEEP_BABY(sysconf(_SC_CLK_TCK) * 1000000L); \
    GET_N_FMT(useVar, arguments->all, __VA_ARGS__, useVar); \
  } \
  break;

#define NEW_LABEL(lbl, declareVar, useVar, ...) \
  case lbl: \
  { \
    declareVar; \
    GET_N_FMT(useVar, arguments->all, __VA_ARGS__, useVar); \
  } \
  break;

#define NEW_NET_LABEL(lbl, declareVar, useVar, num, ...) \
  case lbl: \
  { \
    declareVar; \
    GET_NET_N_FMT(useVar, arg, num, arguments->all, __VA_ARGS__, useVar); \
  } \
  break;

#define NEW_ARG_LABEL(lbl, declareVar, useVar, ...) \
  case lbl: \
  { \
    declareVar; \
    get_##useVar(useVar, arg); \
    GLUE(arguments->all, __VA_ARGS__, useVar); \
  } \
  break;

#define LABEL_WITH_NUM_GENERATOR(func, lbl, declareVar, useVar, num, ...) \
  case lbl: \
  { \
    declareVar; \
    get_##func(useVar, num); \
    GLUE(arguments->all, __VA_ARGS__, useVar); \
  } \
  break;

#define NEW_RAM_LABEL(...) \
  LABEL_WITH_NUM_GENERATOR(ram, __VA_ARGS__);

#define NEW_SSD_LABEL(...) \
  LABEL_WITH_NUM_GENERATOR(ssd, __VA_ARGS__);

#define NEW_KERNEL_LABEL(...) \
  LABEL_WITH_NUM_GENERATOR(kernel, __VA_ARGS__);

#define NEW_SWAPP_LABEL(...) \
  LABEL_WITH_NUM_GENERATOR(swapp, __VA_ARGS__);

#define NEW_MPD_LABEL(...) \
  LABEL_WITH_NUM_GENERATOR(song, __VA_ARGS__);

/* Fire the preprocessor */


#endif /* GENERATOR_H_ */
