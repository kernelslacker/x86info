/*
   08/06/2016

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

/* Functions shared between different operating
 * zyztemz or are short enough that doesn't meet the
 * 100 lines requirement to be put in standalone module */

#include "config.h" /* Auto-generated */

#include <ctype.h>

#include <sys/statvfs.h>
#include <sys/utsname.h>

#if defined(__linux__)
#include <glob.h>
#endif /* __linux__ */

#if defined (HAVE_X11_XLIB_H)
#include <X11/Xlib.h>
#endif

#if defined(HAVE_CDIO_CDIO_H)
#include <cdio/cdio.h>
#include <cdio/mmc.h>
#endif /* HAVE_CDIO_CDIO_H */

#include "include/headers.h"

#if defined(__FreeBSD__)
#include "include/freebzd.h"
#endif /* __FreeBSD__ */

#if defined(__OpenBSD__)
#include "include/openbzd.h"
#endif /* __OpenBSD__ */


#if defined(__linux__)
static uint_fast16_t glob_packages(const char *);
#endif /* __linux__ */

void
exit_with_err(const char *str1, const char *str2) {
  FPRINTF("%s %s\n", str1, str2);
  exit(EXIT_FAILURE);
}


#if defined(__linux__)
void
get_temp(const char *str1, char *str2) {
  uint_least32_t temp = 0;
  FILE *fp = NULL;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  OPEN_X(fp, str1, SCAN_ULINT, &temp);
#pragma GCC diagnostic pop

  if (99999 < temp) { /* > 99C */
    FILL_ARR(str2, ULINT, temp / 10000);
  } else {
    FILL_ARR(str2, ULINT, ((9999 < temp) ?
      temp / 1000 : temp / 100)); /* > 9C || < 9C */
  }
}

#endif /* __linux__ */

#if defined(__FreeBSD__)
void
get_temp(char *str1, uint_least32_t temp) {
  if (9999 < temp) { /* > 99C */
    FILL_ARR(str1, ULINT, temp / 1000);
  } else {
    FILL_ARR(str1, ULINT, ((999 < temp) ?
      temp / 100 : temp / 10)); /* > 9C || < 9C */
  }
}
#endif /* __FreeBSD__ */


void
check_fan_vals(char *str1, uint_fast16_t *rpm, uint_fast16_t iterz) {
  uint_fast16_t x = 0, y = 0;
  char buffer[VLA];
  char *all_fans = buffer;

  for (x = 0; x < iterz; x++) {
    if (0 != rpm[x]) {
      GLUE2(all_fans, UFINT" ", rpm[x]);
    } else {
      ++y; /* non-spinning | removed | failed fan */
    }
  }
  FILL_STR_ARR(1, str1, (y != x ? buffer : NOT_FOUND));

}


void 
get_ssd(char *str1, uint8_t num) {
  uintmax_t val = 0;
  struct statvfs ssd;
  memset(&ssd, 0, sizeof(struct statvfs));

  if (-1 == (statvfs(getenv("HOME"), &ssd))) {
    FUNC_FAILED("statvfs()");
  }

  switch(num) {
    case 1:
      FILL_ARR(str1, FMT_UINT "%s",
        (uintmax_t)(ssd.f_blocks * ssd.f_bsize) / GB, "GB");
      break;
    case 2:
      FILL_ARR(str1, FMT_UINT "%s",
        (uintmax_t)(ssd.f_bfree * ssd.f_bsize) / GB, "GB");
      break;
    case 3:
      FILL_ARR(str1, FMT_UINT "%s",
        (uintmax_t)(ssd.f_bavail * ssd.f_bsize) / GB, "GB");
      break;
    case 4:
      {
        val = (uintmax_t)((
          ssd.f_blocks - ssd.f_bfree) * ssd.f_bsize) / GB;
        FILL_UINT_ARR(str1, val);
      }
      break;
  }
}


void 
get_kernel(char *str1, uint8_t num) {
  struct utsname KerneL;
  memset(&KerneL, 0, sizeof(struct utsname));

  if (-1 == (uname(&KerneL))) {
    FUNC_FAILED("uname()");
  }

  switch(num) {
    case 1:
      FILL_STR_ARR(1, str1, KerneL.sysname);
      break;
    case 2:
      FILL_STR_ARR(1, str1, KerneL.nodename);
      break;
    case 3:
      FILL_STR_ARR(1, str1, KerneL.release);
      break;
    case 4:
      FILL_STR_ARR(1, str1, KerneL.version);
      break;
    case 5:
      FILL_STR_ARR(1, str1, KerneL.machine);
      break;
    case 6:
      FILL_STR_ARR(2, str1, KerneL.sysname, KerneL.release);
      break;
  }
}


#if defined(__linux__)
/* Source (my improved screenfetch-c fork):
 * https://github.com/wifiextender/screenfetch-c/blob/master/src/plat/linux/detect.c */
static uint_fast16_t 
glob_packages(const char *str1) {
  uint_fast16_t packs_num = 0;
  glob_t gl;

  if (0 == (glob(str1, GLOB_NOSORT, NULL, &gl))) {
    packs_num = gl.gl_pathc;
  } else {
    exit_with_err("Could not traverse", str1);
  }
  globfree(&gl);

  return packs_num;
}
#endif /* __linux__ */


/* Let the compiler strip the code
 * instead conditionally checking 
 * each time the program is executed */
void 
get_packs(char *str1) {
  uint_fast16_t packages = 0;
  FILE *pkgs_file = NULL;

#if DISTRO == ARCHLINUX
  packages = glob_packages("/var/lib/pacman/local/*");

#elif DISTRO == FRUGALWARE

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  CHECK_POPEN(pkgs_file, "pacman-g2 -Q 2> /dev/null | wc -l", &packages);
#pragma GCC diagnostic pop

#elif DISTRO == DEBIAN
  packages = glob_packages("/var/lib/dpkg/info/*.list");

#elif DISTRO == SLACKWARE
  packages = glob_packages("/var/log/packages/*");

#elif DISTRO == GENTOO
  packages = glob_packages("/var/db/pkg/*/*");

#elif DISTRO == RHEL

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  CHECK_POPEN(pkgs_file, "rpm -qa 2> /dev/null | wc -l", &packages);
#pragma GCC diagnostic pop

#elif DISTRO == ANGSTROM

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  CHECK_POPEN(pkgs_file, "opkg list-installed 2> /dev/null | wc -l", &packages);
#pragma GCC diagnostic pop

#elif DISTRO == FREEBSD

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  CHECK_POPEN(pkgs_file, "pkg info | wc -l", &packages);
#pragma GCC diagnostic pop

#elif DISTRO == OPENBSD

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  CHECK_POPEN(pkgs_file, "pkg_info | wc -l", &packages);
#pragma GCC diagnostic pop


#endif

  FILL_ARR(str1, UFINT, packages);
}


void
get_uptime(char *str1) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
  struct timespec tc = {0};
#pragma GCC diagnostic pop

  uintmax_t now = 0;

#if defined(__linux__)
  if (-1 == (clock_gettime(CLOCK_BOOTTIME, &tc))) {
    FUNC_FAILED("clock_gettime()");
  }

  now = (uintmax_t)tc.tv_sec;
#else
  int mib[] = { CTL_KERN, KERN_BOOTTIME };
  time_t t;
  size_t len = sizeof(tc);

  if (0 != (sysctl(mib, 2, &tc, &len, NULL, 0))) {
    FUNC_FAILED("sysctl()");
  }

  if (-1 == (t = time(NULL))) {
    FUNC_FAILED("time()");
  }

  now = (uintmax_t)t - (uintmax_t)tc.tv_sec;
#endif /* __linux__ */

  if (0 != (now / 86400)) { /* days */
    FILL_ARR(str1, FMT_UINT "d " FMT_UINT "h " FMT_UINT "m",
      (now / 86400),
      ((now / 3600) % 24),
      ((now / 60) % 60));
    return;
  }
  if (59 < (now / 60)) { /* hours */
    FILL_ARR(str1, FMT_UINT "h " FMT_UINT "m",
      ((now / 3600) % 24),
      ((now / 60) % 60));
    return;
  }

  FILL_ARR(str1, FMT_UINT "m", ((now / 60) % 60));
}


/* The `strftime' man page showed potential bugs */
void 
get_taim(char *str1) {
  char time_str[VLA];
  time_t t;
  struct tm *taim;

  if (-1 == (t = time(NULL)) || 
      NULL == (taim = localtime(&t)) ||
      0 == (strftime(time_str, VLA, "%I:%M %p", taim))) {
    exit_with_err(ERR, "time() or localtime() or strftime() failed");
  }
  FILL_STR_ARR(1, str1, time_str);
}


#if defined (HAVE_X11_XLIB_H)
void 
set_status(const char *str1) {
  Display *display = XOpenDisplay(NULL);

  if (display) {
    XStoreName(display, DefaultRootWindow(display), str1);
    XSync(display, 0);
    XCloseDisplay(display);

  } else {
    exit_with_err(CANNOT_OPEN, "X server");
  }
}
#endif /* HAVE_X11_XLIB_H */


#if !defined(HAVE_SENSORS_SENSORS_H) && !defined(__OpenBSD__)
void 
get_fans(char *str1) {
  bool found_fans = true;
  char tempstr[VLA];
  uint_fast16_t x = 0, y = 0, z = 0, rpm[MAX_FANS+1];

  memset(rpm, 0, sizeof(rpm));

#if defined(__linux__)
  FILE *fp = NULL;
  for (x = 1; x < MAX_FANS; x++, z++) {
    FILL_ARR(tempstr, FAN_FILE, x);

    if (NULL == (fp = fopen(tempstr, "r"))) {
      if (1 == x) { /* no system fans at all */
        FILL_STR_ARR(1, str1, NOT_FOUND);
        found_fans = false;
      }
      break;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    CHECK_FSCANF(fp, SCAN_UFINT, &rpm[z]);
#pragma GCC diagnostic pop
    CLOSE_X(fp);
  }

#else

  u_int fan[3];
  memset(fan, 0, sizeof(fan));
  size_t len = sizeof(fan);

  for (x = 0; x < MAX_FANS; x++, z++) {
    FAN_STR(tempstr, x);
    memset(fan, 0, len);

    if (0 != (sysctlbyname(tempstr, &fan, &len, NULL, 0))) {
      if (0 == x) { /* no system fans at all */
        FILL_STR_ARR(1, str1, NOT_FOUND);
        found_fans = false;
      }
      break;
    }
    rpm[x] = (uint_fast16_t)fan[0];
  }

#endif /* __linux__ */

  if (true == found_fans) {
    check_fan_vals(str1, rpm, z);
  }
}
#endif /* !HAVE_SENSORS_SENSORS_H && !__OpenBSD__ */


#if defined(HAVE_CDIO_CDIO_H)
void
get_dvd(char *str1) {
  CdIo_t *p_cdio = cdio_open(NULL, DRIVER_DEVICE);
  cdio_hwinfo_t hwinfo;

  FILL_STR_ARR(1, str1, "Null");
  if (NULL == p_cdio) {
    return;
  }
  if (true == (mmc_get_hwinfo(p_cdio, &hwinfo))) {
    FILL_STR_ARR(2, str1, hwinfo.psz_vendor, hwinfo.psz_model);
  }
  if (NULL != p_cdio) {
    cdio_destroy(p_cdio);
  }
}
#else

#if defined(__linux__)
void
get_dvd(char *str1) {
  FILE *fp = NULL;
  char vendor[100], model[100];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  OPEN_X(fp, DVD_VEND, "%s", vendor);
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  OPEN_X(fp, DVD_MODEL, "%s", model);
#pragma GCC diagnostic pop

  FILL_STR_ARR(2, str1, vendor, model);
}
#endif /* __linux__ */

#endif /* HAVE_CDIO_CDIO_H */


#if defined(__FreeBSD__) || defined(__OpenBSD__)
void
get_loadavg(char *str1) {
  double up[3];
  memset(up, 0, sizeof(up));

  if (-1 == (getloadavg(up, 3))) {
  	FUNC_FAILED("getloadavg()");
  }
  FILL_ARR(str1, "%.2f %.2f %.2f",
    (float)up[0], (float)up[1], (float)up[2]);
}
#endif /* __FreeBSD__ || __OpenBSD__ */


void
split_n_index(char *str1) {
  char *ptr = str1;
  for (; *ptr; ptr++) {
    if (0 != (isspace((unsigned char) *ptr))) {
      break;
    }
    *str1++ = *ptr;
  }
  *str1 = '\0';
}
