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

#if defined (HAVE_MPD_CLIENT_H)
#include <mpd/client.h>
#endif /* HAVE_MPD_CLIENT_H */

#if defined (HAVE_ALSA_ASOUNDLIB_H)
#include <alsa/asoundlib.h>
#endif /* HAVE_ALSA_ASOUNDLIB_H */

#if defined(HAVE_SYS_SOUNDCARD_H) || defined(HAVE_SOUNDCARD_H)
#include <fcntl.h>
#include <sys/ioctl.h>

#if defined(HAVE_SOUNDCARD_H)
#include <soundcard.h>
#else
#include <sys/soundcard.h>
#endif /* HAVE_SOUNDCARD_H */

#endif /* HAVE_SYS_SOUNDCARD_H || HAVE_SOUNDCARD_H */

#include "include/headers.h"
#include "prototypes/sound.h"

#if defined (HAVE_ALSA_ASOUNDLIB_H)
/* http://www.alsa-project.org/alsa-doc/alsa-lib/group___mixer.html 
 * http://www.alsa-project.org/alsa-doc/alsa-lib/group___simple_mixer.html 
 * for the return values */
void 
get_volume(char *str1) {
  snd_mixer_t *handle = NULL;
  snd_mixer_elem_t *elem = NULL;
  snd_mixer_selem_id_t *s_elem = NULL;
  long int vol = 0L, max = 0L, min = 0L;

  if (0 != (snd_mixer_open(&handle, 0))) {
    FUNC_FAILED("alsa");
  }

  if (0 != (snd_mixer_attach(handle, "default"))) {
    goto error;
  }

  if (0 != (snd_mixer_selem_register(handle, NULL, NULL))) {
    goto error;
  }

  if (0 != (snd_mixer_load(handle))) {
    goto error;
  }

  snd_mixer_selem_id_malloc(&s_elem);
  if (NULL == s_elem) {
    goto error;
  }

  snd_mixer_selem_id_set_name(s_elem, "Master");
  if (NULL == (elem = snd_mixer_find_selem(handle, s_elem))) {
    goto error;
  }

  if (0 != (snd_mixer_selem_get_playback_volume(elem, 0, &vol))) {
    goto error;
  }
  snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

  snd_mixer_selem_id_free(s_elem);
  snd_mixer_close(handle);

  FILL_ARR(str1, "%ld", 
    ((0 != max) ? ((vol * 100) / max) : 0L));
  return;

error:
  if (NULL != s_elem) {
    snd_mixer_selem_id_free(s_elem);
  }
  if (NULL != handle) {
    snd_mixer_close(handle);
  }
  FUNC_FAILED("alsa");
}
#endif /* HAVE_ALSA_ASOUNDLIB_H */


#if defined(HAVE_SYS_SOUNDCARD_H) || defined(HAVE_SOUNDCARD_H)
/* Used the following resource:
    sources.freebsd.org/RELENG_9/src/usr.sbin/mixer/mixer.c
*/
void 
get_volume(char *str1) {
  int devmask = 0, volume = 0, fd = 0;

  if (-1 == (fd = open("/dev/mixer", O_RDONLY))) {
    exit_with_err(CANNOT_OPEN, "/dev/mixer");
  }
  if (-1 == (ioctl(fd, SOUND_MIXER_READ_DEVMASK, &devmask))) {
    CLOSE_FD(fd);
    exit_with_err(ERR, "SOUND_MIXER_READ_DEVMASK");
  }
  if (-1 == (ioctl(fd, MIXER_READ(0), &volume))) {
    CLOSE_FD(fd);
    exit_with_err(ERR, "MIXER_READ()");
  }
  CLOSE_FD(fd);

  FILL_ARR(str1, "%d", ((volume >> 8) & 0x7f));
}
#endif /* HAVE_SYS_SOUNDCARD_H || HAVE_SOUNDCARD_H */


const char *
shorten_stream(const char *str1) {
  /* Dont submit pull request
   * if you dont know the difference
   * between the two examples below.
   *
   * const char *stream = "elo";
   * char *const stream = "elo"; */
  const char *stream = str1;

  if (5 < (strlen(stream))) {
    if (0 == (strncmp(stream, "http", 4))) {
      stream = "..";
    }
  }
  return stream;
}


#if defined (HAVE_MPD_CLIENT_H)
void
get_song(char *str1, int8_t num) {

  struct mpd_connection *conn = NULL;
  struct mpd_song *song = NULL;
  const char *stream = NULL, *taggy = NULL;
  static const int8_t tagz_arr[] = {
    0,
    MPD_TAG_TRACK,
    MPD_TAG_ARTIST,
    MPD_TAG_TITLE,
    MPD_TAG_ALBUM,
    MPD_TAG_DATE
  };

  *str1 = '\0';
  if (NULL == (conn = mpd_connection_new(NULL, 0, 0))) {
    return;
  }
  if (!(mpd_send_command(conn, "currentsong", NULL)) ||
      0 != (mpd_connection_get_error(conn))) {
    goto error;
  }
  if (NULL == (song = mpd_recv_song(conn))) {
    goto error;
  }

  if (6 != num) {
    taggy = mpd_song_get_tag(song, tagz_arr[num], 0);
    if (NULL != taggy) {
      FILL_STR_ARR(1, str1, taggy);
    }
  } else {
    if (NULL != (stream = mpd_song_get_uri(song))) {
      FILL_STR_ARR(1, str1, (shorten_stream(stream)));
    }
  }

error:
  if (NULL != song) {
    mpd_song_free(song);
  }
  if (NULL != conn) {
    mpd_connection_free(conn);
  }
  return;
}

#else

void
get_song(char *str1, int8_t num) {
  FILE *fp = NULL;
  static bool got_stream = false;
  char buf[100], temp[100], *ptr;
  const char *tagz[] = { "artist", "title", "album", "date" };
  const char *idx_tagz = ((6 != num) ? tagz[num-2] : "ohsnap");

  if (NULL == (fp = popen("cmus-remote -Q 2> /dev/null", "r"))) {
    return;
  }

  while (true) {
    if (NULL == (fgets(buf, 99, fp))) {
      break;
    }
    if (6 == num) {
      if ('f' == buf[0] && 'i' == buf[1] && 'l' == buf[2]) {
        CHECK_SSCANF(buf, "%*s %[^\n]", temp);
        if (NULL != (ptr = strrchr(temp, '/'))) {
          for (; *ptr; ptr++) {
            if ('/' != *ptr) {
              *str1++ = *ptr;
            }
          }
          *str1 = '\0';
        }
        break;
      }
    } else {
      if ('t' == buf[0] && 'a' == buf[1] && 'g' == buf[2]) {
        CHECK_SSCANF(buf, "%*s %s", temp);
        if (STREQ(idx_tagz, temp)) {
          CHECK_SSCANF(buf, "%*s %*s %[^\n]", str1);
          break;
        }
      }
      if ('s' == buf[0] && 't' == buf[1] && 'r' == buf[2]) {
        if (1 != (num-2) && false == got_stream) {
          CHECK_SSCANF(buf, "%*s %[^\n]", str1);
          got_stream = true;
          break;
        }
      }
    }
  }

  if (-1 == (pclose(fp))) {
    exit_with_err(CANNOT_CLOSE, "popen()");
  }
}
#endif /* HAVE_MPD_CLIENT_H */
