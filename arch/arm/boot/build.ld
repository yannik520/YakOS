OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_start)
SECTIONS
{
	. = %MEMBASE%;
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

	. = . + %STACKSIZE%; /* 8kB of stack memory */
	stack_top = .;
}
