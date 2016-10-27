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

dnl All funcs below are internal

dnl Error message when smth is wrong
AC_DEFUN([ERR],[
  AC_MSG_ERROR($1)
])

dnl What have to be installed in order to compile the program.
AC_DEFUN([ERR_MUST_INSTALL],[
  ERR([Install $1 in order to compile the program.])
])

dnl Failed to compile and linking some test(s)
AC_DEFUN([LINK_FAILED], [
  ERR([Failed to compile and link the $1 test.])
])

dnl Failed to compile some test(s)
AC_DEFUN([COMPILE_FAILED], [
  ERR([Failed to compile the $1 test.])
])

dnl Missing library function(s)
AC_DEFUN([MISSING_FUNC], [
  ERR([Missing core library functions.])
])

dnl Missing header file(s)
AC_DEFUN([MISSING_HEADER], [
  ERR([Missing core header files.])
])



dnl Not error related funcs,
dnl neither fall in any of the test
dnl categories listed in this folder

dnl When using AC_CHECK_LIB in a loop,
dnl it will substitute the same linker flag
dnl several times, which may cause to
dnl exceed the line limit
AC_DEFUN([UPDATE_LIBS_VAR],[
  LIBS="$1"
  AC_SUBST(LIBS)
])

dnl Same description as the above func,
dnl except it saves the LIBS var before
dnl invoking AC_CHECK_LIB
AC_DEFUN([SAVE_LIBS_VAR],[
  m4_ifdef([ZaVeD], [
    m4_undefine([ZaVeD])
  ])

  m4_define([ZaVeD],[$1])
])
