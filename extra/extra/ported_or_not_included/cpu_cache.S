/* source - https://en.wikipedia.org/wiki/CPUID */
/* used it to port it to pinky-bar */

.section .data

info : .ascii "L2 Cache Size : %u KB\nLine size : %u bytes\n"
.asciz "Associativity : %02xh\n"
err : .asciz "Feature unsupported.\n"

.section .text

.global main
.type main,@function
.align 32
main:
	pushq	%rbp
	movq	%rsp,	%rbp
	pushq	%rbx
	
	movl	$0x80000000,	%eax
	cpuid
	
	cmpl	$0x80000006,	%eax
	jl	error
	
	movl	$0x80000006,	%eax
	cpuid
	
	movl	%ecx,	%eax
	
	movl	%eax,	%edx
	andl	$0xff,	%edx
	
	movl	%eax,	%ecx
	shrl	$12,	%ecx
	andl	$0xf,	%ecx
	
	movl	%eax,	%esi
	shrl	$16,	%esi
	andl	$0xffff,%esi
	
	movq	$info,	%rdi
	xorb	%al,	%al
	call	printf
	
	jmp end
	
.align 16
error:
	movq	$err,	%rdi
	xorb	%al,	%al
	call	printf

.align 16
end:
	popq	%rbx
	movq	%rbp,	%rsp
	popq	%rbp
	xorl	%eax,	%eax
	ret
