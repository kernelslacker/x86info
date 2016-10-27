/*
   10/07/2016

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

#if WITH_WEATHER == 1

#include <ctype.h>
#include <curl/curl.h>
#include "include/headers.h"

#endif /* WITH_WEATHER */

#include "prototypes/weather.h"

#if WITH_WEATHER == 1
static size_t read_curl_data_cb(char *, size_t size, size_t nmemb, char *);

/*
  The 4th arg:
   https://curl.haxx.se/libcurl/c/CURLOPT_WRITEDATA.html

  The JSON data that we are parsing, that's how it's returned:

{"coord":{"lon":-0.13,"lat":51.51},"weather":[{"id":803,"main":"Clouds","description":"broken clouds","icon":"04d"}],"base":"stations","main":{"temp":12.05,"pressure":1030.73,"humidity":70,"temp_min":12.05,"temp_max":12.05,"sea_level":1038.34,"grnd_level":1030.73},"wind":{"speed":3.82,"deg":8.50131},"clouds":{"all":64},"dt":1476114783,"sys":{"message":0.011,"country":"GB","sunrise":1476080264,"sunset":1476119749},"id":2643743,"name":"London","cod":200}

  You can parse anything in the above example.
*/
static size_t
read_curl_data_cb(char *data, size_t size, size_t nmemb, char *str1) {
  uint8_t got_main = 0, got_temp = 0;
  uint8_t y = 0, z = 0;
  char *ptr = NULL;
  const char wind[] = " Wind ", kmh[] = " km/h";
  size_t sz = nmemb * size, x = 0;

  for (ptr = data; *ptr; ptr++, x++) {
    if ((x+7) < sz) { /* Verifying up to *(ptr+7) */

      if ('m' == *ptr) { /* "main":"Clouds" */
        if (0 == got_main) {
          if ('a' == *(ptr+1) && 'i' == *(ptr+2) && 'n' == *(ptr+3)) {
            if ((x+40) < sz) {
              for (; *ptr && *(ptr+7) && z < 29; z++, ptr++) {
                if ('"' == *(ptr+7)) {
                  *str1++ = ' ';
                  break;
                }
                if (0 != (isalpha((unsigned char) *(ptr+7)))) {
                  *str1++ = *(ptr+7);
                }
              }
              got_main = 1;
            }
          }
        }
      }

      if ('t' == *ptr) { /* "temp":12.05 */
        if (0 == got_temp) {
          if ('e' == *(ptr+1) && 'm' == *(ptr+2) && 'p' == *(ptr+3)) {
            if (0 != (isdigit((unsigned char) *(ptr+6)))) {
              *str1++ = *(ptr+6);
              if (0 != (isdigit((unsigned char) *(ptr+7)))) {
                *str1++ = *(ptr+7);
              }
              *str1++ = 'C';
              got_temp = 1;
            }
          }
        }
      }

      if ('s' == *ptr) { /* "speed":3.82 */
        if ('p' == *(ptr+1) && 'e' == *(ptr+2) && 'e' == *(ptr+3)) {
          if ((x+9) < sz) {
            if (0 != (isdigit((unsigned char) *(ptr+7)))) {
              for (y = 0; y < 6; y++) {
                *str1++ = wind[y];
              }
              *str1++ = *(ptr+7);

              if (0 != (isdigit((unsigned char) *(ptr+8)))) {
                *str1++ = *(ptr+8);
              }

              for (y = 0; y < 5; y++) {
                *str1++ = kmh[y];
              }
            }
            *str1 = '\0';
            break;
          }
        }
      }

    }
  }

  if ('\0' != *str1) {
    *str1++ = '\0';
  }
  return sz;
}


/*
  API docs - http://openweathermap.org/current
  Register yourself, create a new API key:
   https://home.openweathermap.org/api_keys
  Comply with your account limits.

  cURL related docs and examples
   https://curl.haxx.se/libcurl/c/curl_easy_setopt.html
   https://github.com/curl/curl/tree/master/docs/examples
*/
void 
get_weather(char *str1) {
  const char *da_url = "http://api.openweathermap.org/data/2.5/weather?q="
    API_TOWN "&units=metric&APPID=" API_KEY;

  CURL *curl = NULL;
  CURLcode res;

  FILL_STR_ARR(1, str1, "0");
  curl_global_init(CURL_GLOBAL_ALL);

  if (NULL == (curl = curl_easy_init())) {
    goto error;
  }

  curl_easy_setopt(curl, CURLOPT_URL, da_url);
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_curl_data_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, str1);

  res = curl_easy_perform(curl);
  if (CURLE_OK != res) {
    goto error;
  }

error:
  if (NULL != curl) {
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return;
}

#else
char *l33t;
#endif /* WITH_WEATHER */
