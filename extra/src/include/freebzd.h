/*
   08/17/2016

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

#ifndef INCLUDE_FREEBZD_HEADERZ_H_
#define INCLUDE_FREEBZD_HEADERZ_H_

#include <sys/types.h>
#include <sys/sysctl.h>

#define SYSCTLVAL(x, y) \
  if (0 != sysctlbyname(x, y, &len, NULL, 0)) { \
    FUNC_FAILED("sysctlbyname()"); \
  }

#endif /* INCLUDE_FREEBZD_HEADERZ_H_ */
