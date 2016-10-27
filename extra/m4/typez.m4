dnl 08/03/2016

dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.

dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
dnl MA 02110-1301, USA.


dnl Internal function to perform
dnl explicit data check type
AC_DEFUN([CHECK_TYPEZ],[
  AC_CHECK_TYPES([$1],[],[
    AC_MSG_WARN([Some C data type failed, checking which one.])
    m4_foreach([tYpe], [$1],[
      AC_CHECK_SIZEOF(tYpe)
    ])dnl
    ERR([Your compiler does not understand C data types.])
  ])
])


dnl TEST_TYPEZ() function in configure.ac
dnl
dnl Check for the presence of all used
dnl library functions, their header files
dnl and some int types. 64bit is not
dnl mandatory since uintmax makes it 
dnl easy for us.
AC_DEFUN([TEST_TYPEZ],[

  CHECK_TYPEZ([
    size_t,
    time_t,
    float,
    double,
    signed char,
    unsigned char,
    signed short int,
    int8_t,
    signed int,
    int16_t,
    signed long int,
    int32_t,
    intmax_t,
    unsigned short int,
    uint8_t,
    unsigned int,
    uint16_t,
    unsigned long int,
    uint32_t,
    uintmax_t,
    int_least8_t,
    int_least16_t,
    int_least32_t,
    uint_least8_t,
    uint_least16_t,
    uint_least32_t,
    int_fast8_t,
    int_fast16_t,
    int_fast32_t,
    uint_fast8_t,
    uint_fast16_t,
    uint_fast32_t
  ])

  AC_CHECK_HEADERS([ \
    time.h           \
    stdio.h          \
    stdbool.h        \
    string.h         \
    inttypes.h       \
    sys/statvfs.h    \
    sys/utsname.h    \
    unistd.h         \
    getopt.h         \
    sys/socket.h     \
  ],[],[
    MISSING_HEADER()
  ])
  
  ifdef([FREEBZD], [
    AC_CHECK_HEADERS([ \
      sys/types.h      \
      sys/sysctl.h     \
      sys/param.h      \
      sys/stat.h       \
      vm/vm_param.h    \
    ],[],[
      MISSING_HEADER()
    ])

  ], [
    ifdef([LINUKS],[
      AC_CHECK_HEADERS([ \
        argp.h           \
        sys/sysinfo.h    \
        glob.h           \
      ],[],[
        MISSING_HEADER()
      ])
    ],[
    ])

  ])

  AC_CHECK_FUNCS([  \
    memset          \
    printf          \
    snprintf        \
    nanosleep       \
    sysconf         \
    strcmp          \
    fscanf          \
    fclose          \
    fopen           \
    statvfs         \
    getenv          \
    popen           \
    uname           \
    strftime        \
    exit            \
    getopt          \
    getopt_long     \

  ],[],[
    MISSING_FUNC()
  ])

  ifdef([FREEBZD], [
    AC_CHECK_FUNCS([  \
      sysctl          \
      sysctlbyname    \
      sysctlnametomib \
      getloadavg      \
      malloc          \
      free            \
    ],[],[
      MISSING_FUNC()
    ])

  ], [

    ifdef([LINUKS],[
      AC_CHECK_FUNCS([  \
        sysinfo         \
        glob            \
        globfree        \
        argp_parse      \
        argp_usage      \
      ],[],[
        MISSING_FUNC()
      ])
    ],[
    ])

  ])

])
