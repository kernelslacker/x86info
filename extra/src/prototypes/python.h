/*
   10/24/2016

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

#ifndef PYTHON_H_
#define PYTHON_H_

#if WITH_PYTHON == 1

#if WITH_PYTHON2 == 1
#define PYFON_ZTR PyString_AsString
#else
#define PYFON_ZTR PyUnicode_AsUTF8
#endif /* WITH_PYTHON2 */

#define RET_PY_STR(x) (NULL != PYFON_ZTR(x) ? PYFON_ZTR(x) : "0")

void get_python(char *);
#endif /* WITH_PYTHON */

#endif /* PYTHON_H_ */
