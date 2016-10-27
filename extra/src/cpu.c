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

#include "include/headers.h"
#include "prototypes/cpu.h"

#if defined(__FreeBSD__)
#include "include/freebzd.h"
#endif /* __FreeBSD__ */

#if defined(__OpenBSD__)
#include "include/openbzd.h"
#endif /* __OpenBSD__ */

void 
get_cpu(char *str1) {
  static uintmax_t previous_total = 0, previous_idle = 0;
  uintmax_t total = 0, percent = 0, diff_total = 0, diff_idle = 0;
  uintmax_t cpu_active[LOOP_ITERZ];
  uint8_t x = 0;

  memset(cpu_active, 0, sizeof(cpu_active));

#if defined(__FreeBSD__)
  size_t len = sizeof(cpu_active);
  SYSCTLVAL("kern.cp_time", &cpu_active);
#endif /* __FreeBSD__ */

#if defined(__OpenBSD__)
  int mib[] = { CTL_KERN, KERN_CPTIME };
  size_t len = sizeof(cpu_active);
  SYSCTLVAL(mib, 2, &cpu_active, &len);
#endif /* __OpenBSD__ */

#if defined(__linux__)
  FILE *fp = fopen("/proc/stat", "r");
  CHECK_FP(fp);

  /* Some kernels will produce 7, 8 and 9 columns
   * We rely on 10, refer to `man proc' for more details */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  if (EOF == (fscanf(fp, "%*s " FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT,
    &cpu_active[0], &cpu_active[1], &cpu_active[2], &cpu_active[3],
    &cpu_active[4], &cpu_active[5], &cpu_active[6], &cpu_active[7],
    &cpu_active[8], &cpu_active[9]))) {
      CLOSE_X(fp);
      exit_with_err(ERR, "Upgrade to a newer kernel");
  }
#pragma GCC diagnostic pop
  CLOSE_X(fp);
#endif /* __linux__ */

  for (x = 0; x < LOOP_ITERZ; x++) {
    total += cpu_active[x];
  }

  diff_total     = total - previous_total;
  diff_idle      = cpu_active[IDLE_NUM] - previous_idle;

  previous_total = total;
  previous_idle  = cpu_active[IDLE_NUM];

  percent        = 0;
  if (0 != diff_total) {
    percent      = ((uintmax_t)TICKZ * (diff_total - diff_idle)) / diff_total;
  }

  FILL_UINT_ARR(str1, percent);
}


void
get_cores_load(char *str1) {
  static uintmax_t previous_total[MAX_CORES], previous_idle[MAX_CORES];
  static bool test_flag = false;
  uint8_t x = 0, y = 0, z = 0;
  uintmax_t percent[MAX_CORES], diff_total[MAX_CORES], core_active[MAX_CORES][LOOP_ITERZ];
  uintmax_t diff_idle[MAX_CORES], total[MAX_CORES];
  char buf[VLA], temp[VLA];
  char *all = temp;

  memset(percent, 0, sizeof(percent));
  memset(diff_total, 0, sizeof(diff_total));
  memset(diff_idle, 0, sizeof(diff_idle));
  memset(total, 0, sizeof(total));
  memset(core_active, 0, sizeof(core_active));

  if (false == test_flag) {
    memset(previous_idle, 0, sizeof(previous_idle));
    memset(previous_total, 0, sizeof(previous_total));
  }

#if defined(__FreeBSD__)
  size_t len = sizeof(core_active);
  SYSCTLVAL("kern.cp_times", &core_active);
#endif /* __FreeBSD__ */

#if defined(__OpenBSD__)
  uintmax_t ncpu = 0;
  int mib[] = { CTL_KERN, KERN_CPTIME2, 0 };
  int mib2[] = { CTL_HW, HW_NCPU };
  size_t len = sizeof(core_active), len2 = sizeof(ncpu);

  SYSCTLVAL(mib2, 2, &ncpu, &len2);
  for (x = 0; x < ncpu; x++, mib[2]++) {
    if (0 != (sysctl(mib, 3, &core_active[x], &len, NULL, 0))) {
      break;
    }
  }
#endif /* __OpenBSD__ */

#if defined(__linux__)
  FILE *fp = fopen("/proc/stat", "r");
  CHECK_FP(fp);

  if (NULL == (fgets(buf, VLA, fp))) {
    CLOSE_X(fp);
    exit_with_err(ERR, "reached /proc/stat EOF");
  }

  for (x = 0; x < MAX_CORES; x++, z++) {
    if (NULL == (fgets(buf, VLA, fp))) {
      CLOSE_X(fp);
      exit_with_err(ERR, "reached /proc/stat EOF");
    }

    if ('c' != buf[0] && 'p' != buf[1] && 'u' != buf[2]) {
      break;
    }

    if (EOF == (sscanf(buf, "%*s " FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT FMT_UINT,
      &core_active[x][0], &core_active[x][1], &core_active[x][2], &core_active[x][3],
      &core_active[x][4], &core_active[x][5], &core_active[x][6], &core_active[x][7],
      &core_active[x][8], &core_active[x][9]))) {
        CLOSE_X(fp);
        exit_with_err(ERR, "Upgrade to a newer kernel");
    }
  }
  CLOSE_X(fp);

  for (x = 0; x < z; x++) {

#else /* FreeBSD || OpenBSD */
  for (x = 0; x < MAX_CORES; x++) {
    if (0 == core_active[x][0] && 0 ==
      core_active[x][1] && 0 ==
      core_active[x][2]) {
	    break;
    }
#endif /* __linux__ */

    for (y = 0; y < LOOP_ITERZ; y++) {
      total[x] += core_active[x][y];
    }

    diff_total[x]     = total[x] - previous_total[x];
    diff_idle[x]      = core_active[x][IDLE_NUM] - previous_idle[x];

    previous_total[x] = total[x];
    previous_idle[x]  = core_active[x][IDLE_NUM];

    percent[x]        = 0;
    if (0 != diff_total[x]) {
      percent[x]      = ((uintmax_t)TICKZ * (diff_total[x] - diff_idle[x]))
                            / diff_total[x];
    }

    GLUE2(all, FMT_UINT"%% ", percent[x]);
  }
  test_flag = true;

  FILL_STR_ARR(1, str1, temp);
}


#if defined(__i386__) || defined(__i686__) || defined(__x86_64__)
uint8_t has_tsc_reg(void) {
  uint_fast16_t vend = 0, leafs = 0;
  uint_fast16_t eax = 0, ecx = 0, edx = 0, ebx = 0;

  CPU_REGS(0x00000000, vend, leafs);
  if (0x00000001 > leafs) {
    return 1;
  }
  if (vend != AmD && vend != InteL) {
    return 1;
  }

  CPU_STR2(0x00000001, eax, ebx, ecx, edx);
  if (0 == (edx & (1 << 4))) {
    return 1;
  }
  return 0;
}
#endif /* __i386__ || __i686__ || __x86_64__  */


/*  Taken from the gcc documentation
 *  https://gcc.gnu.org/onlinedocs/gcc/Machine-Constraints.html
 *
 *  other useful resources:
 *  http://www.felixcloutier.com/x86/RDTSC.html 
 *  http://ref.x86asm.net/coder32.html#x0F31 
*/

/* Not going to test for i486 and i586 */
#if defined(__i386__) || defined(__i686__)
static __inline__ uintmax_t 
rdtsc(void) {
  uintmax_t x = 0;
  if (0 == (has_tsc_reg())) {
    __asm__ __volatile__ (".byte 0x0f, 0x31" : "=A" (x));
  }
  return x;
}

void
get_cpu_clock_speed(char *str1) {
  uintmax_t x = 0, y = 0, z[2];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
  struct timespec start = {0}, stop = {0}, tc = {0};
#pragma GCC diagnostic pop

  tc.tv_nsec = TICKZ * 1000000L;

  x = rdtsc();
  if (-1 == (clock_gettime(CLOCK_MONOTONIC, &start))) {
    FUNC_FAILED("clock_gettime()");
  }
  z[0] = (uintmax_t)(start.tv_nsec - start.tv_sec);

  if (-1 == (nanosleep(&tc, NULL))) {
    FUNC_FAILED("nanosleep()");
  }

  y = rdtsc();
  if (-1 == (clock_gettime(CLOCK_MONOTONIC, &stop))) {
    FUNC_FAILED("clock_gettime()");
  }
  z[1] = (uintmax_t)(stop.tv_nsec - stop.tv_sec);

  if (0 != (z[1] - z[0])) {
    FILL_ARR(str1, FMT_UINT " MHz",
      (1000 * (y - x) / (z[1] - z[0])));
  }
}


#elif defined(__x86_64__)
/* 
 * Thanks to Intel docs for pointing out
 * "Out of Order Execution", added
 * cpuid after reading it and edited the rdtscp
 * code according to the docs
 * https://www-ssl.intel.com/content/dam/www/public/us/en/documents/white-papers/ia-32-ia-64-benchmark-code-execution-paper.pdf */
static __inline__ uintmax_t 
rdtsc(void) {
  unsigned int tickhi = 0, ticklo = 0;
  uint_fast16_t eax = 0, ecx = 0, edx = 0, ebx = 0;
  uint_fast16_t vend = 0, regz = 0, x = 0;

  if (0 != (has_tsc_reg())) {
    goto seeya;
  }
  __asm__ __volatile__ (
    "cpuid\n\t"
    "rdtsc\n\t"
    : "=a"(ticklo), "=d"(tickhi)
    :: "%rbx", "%rcx"
  );

  CPU_FEATURE(0x80000000, regz);
  if (0x80000001 > regz) {
    goto seeya;
  }
  CPU_STR2(0x80000001, eax, ebx, ecx, edx);

  if (0 != (edx & (1 << 27))) {
    for (x = 0; x < 6; x++) {
      __asm__ __volatile__ (
        "rdtscp\n\t"
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t"
        "cpuid\n\t"
        : "=r"(tickhi), "=r"(ticklo)
        :: "%rax", "%rbx", "%rcx", "%rdx"
      );
    }
  }

seeya:
  return (((uintmax_t)tickhi << 32) | (uintmax_t)ticklo);
}

void
get_cpu_clock_speed(char *str1) {
  uintmax_t x = 0, z = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
  struct timespec tc = {0};
#pragma GCC diagnostic pop

  tc.tv_nsec = TICKZ * 1000000L;

  x = rdtsc();
  if (-1 == (nanosleep(&tc, NULL))) {
    FUNC_FAILED("nanosleep()");
  }
  z = rdtsc();

  FILL_ARR(str1, FMT_UINT " MHz", ((z - x) / 100000));
}
#endif /* __i386__ || __i686__ || __x86_64__ */


#if defined(__i386__) || defined(__i686__) || defined(__x86_64__)
void
get_cpu_info(char *str1) {
  uint_fast16_t vend = 0, x = 0, z = 0, corez = 0, leafB = 0, bitz[2];
  uint_fast16_t eax = 0, ecx = 0, edx = 0, ebx = 0, eax_old = 0, leafs = 0;
  uint_fast16_t line_size = 0, regz = 0, clflu6 = 0, caches[3];
  char buffer[VLA], vend_id[13], vend_chars[17];
  char *all = buffer;
  bool got_leafB = false;

  memset(caches, 0, sizeof(caches));
  memset(bitz, 0, sizeof(bitz));
  FILL_STR_ARR(1, str1, "Null");

  CPU_REGS(0x00000000, vend, leafs);              /* movl $0x00000000, %eax */
  if (0x00000001 > leafs) {
    return;
  }
  if (vend != AmD && vend != InteL) {
    return;
  }

  CPU_FEATURE(0x80000000, regz);                  /* movl $0x80000000, %eax */
  if (0x80000004 > regz) {
    return;
  }
  CPU_FEATURE(0x00000001, eax_old);               /* movl $0x00000001, %eax */

  for (x = 0x80000002; x <= 0x80000004; x++) {    /* movl $0x80000002, %esi */
    CPU_STR2(x, eax, ebx, ecx, edx);              /* cmpl $0x80000004, %eax */
    memset(vend_chars, 0, sizeof(vend_chars));

    for (z = 0; z < 4; z++) {
      vend_chars[z] = (char)(eax >> (z * 8));     /* movl %eax, 0 */
      vend_chars[z+4] = (char)(ebx >> (z * 8));   /* movl %ebx, 4 */
      vend_chars[z+8] = (char)(ecx >> (z * 8));   /* movl %ecx, 8 */
      vend_chars[z+12] = (char)(edx >> (z * 8));  /* movl %edx, 12 */
    }
    vend_chars[16] = '\0';
    GLUE2(all, "%s", vend_chars);
  }

  CPU_ID_STR(0x00000000, ebx, ecx, edx);          /* movl $0x00000000, %ebx */
  for (x = 0; x < 4; x++) {
    vend_id[x] = (char)(ebx >> (x * 8));          /* movl %ebx, 0 */
    vend_id[x+4] = (char)(edx >> (x * 8));        /* movl %edx, 4 */
    vend_id[x+8] = (char)(ecx >> (x * 8));        /* movl %ecx, 8 */
  }
  vend_id[12] = '\0';

  if (vend == AmD) {
    if (0x80000005 <= regz) {
      CPU_STR2(0x80000005, eax, ebx, ecx, edx);   /* movl $0x80000005, %eax */
      caches[0] = SHFT2(ecx >> (3 * 8));          /* movl %ecx, 24 */
    }
    CPU_STR2(0x00000001, eax, ebx, ecx, edx);     /* movl $0x00000001, %eax */
    corez = SHFT2(ebx >> (2 * 8));                /* movl %ebx, 16 */
  }

  if (vend == InteL) {
    if (0x0000000B <= leafs) {
      CPU_STR2(0x0000000B, eax, ebx, ecx, edx);   /* movl $0x0000000B, %eax */
      corez  = SHFT2(ebx);                        /* movl %ebx, 0 */
      leafB  = SHFT2(edx);                        /* movl %edx, 0 */
      got_leafB = true;

    } else {
      if (0x00000004 <= leafs) {
        CPU_STR2(0x00000004, eax, ebx, ecx, edx); /* movl $0x00000004, %eax */
        corez  = SHFT2(eax >> 26);                /* movl %eax, 26 */
      }
    }
  }

  if (0x80000006 <= regz) {
    CPU_STR2(0x80000006, eax, ebx, ecx, edx);     /* movl $0x80000006, %eax */
    /* L2, line size */
    caches[1] = (ecx >> (2 * 8)) & 0xffff;        /* movl %ecx, 16 */
    caches[2] = SHFT2(ecx);                       /* movl %ecx, 0 */
  }

  /* 100 mhz steps for the majority of AMD cpu's
   *  movl $0x80000007 %edx, 6
  */

  if (0x80000008 <= regz) {
    CPU_STR2(0x80000008, eax, ebx, ecx, edx);     /* movl $0x80000008, %eax */
    /* physical, virtual */
    bitz[0] = SHFT2(eax);                         /* movl %eax, 0 */
    bitz[1] = SHFT2(eax >> 8);                    /* movl %eax, 8 */

    if (vend == AmD) {
      corez = SHFT2(ecx) + 1;                     /* movl %ecx, 0 */
    }
  }

  CPU_STR2(0x00000001, eax, ebx, ecx, edx);       /* movl $0x00000001, %eax */
  if (0 != (edx & (1 << 19))) {
    clflu6 = SHFT2(ebx >> 8) * 8;                 /* movl %ebx, 8 */
  }

  FILL_ARR(str1,
   UFINT "x %s ID %s"
   " CLFLUSH/Line size " UFINT " " UFINT
   " L1/L2 caches KB " UFINT " " UFINT
   " Stepping " UFINT " Family " UFINT
   " Model " UFINT
   " Bits " UFINT " " UFINT
   " apicid " UFINT,

  /* cores, vendor, vendor id */
    corez, buffer, vend_id,
  /* clflush, line size */
    clflu6, caches[2],
  /* L1, L2 */
    caches[0], caches[1],
  /* stepping, family */
    SHFT(eax_old), (SHFT(eax_old >> 8) +
     ((IZMAX(eax_old)) ? SHFT2(eax_old >> 20) : 0)),
  /* model */
    (SHFT(eax_old >> 4) |
     ((IZMAX(eax_old)) ? ((eax_old >> 12) & 0xf0) : 0)),
  /* physical and virtual bits */
    bitz[0], bitz[1],
  /* apicid */
    (true == got_leafB) ? leafB : SHFT2(ebx >> 24)
  );
}
#endif /* __i386__ || __i686__ || __x86_64__ */
