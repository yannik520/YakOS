OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_start)
SECTIONS
{
	. = 0xc0008000;
	.text : {
	      	arch/arm/boot/setup.o(.text.boot .rodata)
	      	*(.text)
	}
	.data : { *(.data) }
	.bss : ALIGN(4) {
	       	. = ALIGN(4);
		__bss_start = .;
	       	*(.bss) 
	}
	
	. = ALIGN(4);
	_end = .;

	. = ALIGN(4096);
	__heap = .;

	. = . + 0x200000; /* 8kB of stack memory */
	stack_top = .;
}
