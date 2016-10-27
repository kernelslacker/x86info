/*
   07/29/2015, 07/17/2016

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

#ifndef CONSTANTS2_H_
#define CONSTANTS2_H_

#define KB 1024
#define MB 1048576     /* 1024 * 1024     */
#define GB 1073741824  /* MB * 1024       */
#define Mb 131072      /* 1024 * 1024 / 8 */
#define BYTES_TO_MB(x) (x/2/1024) /* block bytes */

#define FMT_UINT "%"PRIuMAX
#define UFINT "%"PRIuFAST16
#define ULINT "%"PRIuLEAST32
#define USINT "%"PRIu8
#define SCAN_UFINT "%"SCNuFAST16
#define SCAN_ULINT "%"SCNuLEAST32
#define SCAN_UINTX "%"PRIxMAX /* hex */

/* stay away from va_list */
#define FILL_ARR(x, z, ...) (snprintf(x, VLA, z, __VA_ARGS__))
#define FILL_UINT_ARR(x, z) (FILL_ARR(x, FMT_UINT, z))
#define FILL_STR_ARR(x, z, ...) (FILL_ARR(z, (1 == x ? "%s" : "%s %s"), __VA_ARGS__))
#define GLUE2(x, z, ...) (x+=snprintf(x, VLA, z, __VA_ARGS__))

/* temperature sensors */
#define HWMON_DIR "/sys/class/hwmon/hwmon0"
#define TEMPERATURE(x) (HWMON_DIR"/temp"x"_input")
#define CPU_TEMP_FILE TEMPERATURE("1")
#define MOBO_TEMP_FILE TEMPERATURE("2")

/* motherboard model and vendor */
#define MOTHERBOARD(x) ("/sys/class/dmi/id/board_"x)
#define MOBO_NAME MOTHERBOARD("name")
#define MOBO_VENDOR MOTHERBOARD("vendor")

/* voltage and fans */
#if defined(__linux__)
#define VOLTAGE_FILE(x) (HWMON_DIR"/in"x"_input")
#define FAN_FILE HWMON_DIR"/fan"UFINT"_input"
#endif /* __linux__ */

#if defined(__FreeBSD__)

#if defined(MOBO_MODL) && defined(CPU_MODL)
#define MOBO_VAL(x) MOBO_MODL "." x
#define CPU_TEMP CPU_MODL
#else
#define MOBO_VAL(x) "dev.aibs.0."x
#define CPU_TEMP "dev.cpu.0.temperature"

#endif /* MOBO_MODL && CPU_MODL */

#define VOLTAGE_FILE(x) MOBO_VAL("volt") "." x
#define FAN_STR(x, z) (FILL_ARR(x, "%s"UFINT, MOBO_VAL("fan") ".", z))
#endif /* __FreeBSD__ */

/* battery reports */
#define BATTERY_NUM(x, y, z) (FILL_ARR(x, "%s"USINT"%s%s", \
  "/sys/class/power_supply/BAT", y, "/charge_", z))
#define BATTERY_USED(x, z)  (BATTERY_NUM(x, z, "now"))
#define BATTERY_TOTAL(x, z) (BATTERY_NUM(x, z, "full"))

/* NIC model and vendor */
#define NIC_NUM(x, y, z) (FILL_ARR(x, "%s%s%s%s", \
  "/sys/class/net/", y, "/device/", z))
#define NIC_VEND(x, z)  (NIC_NUM(x, z, "vendor"))
#define NIC_MODEL(x, z) (NIC_NUM(x, z, "device"))

/* The cdrom/dvdrom vendor and model */
#define DVD_DIR(x) ("/sys/class/block/sr0/device/"x)
#define DVD_VEND DVD_DIR("vendor")
#define DVD_MODEL DVD_DIR("model")

/* exit with error */
#define CANNOT_OPEN "Could not open"
#define CANNOT_CLOSE "Could not close a file handle"
#define CANNOT_OPEN_FP "Could not open a file handle"
#define FSCANF_FAILED "fscanf() failed"
#define ERR "Error:"
#define NOT_FOUND "Not found, "
#define FUNC_FAILED(x) (exit_with_err(ERR, x " failed"))
#define RECOMPILE_WITH(x) (exit_with_err(ERR, "recompile the program --with-" x))

/* Let the preprocessor Do Repeat Myself */
#define CHECK_SSCANF(buf, x, z) \
  if (EOF == (sscanf(buf, x, z))) { \
    exit_with_err(ERR, "sscanf() EOF"); \
  }

#define CHECK_FSCANF(fp, x, z) \
  if (EOF == (fscanf(fp, x, z))) { \
    exit_with_err(ERR, FSCANF_FAILED); \
  }

#define CHECK_FP(fp) \
  if (NULL == fp) { \
    exit_with_err(ERR, CANNOT_OPEN_FP); \
  }

#define CLOSE_X(fp) \
  if (EOF == (fclose(fp))) { \
    exit_with_err(ERR, CANNOT_CLOSE); \
  }

#define OPEN_X(fp, x, y, z) \
  if (NULL == (fp = fopen(x, "r"))) { \
    exit_with_err(CANNOT_OPEN, x); \
  } \
  CHECK_FSCANF(fp, y, z); \
  CLOSE_X(fp);

#define CHECK_POPEN(fp, x, packs) \
  if (NULL == (fp = popen(x, "r"))) { \
    exit_with_err(CANNOT_OPEN, x); \
  } \
  CHECK_FSCANF(fp, SCAN_UFINT, packs); \
  if (-1 == (pclose(fp))) { \
    exit_with_err(CANNOT_CLOSE, x); \
  }

#define CLOSE_FD(fd) \
  if (-1 == (close(fd))) { \
    exit_with_err(ERR, CANNOT_CLOSE); \
  }

#define CLOSE_FD2(fd, res) \
  if (-1 == (close(fd))) { \
    freeaddrinfo(res); \
    exit_with_err(ERR, CANNOT_CLOSE); \
  }

/* How many fans to try for detection */
#define MAX_FANS 20

#define STREQ(x, z) (0 == (strcmp(x, z)))
#define FPRINTF(...) (fprintf(stderr, __VA_ARGS__))

#endif /* CONSTANTS2_H_ */
