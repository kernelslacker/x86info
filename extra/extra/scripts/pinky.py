# 10/24/2016 

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


# Your script should always return a single string.
# Read the above line again.
#
# pinky-bar will always call uzer_func,
# you can write other subroutines/functions
# but they should be called inside uzer_func
#
# Dont worry about the colorization, it's
# handled by pinky-bar.
#
# Semicolins are only mandatory when you write "one liners"
# python2 -c 'import time;print("{0}".format(time.tzname))'
#
# Dont blame me if you have the following environment variable
# export PYTHONOPTIMIZE=3
# The only way to describe it is caching nightmare.
#
# The entire language is in your hands.
import time

def uzer_func():
  rett = '{0} {1} {2}'.format("Hello", time.ctime(), 123)
  return rett
