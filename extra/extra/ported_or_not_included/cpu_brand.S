/* source - https://en.wikipedia.org/wiki/CPUID */
/* used it to port it to pinky-bar */

.section .data

s0 : .asciz "Processor Brand String: %.48s\n"
err : .asciz "Feature unsupported.\n"

.section .text

.global main
.type main,@function
.align 32
main:
	pushq	%rbp
	movq	%rsp,	%rbp
	subq	$48,	%rsp
	pushq	%rbx
	
	movl	$0x80000000,	%eax
	cpuid
	
	cmpl	$0x80000004,	%eax
	jl	error
	
	movl	$0x80000002,	%esi
	movq	%rsp,	%rdi

.align 16
get_brand:
	movl	%esi,	%eax
	cpuid
	
	movl	%eax,	(%rdi)
	movl	%ebx,	4(%rdi)
	movl	%ecx,	8(%rdi)
	movl	%edx,	12(%rdi)
	
	addl	$1,	%esi
	addq	$16,	%rdi
	cmpl	$0x80000004,	%esi
	jle	get_brand

print_brand:
	movq	$s0,	%rdi
	movq	%rsp,	%rsi
	xorb	%al,	%al
	call	printf
	
	jmp	end

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
