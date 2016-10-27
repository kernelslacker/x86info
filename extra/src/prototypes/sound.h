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

#ifndef SOUND_H_
#define SOUND_H_

#if defined (HAVE_ALSA_ASOUNDLIB_H) || defined(HAVE_SYS_SOUNDCARD_H) || \
  defined(HAVE_SOUNDCARD_H)
void get_volume(char *);
#endif

const char *shorten_stream(const char *);
void get_song(char *, int8_t num);

#endif /* SOUND_H_ */
