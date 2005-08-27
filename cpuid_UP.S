	.align 4
	.text

.globl cpuid_UP
	.type	cpuid_UP,@function
cpuid_UP:

#ifdef __x86_64__
/* x86_64 calling convention: Args passed in rdi, rsi, rdx, rcx, r8, r9;
   rbc, r12 -- r15, and rbp need to be preserved. */
	pushq %rbp
	movq %rsp,%rbp
	pushq %rbx
	movq %rdx, %r10
	movq %rcx, %r11
	movq %rdi, %rax
	cpuid
	test %rsi,%rsi
	jz L1	
	movq %rax,(%rsi)

L1:	
	test %r10,%r10
	jz L2
	movq %rbx,(%r10)

L2:	
	test %r11,%r11
	jz L3
	movq %rcx,(%r11)

L3:	
	test %r8,%r8
	jz L4
	movq %rdx,(%r8)

L4:
	popq %rbx
	movq %rbp,%rsp
	popq %rbp
	ret
#else
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
#endif

.section .note.GNU-stack,"", @progbits

