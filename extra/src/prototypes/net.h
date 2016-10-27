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

#ifndef NET_H_
#define NET_H_

#if defined(__linux__)
#define NETFAM AF_PACKET
#define RECVBYTS rx_bytes
#define SENTBYTS tx_bytes
#else

#define NETFAM AF_LINK
#define RECVBYTS ifi_ibytes
#define SENTBYTS ifi_obytes
#endif /* __linux__ */

void get_net(char *, char *, uint8_t num);
void get_nic_info2(char *, char *, uint8_t num);
void get_ip_lookup(char *, char *);
void get_nic_info(char *, char *);

#if defined(__linux__)
void get_wifi(char *, char *, uint8_t num);
#endif /* __linux__ */

#endif /* NET_H_ */
