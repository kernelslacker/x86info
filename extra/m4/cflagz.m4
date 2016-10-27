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
dnl explicit compiler CFLAGS support test
AC_DEFUN([CHECK_CFLAGZ],[
  m4_foreach([fLaG], [$1], [

    AX_APPEND_FLAG([fLaG], [CFLAGS])

    AC_MSG_CHECKING([whether fLaG will compile flawlessly])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[ int main(void) { return 0; } ]])
    ],[cflagz_ok=yes],[cflagz_ok=no])

    AC_MSG_RESULT([$cflagz_ok])

    AS_IF([test "x$cflagz_ok" = "xno"], [
        ERR([Failed to compile a simple test with the above fLaG CFLAG.])
      ]
    )

    AC_LINK_IFELSE([
      AC_LANG_SOURCE([[ int main(void) { return 0; } ]])
    ],[],[
        ERR([Failed to compile and link a simple test with the above fLaG CFLAG.])
      ]
    )

  ])dnl

])

dnl Internal function to check
dnl the compiler for assembly support
AC_DEFUN([TEST_ASSEMBLY],[

  AC_MSG_CHECKING([for assembly support])

  AC_COMPILE_IFELSE([
    AC_LANG_PROGRAM([[
      #include <stdio.h>]],[[
      unsigned int x = 1, z = 2;
      __asm__ __volatile__ (
        "addl %%ebx, %%eax"
        : "=a"(x)
        : "a"(x), "b"(z)
      );
    ]])
  ],[supportz_assembly=yes],[supportz_assembly=no])

  AC_MSG_RESULT([$supportz_assembly])

  AS_IF([test "x$supportz_assembly" = "xno"], [
      ERR([Your compiler does not support assembly.])
    ]
  )

])

dnl TEST_CFLAGZ() function in configure.ac
dnl
dnl Check for the presence and whether
dnl the given FLAG will work flawlessly
dnl with the currently used compiler.
dnl Will substitute each successful flag
dnl and bail out with pre-defined error msg
dnl when some FLAG is unsupported.
AC_DEFUN([TEST_CFLAGZ],[

  dnl Only useful when developing
  dnl pinky-bar
  dnl -Wdeclaration-after-statement,
  dnl -Wno-unused-function,

  dnl Shame on you c++lang, err clang
  dnl Keeping this flag for historical
  dnl reasons just to remind myself and
  dnl anyone reading this file about the
  dnl clangs inabillity to distinguish
  dnl C from C++
  dnl  https://llvm.org/bugs/show_bug.cgi?id=21689
  dnl -Wno-missing-field-initializers

  dnl The twisted gcc vision
  dnl  https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66425
  dnl They say its "glibc" fault
  dnl  https://gcc.gnu.org/bugzilla/show_bug.cgi?id=25509
  dnl -Wno-unused-result,

  dnl For very first time I stumble upon GCC -O2 bug.
  dnl It hangs on pci_init and sensors_init with -O2
  dnl net.m4 will append -O2 or -O0
  dnl according whether --with-pci and --with-sensors
  dnl are used or not. Another "newer" GCC release
  dnl that I just installed hangs on sensors_init.
  dnl One more snafu GCC, and I'll drop -02 in linux

  AC_MSG_NOTICE([checking all CFLAGS])
  CHECK_CFLAGZ([
    -pipe,
    -std=c99,
    -Wextra,
    -Wall,
    -pedantic,
    -Wundef,
    -Wshadow,
    -W,
    -Wwrite-strings,
    -Wcast-align,
    -Wstrict-overflow=5,
    -Wconversion,
    -Wpointer-arith,
    -Wstrict-prototypes,
    -Wformat=0,
    -Wsign-compare,
    -Wendif-labels,
    -Wredundant-decls,
    -Wmissing-prototypes,
    -Winit-self,
    -Wno-unused-variable
  ])
 
  TEST_ASSEMBLY()

  LIBS=''
  AC_SUBST(LIBS)
])
