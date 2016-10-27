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
# The entire language is in your hands.
use strict;
use warnings;

use Time::Piece;

sub uzer_func {
  my $t = localtime;
  my $rett = sprintf("%s %s %d", "Hello", $t->cdate, 123);

  return $rett;
}
