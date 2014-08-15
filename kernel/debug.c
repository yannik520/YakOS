#include <kernel/types.h>
#include <kernel/printk.h>
#include <kernel/debug.h>

void _assert(
	const char*  assertion,
	const char*  file,
	unsigned int line,
	const char*  function)
{
	printk("YakOS failed assertion '%s' at %s:%u in function %s\n",
	       assertion,
	       file,
	       line,
	       function
		);
	halt();
}
