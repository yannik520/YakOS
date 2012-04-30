OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_start)
SECTIONS
{
	. = 0x10000;
	.text : {
	      	setup.o
	      	*(.text)
		}
	.data : { *(.data) }
	.bss : { *(.bss) }
	
	. = ALIGN(4);
	_end = .;
	
	__heap = . + 0x40;

	. = . + 0x4000; /* 8kB of stack memory */
	stack_top = .;
}