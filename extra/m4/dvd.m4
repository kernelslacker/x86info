dnl 08/23/2016

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


dnl TEST_DVD() function in configure.ac
dnl
dnl Substitute the linker flags -lcdio to the
dnl the variable 'DVD_LIBS' if the user enabled
dnl the --with-dvd switch
AC_DEFUN([TEST_DVD],[
  DVD_LIBS=""

  AC_ARG_WITH([dvd],
    AS_HELP_STRING([--with-dvd],
      [dvd linker flag to show the vendor and model name of your cd/dvdrom]),
    [],
    [with_dvd=no]
  )

  AS_IF([test "x$with_dvd" = "xyes"], [
    AC_CHECK_HEADERS([ \
      cdio/cdio.h      \
      cdio/mmc.h       \
    ], [
      DVD_LIBS="-lcdio"
      ],[
        ERR_MUST_INSTALL([libcdio])
    ])

    m4_foreach([LiB], [
        cdio_open        ,
        mmc_get_hwinfo   ,
        cdio_destroy
      ],[
        AC_CHECK_LIB(cdio,LiB,[],[
          MISSING_FUNC()
        ])
    ])
  ])

  AC_SUBST(DVD_LIBS)

  AS_IF([test "x$with_dvd" = "xyes"], [
    AC_LINK_IFELSE([
      AC_LANG_SOURCE([[
        #include <stdio.h>
        #include <stdlib.h>
        #include <cdio/cdio.h>
        #include <cdio/mmc.h>
        int main(void) {
          CdIo_t *p_cdio = cdio_open(NULL, DRIVER_DEVICE);
          cdio_hwinfo_t hwinfo;
          if (NULL == p_cdio) {
            return 0;
          }
          if (mmc_get_hwinfo(p_cdio, &hwinfo)) {
            printf("%s %s\n", hwinfo.psz_vendor, hwinfo.psz_model);
          }
          if (NULL != p_cdio) {
            cdio_destroy(p_cdio);
          }
          return 0;
        }
      ]])
    ],[],[
        LINK_FAILED([cdio])
      ]
    )
  ])

])
