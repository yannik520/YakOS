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
#include <stdlib.h>
#include <kernel/type.h>
#include <kernel/malloc.h>
#include <kernel/printk.h>
#include <module/module_arch.h>

#define ELF32_R_TYPE(info)      ((unsigned char)(info))

#define R_ARM_NONE		0
#define R_ARM_PC24		1
#define R_ARM_ABS32		2
#define R_ARM_THM_CALL		10
#define R_ARM_CALL		28
#define R_ARM_JUMP24		29

int apply_relocate(unsigned int input_addr,
		   struct k_module *output,
		   unsigned int sectionoffset,
		   char *sectionaddr,
		   struct elf32_rela *rela, char *addr)
{
	unsigned int type;
	unsigned int elf_addr;

	type = ELF32_R_TYPE(rela->r_info);
	elf_addr = input_addr + sectionoffset + rela->r_offset;

	switch(type) {
	case R_ARM_ABS32:
	{
		int32_t addend;

		memcpy((char*)&addend, elf_addr, 4);
		addr += addend;
		module_output_write_segment(output,(char*) &addr, 4);
	}
	break;
	case R_ARM_PC24:
	case R_ARM_CALL:
	case R_ARM_JUMP24:
	{
		int32_t addend;
		int32_t offset;

		memcpy((char*)&addend, elf_addr, 4);
		offset = (addend & 0x00ffffff) << 2;
		offset += addr - (sectionaddr + rela->r_offset);
		offset >>= 2;
		addend &= 0xff000000;
		addend |= offset & 0x00ffffff;
		module_output_write_segment(output,(char*) &addend, 4);
	} 
	break;
	case R_ARM_THM_CALL:
	{
		uint16_t instr[2];
		int32_t offset;
		char *base;

		memcpy((char*)instr, elf_addr, 4);
		/* Ignore the addend since it will be zero for calls to symbols,
		   and I can't think of a case when doing a relative call to
		   a non-symbol position */
		base = sectionaddr + (rela->r_offset + 4);

		if (((instr[1]) & 0xe800) == 0xe800) {
			/* BL or BLX */
			if (((uint32_t)addr) & 0x1) {
				/* BL */
				instr[1] |= 0x1800;
			} else {
#if defined(__ARM_ARCH_4T__)
				return MODULE_UNHANDLED_RELOC;
#else
				/* BLX */
				instr[1] &= ~0x1800;
				instr[1] |= 0x0800;
#endif
			}
		}
		/* Adjust address for BLX */
		if ((instr[1] & 0x1800) == 0x0800) {
			addr = (char*)((((uint32_t)addr) & 0xfffffffd)
				       | (((uint32_t)base) & 0x00000002));
		}
		offset = addr -  (sectionaddr + (rela->r_offset + 4));
		if (offset < -(1<<22) || offset >= (1<<22)) {
			printk("offset %d too large for relative call\n", (int)offset);
		}

		instr[0] = (instr[0] & 0xf800) | ((offset>>12)&0x07ff);
		instr[1] = (instr[1] & 0xf800) | ((offset>>1)&0x07ff);
		module_output_write_segment(output, (char*)instr, 4);
	}
	break;
	default:
		printk("Unsupported relocation type %d\n", type);
		return MODULE_UNHANDLED_RELOC;
	}

	return MODULE_OK;
}
