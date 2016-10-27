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


dnl TEST_MPD() function in configure.ac
dnl
dnl Substitute the linker flags -lmpdclient to the
dnl the variable 'MPD_LIBS' if the user enabled
dnl the --with-mpd switch
AC_DEFUN([TEST_MPD],[
  MPD_LIBS=""

  AC_ARG_WITH([mpd],
    AS_HELP_STRING([--with-mpd],
      [mpd linker flag to show the currently played song]),
    [],
    [with_mpd=no]
  )

  AS_IF([test "x$with_mpd" = "xyes"], [
    AC_CHECK_HEADERS([mpd/client.h], [
      MPD_LIBS="-lmpdclient"
      ],[
        ERR_MUST_INSTALL([libmpdclient])
      ])

    m4_foreach([LiB], [
        mpd_connection_new          ,
        mpd_send_command            ,
        mpd_recv_song               ,
        mpd_connection_get_error    ,
        mpd_connection_free         ,
        mpd_song_free               ,
        mpd_song_get_uri
      ],[
        AC_CHECK_LIB(mpdclient,LiB,[],[
          MISSING_FUNC()
        ])
    ])
  ])

  AC_SUBST(MPD_LIBS)

  AS_IF([test "x$with_mpd" = "xyes"], [
    AC_LINK_IFELSE([
      AC_LANG_SOURCE([[
        #include <mpd/client.h>
        int main(void) {
          struct mpd_connection *conn = NULL;
          struct mpd_song *song;
          if (NULL == (conn = mpd_connection_new(NULL, 0, 0))) {
            return 0;
          }
          mpd_connection_free(conn);
          return 0;
        }
      ]])
    ],[],[
        LINK_FAILED([mpd])
      ]
    )
  ])

])
