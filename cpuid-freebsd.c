/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  FreeBSD Routines for retrieving cpuid registers.
 *  Originally submitted by Stanislav Sedov <stas@FreeBSD.org>
 */

#if defined(__FreeBSD__)
# include <sys/ioctl.h>
# include <cpu.h>

void bind_cpu(unsigned int cpunr)
{
	//FIXME:
}

void cpuid(unsigned int CPU_number, unsigned long long idx,
	unsigned int *eax,
	unsigned int *ebx,
	unsigned int *ecx,
	unsigned int *edx)
{
	static int nodriver=0;
	char cpuname[20];
	int fh;
	cpu_cpuid_args_t args;

	if (nodriver == 1) {
		if (native_cpuid(CPU_number, idx, eax,ebx,ecx,edx))
			printf("%s", NATIVE_CPUID_FAILED_MSG);
		return;
	}

	args.level = idx;
	/* Ok, use the /dev/CPU interface in preference to the _up code. */
	(void)snprintf(cpuname,18, "/dev/cpu%u", CPU_number);
	fh = open(cpuname, O_RDONLY);
	if (fh != -1) {
		if (ioctl(fh, CPU_CPUID, &args) != 0) {
			perror(cpuname);
			exit(EXIT_FAILURE);
		}
		if (eax!=0)	*eax = args.data[0];
		if (ebx!=0)	*ebx = args.data[1];
		if (ecx!=0)	*ecx = args.data[2];
		if (edx!=0)	*edx = args.data[3];
		if (close(fh) == -1) {
			perror("close");
			exit(EXIT_FAILURE);
		}
	} else {
		/* Something went wrong, just do UP and hope for the best. */
		nodriver = 1;
		if (nrCPUs != 1)
			perror(cpuname);
		if (native_cpuid(CPU_number, idx, eax,ebx,ecx,edx))
			printf("%s", NATIVE_CPUID_FAILED_MSG);

		return;
	}
}

#endif /* __FreeBSD__ */
