#pragma once

#define	SMP_NO	0
#define SMP_YES	1

void display_mptable(void);
int mptable_scan_fd(int fd, int quiet, int *ncpu);
