/*
 * Copyright (c) 2013 Yannik Li(Yanqing Li)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
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

void hexdump(const void *ptr, size_t len)
{
	addr_t address = (addr_t)ptr;
	size_t count;
	int i;

	for (count = 0 ; count < len; count += 16) {
		printk("0x%08lx: ", address);
		printk("%08x %08x %08x %08x |", *(const uint32_t *)address, *(const uint32_t *)(address + 4), *(const uint32_t *)(address + 8), *(const uint32_t *)(address + 12));
		for (i=0; i < 16; i++) {
			char c = *(const char *)(address + i);
			if (isalpha(c)) {
				printk("%c", c);
			} else {
				printk(".");
			}
		}
		printk("|\n");
		address += 16;
	}
}

void hexdump8(const void *ptr, size_t len)
{
	addr_t address = (addr_t)ptr;
	size_t count;
	int i;

	for (count = 0 ; count < len; count += 16) {
		printk("0x%08lx: ", address);
		for (i=0; i < 16; i++) {
			printk("0x%02hhx ", *(const uint8_t *)(address + i));
		}
		printk("\n");
		address += 16;
	}
}
