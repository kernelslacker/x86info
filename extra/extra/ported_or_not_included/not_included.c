/* "borrowed" from the Hacked Team */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>

struct device_info {
   struct {
      char vendor[128];
      char model[128];
      char cpu[128];
      unsigned char ncpu;
   } hw;
   struct {
      unsigned int memt;
      unsigned int memf;
      unsigned int diskt;
      unsigned int diskf;
   } stat;
   struct {
      char ac[16];
      unsigned char battery;
   } power;
   struct {
      char ver[128];
      char arch[16];
      char lang[16];
      char tzname[8];
      char tzoff[8];
   } os;
   struct {
      char name[16];
      unsigned int uid;
      unsigned int gid;
      char gecos[64];
      char home[64];
   } user;
   struct {
      char list[1024];
   } mount;
};
 
int main(void);
void device_hw(struct device_info *di);
void device_stat(struct device_info *di);
void device_power(struct device_info *di);
void device_os(struct device_info *di);
void device_user(struct device_info *di);
void device_mount(struct device_info *di);

int main(void)
{
   struct device_info di;

   memset(&di, 0x00, sizeof(di));
   device_hw(&di);
   device_stat(&di);
   device_power(&di);
   device_os(&di);
   device_user(&di);
   device_mount(&di);

   printf("Device: %s %s\n", di.hw.vendor, di.hw.model);
   printf("Processor: %u x %s\n", di.hw.ncpu, di.hw.cpu);
   printf("Memory: %uMB (%u%% used)\n", di.stat.memt, 100 - di.stat.memf * 100 / di.stat.memt);
   printf("Disk: %uGB (%u%% used)\n", di.stat.diskt, 100 - di.stat.diskf * 100 / di.stat.diskt);
   printf("Power: AC %s - battery %%\n", di.power.ac);
   printf("\n");
   printf("OS Version: Linux %s (%s)\n", di.os.ver, di.os.arch);
   printf("Locale settings: %s - %s (UTC %s)\n", di.os.lang, di.os.tzname, di.os.tzoff);
   printf("\n");
   printf("Mounted filesystems:\n");
   printf("%s\n", di.mount.list);

   return 0;
}

void device_hw(struct device_info *di)
{
   FILE *fp = NULL;
   char buf[128], *ptr = NULL;

   if(!(fp = fopen("/sys/devices/virtual/dmi/id/sys_vendor", "r")) || !fgets(di->hw.vendor, sizeof(di->hw.vendor), fp)) {
      strncpy(di->hw.vendor, "(unknown)", sizeof(di->hw.vendor) - 1);
   }
   if(fp) fclose(fp);

   if(!(fp = fopen("/sys/devices/virtual/dmi/id/product_name", "r")) || !fgets(di->hw.model, sizeof(di->hw.model), fp)) {
      di->hw.model[0] = '\n';
   }
   if(fp) fclose(fp);

   if(di->hw.vendor[strlen(di->hw.vendor) - 1] == '\n') di->hw.vendor[strlen(di->hw.vendor) - 1] = '\0';
   while(di->hw.vendor[strlen(di->hw.vendor) - 1] == ' ') di->hw.vendor[strlen(di->hw.vendor) - 1] = '\0';
   if(di->hw.model[strlen(di->hw.model) - 1] == '\n') di->hw.model[strlen(di->hw.model) - 1] = '\0';
   while(di->hw.model[strlen(di->hw.model) - 1] == ' ') di->hw.model[strlen(di->hw.model) - 1] = '\0';

   if(!(fp = fopen("/proc/cpuinfo", "r"))) return;
   while(fgets(buf, sizeof(buf), fp)) {
      if(!strncasecmp(buf, "model name", strlen("model name"))) {
         if((ptr = strstr(buf, ": ") + 2)) strncpy(di->hw.cpu, ptr, sizeof(di->hw.cpu) - 1);
      } else if(!strncasecmp(buf, "processor", strlen("processor"))) {
         if((ptr = strstr(buf, ": ") + 2)) di->hw.ncpu = atoi(ptr) + 1;
      }
   }
   fclose(fp);

   if((di->hw.cpu[0]) && (di->hw.cpu[strlen(di->hw.cpu) - 1] == '\n')) di->hw.cpu[strlen(di->hw.cpu) - 1] = '\0';
   
   return;
}

void device_stat(struct device_info *di)
{
   FILE *fp = NULL;
   struct statvfs s;
   char buf[128], *ptr = NULL;

   if((fp = fopen("/proc/meminfo", "r"))) {
      while(fgets(buf, sizeof(buf), fp)) {
         if(!strncasecmp(buf, "MemTotal:", strlen("MemTotal:"))) {
            ptr = buf + strlen("MemTotal:");
            while(*ptr && (*ptr == ' ')) ptr++;
            di->stat.memt = atoll(ptr) / 1024;
         } else if(!strncasecmp(buf, "MemFree:", strlen("MemFree:"))) {
            ptr = buf + strlen("MemFree:");
            while(*ptr && (*ptr == ' ')) ptr++;
            di->stat.memf += atoll(ptr) / 1024;
         } else if(!strncasecmp(buf, "Cached:", strlen("Cached:"))) {
            ptr = buf + strlen("Cached:");
            while(*ptr && (*ptr == ' ')) ptr++;
            di->stat.memf += atoll(ptr) / 1024;
         }
      }
      fclose(fp);
   }

   if(di->stat.memf > di->stat.memt) di->stat.memf = di->stat.memt;

   if((ptr = getcwd(NULL, 0))) {
      if(statvfs(ptr, &s)) return;
      di->stat.diskt = (unsigned int)((unsigned long long)s.f_blocks * (unsigned long long)s.f_bsize / (unsigned long long)1073741824);
      di->stat.diskf = (unsigned int)((unsigned long long)s.f_bavail * (unsigned long long)s.f_bsize / (unsigned long long)1073741824);
      free(ptr);
   }

   return;
}

void device_power(struct device_info *di)
{
   FILE *fp = NULL;
   char buf[128], *ptr = NULL;

   strncpy(di->power.ac, "(unavailable)", sizeof(di->power.ac) - 1);
   if(!(fp = fopen("/proc/acpi/ac_adapter/ADP1/state", "r"))) return;
   while(fgets(buf, sizeof(buf), fp)) {
      if(!strncasecmp(buf, "state:", strlen("state:"))) {
         ptr = buf + strlen("state:");
         while(*ptr && (*ptr == ' ')) ptr++;
         strncpy(di->power.ac, ptr, sizeof(di->power.ac) - 1);
      }
   }
   fclose(fp);

   if(di->power.ac[strlen(di->power.ac) - 1] == '\n') di->power.ac[strlen(di->power.ac) - 1] = '\0';

   return;
}

void device_os(struct device_info *di)
{
   FILE *fp = NULL;
   struct utsname u;
   char buf[128], *ptr = NULL;
   time_t t;
   struct tm ts;

   do {
      strncpy(di->os.ver, "(unknown)", sizeof(di->os.ver) - 1);

      if((fp = fopen("/etc/lsb-release", "r"))) {
        while(fgets(buf, sizeof(buf), fp)) {
           if(!strncasecmp(buf, "DISTRIB_DESCRIPTION=", strlen("DISTRIB_DESCRIPTION="))) {
              ptr = buf + strlen("DISTRIB_DESCRIPTION=");
              while(*ptr && ((*ptr == ' ') || (*ptr == '"'))) ptr++;
              strncpy(di->os.ver, ptr, sizeof(di->os.ver) - 1);
              while((di->os.ver[strlen(di->os.ver) - 1] == '"') || (di->os.ver[strlen(di->os.ver) - 1] == '\n')) di->os.ver[strlen(di->os.ver) - 1] = '\0';
           }
        }
      } else if((fp = fopen("/etc/slackware-version", "r"))) {
         if(fgets(di->os.ver, sizeof(di->os.ver), fp)) break;
      } else if((fp = fopen("/etc/redhat-release", "r"))) {
         if(fgets(di->os.ver, sizeof(di->os.ver), fp)) break;
      } else if((fp = fopen("/etc/gentoo-release", "r"))) {
         if(fgets(di->os.ver, sizeof(di->os.ver), fp)) break;
      }
   } while(0);
   if(fp) fclose(fp);

   if(di->os.ver[strlen(di->os.ver) - 1] == '\n') di->os.ver[strlen(di->os.ver) - 1] = '\0';

   if(!uname(&u)) strncpy(di->os.arch, u.machine, sizeof(di->os.arch) - 1);

   if(!(ptr = getenv("LANG"))) ptr = "(unknown)";
   strncpy(di->os.lang, ptr, sizeof(di->os.lang) - 1);

   t = time(NULL);
   localtime_r(&t, &ts);
   strftime(di->os.tzname, sizeof(di->os.tzname), "%Z", &ts);
   strftime(di->os.tzoff, sizeof(di->os.tzoff), "%z", &ts);
   di->os.tzoff[6] = '\0';
   di->os.tzoff[5] = di->os.tzoff[4];
   di->os.tzoff[4] = di->os.tzoff[3];
   di->os.tzoff[3] = ':';

   return;
}

void device_user(struct device_info *di)
{
   FILE *fp = NULL;
   char buf[128], *name, *gecos, *home, *ptr = NULL;
   unsigned int uid;

   di->user.uid = (unsigned int)getuid();
   di->user.gid = (unsigned int)getuid();

   if(!(fp = fopen("/etc/passwd", "r"))) return;
   while(fgets(buf, sizeof(buf), fp)) {
      if(!(ptr = strchr(buf, ':')) || !(ptr = strchr(++ptr, ':')) || (atoi(++ptr) != di->user.uid)) continue;
      if((gecos = strchr(ptr, ':')) && (gecos = strchr(++gecos, ':'))) *gecos++ = '\0';
      if((home = strchr(gecos, ':'))) *home++ = '\0';
      if((ptr = strchr(home, ':'))) *ptr = '\0';
      printf("%s %s\n", gecos, home);
   }
   fclose(fp);

   return;
}

void device_mount(struct device_info *di)
{
   FILE *fp = NULL;
   char buf[512], *device = NULL, *mountpoint = NULL, *type = NULL, *ptr = NULL;

   if(!(fp = fopen("/etc/mtab", "r"))) return;
   while(fgets(buf, sizeof(buf), fp)) {
      device = buf;
      if((mountpoint = strchr(device, ' '))) *mountpoint++ = '\0';
      if((type = strchr(mountpoint, ' '))) *type++ = '\0';
      if((ptr = strchr(type, ' '))) *ptr = '\0';
      ptr = &di->mount.list[strlen(di->mount.list)];
      snprintf(ptr, sizeof(di->mount.list) - strlen(di->mount.list), "%s %s (%s)\n", device, mountpoint, type);
   }
   fclose(fp);

   if(di->mount.list[strlen(di->mount.list) - 1] == '\n') di->mount.list[strlen(di->mount.list) - 1] = '\0';

   return;
}
