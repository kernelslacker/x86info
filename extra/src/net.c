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

#include "config.h" /* Auto-generated */

#if WITH_NET == 1

#if defined(__linux__)
#include <ctype.h>
#include <netdb.h>
/* #include <sys/types.h> */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>

#if WITH_LIBNL == 1
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>
#include <linux/if.h>

#else
#include <linux/wireless.h>
#endif /* WITH_LIBNL */

#if WITH_PCI == 1
#include <pci/pci.h>
#endif /* WITH_PCI */

#endif /* __linux__ */

#if defined(__FreeBSD__) || defined(__OpenBSD__)
#include <netdb.h>
/* #include <sys/types.h> */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <ifaddrs.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <netinet/in.h>
#endif /* __FreeBSD__ || __OpenBSD__ */

#endif /* WITH_NET */

#include "include/headers.h"
#include "prototypes/net.h"

#if defined(__FreeBSD__)
#include "include/freebzd.h"
#endif /* __FreeBSD__ */

#if defined(__OpenBSD__)
#include "include/openbzd.h"
#endif /* __OpenBSD__ */


#if defined(__linux__)

#if WITH_LIBNL == 1 && WITH_NET == 1
static int call_back(struct nl_msg *, void *);
#endif /* WITH_LIBNL && WITH_NET */

#endif /* __linux__ */


/* Thanks to http://www.matisse.net/bitcalc/?source=print
 * and `man netdevice' */
void
get_net(char *str1, char *str2, uint8_t num) {
#if WITH_NET == 1

#if defined(__linux__)
  struct rtnl_link_stats *stats = NULL;
  struct sockaddr_ll *mac = NULL;
#else
  struct if_data *stats = NULL;
  struct sockaddr_dl *mac = NULL;
#endif /* __linux__ */

  struct ifaddrs *ifaddr = NULL, *ifa = NULL;
  static uintmax_t prev_recv = 0, prev_sent = 0;
  uintmax_t cur_recv = 0, cur_sent = 0;
  unsigned char *umac = NULL;
  void *temp_void = NULL;
  char temp[VLA];

  if (-1 == (getifaddrs(&ifaddr))) {
    FUNC_FAILED("getifaddrs()");
  }
  FILL_STR_ARR(1, str1, "Null");

  for (ifa = ifaddr; NULL != ifa; ifa = ifa->ifa_next) {
    if ((IFF_UP | IFF_BROADCAST) != (ifa->ifa_flags & (
      IFF_UP | IFF_BROADCAST | IFF_POINTOPOINT |
       IFF_LOOPBACK | IFF_NOARP))) {
      continue;
    }
    if (NULL == ifa->ifa_addr) {
      continue;
    }
    if (3 == num || 5 == num || 6 == num) { /* ip | netmask | broadcast */
      if (AF_INET == ifa->ifa_addr->sa_family) {
        if (STREQ(str2, ifa->ifa_name)) {
          switch(num) {
            case 3:
              temp_void = ifa->ifa_addr;
              break;
            case 5:
              temp_void = ifa->ifa_netmask;
              break;
            case 6:
              temp_void = ifa->ifa_broadaddr;
              break;
          }
          inet_ntop(AF_INET, &(((struct sockaddr_in *)temp_void)->sin_addr),
              temp, INET_ADDRSTRLEN);
          FILL_STR_ARR(1, str1, temp);
          break;
        }
      }
    } else if (NETFAM == ifa->ifa_addr->sa_family &&
          NULL != ifa->ifa_data) {
        if (STREQ(str2, ifa->ifa_name)) {

#if defined(__linux__)
          stats = ifa->ifa_data;
#else
          stats = (struct if_data *)ifa->ifa_data;
#endif /* __linux__ */

          if (2 == num) { /* upload and download speeds */
            cur_recv = (uintmax_t)stats->RECVBYTS - prev_recv;
            cur_sent = (uintmax_t)stats->SENTBYTS - prev_sent;

            FILL_ARR(str1, "Down " FMT_UINT " KB, Up " FMT_UINT " KB",
              (cur_recv / KB), (cur_sent / KB));

            prev_recv = cur_recv;
            prev_sent = cur_sent;
          } else if (1 == num) { /* consumed internet so far */

            FILL_ARR(str1, "Down " FMT_UINT " MB, Up " FMT_UINT " MB",
              ((uintmax_t)stats->RECVBYTS / MB),
              ((uintmax_t)stats->SENTBYTS / MB));

#if defined(__linux__)
          } else if (4 == num) { /* mac address */

            temp_void = ifa->ifa_addr;
            mac = (struct sockaddr_ll *)temp_void;
            /*
              6 == ether
              20 == infiniband
            */
            if (6 != mac->sll_halen) {
              break;
            }
            FILL_ARR(str1, "%02x:%02x:%02x:%02x:%02x:%02x",
                mac->sll_addr[0], mac->sll_addr[1],
                mac->sll_addr[2], mac->sll_addr[3],
                mac->sll_addr[4], mac->sll_addr[5]);

          } else if (11 == num) { /* wifi name */
            get_wifi(str1, str2, (uint8_t)(num - 10));

          } else { /* link speed | driver | version | firmware */
            switch(num) {
              case 7:
              case 8:
              case 9:
              case 10:
                get_nic_info2(str1, str2, (uint8_t)(num - 6));
                break;
              default:
                break;
            }
          }
#else
          } else if (4 == num) { /* mac address */
            temp_void = ifa->ifa_addr;
            mac = (struct sockaddr_dl *)temp_void;
            /*
              6 == ether
              20 == infiniband
            */
            if (6 != mac->sdl_alen) {
              break;
            }
            umac = (unsigned char *)LLADDR(mac);
            FILL_ARR(str1, "%02x:%02x:%02x:%02x:%02x:%02x",
                *umac, *(umac + 1), *(umac + 2),
                *(umac + 3), *(umac + 4), *(umac + 5));
          } else if (7 == num) { /* gateway */
            get_nic_info(str1, str2);
          }
#endif /* __linux__ */
          break;
        }
    }
  }
  if (NULL != ifaddr) {
    freeifaddrs(ifaddr);
  }

#else
  (void)str1;
  (void)str2;
  (void)num;
  RECOMPILE_WITH("net");
#endif /* WITH_NET */
}


void
get_ip_lookup(char *str1, char *str2) {
#if WITH_NET == 1

  struct addrinfo *rp = NULL, *result = NULL, hints;
  void *temp_void = NULL;
  char temp[VLA];

  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0; /* udp | tcp */

  if (0 != (getaddrinfo(str2, NULL, &hints, &result))) {
    FUNC_FAILED("getaddrinfo()");
  }

  for (rp = result; NULL != rp; rp = rp->ai_next) {
    if (NULL == rp->ai_addr) {
      continue;
    }
    /* check ipv4 again, despite the "hints" */
    if (AF_INET == rp->ai_family) {
      temp_void = rp->ai_addr;

      inet_ntop(AF_INET, &(((struct sockaddr_in *)temp_void)->sin_addr),
        temp, INET_ADDRSTRLEN);
      FILL_STR_ARR(1, str1, temp);

      break;
    }
  }
  if (NULL != result) {
    freeaddrinfo(result);
  }

#else
  (void)str1;
  (void)str2;
  RECOMPILE_WITH("net");
#endif /* WITH_NET */
}


#if defined(__linux__)
void
get_nic_info2(char *str1, char *str2, uint8_t num) {
#if WITH_NET == 1

  struct ethtool_cmd ecmd;
  struct ethtool_drvinfo drvinfo;
  struct ifreq ifr;
  int sock = 0;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (-1 == sock) {
    return;
  }

  memset(&ecmd, 0, sizeof(struct ethtool_cmd));
  memset(&drvinfo, 0, sizeof(struct ethtool_drvinfo));
  memset(&ifr, 0, sizeof(struct ifreq));

  switch(num) {
    case 1:
      ecmd.cmd = ETHTOOL_GSET;
      ifr.ifr_data = (char *)&ecmd;
      break;
    case 2:
    case 3:
    case 4:
      drvinfo.cmd = ETHTOOL_GDRVINFO;
      ifr.ifr_data = (char *)&drvinfo;
      break;
  }
  snprintf(ifr.ifr_name, IF_NAMESIZE, "%s", str2);

  if (0 != (ioctl(sock, SIOCETHTOOL, &ifr))) {
    CLOSE_FD(sock);
    return;
  }

  switch(num) {
    case 1:
      FILL_ARR(str1, "%d%s", ecmd.speed, "Mbps");
      break;
    case 2:
      FILL_STR_ARR(1, str1, drvinfo.driver);
      break;
    case 3:
      FILL_STR_ARR(1, str1, drvinfo.version);
      break;
    case 4:
      FILL_STR_ARR(1, str1, drvinfo.fw_version);
      break;
  }
  CLOSE_FD(sock);

#else
  (void)str1;
  (void)str2;
  (void)num;
  RECOMPILE_WITH("net");
#endif /* WITH_NET */
}


/*
 Quick spot the bug game.
 code begin:
  struct pci_access *pacc= NULL;
  pacc = pci_alloc();
  pci_init(pacc);
  pci_cleanup(pacc);
 code end;
  Spotted the bug - no ? Well,
  GCC -O2 hangs on pci_init,
  while -O0 executes flawlessly.
  Disclaimer: the code is perfectly valid.
*/
void
get_nic_info(char *str1, char *str2) {
#if WITH_PCI == 1 && WITH_NET == 1

  uintmax_t vendor = 0, model = 0;
  char temp[VLA];
  struct pci_access *pacc = NULL;
  struct pci_dev *dev = NULL;
  FILE *fp = NULL;

  FILL_STR_ARR(1, str1, "Null");
  NIC_VEND(temp, str2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  OPEN_X(fp, temp, SCAN_UINTX, &vendor); /* hex */
#pragma GCC diagnostic pop

  NIC_MODEL(temp, str2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  OPEN_X(fp, temp, SCAN_UINTX, &model); /* hex */
#pragma GCC diagnostic pop

  pacc = pci_alloc();
  if (NULL == pacc) {
    goto error;
  }

  pci_init(pacc);
  if (NULL == pacc) {
    goto error;
  }
  pci_scan_bus(pacc);

  for (dev = pacc->devices; NULL != dev; dev = dev->next) {
    pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);

    if ((uintmax_t)vendor == (uintmax_t)dev->vendor_id &&
        (uintmax_t)model == (uintmax_t)dev->device_id) {

      pci_lookup_name(pacc, temp, VLA,
        PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE,
        (uintmax_t)vendor, (uintmax_t)model);

      FILL_STR_ARR(1, str1, temp);
      break;
    }
  }

error:
  if (NULL != pacc) {
    pci_cleanup(pacc);
  }
  return;

#else
  (void)str1;
  (void)str2;
  RECOMPILE_WITH("pci");
#endif /* WITH_PCI && WITH_NET */
}

#endif /* __linux__ */



#if defined(__FreeBSD__) || defined(__OpenBSD__)
#define ROUNDUP(x) ((x) > 0 ? \
  (1 + (((x) - 1) | (sizeof(long) - 1))) : sizeof(long))

/* Based on:
 http://fossies.org/linux/misc/old/mrt-2.2.2a-src.tar.gz/mrt-2.2.2a/src/lib/kernel/bsd.c
 https://svn.nmap.org/nmap/libdnet-stripped/src/route-bsd.c
*/
void
get_nic_info(char *str1, char *str2) {
#if WITH_NET == 1

  struct rt_msghdr *rtm = NULL;
  struct sockaddr *sa = NULL, *addrs[RTAX_MAX];
  char *buf = NULL, *next = NULL, *lim = NULL, temp[VLA];
  uint8_t x = 0;
  size_t needed = 0;
  void *temp_void = NULL;

  /* No, it's not Men In Black acronym */
  int mib[] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_DUMP, 0 };
  if (0 != (sysctl(mib, 6, NULL, &needed, NULL, 0))) {
    return;
  }

  if (0 == needed) {
    return;
  }

  buf = (char *)malloc(needed);
  if (NULL == buf) {
    return;
  }
  if (0 != (sysctl(mib, 6, buf, &needed, NULL, 0))) {
    goto error;
  }

  lim = buf + needed;
  for (next = buf; next < lim; next += rtm->rtm_msglen) {
    rtm = (struct rt_msghdr *)(void *)next;
    sa = (struct sockaddr *)(rtm + 1);
    if (NULL == sa || NULL == rtm) {
      continue;
    }
    if (AF_INET == sa->sa_family) {
      for (x = 0; x < RTAX_MAX; x++) {
        if (rtm->rtm_addrs & (1 << x)) {
          addrs[x] = sa;
          sa = (struct sockaddr *)((char *)sa + ROUNDUP(sa->sa_len));
        } else {
          addrs[x] = NULL;
        }
      }
      if (((rtm->rtm_addrs & (RTA_DST|RTA_GATEWAY)) == (RTA_DST|RTA_GATEWAY))
           && AF_INET == addrs[RTAX_DST]->sa_family
           && AF_INET == addrs[RTAX_GATEWAY]->sa_family) {

        temp_void = addrs[RTAX_GATEWAY];
        inet_ntop(AF_INET, &(((struct sockaddr_in *)temp_void)->sin_addr),
          temp, INET_ADDRSTRLEN);
        FILL_STR_ARR(1, str1, temp);
        break;
      }
    }
  }
  (void)str2;

error:
  if (NULL != buf) {
    free(buf);
  }
  return;

#else
  (void)str1;
  (void)str2;
  RECOMPILE_WITH("net");
#endif /* WITH_NET */
}
#endif /* __FreeBSD__ || __OpenBSD__ */


#if defined(__linux__)
/*

  Entirely based on iw.c, link.c, genl.c, scan.c (print_bss_handler)
  (v3.17)

  Docs, return vals, and tips:
   https://www.infradead.org/~tgr/libnl/doc/core.html
   https://www.infradead.org/~tgr/libnl/doc/api/group__send__recv.html
   http://lists.shmoo.com/pipermail/hostap/2011-October/024315.html
   https://bugzilla.kernel.org/show_bug.cgi?id=78481
*/
#if WITH_LIBNL == 1

#if WITH_NET == 1
static int call_back(struct nl_msg *msg, void *str1) {
  struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
  struct nlattr *tb[NL80211_ATTR_MAX + 1];
  struct nlattr *bss[NL80211_BSS_MAX + 1];
  struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
    [NL80211_BSS_BSSID] = {.type = NLA_UNSPEC},
    [NL80211_BSS_INFORMATION_ELEMENTS] = {.type = NLA_UNSPEC}
  };
  uint32_t len = 0, x = 0, z = 0;
  char elo = '\0', *ssid = NULL, *ptr = (char *)str1;

  if (0 != (nla_parse(tb,
     NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
     genlmsg_attrlen(gnlh, 0), NULL))) {
    return NL_SKIP;
  }

  if (NULL == tb[NL80211_ATTR_BSS]) {
    return NL_SKIP;
  }
  if (0 != (nla_parse_nested(bss,
     NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], bss_policy))) {
    return NL_SKIP;
  }
  if (NULL == bss[NL80211_BSS_STATUS]) {
    return NL_SKIP;
  }

  switch(nla_get_u32(bss[NL80211_BSS_STATUS])) {
    case NL80211_BSS_STATUS_ASSOCIATED:
    case NL80211_BSS_STATUS_AUTHENTICATED:
    case NL80211_BSS_STATUS_IBSS_JOINED:
      break;
    default:
      return NL_SKIP;
  }

  if (NULL == bss[NL80211_BSS_BSSID]) {
    return NL_SKIP;
  }

  if (bss[NL80211_BSS_INFORMATION_ELEMENTS]) {
    ssid = (char *)(nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]));
    len = (uint32_t)nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]);

    if (NULL != ssid && 0 != len) {
      ssid += 2;
      for (x = 0; x < len; x++) {
        elo = ssid[x];
        if (0 == (isprint((unsigned char)elo))) {
          break;
        }
        if (VLA > x) {
          *ptr++ = elo;
        }
      }
      *ptr = '\0';
    }
  }
  return NL_SKIP;
}
#endif /* WITH_NET */

void
get_wifi(char *str1, char *str2, uint8_t num) {
#if WITH_NET == 1

  struct nl_sock *sock = NULL;
  struct nl_msg *msg = NULL;
  int fam = 0;
  uint32_t dev = 0;
  void *scan_ret = NULL;

  if (NULL == (sock = nl_socket_alloc())) {
    return;
  }
  if (0 != (genl_connect(sock))) {
    goto error;
  }

  if (0 != (nl_socket_modify_cb(sock,
     NL_CB_VALID, NL_CB_CUSTOM, call_back, str1))) {
    goto error;
  }
  if (0 > (fam = genl_ctrl_resolve(sock, "nl80211"))) {
    goto error;
  }

  dev = if_nametoindex(str2);
  msg = nlmsg_alloc();
  if (0 == dev || NULL == msg) {
    goto error_msg;
  }

  scan_ret = genlmsg_put(msg, 0, 0, fam, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);
  if (NULL == scan_ret ||
     0 != (nla_put_u32(msg, NL80211_ATTR_IFINDEX, dev))) {
    goto error_msg;
  }

  if (0 != (nl_send_sync(sock, msg))) {
    goto error;
  }
  (void)num;

error:
  if (NULL != sock) {
    nl_socket_free(sock);
  }
  return;

error_msg:
  if (NULL != msg) {
    nlmsg_free(msg);
  }
  if (NULL != sock) {
    nl_socket_free(sock);
  }
  return;

#else
  (void)str1;
  (void)str2;
  (void)num;
  RECOMPILE_WITH("net");
#endif /* WITH_NET */
}

#else
void
get_wifi(char *str1, char *str2, uint8_t num) {
#if WITH_NET == 1

  struct iwreq iwr;
  int sock = 0;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (-1 == sock) {
    return;
  }

  memset(&iwr, 0, sizeof(struct iwreq));

  iwr.u.essid.pointer = str2;
  iwr.u.essid.length  = IW_ESSID_MAX_SIZE + 1;
  iwr.u.essid.flags   = 0;

  snprintf(iwr.ifr_name, IF_NAMESIZE, "%s", str2);

  if (0 != (ioctl(sock, SIOCGIWESSID, &iwr))) {
    CLOSE_FD(sock);
    return;
  }

  switch(num) {
    case 1:
      FILL_STR_ARR(1, str1, str2);
      break;
  }
  CLOSE_FD(sock);

#else
  (void)str1;
  (void)str2;
  (void)num;
  RECOMPILE_WITH("net");
#endif /* WITH_NET */
}
#endif /* WITH_LIBNL */
#endif /* __linux__ */
