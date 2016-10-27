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


dnl Internal function to let the user know
dnl when some of the compile tests is about to begin
AC_DEFUN([NOTIFY],[
  AC_MSG_NOTICE([performing $1 compile test.])
])


dnl TEST_SOME_FUNCS() function in configure.ac
dnl
dnl The tests are simple enough, just to
dnl catch misbehaving compiler and/or
dnl installed C libraries.
AC_DEFUN([TEST_SOME_FUNCS],[

  NOTIFY([strftime])
  AC_COMPILE_IFELSE([
    AC_LANG_SOURCE([[
      #include <time.h>
      int main(void) {
        char test[200];
        time_t t = time(NULL);
        strftime(test, 190, "%I:%M %p", localtime(&t));
        return 0;
      }
    ]])
  ],[],[
    COMPILE_FAILED([strftime])
    ]
  )


  NOTIFY([statvfs])
  AC_COMPILE_IFELSE([
    AC_LANG_SOURCE([[
      #include <stdlib.h>
      #include <sys/statvfs.h>
      int main(void) {
        struct statvfs ssd;
        statvfs(getenv("HOME"), &ssd);
        ssd.f_blocks;
        ssd.f_bsize;
        ssd.f_bavail;
        ssd.f_bfree;
        return 0;
      }
    ]])
  ],[],[
    COMPILE_FAILED([statvfs storage])
    ]
  )


  NOTIFY([uname])
  AC_COMPILE_IFELSE([
    AC_LANG_SOURCE([[
      #include <sys/utsname.h>
      int main(void) {
        struct utsname KerneL;
        uname(&KerneL);
        KerneL.sysname;
        KerneL.nodename;
        KerneL.release;
        KerneL.version;
        KerneL.machine;
        return 0;
      }
    ]])
  ],[],[
    COMPILE_FAILED([uname Kernel])
    ]
  )


  ifdef([LINUKS],[
    NOTIFY([sysinfo])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[
        #include <sys/sysinfo.h>
        int main(void) {
          struct sysinfo inf;
          sysinfo(&inf);
          inf.totalram;
          inf.freeram;
          inf.sharedram;
          inf.bufferram;
          inf.loads[0];
          inf.loads[1];
          inf.loads[2];
          return 0;
        }
      ]])
    ],[],[
      COMPILE_FAILED([sysinfo RAM and average load])
      ]
    )

  ],[])


  NOTIFY([openNreadFile])
  AC_COMPILE_IFELSE([
    AC_LANG_SOURCE([[
      #include <stdio.h>
      int main(void) {
        FILE *test = fopen("/proc/stat", "r");
        if (NULL == test) {
          return 0;
        }
        fclose(test);
        return 0;
      }
    ]])
  ],[],[
    COMPILE_FAILED([openNreadFile])
    ]
  )


  NOTIFY([memset])
  AC_COMPILE_IFELSE([
    AC_LANG_SOURCE([[
      #include <string.h>
      #include <stdint.h>
      int main(void) {
        uintmax_t test[5];
        memset(test, 0, sizeof(test));
        return 0;
      }
    ]])
  ],[],[
    COMPILE_FAILED([memset])
    ]
  )


  ifdef([LINUKS],[
    NOTIFY([glob])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[
        #include <glob.h>
        int main(void) {
          glob_t gl;
          return 0;
        }
      ]])
    ],[],[
      COMPILE_FAILED([glob])
      ]
    )

  ],[])


  NOTIFY([sysconf])
  AC_COMPILE_IFELSE([
    AC_LANG_SOURCE([[
      #include <time.h>
      #include <unistd.h>
      int main(void) {
        if (-1 == (sysconf(_SC_CLK_TCK))) {
          return 0;
        }
        return 0;
      }
    ]])
  ],[],[
    COMPILE_FAILED([sysconf])
    ]
  )


  NOTIFY([snprintf])
  AC_COMPILE_IFELSE([
    AC_LANG_SOURCE([[
      #include <stdio.h>
      int main(void) {
        char test[10];
        snprintf(test, 8, "%s", "hi");
        return 0;
      }
    ]])
  ],[],[
    COMPILE_FAILED([snprintf])
    ]
  )


  NOTIFY([getopt])
  AC_COMPILE_IFELSE([
    AC_LANG_SOURCE([[
      #include <getopt.h>
      #define NULL ((void *)0)
      int main(void) {
        const struct option options[] = {
          { "mpd",          no_argument,       NULL, 'M' },
          { "statio",       required_argument, NULL, 'S' },
          { NULL,           0,                 NULL,  0  }
        };

        return 0;
      }
    ]])
  ],[],[
    COMPILE_FAILED([getopt])
    ]
  )


  NOTIFY([timespec])
  AC_COMPILE_IFELSE([
    AC_LANG_SOURCE([[
      #include <time.h>
      int main(void) {
        struct timespec tc = {0};
        tc.tv_nsec = 85000L;
        return 0;
      }
    ]])
  ],[],[
    COMPILE_FAILED([timespec])
    ]
  )


  NOTIFY([popen])
  AC_COMPILE_IFELSE([
    AC_LANG_SOURCE([[
      #include <stdio.h>
      int main(void) {
        char hi[30];
        FILE *test = popen("echo elo", "r");
        fscanf(test, "%s", hi);
        pclose(test);
        return 0;
      }
    ]])
  ],[],[
    COMPILE_FAILED([popen])
    ]
  )


  NOTIFY([clock_gettime])
  AC_COMPILE_IFELSE([
    AC_LANG_SOURCE([[
      #include <time.h>
      int main(void) {
        struct timespec tc1 = {0}, tc2 = {0};
        clock_gettime(CLOCK_MONOTONIC, &tc1);
        
      #if defined(__linux__)
        clock_gettime(CLOCK_BOOTTIME, &tc2);
      #endif

        return 0;
      }
    ]])
  ],[],[
    COMPILE_FAILED([clock_gettime])
    ]
  )
  
  
  ifdef([FREEBZD],[
    NOTIFY([getloadavg])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[
        #include <stdlib.h>
        int main(void) {
          double up[3];
          getloadavg(up, 3);
          return 0;
        }
      ]])
    ],[],[
      COMPILE_FAILED([getloadavg])
      ]
    )


    NOTIFY([sysctl])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[
        #include <stdio.h>
        #include <stdlib.h>
        #include <sys/types.h>
        #include <sys/sysctl.h>
        #define SYSCTLVAL(x, y, z) \
        if (0 != sysctlbyname(x, y, z, NULL, 0)) { \
          return 0; \
        }
        int main(void) {
          u_int dummy = 0;
          size_t len = sizeof(dummy);
          SYSCTLVAL("vm.stats.vm.v_page_size", &dummy, &len);
          return 0;
        }
      ]])
    ],[],[
      COMPILE_FAILED([sysctl])
      ]
    )


    NOTIFY([malloc])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[
        #include <stdio.h>
        #include <stdlib.h>
        int main(void) {
          char *a = (char *)malloc(10);
          if (NULL == a) {
            return 0;
          }
          free(a);
          return 0;
        }
      ]])
    ],[],[
      COMPILE_FAILED([malloc])
      ]
    )

    m4_foreach([LiB], [
        devstat_checkversion        ,
        devstat_getdevs             ,
        devstat_selectdevs
      ],[
        AC_CHECK_LIB(devstat,LiB,[],[
          MISSING_FUNC()
        ])
    ])

    NOTIFY([devstat])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[
        #include <stdio.h>
        #include <stdlib.h>
        #include <devstat.h>
        int main(void) {
          struct statinfo stats;
          struct device_selection *dev_select = NULL;
          struct devstat *d = NULL;
          devstat_checkversion(NULL);
          return 0;
        }
      ]])
    ],[],[
      COMPILE_FAILED([devstat])
      ]
    )

    m4_foreach([LiB], [
        argp_parse                  ,
        argp_usage
      ],[
        AC_CHECK_LIB(argp,LiB,[],[
          MISSING_FUNC()
        ])
    ])


    NOTIFY([xswdev-swap])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[
        #include <stdio.h>
        #include <string.h>
        #include <stdlib.h>
        #include <inttypes.h>
        #include <sys/types.h>
        #include <sys/param.h>
        #include <sys/stat.h>
        #include <sys/sysctl.h>
        #include <vm/vm_param.h>
        int main(void) {
          struct xswdev xsw;
          u_int pagesize = 4096, dummy = 0;
          uintmax_t total = 0, used = 0, pz = 0;
          int mib[20];
          memset(mib, 0, sizeof(mib));
          size_t mibi = sizeof(mib) / sizeof(mib[0]);
          size_t len = sizeof(dummy), sisi = sizeof(struct xswdev);

          pz = (uintmax_t)pagesize;
          if (0 != (sysctlnametomib("vm.swap_info", mib, &mibi))) {
            return 0;
          }
          if (0 != (sysctl(mib, (u_int)(mibi + 1), &xsw, &sisi, NULL, 0))) {
            return 0;
          }
          if (xsw.xsw_version != XSWDEV_VERSION) {
            return 0;
          }
          used  = (uintmax_t)xsw.xsw_used;
          total = (uintmax_t)xsw.xsw_nblks;

          return 0;
        }
      ]])
    ],[],[
      COMPILE_FAILED([xswdev-swap])
      ]
    )

  ],[])


  ifdef([OPENBZD],[
    NOTIFY([swapctl])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[
        #include <stdio.h>
        #include <sys/swap.h>
        int main(void) {
          struct swapent *dev = NULL;
          swapctl(SWAP_NSWAP, 0, 0);
          return 0;
        }
      ]])
    ],[],[
      COMPILE_FAILED([swapctl])
      ]
    )

    NOTIFY([apm battery])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[
        #include <stdio.h>
        #include <string.h>
        #include <stdint.h>
        #include <fcntl.h>
        #include <sys/ioctl.h>
        #include <machine/apmvar.h>
        int main(void) {
          struct apm_power_info bstate;
          int fd = 0;
          uintmax_t dummy = 0;
          memset(&bstate, 0, sizeof(struct apm_power_info));
          if (0 != (fd = open("/dev/apm", O_RDONLY))) {
            return 0;
          }
          if (0 != (ioctl(fd, APM_IOC_GETPOWER, &bstate))) {
            close(fd);
            return 0;
          }
          close(fd);
          if (APM_BATT_UNKNOWN == bstate.battery_state ||
              APM_BATTERY_ABSENT == bstate.battery_state) {
            return 0;
          }
          dummy = (uintmax_t)bstate.battery_life;
          return 0;
        }
      ]])
    ],[],[
      COMPILE_FAILED([apm battery])
      ]
    )

  ],[])



])
