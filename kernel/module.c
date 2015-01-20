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
#include <kernel/types.h>
#include <mm/malloc.h>
#include <kernel/list.h>
#include <kernel/semaphore.h>
#include <kernel/printk.h>
#include <module/module.h>
#include <module/module_arch.h>
#include <module/symtab.h>
#include <stddef.h>
#include <string.h>
#include <compiler.h>

#include <kernel/debug.h>

struct img_operations;
struct elf_operations;

struct elf_info {
	unsigned int		 input_addr;
	struct img_operations	*ops;
	struct elf32_ehdr	 ehdr;
	struct sec_info		 sec_info[SEC_TYPE_MAX];
	unsigned int		 strs;

};

struct img_operations {
	int (*seek_read)(struct elf_info *info, unsigned int offset, char *buf, int len);	
};

struct load_info {
	struct elf_info		elf_info;
	struct elf_operations	*ops;
};

struct elf_operations {
	int (*check_header)(struct load_info *info);
	int (*get_sec_info)(struct load_info *info);
};

char module_unknown[30];
LIST_HEAD(k_module_root);
DEFINE_SEMAPHORE(kmod_sem);

static const unsigned char elf_magic_header[] =
{
	0x7f, 0x45, 0x4c, 0x46,  /* 0x7f, 'E', 'L', 'F' */
	0x01,                    /* Only 32-bit objects. */
	0x01,                    /* Only LSB data. */
	0x01,                    /* Only ELF version 1. */
};

/* Copy data from the module buffer to a segment */
static int copy_segment_data(unsigned int input_addr, unsigned int offset,
			     struct k_module *output, unsigned int len)
{
	char		buffer[16];
	int		res;
	unsigned int	addr = input_addr + offset;

	while(len > sizeof(buffer)) {
		memcpy(buffer, (uint32_t *)addr, sizeof(buffer));
		res = module_output_write_segment(output, buffer, sizeof(buffer));
		if (res != sizeof(buffer)) 
			return MODULE_OUTPUT_ERROR;
		len  -= sizeof(buffer);
		addr += sizeof(buffer);
	}
	memcpy(buffer, (void *)addr, len);
	res = module_output_write_segment(output, buffer, len);
	if ((uint32_t)res != len) return MODULE_OUTPUT_ERROR;
	return MODULE_OK;
}

static int elf_seek_read(struct elf_info *info, unsigned int offset, char *buf, int len)
{
	if (unlikely((NULL == info) || (NULL == buf)))
		return -1;
	
	memcpy(buf, (uint32_t *)(info->input_addr + offset), len);
	
	return len;
}

static void *find_local_symbol(struct load_info *info, const char *symbol,
			       struct k_module *this_module)
{
	struct elf_info *elf	    = &info->elf_info;
	unsigned int	 symtab	    = elf->sec_info[SEC_TYPE_SYMTAB].s_offset;
	unsigned short	 symtabsize = elf->sec_info[SEC_TYPE_SYMTAB].s_size;
	unsigned int	 strtab	    = elf->sec_info[SEC_TYPE_STRTAB].s_offset;
	struct elf32_sym	 s;
	unsigned int		 a;
	char			 name[30];
	struct relevant_section *sect;
	int			 ret;
  
	for(a = symtab; a < symtab + symtabsize; a += sizeof(s)) {
		ret = elf->ops->seek_read(elf, a, (char *)&s, sizeof(s));
		if (ret < 0) {
			return NULL;
		}

		if(s.st_name != 0) {
			ret = elf->ops->seek_read(elf, strtab + s.st_name, name, sizeof(name));
			if (ret < 0) return NULL;
      
			if(strcmp(name, symbol) == 0) {
				if(s.st_shndx == this_module->seg_info.bss.number) {
					sect = &this_module->seg_info.bss;
				} else if(s.st_shndx == this_module->seg_info.data.number) {
					sect = &this_module->seg_info.data;
				} else if(s.st_shndx == this_module->seg_info.text.number) {
					sect = &this_module->seg_info.text;
				} else {
					return NULL;
				}
				return &(sect->address[s.st_value]);
			}
		}
	}
	return NULL;
}

static int relocate_section(struct load_info *info,
			    struct k_module *output,
			    char *sectionbase,
			    unsigned int sec_type)
{
	struct elf_info *elf	     = &info->elf_info;
	unsigned int	 input_addr  = elf->input_addr;
	struct sec_info *sec	     = &elf->sec_info[sec_type];
	struct sec_info *symtab	     = &elf->sec_info[SEC_TYPE_SYMTAB];
	struct sec_info *strtab	     = &elf->sec_info[SEC_TYPE_STRTAB];
	unsigned char	 using_relas = sec[sec_type].s_relas;

	struct elf32_rela	 rela;
	int			 rel_size = 0;
	struct elf32_sym	 s;
	unsigned int		 a;
	char			 name[30];
	char			*addr;
	struct relevant_section *sect;
	int			 ret;

	/* determine correct relocation entry sizes */
	if(using_relas) {
		rel_size = sizeof(struct elf32_rela);
	} else {
		rel_size = sizeof(struct elf32_rel);
	}

	for(a = sec->s_reloff; a < sec->s_reloff + sec->s_relsize; a += rel_size) {
		ret = elf->ops->seek_read(elf, a, (char *)&rela, rel_size);
		if (ret < 0) return MODULE_INPUT_ERROR;

		ret = elf->ops->seek_read(elf,
				(symtab->s_offset +
				 sizeof(struct elf32_sym) * ELF32_R_SYM(rela.r_info)),
				(char *)&s, sizeof(s));
		if (ret < 0) return MODULE_INPUT_ERROR;

		if(s.st_name != 0) {
			ret = elf->ops->seek_read(elf, strtab->s_offset + s.st_name, name, sizeof(name));
			if (ret < 0) return MODULE_INPUT_ERROR;
			dbg("name: %s\n", name);
			addr = (char *)symtab_lookup(name);
			if(addr == NULL) {
				dbg("name not found in global: %s\n", name);
				addr = find_local_symbol(info, name, output);
				dbg("found address %p\n", addr);
			}
			if(addr == NULL) {
				if(s.st_shndx == output->seg_info.bss.number) {
					sect = &output->seg_info.bss;
				} else if(s.st_shndx == output->seg_info.data.number) {
					sect = &output->seg_info.data;
				} else if(s.st_shndx == output->seg_info.rodata.number) {
					sect = &output->seg_info.rodata;
				} else if(s.st_shndx == output->seg_info.text.number) {
					sect = &output->seg_info.text;
				} else {
					printk("unknown name: '%30s'\n", name);
					memcpy(module_unknown, name, sizeof(module_unknown));
					module_unknown[sizeof(module_unknown) - 1] = 0;
					return MODULE_SYMBOL_NOT_FOUND;
				}
				addr = sect->address;
			}
		} else {
			if(s.st_shndx == output->seg_info.bss.number) {
				sect = &output->seg_info.bss;
			} else if(s.st_shndx == output->seg_info.data.number) {
				sect = &output->seg_info.data;
			} else if(s.st_shndx == output->seg_info.rodata.number) {
				sect = &output->seg_info.rodata;
			} else if(s.st_shndx == output->seg_info.text.number) {
				sect = &output->seg_info.text;
			} else {
				return MODULE_SEGMENT_NOT_FOUND;
			}
      
			addr = sect->address;
		}
    
		{
			/* Copy data up to the next relocation */
			unsigned int offset = module_output_segment_offset(output);
			if (rela.r_offset < offset) {
				printk("relocation out of offset order\n");
	
			}
			if (rela.r_offset > offset) {
				ret = copy_segment_data(input_addr, offset+sec->s_offset, output,
							rela.r_offset - offset);
				if (ret != MODULE_OK) return ret;
			}
		}
		ret = apply_relocate(input_addr, output, sec->s_offset, sectionbase,
						  &rela, addr);
		if (ret != MODULE_OK) return ret;
	}
	return MODULE_OK;
}

static void *
find_module_entry(struct load_info *info, struct k_module *this_module)
{
	struct elf_info		*elf	= &info->elf_info;
	unsigned int		 symtab	= elf->sec_info[SEC_TYPE_SYMTAB].s_offset;
	unsigned short		 size	= elf->sec_info[SEC_TYPE_SYMTAB].s_size;
	unsigned int		 strtab	= elf->sec_info[SEC_TYPE_STRTAB].s_offset;
	struct elf32_sym	 s;
	unsigned int		 a;
	char			 name[30];
  
	for(a = symtab; a < symtab + size; a += sizeof(s)) {
		elf->ops->seek_read(elf, a, (char *)&s, sizeof(s));

		if(s.st_name != 0) {
			elf->ops->seek_read(elf, strtab + s.st_name, name, sizeof(name));
			if(strcmp(name, "mod") == 0) {
				return &this_module->seg_info.data.address[s.st_value];
			}
		}
	}
	return NULL;
}

static int
copy_segment(struct load_info *info,
	     struct k_module *output,
	     char *sectionbase,
	     unsigned int seg_type)
{
	unsigned int	 input_addr = info->elf_info.input_addr;
	struct sec_info *sec	    = info->elf_info.sec_info;
	unsigned int	 offset;
	int		 ret;

	ret = module_output_start_segment(output, seg_type, sectionbase, sec[seg_type-1].s_size);
	if (ret != MODULE_OK) {
		return ret;
	}

	ret = relocate_section(info, output, sectionbase, seg_type - 1);
	if (ret != MODULE_OK) {
		return ret;
	}

	offset = module_output_segment_offset(output);
	ret = copy_segment_data(input_addr, offset+sec[seg_type-1].s_offset, output, sec[seg_type-1].s_size - offset);
	if (ret != MODULE_OK) {
		return ret;
	}

	return module_output_end_segment(output);
}

#define alloc_segment(output, size, seg_name, seg_type) do {		\
		if (size)						\
			module_output_alloc_segment(output, seg_type, size); \
	} while(0)


static int elf_header_check(struct load_info *info)
{
	struct elf_info		*elf	    = &info->elf_info;
	struct elf32_ehdr	*ehdr	    = &elf->ehdr;
	int ret;
	
	/* The ELF header is located at the start of the buffer. */
	ret = elf->ops->seek_read(&info->elf_info, 0, (char *)ehdr, sizeof(*ehdr));
	if (ret != sizeof(*ehdr))
		return MODULE_INPUT_ERROR;

	/* To make sure that we have a correct and compatible ELF header. */
	if (memcmp(ehdr->e_ident, elf_magic_header, sizeof(elf_magic_header)) != 0) {
		printk("ELF header error.\n");
		return MODULE_BAD_ELF_HEADER;
	}

	return MODULE_OK;
}

static int elf_get_sec_info(struct load_info *info)
{
	struct elf_info		*elf = &info->elf_info;
	struct sec_info		*sec_info   = elf->sec_info;
	struct elf32_ehdr	*ehdr	    = &elf->ehdr;
	struct elf32_shdr	 shdr;
	struct elf32_shdr	 strtable;
	unsigned int		 shdrptr;
	unsigned int		 nameptr;
	unsigned short		 shdrnum, shdrsize;
	char			 name[12];
	int			 ret;
	int			 i;

	/* Get the section header. */
	shdrptr = ehdr->e_shoff;
	ret	= elf->ops->seek_read(elf, shdrptr, (char *)&shdr, sizeof(shdr));
	if (ret != sizeof(shdr)) {
		return MODULE_INPUT_ERROR;
	}
  
	/* Get the size and number of entries of the section header. */
	shdrsize = ehdr->e_shentsize;
	shdrnum	 = ehdr->e_shnum;

	/* The string table section: holds the names of the sections. */
	ret = elf->ops->seek_read(elf, ehdr->e_shoff + shdrsize * ehdr->e_shstrndx,
			(char *)&strtable, sizeof(strtable));
	if (ret != sizeof(strtable)) {
		return MODULE_INPUT_ERROR;
	}

	elf->strs = strtable.sh_offset;
	shdrptr	  = ehdr->e_shoff;

	for (i = 0; i < shdrnum; ++i) {
		ret = elf->ops->seek_read(elf, shdrptr, (char *)&shdr, sizeof(shdr));
		if (ret != sizeof(shdr)) {
			return MODULE_INPUT_ERROR;
		}
    
		/* The name of the section is contained in the strings table. */
		nameptr = elf->strs + shdr.sh_name;
		ret = elf->ops->seek_read(elf, nameptr, name, sizeof(name));
		if (ret != sizeof(name)) {
			return MODULE_INPUT_ERROR;
		}
    
		/* Match the name of the section with a predefined set of names
		   (.text, .data, .bss, .rela.text, .rela.data, .symtab, and
		   .strtab). */
		/* added support for .rodata, .rel.text and .rel.data). */

		if(strncmp(name, ".text", 5) == 0) {
			sec_info[SEC_TYPE_TEXT].s_offset = shdr.sh_offset;
			sec_info[SEC_TYPE_TEXT].s_size	 = shdr.sh_size;
			sec_info[SEC_TYPE_TEXT].s_number = i;
		} else if(strncmp(name, ".rel.text", 9) == 0) {
			sec_info[SEC_TYPE_TEXT].s_relas	  = 0;
			sec_info[SEC_TYPE_TEXT].s_reloff  = shdr.sh_offset;
			sec_info[SEC_TYPE_TEXT].s_relsize = shdr.sh_size;
		} else if(strncmp(name, ".rela.text", 10) == 0) {
			sec_info[SEC_TYPE_TEXT].s_relas	  = 1;
			sec_info[SEC_TYPE_TEXT].s_reloff  = shdr.sh_offset;
			sec_info[SEC_TYPE_TEXT].s_relsize = shdr.sh_size;
		} else if(strncmp(name, ".rodata", 7) == 0) {
			/* read-only data handled the same way as regular text section */
			sec_info[SEC_TYPE_RODATA].s_offset = shdr.sh_offset;
			sec_info[SEC_TYPE_RODATA].s_size   = shdr.sh_size;
			sec_info[SEC_TYPE_RODATA].s_number = i;
		} else if(strncmp(name, ".rel.rodata", 11) == 0) {
			/* using elf32_rel instead of rela */
			sec_info[SEC_TYPE_RODATA].s_relas   = 1;
			sec_info[SEC_TYPE_RODATA].s_reloff  = shdr.sh_offset;
			sec_info[SEC_TYPE_RODATA].s_relsize = shdr.sh_size;
		} else if(strncmp(name, ".rela.rodata", 12) == 0) {
			sec_info[SEC_TYPE_RODATA].s_relas = 1;
			sec_info[SEC_TYPE_RODATA].s_reloff  = shdr.sh_offset;
			sec_info[SEC_TYPE_RODATA].s_relsize = shdr.sh_size;
		} else if(strncmp(name, ".data", 5) == 0) {
			sec_info[SEC_TYPE_DATA].s_offset = shdr.sh_offset;
			sec_info[SEC_TYPE_DATA].s_size	 = shdr.sh_size;
			sec_info[SEC_TYPE_DATA].s_number = i;
		} else if(strncmp(name, ".rel.data", 9) == 0) {
			/* using elf32_rel instead of rela */
			sec_info[SEC_TYPE_DATA].s_relas	  = 0;
			sec_info[SEC_TYPE_DATA].s_reloff  = shdr.sh_offset;
			sec_info[SEC_TYPE_DATA].s_relsize = shdr.sh_size;
		} else if(strncmp(name, ".rela.data", 10) == 0) {
			sec_info[SEC_TYPE_DATA].s_relas	  = 1;
			sec_info[SEC_TYPE_DATA].s_reloff  = shdr.sh_offset;
			sec_info[SEC_TYPE_DATA].s_relsize = shdr.sh_size;
		} else if(strncmp(name, ".symtab", 7) == 0) {
			sec_info[SEC_TYPE_SYMTAB].s_offset = shdr.sh_offset;
			sec_info[SEC_TYPE_SYMTAB].s_size   = shdr.sh_size;
		} else if(strncmp(name, ".strtab", 7) == 0) {
			sec_info[SEC_TYPE_STRTAB].s_offset = shdr.sh_offset;
			sec_info[SEC_TYPE_STRTAB].s_size   = shdr.sh_size;
		} else if(strncmp(name, ".bss", 4) == 0) {
			sec_info[SEC_TYPE_BSS].s_size	= shdr.sh_size;
			sec_info[SEC_TYPE_BSS].s_number = i;
			sec_info[SEC_TYPE_BSS].s_offset = 0;
		}

		/* Move on to the next section header. */
		shdrptr += shdrsize;
	}

	if(sec_info[SEC_TYPE_SYMTAB].s_size == 0) {
		return MODULE_NO_SYMTAB;
	}
	if(sec_info[SEC_TYPE_STRTAB].s_size == 0) {
		return MODULE_NO_STRTAB;
	}
	if(sec_info[SEC_TYPE_TEXT].s_size == 0) {
		return MODULE_NO_TEXT;
	}

	return MODULE_OK;
}

static void set_seg_info(struct load_info *info, struct k_module *output)
{
	struct sec_info	*sec_info = info->elf_info.sec_info;

	output->seg_info.text.number   = sec_info[SEC_TYPE_TEXT].s_number;
	output->seg_info.text.offset   = sec_info[SEC_TYPE_TEXT].s_offset;
	output->seg_info.rodata.number = sec_info[SEC_TYPE_RODATA].s_number;
	output->seg_info.rodata.offset = sec_info[SEC_TYPE_RODATA].s_offset;
	output->seg_info.data.number   = sec_info[SEC_TYPE_DATA].s_number;
	output->seg_info.data.offset   = sec_info[SEC_TYPE_DATA].s_offset;
	output->seg_info.bss.number    = sec_info[SEC_TYPE_BSS].s_number;
	output->seg_info.bss.offset    = sec_info[SEC_TYPE_BSS].s_offset;

	alloc_segment(output, sec_info[SEC_TYPE_BSS].s_size, bss, MODULE_SEG_BSS);
	alloc_segment(output, sec_info[SEC_TYPE_TEXT].s_size, text, MODULE_SEG_TEXT);
	alloc_segment(output, sec_info[SEC_TYPE_RODATA].s_size, rodata, MODULE_SEG_RODATA);
	alloc_segment(output, sec_info[SEC_TYPE_DATA].s_size, data, MODULE_SEG_DATA);
	
	dbg("bss base address:    bss.address    = 0x%08x\n", output->seg_info.bss.address);
	dbg("data base address:   data.address   = 0x%08x\n", output->seg_info.data.address);
	dbg("text base address:   text.address   = 0x%08x\n", output->seg_info.text.address);
	dbg("rodata base address: rodata.address = 0x%08x\n", output->seg_info.rodata.address);
}

static int relocate_segment( struct load_info *info, struct k_module *output)
{
	unsigned int	 input_addr = info->elf_info.input_addr;
	struct sec_info *sec_info   = info->elf_info.sec_info;
	int i, ret;

	dbg("module: relocate each segment\n");

	/* Process segment relocations */
	for (i = SEC_TYPE_TEXT; i < SEC_TYPE_SYMTAB; i++) {
		if(sec_info[i].s_relsize > 0) {
			ret = copy_segment(info, output,
					   ((struct relevant_section *)&(output->seg_info)+i)->address,
					   MODULE_SEG_TEXT + i);
			if(MODULE_OK != ret) {
				return ret;
			}
		}
		else {
			memcpy(((struct relevant_section *)&(output->seg_info)+i)->address,
			       (uint32_t *)(input_addr+sec_info[i].s_offset),
			       sec_info[i].s_size);
		}
	}

	/* process bss segment */
	{
		/* set all data in the bss segment to zero */
		unsigned int	len	    = sec_info[SEC_TYPE_BSS].s_size;
		static const char zeros[16] = {0};

		ret = module_output_start_segment(output, MODULE_SEG_BSS,
						  output->seg_info.bss.address,
						  len);
		if (MODULE_OK != ret) {
			return ret;
		}

		while(len > sizeof(zeros)) {
			ret = module_output_write_segment(output, zeros, sizeof(zeros));
			if (ret != sizeof(zeros)) {
				return MODULE_OUTPUT_ERROR;
			}
			len -= sizeof(zeros);
		}

		ret = module_output_write_segment(output, zeros, len);
		if (ret != (int)len) {
			return MODULE_OUTPUT_ERROR;
		}
	}

	return MODULE_OK;
}

static int get_elf_info(struct load_info *info)
{
	int ret;
	
	if (unlikely(NULL == info))
		return MODULE_INPUT_ERROR;

	ret = info->ops->check_header(info);
	if (MODULE_OK != ret)
		return ret;

	ret = info->ops->get_sec_info(info);
	if (MODULE_OK != ret)
		return ret;

	return MODULE_OK;
}

static struct img_operations img_ops = {
	.seek_read = elf_seek_read
};

static struct elf_operations elf_ops = {
	.check_header = elf_header_check,
	.get_sec_info = elf_get_sec_info
};

static inline int _load_module(unsigned int input_addr, struct k_module *output)
{
	struct load_info	 info;
	void			*module;
	int			 ret;

	memset(&info, 0, sizeof(struct load_info));
	
	info.elf_info.input_addr = input_addr;
	info.elf_info.ops	 = &img_ops;
	info.ops		 = &elf_ops;
	
	module_unknown[0] = 0;
	
	ret = get_elf_info(&info);
	if (MODULE_OK != ret)
		return ret;

	set_seg_info(&info, output);
	
	ret = relocate_segment(&info, output);
	if (MODULE_OK != ret)
		return ret;
	
	module = find_local_symbol(&info, "mod_entry", output);
	if(likely(NULL != module)) {
		dbg("module-loader: autostart found\n");
		output->entry = module;
		return MODULE_OK;
	} else {
		printk("module-loader: no autostart\n");
		module = find_module_entry(&info, output);
		if(NULL != module) {
			printk("module-loader: FOUND PRG\n");
		}
		return MODULE_NO_STARTPOINT;
	}
}

int load_kmodule(unsigned int input_addr, struct k_module *mod)
{
	int ret;

	if (unlikely(NULL == mod)) {
		printk("The module pointer is invalid!\n");
		return -1;
	}

	ret = _load_module(input_addr, mod);

	return ret;
}

#define assign_seg_address(output, seg_name, addr) do {			\
		if (output->seg_info.seg_name.address) {		\
			kfree(output->seg_info.seg_name.address);	\
		}							\
		output->seg_info.seg_name.address = addr;		\
	} while(0)

static void * allocate_segment(struct k_module * const output,
			       unsigned int type, int size)
{
	void *block = kmalloc(size);

	if (!block) return NULL;

	switch(type) {
	case MODULE_SEG_TEXT:
		assign_seg_address(output, text, block);
		break;
	case MODULE_SEG_RODATA:
		assign_seg_address(output, rodata, block);
		break;
	case MODULE_SEG_DATA:
		assign_seg_address(output, data, block);
		break;
	case MODULE_SEG_BSS:
		assign_seg_address(output, bss, block);
		break;
	default:
		kfree(block);
		return NULL;
	}
	return block;
}

static void free_segment(struct k_module * const output, unsigned int type)
{
	if (!output)
		return;

	switch(type) {
	case MODULE_SEG_TEXT:
		if (NULL != output->seg_info.text.address) {
			kfree(output->seg_info.text.address);
		}
		break;
	case MODULE_SEG_RODATA:
		if (NULL != output->seg_info.rodata.address) {
			kfree(output->seg_info.rodata.address);
		}
		break;
	case MODULE_SEG_DATA:
		if (NULL != output->seg_info.data.address) {
			kfree(output->seg_info.data.address);
		}
		break;
	case MODULE_SEG_BSS:
		if (NULL != output->seg_info.bss.address) {
			kfree(output->seg_info.bss.address);
		}
		break;
	default:
		printk("please give a correct segment type.\n");
		break;
	}
	return;
}

static int start_segment(struct k_module *output,
			 unsigned int type, void *addr, int size)
{
	output->base   = addr;
	output->offset = 0;

	return MODULE_OK;
}

static int end_segment(struct k_module *output)
{
	return MODULE_OK;
}

static int write_segment(struct k_module *output, const char *buf,
			 unsigned int len)
{
	memcpy(output->base + output->offset, buf, len);
	output->offset += len;

	return len;
}

static unsigned int segment_offset(struct k_module *output)
{
	return output->offset;
}

const struct module_output_ops mod_output_ops =
{
	.allocate_segment = allocate_segment,
	.free_segment	  = free_segment,
	.start_segment	  = start_segment,
	.end_segment	  = end_segment,
	.write_segment	  = write_segment,
	.segment_offset	  = segment_offset
};

struct k_module *alloc_kmodule(void)
{
	struct k_module *mod	  = NULL;
	
	mod = (struct k_module *)kmalloc(sizeof(struct k_module));
	if (unlikely(NULL == mod)) {
		printk("Alloc kmodule failed!\n");
		return NULL;
	}

	memset(mod, 0, sizeof(struct k_module));
	mod->ops = &mod_output_ops;

	down(&kmod_sem);
	list_add_tail(&mod->list, &k_module_root);
	up(&kmod_sem);

	return mod;
}

void free_kmodule(struct k_module *kmod)
{
	if (unlikely(NULL == kmod)) {
		return;
	}
	
	down(&kmod_sem);
	list_del(&kmod->list);
	up(&kmod_sem);

	/* Free text segment */
	module_output_free_segment(kmod, MODULE_SEG_TEXT);

	/* Free rodata segment */
	module_output_free_segment(kmod, MODULE_SEG_RODATA);

	/* Free data segment */
	module_output_free_segment(kmod, MODULE_SEG_DATA);

	/* Free bss segment */
	module_output_free_segment(kmod, MODULE_SEG_BSS);

	kfree(kmod);
}

struct k_module *find_module_by_name(const char *name)
{
	struct k_module *kmod;

	if (NULL == name)
		return NULL;
	
	list_for_each_entry(kmod, &k_module_root, list) {
		if (0 == strcmp(kmod->entry->name, name))
			return kmod;
	}

	return NULL;
}
