/*
   08/08/2016

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

#include <argp.h>
#include <limits.h>

#include "config.h" /* Auto-generated */
#include "include/headers.h"
#include "prototypes/cpu.h"
#include "prototypes/sound.h"
#include "prototypes/net.h"
#include "prototypes/options.h"
#include "prototypes/weather.h"
#include "prototypes/smart.h"
#include "prototypes/perl.h"
#include "prototypes/python.h"

/* Because we ran out of a-z A-Z options,
 * only long ones will be supported from now on.
 * The enumerated argp_option keys below will be used as 
 * case labels by the parse_opt switch */
enum {
  NICDRV = CHAR_MAX + 1,
  KERNODE,
  DRIVETEMP,
  PERLSCRIPT,
  PYTHONSCRIPT,
  BULLSHIFT
};
const char *argp_program_version = PACKAGE_STRING;
const char *argp_program_bug_address = "https://gitlab.com/void0/pinky-bar ";
static const char doc[] = "Statusbar program for anything (Window Manager, terminal multiplexer, etc..)";
static const struct argp_option options[] = {
  { .doc = "Available options:" },
  { .name = "mpd",          .key = 'M',                .doc = "The song filename."                                       },
  { .name = "mpdartist",    .key = 'x',                .doc = "The song artist(s) name(s)."                              },
  { .name = "mpdtitle",     .key = 'X',                .doc = "The song title."                                          },
  { .name = "mpdalbum",     .key = 'y',                .doc = "The song album name."                                     },
  { .name = "mpddate",      .key = 'Y',                .doc = "The song date."                                           },
  { .name = "cpu",          .key = 'c',                .doc = "The current cpu load (summed up all cores/threads)."      },
  { .name = "coresload",    .key = 'L',                .doc = "Show the load regarding each individual cpu core/thread." },
  { .name = "cpuspeed",     .key = 'C',                .doc = "The current cpu temperature."                             },
  { .name = "cpuinfo",      .key = 'I',                .doc = "Show your maximum cpu clock speed in MHz."                },
  { .name = "cputemp",      .key = 'T',                .doc = "Detect your CPU vendor, stepping, family."                },
  { .name = "ramperc",      .key = 'r',                .doc = "The used ram in percentage."                              },
  { .name = "ramtotal",     .key = 'J',                .doc = "The total ram."                                           },
  { .name = "ramfree",      .key = 'K',                .doc = "The free ram."                                            },
  { .name = "driveperc",    .key = 's',                .doc = "The used drive storage in percentage."                    },
  { .name = "drivetotal",   .key = 'n',                .doc = "The total drive storage."                                 },
  { .name = "drivefree",    .key = 'N',                .doc = "The free drive storage."                                  },
  { .name = "driveavail",   .key = 'O',                .doc = "The available drive storage."                             },
  { .name = "drivetemp",    .key = DRIVETEMP,          .doc = "Read the drive temperature from S.M.A.R.T"                },
  { .name = "dvdstr",       .key = 'z',                .doc = "The vendor and model name of your cdrom/dvdrom."          },
  { .name = "battery",      .key = 'g',                .doc = "The remaining battery charge."                            },
  { .name = "packages",     .key = 'p',                .doc = "The number of installed packages."                        },
  { .name = "kernsys",      .key = 'P',                .doc = "The kernel name."                                         },
  { .name = "kernode",      .key = KERNODE,            .doc = "The network node hostname."                               },
  { .name = "kernrel",      .key = 'Q',                .doc = "The kernel release."                                      },
  { .name = "kernver",      .key = 'R',                .doc = "The kernel version."                                      },
  { .name = "kernarch",     .key = 'u',                .doc = "The machine architecture."                                },
  { .name = "kernel",       .key = 'k',                .doc = "Combined kernel name and version."                        },
  { .name = "uptime",       .key = 'U',                .doc = "The system uptime"                                        },
  { .name = "loadavg",      .key = 'w',                .doc = "The system average load for past 1, 5 and 15 minutes"     },
  { .name = "voltage",      .key = 'v',                .doc = "The system voltage"                                       },
  { .name = "fans",         .key = 'f',                .doc = "All system fans and their speed in RPM."                  },
  { .name = "mobo",         .key = 'm',                .doc = "The motherboard vendor and model names."                  },
  { .name = "mobotemp",     .key = 'd',                .doc = "The motherboard temperature."                             },
  { .name = "volume",       .key = 'V',                .doc = "The volume."                                              },
  { .name = "time",         .key = 't',                .doc = "The current time."                                        },
  { .name = "ipaddr",       .key = 'a', .arg = "eth0", .doc = "The local IP address."                                    },
  { .name = "iface",        .key = 'i', .arg = "eth0", .doc = "The consumed internet bandwidth so far."                  },
  { .name = "bandwidth",    .key = 'b', .arg = "eth0", .doc = "The current download and upload speeds."                  },
  { .name = "ipmac",        .key = 'A', .arg = "eth0", .doc = "The NIC mac address."                                     },
  { .name = "ipmask",       .key = 'B', .arg = "eth0", .doc = "The NIC subnet mask address."                             },
  { .name = "ipcast",       .key = 'D', .arg = "eth0", .doc = "The NIC broadcast address."                               },
  { .name = "iplookup",     .key = 'E', .arg = "site", .doc = "Mini website IP lookup."                                  },
  { .name = "statio",       .key = 'S', .arg = "sda",  .doc = "Read and written MBs to the drive so far."                },

#if WITH_PERL == 1
  { .name = "perl",         .key = PERLSCRIPT,         .doc = "Extend the program with perl, read README."               },
#endif /* WITH_PERL */

#if WITH_PYTHON == 1
  { .name = "python",       .key = PYTHONSCRIPT,       .doc = "Extend the program with python, read README."             },
#endif /* WITH_PYTHON */

#if WITH_WEATHER == 1
  { .name = "weather",      .key = 'q',                .doc = "The temperature outside."                                 },
#endif /* WITH_WEATHER */

#if defined(HAVE_MPD_CLIENT_H)
  { .name = "mpdtrack",     .key = 'W',                .doc = "The song track name."                                     },
#endif /* HAVE_MPD_CLIENT_H */

#if !defined(__OpenBSD__)
  { .name = "ramshared",    .key = 'l',                .doc = "The shared ram."                                          },
  { .name = "rambuffer",    .key = 'o',                .doc = "The buffered ram."                                        },
#endif /* !__OpenBSD__ */


#if defined(__OpenBSD__)
  { .name = "ramused",      .key = 'l',                .doc = "The used ram in MB."                                      },
#endif /* __OpenBSD__ */

#if defined(__linux__)
  { .name = "nicfw",        .key = 'j', .arg = "eth0", .doc = "The NIC firmware."                                        },
  { .name = "drivemodel",   .key = 'F', .arg = "sda",  .doc = "The vendor name of your drive."                           },
  { .name = "nicdrv",    .key = NICDRV, .arg = "eth0", .doc = "The NIC driver."                                          },
  { .name = "nicver",       .key = 'H', .arg = "eth0", .doc = "The NIC version."                                         },
  { .name = "iplink",       .key = 'e', .arg = "eth0", .doc = "The NIC link speed (useful for wireless/wifi)."           },
  { .name = "nicinfo",      .key = 'G', .arg = "eth0", .doc = "The NIC vendor and model."                                },
  { .name = "wifiname",     .key = 'h', .arg = "eth0", .doc = "The name of currently connected wireless/wifi network."   },
#endif /* __linux__ */

#if defined(__FreeBSD__) || defined(__OpenBSD__)
  { .name = "swapused",     .key = 'Z',                .doc = "The used drive swap in MB."                               },
  { .name = "swaperc",      .key = 'F',                .doc = "The used drive swap in percentage."                       },
  { .name = "swaptotal",    .key = 'h',                .doc = "The total drive swap."                                    },
  { .name = "swapavail",    .key = 'H',                .doc = "The available drive swap."                                },
  { .name = "nicgw",        .key = 'j', .arg = "re0",  .doc = "The NIC gateway address."                                 },
#endif /* __FreeBSD__ || __OpenBSD__ */

  { .doc = NULL }
};
struct arguments {
  char *all;
};

static error_t
parse_opt(int key, char *arg, struct argp_state *state) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
  struct timespec tc = {0};
#pragma GCC diagnostic pop

  struct arguments *arguments = state->input;
  switch(key) {

    NEW_MPD_LABEL('x', char song_artist[VLA], song_artist, 2, FMT_SONG);

    NEW_MPD_LABEL('X', char song_title[VLA], song_title, 3, FMT_SONG);

    NEW_MPD_LABEL('y', char song_album[VLA], song_album, 4, FMT_SONG);

    NEW_MPD_LABEL('Y', char song_date[VLA], song_date, 5, FMT_SONG);

    NEW_MPD_LABEL('M', char song[VLA], song, 6, FMT_SONG);

    NEW_CPU_LABEL('c', char cpu[VLA], cpu, FMT_CPU, CPU_STR);

    NEW_CPU_LABEL('L', char cores_load[VLA], cores_load, FMT_CORES, CPU_STR);

    NEW_LABEL('T', char cpu_temp[VLA], cpu_temp, FMT_TEMP);

    NEW_RAM_LABEL('J', char ram_total[VLA], ram_total, 1, FMT_RAM2, RAM_STR);

    NEW_RAM_LABEL('K', char ram_free[VLA], ram_free, 2, FMT_RAM2, RAM_STR);

    NEW_RAM_LABEL('r', char ram_perc[VLA], ram_perc, 5, FMT_RAM, RAM_STR);

    NEW_SSD_LABEL('n', char ssd_total[VLA], ssd_total, 1, FMT_SSD2, SSD_STR);

    NEW_SSD_LABEL('N', char ssd_free[VLA], ssd_free, 2, FMT_SSD2, SSD_STR);

    NEW_SSD_LABEL('O', char ssd_avail[VLA], ssd_avail, 3, FMT_SSD2, SSD_STR);

    NEW_SSD_LABEL('s', char ssd_perc[VLA], ssd_perc, 4, FMT_SSD, SSD_STR);

    NEW_LABEL('p', char packs[VLA], packs, FMT_PKGS, PKG_STR);

    NEW_KERNEL_LABEL('P', char kernel_sys[VLA], kernel_sys, 1, FMT_KERN);

    NEW_KERNEL_LABEL(KERNODE, char kernel_node[VLA], kernel_node, 2, FMT_KERN);

    NEW_KERNEL_LABEL('Q', char kernel_rel[VLA], kernel_rel, 3, FMT_KERN);

    NEW_KERNEL_LABEL('R', char kernel_ver[VLA], kernel_ver, 4, FMT_KERN);

    NEW_KERNEL_LABEL('u', char kernel_arch[VLA], kernel_arch, 5, FMT_KERN);

    NEW_KERNEL_LABEL('k', char kernel[VLA], kernel, 6, FMT_KERN);

    NEW_LABEL('U', char uptime[VLA], uptime, FMT_UP, UP_STR);

    NEW_LABEL('w', char loadavg[VLA], loadavg, FMT_LOAD, LOAD_STR);

    NEW_LABEL('v', char voltage[VLA], voltage, FMT_VOLT, VOLT_STR);

    NEW_LABEL('m', char mobo[VLA], mobo, FMT_MOBO, MOBO_STR);

    NEW_LABEL('d', char mobo_temp[VLA], mobo_temp, FMT_TEMP);

    NEW_LABEL('f', char fans[VLA], fans, FMT_FANS, FANS_STR);

    NEW_LABEL('t', char taim[VLA], taim, FMT_TIME" ", TIME_STR);

    NEW_NET_LABEL('b', char net[VLA], net, 1, FMT_NET, NET_STR);

    NEW_NET_LABEL('i', char net_speed[VLA], net_speed, 2, FMT_NET, SPEED_STR);

    NEW_NET_LABEL('a', char net_addr[VLA], net_addr, 3, FMT_KERN);

    NEW_NET_LABEL('A', char mac[VLA], mac, 4, FMT_KERN);

    NEW_NET_LABEL('B', char mask[VLA], mask, 5, FMT_KERN);

    NEW_NET_LABEL('D', char net_cast[VLA], net_cast, 6, FMT_KERN);

    NEW_ARG_LABEL('E', char ip_lookup[VLA], ip_lookup, FMT_KERN);

    NEW_RAM_LABEL('l', char ram_shared[VLA], ram_shared, 3, FMT_RAM2, RAM_STR);

    NEW_LABEL('g', char battery[VLA], battery, FMT_BATT, BATT_STR);

    NEW_ARG_LABEL('S', char statio[VLA], statio, FMT_STATIO, STATIO_STR);

    NEW_LABEL(DRIVETEMP, char drivetemp[VLA], drivetemp, FMT_TEMP);


#if WITH_PERL == 1
    NEW_LABEL(PERLSCRIPT, char perl[VLA], perl, FMT_KERN);
#endif /* WITH_PERL */


#if WITH_PYTHON == 1
    NEW_LABEL(PYTHONSCRIPT, char python[VLA], python, FMT_KERN);
#endif /* WITH_PYTHON */


#if WITH_WEATHER == 1
    NEW_LABEL('q', char weather[VLA], weather, OUT_STR YELLOW STR_SPEC" ");
#endif /* WITH_WEATHER */


#if defined(HAVE_MPD_CLIENT_H)
    NEW_MPD_LABEL('W', char song_track[VLA], song_track, 1, FMT_SONG);
#endif /* HAVE_MPD_CLIENT_H */


#if defined(HAVE_CDIO_CDIO_H) || defined(__linux__)
    NEW_LABEL('z', char dvd[VLA], dvd, FMT_KERN);
#endif /* HAVE_CDIO_CDIO_H || __linux__ */


#if !defined(__OpenBSD__)
    NEW_RAM_LABEL('o', char ram_buffer[VLA], ram_buffer, 4, FMT_RAM2, RAM_STR);
#endif /* !__OpenBSD__ */


#if defined(__FreeBSD__) || defined(__OpenBSD__)
    NEW_SWAPP_LABEL('h', char swapp_total[VLA], swapp_total, 1, FMT_SSD2, SSD_STR);

    NEW_SWAPP_LABEL('H', char swapp_avail[VLA], swapp_avail, 2, FMT_SSD2, SSD_STR);

    NEW_SWAPP_LABEL('Z', char swapp_used[VLA], swapp_used, 3, FMT_SSD2, SSD_STR);

    NEW_SWAPP_LABEL('F', char swapp_perc[VLA], swapp_perc, 4, FMT_SSD2, SSD_STR);

    NEW_NET_LABEL('j', char nic_info[VLA], nic_info, 7, FMT_KERN);
#endif /* __FreeBSD__ || __OpenBSD__ */


#if defined(__linux__)
    NEW_NET_LABEL('j', char nic_info[VLA], nic_info, 10, FMT_KERN);

    NEW_NET_LABEL('h', char wifiname[VLA], wifiname, 11, FMT_KERN);

    NEW_ARG_LABEL('F', char ssd_model[VLA], ssd_model, FMT_KERN);

    NEW_NET_LABEL(NICDRV, char nic_drv[VLA], nic_drv, 8, FMT_KERN);

    NEW_NET_LABEL('H', char nic_ver[VLA], nic_ver, 9, FMT_KERN);

    NEW_ARG_LABEL('G', char nic_info[VLA], nic_info, FMT_KERN);

    NEW_NET_LABEL('e', char link_speed[VLA], link_speed, 7, FMT_KERN);
#endif /* __linux__ */


    case 'V':
#if defined(HAVE_ALSA_ASOUNDLIB_H) || defined(HAVE_SYS_SOUNDCARD_H) || \
    defined(HAVE_SOUNDCARD_H)
      {
        char volume[VLA];
        GET_N_FMT(volume, arguments->all, FMT_VOL, VOL_STR, volume);
      }
      break;
#else
      FPRINTF("%s\n", "recompile the program --with-alsa or --with-oss");
      return ARGP_KEY_ERROR;
#endif /* HAVE_ALSA_ASOUNDLIB_H || HAVE_SYS_SOUNDCARD_H || HAVE_SOUNDCARD_H */

    case 'C':
#if defined(__i386__) || defined(__i686__) || defined(__x86_64__)
      {
        char cpu_clock_speed[VLA];
        GET_N_FMT(cpu_clock_speed, arguments->all, FMT_CPUSPEED, cpu_clock_speed);
      }
      break;
#else
      FPRINTF("%s\n", "This option is not supported "
                "by your CPU architecture");
      return ARGP_KEY_ERROR;
#endif /* __i386__ || __i686__ || __x86_64__ */

    case 'I':
#if defined(__i386__) || defined(__i686__) || defined(__x86_64__)
      {
        char cpu_info[VLA];
        GET_N_FMT(cpu_info, arguments->all, FMT_CPUSPEED, cpu_info);
      }
      break;
#else
      FPRINTF("%s\n", "This option is not supported "
                "by your CPU architecture");
      return ARGP_KEY_ERROR;
#endif /* __i386__ || __i686__ || __x86_64__ */

    default:
      return ARGP_ERR_UNKNOWN;
  }
  return EXIT_SUCCESS;
}


void
parse_opts(int argc, char *argv[], char *combined) {
  struct arguments arguments;
  arguments.all = combined;

  static const struct argp arg_parser = {
    .parser = parse_opt,
    .options = options,
    .doc = doc
  };
  argp_parse(&arg_parser, argc, argv, ARGP_IN_ORDER, NULL, &arguments);
}
