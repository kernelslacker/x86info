	.align 4
	.text

#ifdef linux

.globl cpuid_UP
	.type	cpuid_UP,@function
cpuid_UP:

#else /* BSD or WIN32 */

.globl _cpuid_UP

#ifdef __WIN32__
	.def	_cpuid_UP;  .scl    2;      .type   32;     .endef
#else /* BSD */
	.type	_cpuid_UP,@function
#endif /* __WIN32__ */

_cpuid_UP:
#endif /* linux */

	pushl %ebp
	movl %esp,%ebp
	pushl %edi
	pushl %ebx
	pushl %ecx
	pushl %edx
	movl 8(%ebp),%eax
	cpuid
	movl 12(%ebp),%edi
	test %edi,%edi
	jz L1	
	movl %eax,(%edi)

L1:	movl 16(%ebp),%edi
	test %edi,%edi
	jz L2
	movl %ebx,(%edi)

L2:	movl 20(%ebp),%edi
	test %edi,%edi
	jz L3
	movl %ecx,(%edi)

L3:	movl 24(%ebp),%edi
	test %edi,%edi
	jz L4
	movl %edx,(%edi)

L4:	popl %edx
	popl %ecx
	popl %ebx
	popl %edi
	movl %ebp,%esp
	popl %ebp
	ret
