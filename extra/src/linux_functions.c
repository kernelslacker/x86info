/*
   07/18/2016

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

/* The pragma directives are here
 * to mute the gcc twisted vision,
 * and clangs inabillity to distinguish
 * C from C++
 *
 * https://llvm.org/bugs/show_bug.cgi?id=21689 
 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66425
 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=25509
 *
 * Do not add any -Wno flags just to mute the compilers snafus
 * */

#include "config.h" /* Auto-generated */

#include <sys/sysinfo.h>

#if defined(HAVE_SENSORS_SENSORS_H)
#include <sensors/sensors.h>
#endif /* HAVE_SENSORS_SENSORS_H */

#include "include/headers.h"

void 
get_ram(char *str1, uint8_t num) {
  uintmax_t used = 0, total = 0;
  struct sysinfo mem;
  memset(&mem, 0, sizeof(struct sysinfo));

  if (-1 == (sysinfo(&mem))) {
    FUNC_FAILED("sysinfo()");
  }

  switch(num) {
    case 1:
      FILL_ARR(str1, FMT_UINT "%s",
        (uintmax_t)mem.totalram / MB, "MB");
      break;
    case 2:
      FILL_ARR(str1, FMT_UINT "%s",
        (uintmax_t)mem.freeram / MB, "MB");
      break;
    case 3:
      FILL_ARR(str1, FMT_UINT "%s",
        (uintmax_t)mem.sharedram / MB, "MB");
      break;
    case 4:
      FILL_ARR(str1, FMT_UINT "%s",
        (uintmax_t)mem.bufferram / MB, "MB");
      break;
    case 5:
      {
        total   = (uintmax_t) mem.totalram / MB;
        used    = (uintmax_t) (mem.totalram - mem.freeram -
                         mem.bufferram - mem.sharedram) / MB;
        FILL_UINT_ARR(str1,
          ((0 != total) ? ((used * 100) / total) : 0));
      }
      break;
  }
}


void
get_ssd_model(char *str1, char *str2) {
  FILE *fp = NULL;
  char model[VLA];
  FILL_ARR(model, "%s%s%s", "/sys/block/", str2, "/device/model");

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  OPEN_X(fp, model, "%[^\n]", model);
#pragma GCC diagnostic pop

  FILL_STR_ARR(1, str1, model);
}


void
get_loadavg(char *str1) {
  struct sysinfo up;
  memset(&up, 0, sizeof(struct sysinfo));

  if (-1 == (sysinfo(&up))) {
    FUNC_FAILED("sysinfo()");
  }
  FILL_ARR(str1, "%.2f %.2f %.2f",
    (float)up.loads[0] / 65535.0f,
    (float)up.loads[1] / 65535.0f,
    (float)up.loads[2] / 65535.0f);
}

/* My inital attempt was to make this code FreeBSD exclusive as the
 * sensors parsing there is much more difficult than it is in linux.
 */
#if defined(HAVE_SENSORS_SENSORS_H)
void
match_feature(char *str1, uint8_t num) {
#if SENSORS_API_VERSION >= 0x400 && SENSORS_API_VERSION <= 0x499
  const sensors_chip_name *chip;
  const sensors_feature *features;
  const sensors_subfeature *subfeatures;
  char buffer[VLA];
  char *label = NULL, *all = buffer;
  double value = 0.0;
  int nr = 0, nr2 = 0, nr3 = 0;
  uint8_t x = 0, y = 0, z = 0, dummy = 0;
  uint_fast16_t rpm[MAX_FANS+1];
  bool found_fans = false;
  const char *temp_sens[] = { "MB Temperature", "CPU Temperature" };
  size_t len = 0;

  if (3 == num) {
    memset(rpm, 0, sizeof(rpm));
  }

  /* In case I need to parse the rest
   * min, max, input values */
  /* struct stats_pwr_in *st_pwr_in_i; */
  /* struct stats_pwr_temp *st_pwr_temp_i; */
  /* struct stats_pwr_fan *st_pwr_fan_i; */

  FILL_STR_ARR(1, str1, "Null");
  if (0 != (sensors_init(NULL))) {
    FUNC_FAILED("sensors_init()");
  }

  while (NULL != (chip = sensors_get_detected_chips(NULL, &nr))) {
    nr2 = 0;
    while (NULL != (features = sensors_get_features(chip, &nr2))) {
      nr3 = 0;
      while (NULL != (subfeatures = sensors_get_all_subfeatures(chip, features, &nr3))) {
        switch(subfeatures->type) {

          case SENSORS_SUBFEATURE_IN_INPUT:
            {
              if (1 == num) {
                if (0 != (sensors_get_value(chip, subfeatures->number, &value))) {
                  break;
                }
                GLUE2(all, "%.2f ", (float)value);
              }
            }
            break;

          case SENSORS_SUBFEATURE_TEMP_INPUT:
            {
              if (2 == num || 4 == num) {
                dummy = (2 == num ? 0 : 1);
                if (0 != (sensors_get_value(chip, subfeatures->number, &value))) {
                  break;
                }
                if (NULL == (label = sensors_get_label(chip, features))) {
                  break;
                }
                if (STREQ(temp_sens[dummy], label)) {
                  FILL_ARR(str1, UFINT, (uint_fast16_t)value);
                }
                if (NULL != label) {
                  free(label);
                }
              }
            }
            break;

          case SENSORS_SUBFEATURE_FAN_INPUT:
            {
              if (3 == num) {
                if (0 != (sensors_get_value(chip, subfeatures->number, &value))) {
                  break;
                }
                if (MAX_FANS != z) {
                  rpm[z++] = (uint_fast16_t)value;
                }
                found_fans = true;
              }
            }
            break;

          default:
            continue;
        }
      }
    }
  }
  sensors_cleanup();

  if (1 == num && '\0' != buffer[0]) {
    len = strlen(buffer);
    buffer[len-1] = '\0';

    FILL_STR_ARR(1, str1, buffer);
    return;
  }

  if (true == found_fans) {
    check_fan_vals(str1, rpm, z);
  }

#else
  exit_with_err(ERR, "The version of your sensors API is not supported by this program.");
#endif /* SENSORS_API_VERSION >= 0x400 && SENSORS_API_VERSION <= 0x499 */
}


void
get_voltage(char *str1) {
  match_feature(str1, 1);
}


void
get_mobo_temp(char *str1) {
  match_feature(str1, 2);
}


void
get_fans(char *str1) {
  match_feature(str1, 3);
}


void
get_cpu_temp(char *str1) {
  match_feature(str1, 4);
}


#else /* fall back */
void 
get_voltage(char *str1) {
  float voltage[4];
  FILE *fp = NULL;
  uint8_t x = 0;
  memset(voltage, 0, sizeof(voltage));

  const char *voltage_files[] = {
    VOLTAGE_FILE("0"),
    VOLTAGE_FILE("1"),
    VOLTAGE_FILE("2"),
    VOLTAGE_FILE("3")
  };

  for (x = 0; x < 4; x++) {
    if (NULL == (fp = fopen(voltage_files[x], "r"))) {
      exit_with_err(CANNOT_OPEN, voltage_files[x]);
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    CHECK_FSCANF(fp, "%f", &voltage[x]);
#pragma GCC diagnostic pop
    CLOSE_X(fp);

    voltage[x] /= 1000.0f;
  }
  FILL_ARR(str1, "%.2f %.2f %.2f %.2f",
    voltage[0], voltage[1], voltage[2], voltage[3]);
}


void
get_mobo_temp(char *str1) {
  get_temp(MOBO_TEMP_FILE, str1);
}


void
get_cpu_temp(char *str1) {
  get_temp(CPU_TEMP_FILE, str1);
}
#endif /* HAVE_SENSORS_SENSORS_H */


void 
get_mobo(char *str1) {
  FILE *fp = NULL;
  char vendor[100], name[100];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  OPEN_X(fp, MOBO_VENDOR, "%s", vendor);
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  OPEN_X(fp, MOBO_NAME, "%s", name);
#pragma GCC diagnostic pop

  FILL_STR_ARR(2, str1, vendor, name);
}


void 
get_statio(char *str1, char *str2) {
  uintmax_t statio[7];
  char stat_file[VLA];
  FILL_ARR(stat_file, "%s%s%s", "/sys/block/", str2, "/stat");

  memset(statio, 0, sizeof(statio));

  FILE *fp = fopen(stat_file, "r");
  CHECK_FP(fp);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  if (EOF == (fscanf(fp, FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT,
    &statio[0], &statio[1], &statio[2], &statio[3],
    &statio[4], &statio[5], &statio[6]))) {
      CLOSE_X(fp);
      exit_with_err(ERR, "reading the stat file failed");
  }
#pragma GCC diagnostic pop
  CLOSE_X(fp);

  FILL_ARR(str1, "Read " FMT_UINT " MB, Written " FMT_UINT " MB",
    BYTES_TO_MB(statio[2]), BYTES_TO_MB(statio[6]));
}


/* Thanks to https://bugzilla.kernel.org/show_bug.cgi?id=83411 */
void
get_battery(char *str1) {
  uintmax_t used = 0, total = 0;
  uint8_t num = 0;
  char temp[VLA];
  BATTERY_TOTAL(temp, num);

  FILE *fp = fopen(temp, "r");
  if (NULL == fp) {
    num = 1;
    BATTERY_TOTAL(temp, num);

    if (NULL == (fp = fopen(temp, "r"))) {
      exit_with_err(CANNOT_OPEN, "BAT0 and BAT1");
    }
  }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  CHECK_FSCANF(fp, FMT_UINT, &total);
#pragma GCC diagnostic pop
  CLOSE_X(fp);

  BATTERY_USED(temp, num);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  OPEN_X(fp, temp, FMT_UINT, &used);
#pragma GCC diagnostic pop

  FILL_UINT_ARR(str1,
    ((0 != total) ? ((used * 100) / total) : 0));
}
