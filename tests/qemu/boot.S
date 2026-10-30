/* A reset-vector ROM: enter 32-bit mode without requiring a guest OS. */
.section .text.boot,"ax"
.code16
.global _start
_start:
	cli
	cld
	mov $0xf000, %ax
	mov %ax, %ds
	lgdt (gdt_pointer - _start)
	mov %cr0, %eax
	or $1, %eax
	mov %eax, %cr0
	ljmpl $8, $protected
.code32
protected:
	mov $16, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov $0x80000, %esp
	call collect
1:
	hlt
	jmp 1b
.align 8
gdt:
	.quad 0, 0x00cf9a000000ffff, 0x00cf92000000ffff
gdt_pointer:
	.word 23
	.long gdt
.section .reset,"ax"
.code16
	ljmp $0xf000, $0
.section .note.GNU-stack,"",@progbits
