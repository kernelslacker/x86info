dnl 10/24/2016

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

dnl TEST_PERL() function in configure.ac
dnl
dnl Substitute perl related linker and
dnl cflags to the variables PERL_CF and
dnl PERL_LZ if the user enabled the
dnl --with-perl switch
AC_DEFUN([TEST_PERL],[
  PERL_LZ=""
  PERL_CF=""
  WITH_PERL=0
  UZER_ZCRIPT=""
  PYTHC=""

  AC_ARG_WITH([perl],
    AS_HELP_STRING([--with-perl],
      [Extend the program via perl scripts]),
    [],
    [with_perl=no]
  )

  AS_IF([test "x$with_perl" = "xyes"], [
    CHECK_CFLAGZ([-O0])

    AC_PATH_PROG(PERL,perl)
    if [[ -z "${PERL}" ]]
    then
      ERR([Couldnt find perl.])
    fi


    PERL_LZ=`$PERL -MExtUtils::Embed -e ldopts`
    PERL_CF=`$PERL -MExtUtils::Embed -e ccopts`

    AC_ARG_VAR(perl_script, [user perl script])
    if [[ ! -z "${perl_script}" ]]
    then
      UZER_ZCRIPT=\""${perl_script}"\"
    fi

    dnl m4_foreach([LiB], [
    dnl     perl_construct,
    dnl     perl_parse,
    dnl     perl_run,
    dnl     perl_destruct,
    dnl     perl_alloc,
    dnl     perl_free
    dnl   ],[
    dnl     AC_CHECK_LIB(perl,LiB,[],[
    dnl       MISSING_FUNC()
    dnl     ])
    dnl ])

    WITH_PERL=1
  ])

  AC_SUBST(PERL_LZ)
  AC_SUBST(PERL_CF)
  AC_DEFINE_UNQUOTED([WITH_PERL],[$WITH_PERL],[Extend the program via perl scripts])
  AC_DEFINE_UNQUOTED([UZER_ZCRIPT],[$UZER_ZCRIPT],[Extend the program via perl scripts])


  dnl AS_IF([test "x$with_perl" = "xyes"], [
  dnl   AC_LINK_IFELSE([
  dnl     AC_LANG_SOURCE([[
  dnl       #include <stdio.h>
  dnl       #include <string.h>
  dnl       #include <EXTERN.h>
  dnl       #include <perl.h>
  dnl       int main(void) {
  dnl         static PerlInterpreter *my_perl = NULL;
  dnl         PERL_SYS_INIT3((int *)NULL, (char ***)NULL, (char ***)NULL);
  dnl         my_perl = perl_alloc();
  dnl         if (NULL == my_perl) {
  dnl           PERL_SYS_TERM();
  dnl           return -1;
  dnl         }
  dnl         perl_construct(my_perl);
  dnl         perl_parse(my_perl, NULL, 0, (char **)NULL, (char **)NULL);
  dnl         PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
  dnl         perl_run(my_perl);
  dnl         perl_destruct(my_perl);
  dnl         perl_free(my_perl);
  dnl         PERL_SYS_TERM();
  dnl         return 0;
  dnl       }
  dnl     ]])
  dnl   ],[],[
  dnl       LINK_FAILED([perl])
  dnl     ]
  dnl   )
  dnl ])

])


dnl TEST_python() function in configure.ac
dnl
dnl Substitute python related linker and
dnl cflags to the variables PYTHON_CF and
dnl PYTHON_LZ if the user enabled the
dnl --with-python switch
AC_DEFUN([TEST_PYTHON],[
  PYTHON_LZ=""
  PYTHON_CF=""
  WITH_PYTHON2=0
  WITH_PYTHON=0
  UZER_ZCRIPT2="none"
  UZER_PAHT="none"
  curpycfver="none"

  AC_ARG_WITH([python2],
    AS_HELP_STRING([--with-python2],
      [Extend the program via python scripts]),
    [],
    [with_python2=no]
  )

  AC_ARG_WITH([python3],
    AS_HELP_STRING([--with-python3],
      [Extend the program via python scripts]),
    [],
    [with_python3=no]
  )

  AC_ARG_VAR(python_script, [user python script])
  if [[ ! -z "${python_script}" ]]
  then
    just_script="${python_script##*/}"
    UZER_ZCRIPT2=\""${just_script%.*}"\"
    UZER_PAHT=\""${python_script%/*}"\"
  fi

  AS_IF([test "x$with_python2" = "xyes" || test "x$with_python3" = "xyes"], [
    CHECK_CFLAGZ([-O0])

    AS_IF([test "x$with_python2" = "xyes"], [
      AM_PATH_PYTHON([2],[
      ],[
        ERR([Couldnt determine CFLAGS and LDFLAGS for the requested python version.])
      ])
      WITH_PYTHON2=1
    ])

    AS_IF([test "x$with_python3" = "xyes"], [
      AM_PATH_PYTHON([3],[
      ],[
        ERR([Couldnt determine CFLAGS and LDFLAGS for the requested python version.])
      ])
    ])

    dnl What a python versioning mess.
    dnl We have to check back and forth different scenarious
    dnl to make sure we can find appropriate CFLAGS and LDFLAGS
    dnl for the requested python version.
    dnl When there is only 1 python version installed the file
    dnl naming is one, when there are 2 or more different
    dnl python version installed, the file naming is other.
    dnl Still reading or get bored ?
    m4_define([testveR],[python$PYTHON_VERSION])
    m4_define([testveR2],[python-config-$PYTHON_VERSION])

    dnl First check whether python and python-9.9 exist
    AC_PATH_PROG(pyvf1,testveR,no)
    AC_PATH_PROG(pyvf2,python,no)

    dnl Next check whether python-config and python-config-9.9 exist
    AC_PATH_PROG(pycf1,testveR2,no)
    AC_PATH_PROG(pycf2,python-config,no)

    dnl Check whether any of the python versions was found
    AS_IF([test "x$pyvf1" = "xno" && test "x$pyvf2" = "xno"], [
      ERR([Couldnt find python])
    ])

    dnl Check whether any of the python-config versions was found
    AS_IF([test "x$pycf1" = "xno" && test "x$pycf2" = "xno"], [
      ERR([Couldnt find python-config])
    ])

    dnl We firts check for the python-config-9.9 version
    AS_IF([test "x$pycf1" != "xno"], [
      curpycfver="${pycf1}"
    ])

    dnl We now know that python-config is the only version available
    AS_IF([test "x$pycf2" != "xno" && test "x$curpycfver" = "xnone"], [
      curpycfver="${pycf2}"
    ])

    WITH_PYTHON=1
    PYTHON_LZ=`${curpycfver} --ldflags`
    PYTHON_CF=`${curpycfver} --cflags`

    dnl m4_foreach([LiB], [
    dnl     Py_GetPath,
    dnl     Py_Initialize,
    dnl     PyImport_Import,
    dnl     PyObject_GetAttrString,
    dnl     PyCallable_Check,
    dnl     PyObject_CallObject,
    dnl     Py_Finalize
    dnl   ],[
    dnl     AC_CHECK_LIB(testveR,LiB,[],[
    dnl       MISSING_FUNC()
    dnl     ])
    dnl ])

  ])


  AC_SUBST(PYTHON_LZ)
  AC_SUBST(PYTHON_CF)
  AC_DEFINE_UNQUOTED([WITH_PYTHON],[$WITH_PYTHON],[Extend the program via python scripts])
  AC_DEFINE_UNQUOTED([WITH_PYTHON2],[$WITH_PYTHON2],[Extend the program via python scripts])
  AC_DEFINE_UNQUOTED([UZER_ZCRIPT2],[$UZER_ZCRIPT2],[Extend the program via python scripts])
  AC_DEFINE_UNQUOTED([UZER_PAHT],[$UZER_PAHT],[Extend the program via python scripts])

  dnl AS_IF([test "x$with_python2" = "xyes" || test "x$with_python3" = "xyes"], [
  dnl   AC_CHECK_HEADERS([testveR/Python.h],[],[
  dnl     MISSING_HEADER()
  dnl   ])

  dnl   AC_LINK_IFELSE([
  dnl     AC_LANG_SOURCE([[
  dnl       #include <stdio.h>
  dnl       #include <string.h>
  dnl       #include <Python.h>
  dnl       int main(void) {
  dnl         Py_Initialize();
  dnl         PyRun_SimpleString("from time import time,ctime\n"
  dnl                            "print(ctime(time()))\n");
  dnl         Py_Finalize();
  dnl         return 0;
  dnl       }
  dnl     ]])
  dnl   ],[],[
  dnl       LINK_FAILED([python])
  dnl     ]
  dnl   )
  dnl ])

])
