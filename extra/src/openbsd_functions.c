/*
   09/08/2016

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

#include "config.h" /* Auto-generated */

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/swap.h>
#include <sys/disk.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/sensors.h>
#include <machine/apmvar.h>

#include "include/headers.h"
#include "include/openbzd.h"

/*
 Used resources:
  $OpenBSD: uvm_meter.c,v 1.36 2015/03/14 03:38:53 jsg Exp $
  $OpenBSD: machine.c,v 1.63 2007/11/01 19:19:48 otto Exp $
*/

void
get_ram(char *str1, uint8_t num) {
  uintmax_t total = 0, freeram = 0, pz = 0;

  pz      = (uintmax_t)sysconf(_SC_PAGESIZE);
  total   = ((uintmax_t)sysconf(_SC_PHYS_PAGES) * pz) / MB;
  freeram = ((uintmax_t)sysconf(_SC_AVPHYS_PAGES) * pz) / MB;

  switch(num) {
    case 1:
    case 2:
      FILL_ARR(str1, FMT_UINT "%s",
        ((1 == num) ? total: freeram), "MB");
      break;
    case 3:
      FILL_ARR(str1, FMT_UINT "%s", (total - freeram), "MB");
      break;
    case 5:
      FILL_UINT_ARR(str1,
        ((0 != total) ? (((total - freeram) * 100) / total) : 0));
      break;
  }
}


void
match_feature(char *str1, int8_t sens_type, int8_t sens_num) {
  int mib[] = { CTL_HW, HW_SENSORS, 0, sens_type, 0 };
  struct sensordev dev;
  struct sensor sens;
  uint_fast16_t z = 0, rpm[MAX_FANS+1];
  int8_t x = 0, y = 0;
  uintmax_t temp = 0;
  char buffer[VLA];
  char *all = buffer;
  bool found_fans = false;
  size_t len = 0;

  memset(rpm, 0, sizeof(rpm));
  memset(&dev, 0, sizeof(struct sensordev));
  memset(&sens, 0, sizeof(struct sensor));

  size_t dev_len = sizeof(dev), sens_len = sizeof(sens);
  SYSCTLVAL(mib, 3, &dev, &dev_len);

  for (mib[4] = 0; mib[4] < dev.maxnumt[sens_type]; mib[4]++) {
    if (0 != (sysctl(mib, 5, &sens, &sens_len, NULL, 0))) {
      break;
    }
    if (0 != sens_len &&
        !(sens.flags & (SENSOR_FINVALID | SENSOR_FUNKNOWN))) {

      switch(sens_type) {
        case SENSOR_VOLTS_DC:
          GLUE2(all, "%.2f ", ((float)sens.value / 1000000.0f));
          break;

        case SENSOR_TEMP:
          {
            if (x == sens_num) {
              temp = (uintmax_t)sens.value;
            }
            x++;
          }
          break;

        case SENSOR_FANRPM:
          {
            if (MAX_FANS != z) {
              rpm[z++] = (uint_fast16_t)sens.value;
            }
            found_fans = true;
          }
          break;

        default:
          continue;
      }
    }
  }

  if (SENSOR_VOLTS_DC == sens_type && '\0' != buffer[0]) {
    len = strlen(buffer);
    buffer[len-1] = '\0';

    FILL_STR_ARR(1, str1, buffer);
    return;
  }
  if (SENSOR_TEMP == sens_type) {
    if (999999999 < temp) { /* > 99C */
      FILL_UINT_ARR(str1, temp / 100000000);
    } else {
      FILL_UINT_ARR(str1, ((99999999 < temp) ?
        temp / 10000000 : temp / 1000000)); /* > 9C || < 9C */
    }
    return;
  }
  if (true == found_fans) {
    check_fan_vals(str1, rpm, z);
  }
}


void
get_voltage(char *str1) {
  match_feature(str1, SENSOR_VOLTS_DC, 0);
}


void
get_mobo_temp(char *str1) {
  match_feature(str1, SENSOR_TEMP, 1);
}


void
get_fans(char *str1) {
  match_feature(str1, SENSOR_FANRPM, 0);
}


void
get_cpu_temp(char *str1) {
  match_feature(str1, SENSOR_TEMP, 0);
}


void
get_mobo(char *str1) {
  int mib[] = { CTL_HW, HW_VENDOR };
  int mib2[] = { CTL_HW, HW_PRODUCT };
  char vendor[100], model[100];
  size_t len = sizeof(vendor);

  SYSCTLVAL(mib, 2, &vendor, &len);
  SYSCTLVAL(mib2, 2, &model, &len);

  split_n_index(vendor);
  split_n_index(model);

  FILL_STR_ARR(2, str1, vendor, model);
}


/*
 * Entirely based on:
 *  $OpenBSD: apmd.c,v 1.49 2007/11/24 14:58:44 deraadt Exp $
 * The source mentioned different values when
 * using Crapple machine that is charging the battery
*/
void
get_battery(char *str1) {
  struct apm_power_info bstate;
  int fd = 0;
  uintmax_t dummy = 0;

  FILL_STR_ARR(1, str1, "Null");
  memset(&bstate, 0, sizeof(struct apm_power_info));

  if (0 != (fd = open("/dev/apm", O_RDONLY))) {
    return;
  }
  if (0 != (ioctl(fd, APM_IOC_GETPOWER, &bstate))) {
    CLOSE_FD(fd);
    return;
  }
  CLOSE_FD(fd);

  if (APM_BATT_UNKNOWN == bstate.battery_state ||
      APM_BATTERY_ABSENT == bstate.battery_state) {
    return;
  }

  dummy = (uintmax_t)bstate.battery_life;
  FILL_UINT_ARR(str1, (101 < dummy ? 0 : dummy));
}


/*
 * Based on:
 *  $OpenBSD: swap.c,v 1.27 2015/08/20 22:32:42 deraadt Exp $
*/
void
get_swapp(char *str1, uint8_t num) {
  struct swapent *dev = NULL;
  int count = 0, stats = 0, x = 0;
  uintmax_t total = 0, used = 0;

  FILL_STR_ARR(1, str1, "0 MB");
  if (-1 == (count = swapctl(SWAP_NSWAP, 0, 0))) {
    return;
  }

  dev = (struct swapent *)malloc(
      (unsigned long)count * sizeof(struct swapent));
  if (NULL == dev) {
    return;
  }

  stats = swapctl(SWAP_STATS, dev, count);
  if (-1 == stats || stats != count) {
    goto error;
  }

  for (x = 0; x < count; x++) {
    total += (uintmax_t)dev->se_nblks;
    used  += (uintmax_t)dev->se_inuse;
  }

  switch(num) {
    case 1:
      FILL_ARR(str1, FMT_UINT" %s", BYTES_TO_MB(total), "MB");
      break;
    case 2:
      FILL_ARR(str1, FMT_UINT" %s",
        BYTES_TO_MB((total - used)), "MB");
      break;
    case 3:
      FILL_ARR(str1, FMT_UINT" %s", BYTES_TO_MB(used), "MB");
      break;
    case 4:
      FILL_ARR(str1, FMT_UINT"%%",
        ((0 != total) ? ((used * 100) / total) : 0));
      break;
  }

error:
  if (NULL != dev) {
    free(dev);
  }
  return;
}


void
get_statio(char *str1, char *str2) {
  struct diskstats *ds = NULL;
  int mib[] = { CTL_HW, HW_DISKSTATS };
  uint_least16_t x = 0;
  size_t len = 0, drivez = 0;

  FILL_STR_ARR(1, str1, "Null");
  SYSCTLVAL(mib, 2, NULL, &len);

  if (0 == (drivez = (len / sizeof(*ds)))) {
    return;
  }

  ds = (struct diskstats *)malloc(drivez * sizeof(char *));
  if (NULL == ds) {
    return;
  }
  if (0 != (sysctl(mib, 2, ds, &len, NULL, 0))) {
    goto error;
  }

  for (x = 0; x < drivez; x++) {
    if (STREQ(str2, ds[x].ds_name)) {
      FILL_ARR(str1, "Read " FMT_UINT " MB, Written " FMT_UINT " MB",
        (uintmax_t)ds[x].ds_rbytes / MB,
        (uintmax_t)ds[x].ds_wbytes / MB);
      break;
    }
  }

error:
  if (NULL != ds) {
    free(ds);
  }
  return;
}
