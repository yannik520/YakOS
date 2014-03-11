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
#ifndef __ELF_H__
#define __ELF_H__

#define EI_NIDENT 16

typedef unsigned long	elf32_word;
typedef signed long	elf32_sword;
typedef unsigned short	elf32_half;
typedef unsigned long	elf32_off;
typedef unsigned long	elf32_addr;

struct elf32_rela {
	elf32_addr      r_offset;       /* Location to be relocated. */
	elf32_word      r_info;         /* Relocation type and symbol index. */
	elf32_sword     r_addend;       /* Addend. */
};

struct elf32_ehdr {
	unsigned char	e_ident[EI_NIDENT];	/* ident bytes */
	elf32_half	e_type;		/* file type */
	elf32_half	e_machine;	/* target machine */
	elf32_word	e_version;	/* file version */
	elf32_addr	e_entry;	/* start address */
	elf32_off	e_phoff;	/* phdr file offset */
	elf32_off	e_shoff;	/* shdr file offset */
	elf32_word	e_flags;	/* file flags */
	elf32_half	e_ehsize;	/* sizeof ehdr */
	elf32_half	e_phentsize;	/* sizeof phdr */
	elf32_half	e_phnum;	/* number phdrs */
	elf32_half	e_shentsize;	/* sizeof shdr */
	elf32_half	e_shnum;	/* number shdrs */
	elf32_half	e_shstrndx;	/* shdr string index */
};

/* Values for e_type. */
#define ET_NONE         0       /* Unknown type. */
#define ET_REL          1       /* Relocatable. */
#define ET_EXEC         2       /* Executable. */
#define ET_DYN          3       /* Shared object. */
#define ET_CORE         4       /* Core file. */

struct elf32_shdr {
	elf32_word	sh_name;	/* section name */
	elf32_word	sh_type;	/* SHT_... */
	elf32_word	sh_flags;	/* SHF_... */
	elf32_addr	sh_addr;	/* virtual address */
	elf32_off	sh_offset;	/* file offset */
	elf32_word	sh_size;	/* section size */
	elf32_word	sh_link;	/* misc info */
	elf32_word	sh_info;	/* misc info */
	elf32_word	sh_addralign;	/* memory alignment */
	elf32_word	sh_entsize;	/* entry size if table */
};

/* sh_type */
#define SHT_NULL        0               /* inactive */
#define SHT_PROGBITS    1               /* program defined information */
#define SHT_SYMTAB      2               /* symbol table section */
#define SHT_STRTAB      3               /* string table section */
#define SHT_RELA        4               /* relocation section with addends*/
#define SHT_HASH        5               /* symbol hash table section */
#define SHT_DYNAMIC     6               /* dynamic section */
#define SHT_NOTE        7               /* note section */
#define SHT_NOBITS      8               /* no space section */
#define SHT_REL         9               /* relation section without addends */
#define SHT_SHLIB       10              /* reserved - purpose unknown */
#define SHT_DYNSYM      11              /* dynamic symbol table section */
#define SHT_LOPROC      0x70000000      /* reserved range for processor */
#define SHT_HIPROC      0x7fffffff      /* specific section header types */
#define SHT_LOUSER      0x80000000      /* reserved range for application */
#define SHT_HIUSER      0xffffffff      /* specific indexes */

struct elf32_rel {
	elf32_addr      r_offset;       /* Location to be relocated. */
	elf32_word      r_info;         /* Relocation type and symbol index. */
};

struct elf32_sym {
	elf32_word      st_name;        /* String table index of name. */
	elf32_addr      st_value;       /* Symbol value. */
	elf32_word      st_size;        /* Size of associated object. */
	unsigned char   st_info;        /* Type and binding information. */
	unsigned char   st_other;       /* Reserved (not used). */
	elf32_half      st_shndx;       /* Section index of symbol. */
};

#define ELF32_R_SYM(info)       ((info) >> 8)
#define ELF32_R_TYPE(info)      ((unsigned char)(info))

struct relevant_section {
	unsigned char	 number;
	unsigned int	 offset;
	char		*address;
};

struct sec_info {
	elf32_off	s_offset;
	elf32_word	s_size;
	elf32_off	s_reloff;
	elf32_word	s_relsize;
};

enum SEC_TYPE {
	SEC_TYPE_TEXT,
	SEC_TYPE_RODATA,
	SEC_TYPE_DATA,
	SEC_TYPE_SYMTAB,
	SEC_TYPE_STRTAB,
	SEC_TYPE_MAX
};

#endif
