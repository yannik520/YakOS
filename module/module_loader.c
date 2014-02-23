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
#include <kernel/malloc.h>
#include <module/module_loader.h>
#include <module/module_loader_arch.h>
#include <module/symtab.h>
#include <stddef.h>
#include <string.h>

char module_unknown[30];
//void *this_module;


struct k_module k_module_root = {0};
struct k_module *this_module = &k_module_root;

//static struct relevant_section bss, data, rodata, text;

const static unsigned char elf_magic_header[] =
{
	0x7f, 0x45, 0x4c, 0x46,  /* 0x7f, 'E', 'L', 'F' */
	0x01,                    /* Only 32-bit objects. */
	0x01,                    /* Only LSB data. */
	0x01,                    /* Only ELF version 1. */
};

/* Copy data from the module buffer to a segment */
static int copy_segment_data(unsigned int input_addr, unsigned int offset,
			     struct module_output *output, unsigned int len)
{
	char		buffer[16];
	int		res;
	unsigned int	addr = input_addr + offset;

	while(len > sizeof(buffer)) {
		memcpy(buffer, addr, sizeof(buffer));
		res = module_output_write_segment(output, buffer, sizeof(buffer));
		if (res != sizeof(buffer)) 
			return MODULE_OUTPUT_ERROR;
		len -= sizeof(buffer);
		addr += sizeof(buffer);
	}
	memcpy(buffer, addr, len);
	res = module_output_write_segment(output, buffer, len);
	if (res != len) return MODULE_OUTPUT_ERROR;
	return MODULE_OK;
}

static int seek_read(unsigned int addr, unsigned int offset, char *buf, int len)
{
	memcpy(buf, addr+offset, len);
	return len;
}

static void * find_local_symbol(unsigned int input_addr, const char *symbol,
				unsigned int symtab, unsigned short symtabsize,
				unsigned int strtab)
{
	struct elf32_sym	 s;
	unsigned int		 a;
	char			 name[30];
	struct relevant_section *sect;
	int			 ret;
  
	for(a = symtab; a < symtab + symtabsize; a += sizeof(s)) {
		ret = seek_read(input_addr, a, (char *)&s, sizeof(s));
		if (ret < 0) {
			return NULL;
		}

		if(s.st_name != 0) {
			ret = seek_read(input_addr, strtab + s.st_name, name, sizeof(name));
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

static int relocate_section(unsigned int input_addr,
			    struct module_output *output,
			    unsigned int section, unsigned short size,
			    unsigned int sectionaddr,
			    char *sectionbase,
			    unsigned int strs,
			    unsigned int strtab,
			    unsigned int symtab, unsigned short symtabsize,
			    unsigned char using_relas)
{
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

	for(a = section; a < section + size; a += rel_size) {
		ret = seek_read(input_addr, a, (char *)&rela, rel_size);
		if (ret < 0) return MODULE_INPUT_ERROR;

		ret = seek_read(input_addr,
				(symtab +
				 sizeof(struct elf32_sym) * ELF32_R_SYM(rela.r_info)),
				(char *)&s, sizeof(s));
		if (ret < 0) return MODULE_INPUT_ERROR;

		if(s.st_name != 0) {
			ret = seek_read(input_addr, strtab + s.st_name, name, sizeof(name));
			if (ret < 0) return MODULE_INPUT_ERROR;
			//printk("name: %s\n", name);
			addr = (char *)symtab_lookup(name);
			if(addr == NULL) {
				//printk("name not found in global: %s\n", name);
				addr = find_local_symbol(input_addr, name, symtab, symtabsize, strtab);
				//printk("found address %p\n", addr);
			}
			if(addr == NULL) {
				if(s.st_shndx == this_module->seg_info.bss.number) {
					sect = &this_module->seg_info.bss;
				} else if(s.st_shndx == this_module->seg_info.data.number) {
					sect = &this_module->seg_info.data;
				} else if(s.st_shndx == this_module->seg_info.rodata.number) {
					sect = &this_module->seg_info.rodata;
				} else if(s.st_shndx == this_module->seg_info.text.number) {
					sect = &this_module->seg_info.text;
				} else {
					printk("unknown name: '%30s'\n", name);
					memcpy(module_unknown, name, sizeof(module_unknown));
					module_unknown[sizeof(module_unknown) - 1] = 0;
					return MODULE_SYMBOL_NOT_FOUND;
				}
				addr = sect->address;
			}
		} else {
			if(s.st_shndx == this_module->seg_info.bss.number) {
				sect = &this_module->seg_info.bss;
			} else if(s.st_shndx == this_module->seg_info.data.number) {
				sect = &this_module->seg_info.data;
			} else if(s.st_shndx == this_module->seg_info.rodata.number) {
				sect = &this_module->seg_info.rodata;
			} else if(s.st_shndx == this_module->seg_info.text.number) {
				sect = &this_module->seg_info.text;
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
				ret = copy_segment_data(input_addr, offset+sectionaddr, output,
							rela.r_offset - offset);
				if (ret != MODULE_OK) return ret;
			}
		}
		ret = module_loader_arch_relocate(input_addr, output, sectionaddr, sectionbase,
						  &rela, addr);
		if (ret != MODULE_OK) return ret;
	}
	return MODULE_OK;
}
/*---------------------------------------------------------------------------*/
static void *
find_module_entry(unsigned int input_addr,
		  unsigned int symtab, unsigned short size,
		  unsigned int strtab)
{
	struct elf32_sym	s;
	unsigned int		a;
	char			name[30];
  
	for(a = symtab; a < symtab + size; a += sizeof(s)) {
		seek_read(input_addr, a, (char *)&s, sizeof(s));

		if(s.st_name != 0) {
			seek_read(input_addr, strtab + s.st_name, name, sizeof(name));
			if(strcmp(name, "mod") == 0) {
				return &this_module->seg_info.data.address[s.st_value];
			}
		}
	}
	return NULL;
}

static int
copy_segment(unsigned int input_addr,
	     struct module_output *output,
	     unsigned int section, unsigned short size,
	     unsigned int sectionaddr,
	     char *sectionbase,
	     unsigned int strs,
	     unsigned int strtab,
	     unsigned int symtab, unsigned short symtabsize,
	     unsigned char using_relas,
	     unsigned int seg_size, unsigned int seg_type)
{
	unsigned int	offset;
	int		ret;

	ret = module_output_start_segment(output, seg_type,sectionbase, seg_size);
	if (ret != MODULE_OK) {
		return ret;
	}

	ret = relocate_section(input_addr, output,
			       section, size,
			       sectionaddr,
			       sectionbase,
			       strs,
			       strtab,
			       symtab, symtabsize, using_relas);
	if (ret != MODULE_OK) {
		return ret;
	}

	offset = module_output_segment_offset(output);
	ret = copy_segment_data(input_addr, offset+sectionaddr, output,seg_size - offset);
	if (ret != MODULE_OK) {
		return ret;
	}

	return module_output_end_segment(output);
}

struct k_module * alloc_kmodule(void)
{
	struct k_module *mod = NULL;
	
	mod = (struct k_module *)kmalloc(sizeof(struct k_module));
	if (NULL == mod) {
		printk("Alloc kmodule failed!\n");
	}
	
	return mod;
}

int load_module(unsigned int input_addr, struct module_output *output)
{
	struct elf32_ehdr ehdr;
	struct elf32_shdr shdr;
	struct elf32_shdr strtable;
	unsigned int strs;
	unsigned int shdrptr;
	unsigned int nameptr;
	char name[12];
  
	int i;
	unsigned short shdrnum, shdrsize;

	unsigned char using_relas = -1;
	unsigned short textoff = 0, textsize, textrelaoff = 0, textrelasize;
	unsigned short dataoff = 0, datasize, datarelaoff = 0, datarelasize;
	unsigned short rodataoff = 0, rodatasize, rodatarelaoff = 0, rodatarelasize;
	unsigned short symtaboff = 0, symtabsize;
	unsigned short strtaboff = 0, strtabsize;
	unsigned short bsssize = 0;

	void *module;
	int ret;

	module_unknown[0] = 0;

	/* The ELF header is located at the start of the buffer. */
	ret = seek_read(input_addr, 0, (char *)&ehdr, sizeof(ehdr));
	if (ret != sizeof(ehdr)) return MODULE_INPUT_ERROR;

	/* Make sure that we have a correct and compatible ELF header. */
	if (memcmp(ehdr.e_ident, elf_magic_header, sizeof(elf_magic_header)) != 0) {
		printk("ELF header problems\n");
		return MODULE_BAD_ELF_HEADER;
	}

	/* Grab the section header. */
	shdrptr = ehdr.e_shoff;
	ret = seek_read(input_addr, shdrptr, (char *)&shdr, sizeof(shdr));
	if (ret != sizeof(shdr)) {
		return MODULE_INPUT_ERROR;
	}
  
	/* Get the size and number of entries of the section header. */
	shdrsize = ehdr.e_shentsize;
	shdrnum	 = ehdr.e_shnum;

	/* The string table section: holds the names of the sections. */
	ret = seek_read(input_addr, ehdr.e_shoff + shdrsize * ehdr.e_shstrndx,
			(char *)&strtable, sizeof(strtable));
	if (ret != sizeof(strtable)) {
		return MODULE_INPUT_ERROR;
	}

	strs = strtable.sh_offset;

	/* Initialize the segment sizes to zero so that we can check if
	   their sections was found in the file or not. */
	textsize = textrelasize = datasize = datarelasize =
		rodatasize = rodatarelasize = symtabsize = strtabsize = 0;

	this_module->seg_info.text.number   = -1;
	this_module->seg_info.rodata.number = -1;
	this_module->seg_info.data.number   = -1;
	this_module->seg_info.bss.number    = -1;

	shdrptr = ehdr.e_shoff;
	for (i = 0; i < shdrnum; ++i) {

		ret = seek_read(input_addr, shdrptr, (char *)&shdr, sizeof(shdr));
		if (ret != sizeof(shdr)) {
			return MODULE_INPUT_ERROR;
		}
    
		/* The name of the section is contained in the strings table. */
		nameptr = strs + shdr.sh_name;
		ret = seek_read(input_addr, nameptr, name, sizeof(name));
		if (ret != sizeof(name)) {
			return MODULE_INPUT_ERROR;
		}
    
		/* Match the name of the section with a predefined set of names
		   (.text, .data, .bss, .rela.text, .rela.data, .symtab, and
		   .strtab). */
		/* added support for .rodata, .rel.text and .rel.data). */

		if(strncmp(name, ".text", 5) == 0) {
			textoff = shdr.sh_offset;
			textsize = shdr.sh_size;
			this_module->seg_info.text.number = i;
			this_module->seg_info.text.offset = textoff;
		} else if(strncmp(name, ".rel.text", 9) == 0) {
			using_relas = 0;
			textrelaoff = shdr.sh_offset;
			textrelasize = shdr.sh_size;
		} else if(strncmp(name, ".rela.text", 10) == 0) {
			using_relas = 1;
			textrelaoff = shdr.sh_offset;
			textrelasize = shdr.sh_size;
		} else if(strncmp(name, ".data", 5) == 0) {
			dataoff = shdr.sh_offset;
			datasize = shdr.sh_size;
			this_module->seg_info.data.number = i;
			this_module->seg_info.data.offset = dataoff;
		} else if(strncmp(name, ".rodata", 7) == 0) {
			/* read-only data handled the same way as regular text section */
			rodataoff = shdr.sh_offset;
			rodatasize = shdr.sh_size;
			this_module->seg_info.rodata.number = i;
			this_module->seg_info.rodata.offset = rodataoff;
		} else if(strncmp(name, ".rel.rodata", 11) == 0) {
			/* using elf32_rel instead of rela */
			using_relas = 0;
			rodatarelaoff = shdr.sh_offset;
			rodatarelasize = shdr.sh_size;
		} else if(strncmp(name, ".rela.rodata", 12) == 0) {
			using_relas = 1;
			rodatarelaoff = shdr.sh_offset;
			rodatarelasize = shdr.sh_size;
		} else if(strncmp(name, ".rel.data", 9) == 0) {
			/* using elf32_rel instead of rela */
			using_relas = 0;
			datarelaoff = shdr.sh_offset;
			datarelasize = shdr.sh_size;
		} else if(strncmp(name, ".rela.data", 10) == 0) {
			using_relas = 1;
			datarelaoff = shdr.sh_offset;
			datarelasize = shdr.sh_size;
		} else if(strncmp(name, ".symtab", 7) == 0) {
			symtaboff = shdr.sh_offset;
			symtabsize = shdr.sh_size;
		} else if(strncmp(name, ".strtab", 7) == 0) {
			strtaboff = shdr.sh_offset;
			strtabsize = shdr.sh_size;
		} else if(strncmp(name, ".bss", 4) == 0) {
			bsssize = shdr.sh_size;
			this_module->seg_info.bss.number = i;
			this_module->seg_info.bss.offset = 0;
		}

		/* Move on to the next section header. */
		shdrptr += shdrsize;
	}

	if(symtabsize == 0) {
		return MODULE_NO_SYMTAB;
	}
	if(strtabsize == 0) {
		return MODULE_NO_STRTAB;
	}
	if(textsize == 0) {
		return MODULE_NO_TEXT;
	}

	if (bsssize) {
		this_module->seg_info.bss.address = (char *)
			module_output_alloc_segment(output, MODULE_SEG_BSS, bsssize);
		if (!this_module->seg_info.bss.address) {
			return MODULE_OUTPUT_ERROR;
		}
	}
	if (datasize) {
		this_module->seg_info.data.address = (char *)
			module_output_alloc_segment(output, MODULE_SEG_DATA, datasize);
		if (!this_module->seg_info.data.address) {
			return MODULE_OUTPUT_ERROR;
		}
	}
	if (textsize) {
		this_module->seg_info.text.address = (char *)
			module_output_alloc_segment(output, MODULE_SEG_TEXT, textsize);
		if (!this_module->seg_info.text.address) {
			return MODULE_OUTPUT_ERROR;
		}
	}
	if (rodatasize) {
		this_module->seg_info.rodata.address =  (char *)
			module_output_alloc_segment(output, MODULE_SEG_RODATA, rodatasize);
		if (!this_module->seg_info.rodata.address) {
			return MODULE_OUTPUT_ERROR;
		}
	}

	/*
	  printk("bss base address: bss.address = 0x%08x\n", this_module->seg_info.bss.address);
	  printk("data base address: data.address = 0x%08x\n", this_module->seg_info.data.address);
	  printk("text base address: text.address = 0x%08x\n", this_module->seg_info.text.address);
	  printk("rodata base address: rodata.address = 0x%08x\n", this_module->seg_info.rodata.address);
	*/

	/* If we have text segment relocations, we process them. */
	//printk("elfloader: relocate text\n");
	if(textrelasize > 0) {
		ret = copy_segment(input_addr, output,
				   textrelaoff, textrelasize,
				   textoff,
				   this_module->seg_info.text.address,
				   strs,
				   strtaboff,
				   symtaboff, symtabsize, using_relas,
				   textsize, MODULE_SEG_TEXT);
		if(ret != MODULE_OK) {
			return ret;
		}
	}
	else {
		memcpy(this_module->seg_info.text.address, input_addr+textoff, textsize);
	}

	/* If we have any rodata segment relocations, we process them too. */
	//printk("elfloader: relocate rodata\n");
	if(rodatarelasize > 0) {
		ret = copy_segment(input_addr, output,
				   rodatarelaoff, rodatarelasize,
				   rodataoff,
				   this_module->seg_info.rodata.address,
				   strs,
				   strtaboff,
				   symtaboff, symtabsize, using_relas,
				   rodatasize, MODULE_SEG_RODATA);
		if(ret != MODULE_OK) {
			printk("elfloader: data failed\n");
			return ret;
		}
	}
	else {
		memcpy(this_module->seg_info.rodata.address, input_addr+rodataoff, rodatasize);
	}

	/* If we have any data segment relocations, we process them too. */
	//printk("module-loader: relocate data\n");
	if(datarelasize > 0) {
		ret = copy_segment(input_addr, output,
				   datarelaoff, datarelasize,
				   dataoff,
				   this_module->seg_info.data.address,
				   strs,
				   strtaboff,
				   symtaboff, symtabsize, using_relas,
				   datasize, MODULE_SEG_DATA);
		if(ret != MODULE_OK) {
			printk("module-loader: data failed\n");
			return ret;
		}
		ret = module_output_end_segment(output);
		if (ret != MODULE_OK) return ret;
	}

	{
		/* Write zeros to bss segment */
		unsigned int len = bsssize;
		static const char zeros[16] = {0};
		ret = module_output_start_segment(output, MODULE_SEG_BSS,
						  this_module->seg_info.bss.address,bsssize);
		if (ret != MODULE_OK) {
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
		if (ret != len) {
			return MODULE_OUTPUT_ERROR;
		}
	}

	module = find_local_symbol(input_addr, "mod_entry", symtaboff, symtabsize, strtaboff);
	if(module != NULL) {
		//printk("module-loader: autostart found\n");
		this_module->entry = module;
		return MODULE_OK;
	} else {
		printk("module-loader: no autostart\n");
		module = find_module_entry(input_addr, symtaboff, symtabsize, strtaboff);
		if(module != NULL) {
			printk("module-loader: FOUND PRG\n");
		}
		return MODULE_NO_STARTPOINT;
	}
}
