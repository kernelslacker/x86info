/*
   08/16/2016

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
#include <vm/vm_param.h>
#include <devstat.h>

#if GOT_APM == 1 && defined(HAVE_MACHINE_APM_BIOS_H)
#include <fcntl.h>
#include <sys/ioctl.h>
#include <machine/apm_bios.h>
#endif /* GOT_APM && HAVE_MACHINE_APM_BIOS_H */

#include "include/headers.h"
#include "include/freebzd.h"


void 
get_ram(char *str1, uint8_t num) {
  u_int total = 0, freeram = 0, inactive = 0, pagesize = 0;
  u_int cache = 0, bufferram = 0, dummy = 0;
  uintmax_t utotal = 0, used = 0, pz = 0;
  size_t len = sizeof(dummy);

  SYSCTLVAL("vm.stats.vm.v_page_size", &pagesize);
  SYSCTLVAL("vm.stats.vm.v_page_count", &total);
  SYSCTLVAL("vm.stats.vm.v_inactive_count", &inactive);
  SYSCTLVAL("vm.stats.vm.v_free_count", &freeram);
  SYSCTLVAL("vm.stats.vm.v_cache_count", &cache);
  SYSCTLVAL("vfs.bufspace", &bufferram);

  pz = (uintmax_t)pagesize;
  used = ((uintmax_t)total * pz - (uintmax_t)freeram * pz
          - (uintmax_t)inactive * pz
          - (uintmax_t)cache * pz - (uintmax_t)bufferram) / MB;
  utotal = ((uintmax_t)total * pz) / MB;

  switch(num) {
    case 1:
      FILL_ARR(str1, FMT_UINT "%s", utotal, "MB");
      break;
    case 2:
      FILL_ARR(str1, FMT_UINT "%s",
        ((uintmax_t)freeram * pz) / MB, "MB");
      break;
    case 3:
      FILL_ARR(str1, FMT_UINT "%s",
        ((uintmax_t)inactive * pz) / MB, "MB");
      break;
    case 4:
      FILL_ARR(str1, FMT_UINT "%s",
        (uintmax_t)bufferram / MB, "MB");
      break;
    case 5:
      FILL_UINT_ARR(str1,
        ((0 != utotal) ? ((used * 100) / utotal) : 0));
      break;
  }
}


void
get_voltage(char *str1) {
  u_int  vol0[3], vol1[3], vol2[3], vol3[3];
  
  memset(vol0, 0, sizeof(vol0));
  memset(vol1, 0, sizeof(vol1));
  memset(vol2, 0, sizeof(vol2));
  memset(vol3, 0, sizeof(vol3));

  size_t len = sizeof(vol0);
  
  SYSCTLVAL(VOLTAGE_FILE("0"), &vol0);
  SYSCTLVAL(VOLTAGE_FILE("1"), &vol1);
  SYSCTLVAL(VOLTAGE_FILE("2"), &vol2);
  SYSCTLVAL(VOLTAGE_FILE("3"), &vol3);

  FILL_ARR(str1, "%.2f %.2f %.2f %.2f",
    (float)vol0[0] / 1000.0f,
    (float)vol1[0] / 1000.0f,
    (float)vol2[0] / 1000.0f,
    (float)vol3[0] / 1000.0f);
}


void
get_cpu_temp(char *str1) {
  u_int temp = 0;
  size_t len = sizeof(temp);

  SYSCTLVAL(CPU_TEMP, &temp);
  get_temp(str1, (uint_least32_t)temp);
}


void
get_mobo_temp(char *str1) {
  u_int  temp[3];
  memset(temp, 0, sizeof(temp));
  size_t len = sizeof(temp);

  SYSCTLVAL(MOBO_VAL("temp.1"), &temp);
  get_temp(str1, (uint_least32_t)temp[0]);
}


void
get_mobo(char *str1) {
  char temp[VLA];
  size_t len = sizeof(temp);

  SYSCTLVAL(MOBO_VAL("%desc"), &temp);
  FILL_STR_ARR(1, str1, temp);
}


/*
  Couldn't add this option on my own.
  Used the following resources to make this function happen:
    http://ftp.stu.edu.tw/FreeBSD/branches/3.0-stable/src/libexec/rpc.rstatd/rstat_proc.c
    https://github.com/giampaolo/psutil/blob/master/psutil/arch/bsd/freebsd.c
    http://opensource.apple.com/source/net_snmp/net_snmp-16/net-snmp/agent/mibgroup/ucd-snmp/diskio.c
    https://searchcode.com/codesearch/view/29835031/
    http://fossies.org/linux/pcp/src/pmdas/freebsd/disk.c
  Had to use Valgrind since we allocate memory with malloc.
*/
void
get_statio(char *str1, char *str2) {
  struct statinfo stats;
  struct device_selection *dev_select = NULL;
  struct devstat *d = NULL;
  long int select_generation = 0;
  int x = 0, num_devices = 0, num_selected = 0, num_selections = 0;

  FILL_STR_ARR(1, str1, "Null");
  if(0 != devstat_checkversion(NULL)) {
    return;
  }

  memset(&stats, 0, sizeof(struct statinfo));
  stats.dinfo = (struct devinfo *)malloc(sizeof(struct devinfo));
  if (NULL == stats.dinfo) {
    return;
  }

  if(-1 == (devstat_getdevs(NULL, &stats))) {
    goto error;
  }

  num_devices = stats.dinfo->numdevs;
  if (-1 == (devstat_selectdevs(&dev_select, &num_selected, &num_selections,
    &select_generation, stats.dinfo->generation, stats.dinfo->devices, num_devices,
    NULL, 0, NULL, 0, DS_SELECT_ADD, 16, 0))) {
    goto error;
  }

  for (x = 0; x < 16; x++) {
    d = &stats.dinfo->devices[x];
    if (STREQ(str2, d->device_name)) {

      if (d->device_type != DEVSTAT_TYPE_DIRECT &&
        d->device_type != DEVSTAT_TYPE_IF_SCSI &&
        d->device_type != DEVSTAT_TYPE_IF_IDE) {
        break;
      }

      FILL_ARR(str1, "Read " FMT_UINT " MB, Written " FMT_UINT " MB",
        (uintmax_t)d->bytes[DEVSTAT_READ] / MB,
        (uintmax_t)d->bytes[DEVSTAT_WRITE] / MB);
      break;
    }
  }

error:
  if (NULL != dev_select) {
    free(dev_select);
  }
  if (NULL != stats.dinfo) {
    if (stats.dinfo->mem_ptr) {
      free(stats.dinfo->mem_ptr);
    }
    free(stats.dinfo);
  }
  return;
}


/* https://www.freebsd.org/doc/handbook/acpi-overview.html
    ACPI_BATT_STAT_NOT_PRESENT
*/
#if GOT_APM == 1 && defined(HAVE_MACHINE_APM_BIOS_H)
void
get_battery(char *str1) {
  struct apm_info bstate;
  int fd = 0;
  uintmax_t dummy = 0;

  FILL_STR_ARR(1, str1, "Null");
  memset(&bstate, 0, sizeof(struct apm_info));

  if (0 != (fd = open("/dev/apm", O_RDONLY))) {
    return;
  }
  if (0 != (ioctl(fd, APMIO_GETINFO, &bstate))) {
    CLOSE_FD(fd);
    return;
  }
  CLOSE_FD(fd);

  dummy = (uintmax_t)bstate.ai_batt_life;
  FILL_UINT_ARR(str1, (101 < dummy ? 0 : dummy));
}

#else

void 
get_battery(char *str1) {
  u_int dummy = 0;
  uint_least32_t perc = 0;
  size_t len = sizeof(dummy);
  SYSCTLVAL("hw.acpi.battery.life", &dummy);

  perc = (uint_least32_t)dummy;
  FILL_ARR(str1, ULINT, (101 < perc ? 0 : perc));
}
#endif /* GOT_APM && HAVE_MACHINE_APM_BIOS_H */


void
get_swapp(char *str1, uint8_t num) {
  struct xswdev xsw;
  u_int pagesize = 0, dummy = 0;
  uintmax_t total = 0, used = 0, pz = 0;
  int mib[20];

  memset(mib, 0, sizeof(mib));
  memset(&xsw, 0, sizeof(struct xswdev));
  size_t mibi = sizeof(mib) / sizeof(mib[0]);
  size_t len = sizeof(dummy), sisi = sizeof(struct xswdev);

  FILL_STR_ARR(1, str1, "0 MB");
  SYSCTLVAL("vm.stats.vm.v_page_size", &pagesize);
  pz = (uintmax_t)pagesize;

  if (0 != (sysctlnametomib("vm.swap_info", mib, &mibi))) {
    return;
  }
  if (0 != (sysctl(mib, (u_int)(mibi + 1), &xsw, &sisi, NULL, 0))) {
    return;
  }
  if (xsw.xsw_version != XSWDEV_VERSION) {
    return;
  }
  used  = (uintmax_t)xsw.xsw_used;
  total = (uintmax_t)xsw.xsw_nblks;

  switch(num) {
    case 1:
      FILL_ARR(str1, FMT_UINT" %s", ((total * pz) / MB), "MB");
      break;
    case 2:
      FILL_ARR(str1, FMT_UINT" %s",
        (((total - used) * pz) / MB), "MB");
      break;
    case 3:
      FILL_ARR(str1, FMT_UINT" %s", ((used * pz) / MB), "MB");
      break;
    case 4:
      FILL_ARR(str1, FMT_UINT"%%",
        ((0 != total) ? ((used * 100) / total) : 0));
      break;
  }
}
