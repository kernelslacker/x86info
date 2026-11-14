/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  CPUID 1 EAX signature decoding.
 */

#include <x86info.h>

/*
 * Split a CPUID 1 EAX signature. The extended family only exists at base
 * family 0xf; the extended model at 0xf, and from 6 up except on AMD.
 */
void decode_cpu_signature(unsigned int eax, bool amd, struct cpu_signature *sig)
{
	sig->stepping = eax & 0xf;
	sig->model = (eax >> 4) & 0xf;
	sig->family = (eax >> 8) & 0xf;
	sig->efamily = 0;
	sig->emodel = 0;
	if (sig->family == 0xf)
		sig->efamily = (eax >> 20) & 0xff;
	if (sig->family == 0xf || (sig->family >= 6 && !amd))
		sig->emodel = (eax >> 16) & 0xf;
}
