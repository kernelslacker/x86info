dnl 08/10/2015

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


dnl TEST_NET() function in configure.ac
dnl
dnl Allow the user to compile the program
dnl without net related functions, thus
dnl decreasing the required dependencies.
dnl
dnl Did not included tests for some of the Net
dnl headers as they fail to compile just by including
dnl them in gcc, but pass the tests in clang. Is there
dnl are any _POSIX_SOURCE m4 alternative to compile a
dnl test case on the fly ?
dnl
dnl --with-net will check for the presence of iproute2 headers
dnl used in the net functions and substitute appropriate macro
dnl so the program to be compiled with/out net functions support.
AC_DEFUN([TEST_NET],[
  WITH_NET=1
  WITH_LIBNL=0
  LIBNL_CF=""
  LIBNL_LZ=""

  AC_ARG_WITH([net],
    AS_HELP_STRING([--with-net],
      [Net funcs]),
    [],
    [with_net=yes]
  )

  AS_IF([test "x$with_net" = "xno"], [
    WITH_NET=0
  ])

  AS_IF([test "x$with_net" = "xyes"], [

    AC_CHECK_HEADERS([    \
      ifaddrs.h           \
      arpa/inet.h         \
      sys/socket.h        \
      sys/ioctl.h         \
      netdb.h             \
    ],[],[
      MISSING_HEADER()
    ])
    
    ifdef([FREEBZD], [
      AC_CHECK_HEADERS([    \
        arpa/nameser.h      \
        netinet/in.h        \
        net/if.h            \
        net/if_dl.h         \
      ],[],[
        MISSING_HEADER()
      ])

    ],[
    ])

    ifdef([LINUKS], [
      AC_CHECK_HEADERS([    \
        linux/if_link.h     \
        netpacket/packet.h  \
        linux/sockios.h     \
        linux/ethtool.h     \
      ],[],[
        MISSING_HEADER()
      ])

      AC_ARG_WITH([libnl],
        AS_HELP_STRING([--with-libnl],
          [wifi funcs]),
        [],
        [with_libnl=no]
      )

      AS_IF([test "x$with_libnl" = "xyes"], [

        m4_ifndef([PKG_PROG_PKG_CONFIG], [
          AC_MSG_ERROR([Either you dont have pkg-config installed, or pkg.m4 is not in 'ls /usr/share/aclocal | grep pkg', if thats so try exporting the following env var: execute 'aclocal --print-ac-dir' without quotes, then: 'export ACLOCAL_PATH=/tmp' where /tmp is the directory printed from the previous command.])
        ])
        PKG_PROG_PKG_CONFIG()

        PKG_CHECK_MODULES([LIBNL], [libnl-3.0 >= 3.0 libnl-genl-3.0 >= 3.0], [
          AC_CHECK_LIB(nl-3,nlmsg_data,[],[
            MISSING_FUNC()
          ])
          AC_CHECK_LIB(nl-genl-3,genlmsg_put,[],[
            MISSING_FUNC()
          ])

        ],[
          AC_MSG_ERROR([Your libnl version is too old, consider updating to version three at least])

        ])
        LIBNL_CF=$LIBNL_CFLAGS
        LIBNL_LZ=$LIBNL_LIBS
        WITH_LIBNL=1

      ])

    ],[
    ])

    AC_CHECK_FUNCS([ \
      getifaddrs     \
      freeifaddrs    \
      getaddrinfo    \
      freeaddrinfo   \
      socket         \
      ioctl          \
      inet_ntop      \
    ],[],[
      MISSING_FUNC()
    ])
    
    NOTIFY([addrinfo])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[
        #include <stdio.h>
        #include <string.h>
        #include <netdb.h>
        #include <sys/socket.h>
        int main(void) {
          struct addrinfo *rp = NULL, *result = NULL;
          struct addrinfo hints;
          memset(&hints, 0, sizeof(hints));
          hints.ai_family = AF_INET;
          hints.ai_socktype = SOCK_DGRAM;
          hints.ai_flags = 0;
          hints.ai_protocol = 0;
          return 0;
        }
      ]])
    ],[],[
      COMPILE_FAILED([addrinfo])
      ]
    )

    NOTIFY([getifaddrs-lite])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[
        #include <ifaddrs.h>
        int main(void) {
          struct ifaddrs *ifaddr;
          if (-1 == getifaddrs(&ifaddr)) {
            return 0;
          }
          freeifaddrs(ifaddr);
          return 0;
        }
      ]])
    ],[],[
      COMPILE_FAILED([getifaddrs-lite])
      ]
    )

    ifdef([FREEBZD],[
      NOTIFY([getifaddrs-heavy])
      AC_COMPILE_IFELSE([
        AC_LANG_SOURCE([[
          #include <arpa/inet.h>
          #include <arpa/nameser.h>
          #include <ifaddrs.h>
          #include <net/if.h>
          #include <net/if_dl.h>
          int main(void) {
            struct ifaddrs *ifaddr = NULL, *ifa = NULL;
            struct if_data *stats = NULL;
            struct sockaddr_dl *mac = NULL;
            if (-1 == getifaddrs(&ifaddr)) {
              return 0;
            }
            freeifaddrs(ifaddr);
            return 0;
          }
        ]])
      ],[],[
        COMPILE_FAILED([getifaddrs-heavy])
        ]
      )
    ],[])


  ])

  AC_SUBST(LIBNL_CF)
  AC_SUBST(LIBNL_LZ)
  AC_DEFINE_UNQUOTED([WITH_NET],[$WITH_NET],[Net funcs])
  AC_DEFINE_UNQUOTED([WITH_LIBNL],[$WITH_LIBNL],[wifi funcs])

])

dnl TEST_PCI() function in configure.ac
dnl
dnl Allow the user to compile the program
dnl without pci related functions, thus
dnl decreasing the required dependencies.
dnl
dnl --with-pci will check for the presence of pci headers and
dnl substitute the linker flags -lpci to the variable PCI_LIBS
dnl as well will mitigate the GCC -O2 bug.
AC_DEFUN([TEST_PCI],[
  WITH_PCI=1
  PCI_LIBS=""

  AC_ARG_WITH([pci],
    AS_HELP_STRING([--with-pci],
      [PCI funcs]),
    [],
    [with_pci=yes]
  )

  ifdef([LINUKS],[],[
    WITH_PCI=0
    CHECK_CFLAGZ([-O2])
  ])

  ifdef([LINUKS],[
    AS_IF([test "x$with_pci" = "xno"], [
      WITH_PCI=0
      ifdef([GOT_SENSR],[
        CHECK_CFLAGZ([-O0])
      ],[
        CHECK_CFLAGZ([-O2])
      ])
      m4_define([NO_PCI],[thenFryAGP])
    ])

    AS_IF([test "x$with_pci" = "xyes"], [

      dnl For very first time I stumble upon GCC -O2 bug.
      dnl It hangs on pci_init with -O2
      CHECK_CFLAGZ([-O0])

      AC_CHECK_HEADERS([pci/pci.h], [
          PCI_LIBS="-lpci"
        ],[
          ERR_MUST_INSTALL([pciutils])
        ])

      m4_foreach([LiB], [
          pci_alloc        ,
          pci_init         ,
          pci_scan_bus     ,
          pci_fill_info    ,
          pci_lookup_name  ,
          pci_cleanup
        ],[
          AC_CHECK_LIB(pci,LiB,[],[
            MISSING_FUNC()
          ])
      ])
    ])

    AC_SUBST(PCI_LIBS)

    AS_IF([test "x$with_pci" = "xyes"], [
      AC_LINK_IFELSE([
        AC_LANG_SOURCE([[
          #include <stdio.h>
          #include <pci/pci.h>
          int main(void) {
            struct pci_access *pacc= NULL;
            pacc = pci_alloc();
            pci_init(pacc);
            pci_scan_bus(pacc);
            if (NULL != pacc) {
              pci_cleanup(pacc);
            }
            return 0;
          }
        ]])
      ],[],[
          LINK_FAILED([pci])
        ]
      )
    ])

  ],[
  ])

  AC_DEFINE_UNQUOTED([WITH_PCI],[$WITH_PCI],[PCI funcs])

])
