#!/usr/bin/env bash

# 10/23/2016 

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301, USA.

# Edit /dev/sda according to
# your operating system drive naming
# convention

# To use atactl replace "smartctl" with
# atactl sd0 readattr 
# replace $arr[9] with $arr[3]
while true; do
  sudo smartctl -a /dev/sd0a | \
    perl -Mstrict -Mwarnings -ne '
      my @arr = split(" ", $_);
      my $tempnum = 0;

      if ($arr[1] and lc $arr[1] =~ /temperature/i) {
        $tempnum = $arr[9] || 0;
        printf("%d\n",$tempnum);
      }' > /tmp/pinkytemp

  sleep 20

done &
