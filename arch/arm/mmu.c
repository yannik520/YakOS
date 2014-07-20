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
#include <arch/arm.h>
#include <arch/mmu.h>
#include <arch/memmap.h>
#include <arch/memory.h>
#include <kernel/types.h>
#include <kernel/malloc.h>
#include <kernel/printk.h>

#define MB	(1024 * 1024)


uint32_t *	pgd_base = (uint32_t *)PAGE_OFFSET;


static inline uint32_t arm_read_cr1(void)
{
	uint32_t reg_val = 0;

	__asm__ __volatile__ (
		"mrc p15, 0, %0, c1, c0, 0\n"
		: "=r"(reg_val) /* output */
		: /* no input */
		);
	
	return reg_val;
}

static inline void arm_write_cr1(uint32_t reg_val)
{

	__asm__ __volatile__ (
		"mcr p15, 0, %0, c1, c0, 0\n"
		: /* no output */
		: "r"(reg_val) /* input */
		);
}

static inline void arm_write_dacr(uint32_t reg_val)
{
	__asm__ __volatile__ (
		"mcr p15, 0, %0, c3, c0, 0\n"
		: /* no output */
		: "r"(reg_val) /* input */
		);
}

static inline void arm_write_ttbr(uint32_t reg_val)
{
	__asm__ __volatile__ (
		"mcr p15, 0, %0, c2, c0, 0\n"
		: /* no output */
		: "r"(reg_val) /* input */
		);
}

static inline void arm_invalidate_tlb(void)
{
	__asm__ __volatile__ (
		"mov r0, #0\n"
		"mcr p15, 0, r0, c8, c7, 0\n"
		: /* no output */
		: /* no input */
		);
}

static inline void armv4_mmu_cache_off(void)
{
	__asm__ __volatile__ (
		"mrc p15, 0, r0, c1, c0, 0\n" /* read cr1 */
		"bic r0, r0, #0x000d\n"
		"mcr p15, 0, r0, c1, c0, 0\n" /* turn MMU and cache off */
		"mov r0, #0\n"
		"mcr p15, 0, r0, c7, c7, 0\n" /* invalidate whole cache v4 */
		"mcr p15, 0, r0, c8, c7, 0\n" /* invalidate whole TLB v4 */
		: /* no output */
		: /* no input */
		);
}

static inline void flush_pgd_entry(pgd_t *gpd)
{
	asm("mcr p15, 0, %0, c7, c10, 1"
	    :
	    :"r" (gpd)
	    :"cc");
	DSB;
}

void arm_mmu_map_section (addr_t vaddr, addr_t paddr, uint32_t flags)
{
	uint32_t	 AP  = 0;
	uint32_t	 CB  = 0;
	pgd_t		*pgd = pgd_offset(pgd_base, vaddr);

	AP   = flags &	TTB_AP;
	CB   = (flags & TTB_CACHEABLE) |
		(flags & TTB_BUFFERABLE);
	*pgd = (pgd_t)((paddr & SECTION_MASK) |
		       AP | CB | TTB_SD);

	//flush_pgd_entry(pgd);
	arm_invalidate_tlb();
}

void arm_mmu_unmap_section(addr_t vaddr)
{
	pgd_t	*pgd = pgd_offset(pgd_base, vaddr);
	*pgd	     = 0;
	flush_pgd_entry(pgd);
}

static pte_t *pte_offset(pte_t *pt, unsigned long virtual) {
	int index = (virtual & ~SECTION_MASK) >> PAGE_SHIFT;
	return (pt + index);
}

void arm_mmu_map_page(addr_t vaddr, addr_t paddr, uint32_t flags)
{
	pgd_t		*pgd		      = pgd_offset(pgd_base, vaddr);
	pgd_t		 pgd_value;
	pte_t		 pte_value, *pte, *pt = NULL;
	uint32_t	 AP;
	uint32_t	 CB;

	AP = flags & TTB_SPGTD_AP0_MASK; /* AP0 */
	AP |= flags & TTB_SPGTD_AP1_MASK; /* AP1 */
	AP |= flags & TTB_SPGTD_AP2_MASK; /* AP2 */
	AP |= flags & TTB_SPGTD_AP3_MASK; /* AP3 */
	
	CB = flags & TTB_SPGTD_CACHEABLE; /* C bit */
	CB |= flags & TTB_SPGTD_BUFFERABLE; /* B bit */
	
	pgd = (pgd_t *)ALIGN(pgd, 4);
	//printk("pgd=0x%x, paddr=0x%x, vaddr=0x%x\n", pgd, paddr, vaddr);
	//printk("AP|CB=0x%x\n", AP|CB);
	/* One coarse page table contain 256 page table entries,
	one entry consumed 4 bytes memory, so one coarse totally
	consumed 4*256=1k bytes memory. */
	if (NULL == pgd) {
		int i;

		/* allock memmory to store page table */
		pt = kmalloc(PAGE_SIZE); //4k bytes
		//printk("pt=0x%x\n", pt);
		for (i=0; i < 4; i++) {
			pgd_value = (virt_to_phys(pt) + i * 256 * sizeof(pte_t)) | TTB_CPTD;
			pgd[i] = pgd_value;
			//printk("i=%d, pgd_value=0x%x\n", i, pgd_value);
			//flush_pgd_entry(&pgd[i]);
		}
	}

	pt	  = (pte_t *)ALIGN(phys_to_virt((uint32_t)*pgd), 1024);
	pte	  = pte_offset(pt, vaddr);
	pte_value = (paddr & PAGE_MASK) | AP | CB;
	*pte	  = pte_value;
	//printk("pte=0x%x, pte_value=0x%x\n", pte, pte_value);
}

void arm_mmu_create_mapping(struct map_desc *md)
{
	if ( NULL == md) {
		printk("%s %d: NULL pointer\n", __FILE__, __LINE__);
	}

	switch(md->type) {
	case MAP_DESC_TYPE_SECTION:
		if ( (md->paddr & (~SECTION_MASK)) ||
		     (md->vaddr & (~SECTION_MASK)) ||
		     (md->length & (~SECTION_MASK))) {
			printk("%s %d: section address is not aligned\n", __FILE__, __LINE__);
			return;
		} else {
			addr_t paddr = md->paddr;
			addr_t vaddr = md->vaddr;
			
			while (vaddr < (md->vaddr + md->length)) {
				arm_mmu_map_section(vaddr, paddr, md->attr);
				paddr += SECTION_SIZE;
				vaddr += SECTION_SIZE;
			}
		}
		break;
	case MAP_DESC_TYPE_PAGE:
		if ((md->paddr & (~PAGE_MASK)) ||
		    (md->vaddr & (~PAGE_MASK)) ||
		    (md->length & (~PAGE_MASK))) {
			printk("%s %d: page address is not aligned\n", __FILE__, __LINE__);
			return;
		} else {
			addr_t paddr = md->paddr;
			addr_t vaddr = md->vaddr;
			
			while (vaddr < (md->vaddr + md->length)) {
				arm_mmu_map_page(vaddr, paddr, md->attr);
				paddr += PAGE_SIZE;
				vaddr += PAGE_SIZE;
			}
		}
		break;
	default:
		printk("%s %d: please give a valid map type\n");
		break;
	}
}

static void arm_mmu_map_low_memory() {
	struct map_desc map;
	map.paddr  = PHYS_OFFSET;
	map.vaddr  = PAGE_OFFSET;
	map.length = MEMBANK_SIZE;
	map.attr   = TTB_AP_WR | TTB_CACHEABLE | TTB_BUFFERABLE;
	map.type   = MAP_DESC_TYPE_SECTION;
	arm_mmu_create_mapping(&map);
}

static void arm_mmu_map_vector_memory() {
	struct map_desc map;
	//map.paddr  = __virt_to_phys((unsigned long)kmalloc(PAGE_SIZE));
	map.paddr  = 0;
	map.vaddr  = EXCEPTION_BASE;
	map.length = SECTION_SIZE;
	map.attr   = TTB_AP_WR;
	map.type   = MAP_DESC_TYPE_SECTION;
	arm_mmu_create_mapping(&map);
}

static void arm_mmu_map_register() {
	struct map_desc map;
	map.paddr  = REGISTER_BASE;
	map.vaddr  = REGISTER_VADDR;
	map.length = REGISTER_SIZE;
	map.attr   = TTB_AP_WR;
	map.type   = MAP_DESC_TYPE_SECTION;
	arm_mmu_create_mapping(&map);
}

void arm_mmu_init(void)
{
	armv4_mmu_cache_off();
	arm_mmu_map_low_memory();
	arm_mmu_map_vector_memory();
	arm_mmu_map_register();
	arm_write_ttbr(PAGE_OFFSET);
	arm_write_dacr(DOMAIN_CLIENT);
	/* turn on the mmu */
	arm_write_cr1(arm_read_cr1() | 0x1);
}
