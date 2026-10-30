OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)
SECTIONS
{
	. = 0xf0000;
	.text : { *(.text.boot) *(.text*) *(.rodata*) }
	ASSERT(. < 0xffff0, "collector exceeds ROM size")
	. = 0xffff0;
	.reset : { *(.reset) }
	. = 0xfffff;
	.end : { BYTE(0) }
	/DISCARD/ : { *(.eh_frame*) *(.comment) *(.note*) }
}
