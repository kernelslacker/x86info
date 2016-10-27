/*
 * Sample program to demonstate the usage of libatasmart
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <atasmart.h>

int main(void) {
  uint64_t mkelvin = 0;
  const char *device = "/dev/sda";
  SkDisk *d = NULL;

  if (-1 == (sk_disk_open(device, &d))) {
    fprintf(stderr, "Failed to open disk %s: %s\n", device, strerror(errno));
    return 1;
  }

  if (-1 == (sk_disk_smart_read_data(d))) {
    fprintf(stderr, "Failed to read SMART data: %s\n", strerror(errno));
    goto finish;
  }

  if (-1 == (sk_disk_smart_get_temperature(d, &mkelvin))) {
    fprintf(stderr, "Failed to get temperature: %s\n", strerror(errno));
    goto finish;
  }

  printf("%llu\n", (unsigned long long)mkelvin);

finish:
  if (NULL != d) {
    sk_disk_free(d);
  }
  return 0;
}
