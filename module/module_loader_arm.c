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
#include <module/module_loader_arch.h>

#define ELF32_R_TYPE(info)      ((unsigned char)(info))

/* Supported relocations */

#define R_ARM_NONE		0
#define R_ARM_PC24		1
#define R_ARM_ABS32		2
#define R_ARM_THM_CALL		10
#define R_ARM_CALL		28
#define R_ARM_JUMP24		29


/* Adapted from elfloader-avr.c */

int
module_loader_arch_relocate(unsigned int input_addr,
			struct module_output *output,
			unsigned int sectionoffset,
			char *sectionaddr,
                        struct elf32_rela *rela, char *addr)
{
  unsigned int type;
  unsigned int elf_addr;

  type = ELF32_R_TYPE(rela->r_info);

  //cfs_seek(input_fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
  elf_addr = input_addr + sectionoffset + rela->r_offset;

/*   PRINTF("elfloader_arch_relocate: type %d\n", type); */
/*   PRINTF("Addr: %p, Addend: %ld\n",   addr, rela->r_addend); */
  switch(type) {
  case R_ARM_ABS32:
    {
      int32_t addend;
      //cfs_read(input_fd, (char*)&addend, 4);
      memcpy((char*)&addend, elf_addr, 4);
      addr += addend;
      module_output_write_segment(output,(char*) &addr, 4);
      //printk("%p: addr: %p sectionaddr: %p r_offset: %p\n", sectionaddr +rela->r_offset,addr, sectionaddr, rela->r_offset);
    }
    break;
  case R_ARM_PC24:
  case R_ARM_CALL:
  case R_ARM_JUMP24:
  {
    int32_t addend;
    int32_t offset;

    memcpy((char*)&addend, elf_addr, 4);
    //printk("1 %p: addend: %p\n", sectionaddr +rela->r_offset, addend);
    offset = (addend & 0x00ffffff) << 2;
    offset += addr - (sectionaddr + rela->r_offset);
    //printk("2 addr=%p: offset: %p\n", addr,offset);
    offset >>= 2;
    //printk("2 addr=%p: offset: %p\n", addr,offset);
    addend &= 0xff000000;
    addend |= offset & 0x00ffffff;
    module_output_write_segment(output,(char*) &addend, 4);
    //printk("%p: addend: %p\n", sectionaddr +rela->r_offset,addend);
  } 
  break;
  case R_ARM_THM_CALL:
    {
      uint16_t instr[2];
      int32_t offset;
      char *base;
      //cfs_read(input_fd, (char*)instr, 4);
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
	printk("elfloader-arm.c: offset %d too large for relative call\n",
	       (int)offset);
      }
    /*   PRINTF("%p: %04x %04x  offset: %d addr: %p\n", sectionaddr +rela->r_offset, instr[0], instr[1], (int)offset, addr);  */
      instr[0] = (instr[0] & 0xf800) | ((offset>>12)&0x07ff);
      instr[1] = (instr[1] & 0xf800) | ((offset>>1)&0x07ff);
      module_output_write_segment(output, (char*)instr, 4);
  /*     PRINTF("cfs_write: %04x %04x\n",instr[0], instr[1]);  */
    }
    break;
    
  default:
    printk("elfloader-arm.c: unsupported relocation type %d\n", type);
    return MODULE_UNHANDLED_RELOC;
  }
  return MODULE_OK;
}
