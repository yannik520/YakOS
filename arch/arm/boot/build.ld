OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_start)
SECTIONS
{
	. = %MEMBASE%;
	.text : {
	      	arch/arm/boot/setup.o
	      	*(.text)
		}
	.data : { *(.data) }
	.bss : { *(.bss) }
	
	. = ALIGN(4);
	_end = .;
	
	__heap = . + 0x40;

	. = . + %STACKSIZE%; /* 8kB of stack memory */
	stack_top = .;
}
