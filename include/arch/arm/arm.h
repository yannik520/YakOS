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
#ifndef __ARM_H__
#define __ARM_H__

#include <arch/types.h>

#define DSB __asm__ volatile("dsb" ::: "memory")
#define ISB __asm__ volatile("isb" ::: "memory")

/*
 * CPSR/SPSR bits
 */
#define USR26_MODE	0x00000000
#define FIQ26_MODE	0x00000001
#define IRQ26_MODE	0x00000002
#define SVC26_MODE	0x00000003
#define USR_MODE	0x00000010
#define FIQ_MODE	0x00000011
#define IRQ_MODE	0x00000012
#define SVC_MODE	0x00000013
#define ABT_MODE	0x00000017
#define UND_MODE	0x0000001b
#define SYSTEM_MODE	0x0000001f
#define MODE32_BIT	0x00000010
#define MODE_MASK	0x0000001f
#define PSR_T_BIT	0x00000020
#define PSR_F_BIT	0x00000040
#define PSR_I_BIT	0x00000080
#define PSR_A_BIT	0x00000100
#define PSR_E_BIT	0x00000200
#define PSR_J_BIT	0x01000000
#define PSR_Q_BIT	0x08000000
#define PSR_V_BIT	0x10000000
#define PSR_C_BIT	0x20000000
#define PSR_Z_BIT	0x40000000
#define PSR_N_BIT	0x80000000

#define PSR_ENDSTATE	0

/*
 * CR1 bits (CP#15 CR1)
 */
#define CR_M	(1 << 0)	/* Enable or disable the MMU 1:enable 0:disable */
#define CR_A	(1 << 1)	/* Alignment abort enable 1:enable 0:disable */
#define CR_C	(1 << 2)	/* Enables or disables level one data cache 1:enable 0:disable */
#define CR_B	(1 << 7)	/* Determines operation as little-endian or big-endian 1:Big-endian 0:Little-endian */
#define CR_Z	(1 << 11)	/* Enables programme flow prediction 1:enable 0:disable */
#define CR_I	(1 << 12)	/* Enable or disable level one instruction cache 1:enable 0:disable */
#define CR_V	(1 << 13)	/* Determines the location of exception vectors 0:0x00000000-0x0000001C 1:0xFFFF0000-0xFFFF001C */
#define CR_RR	(1 << 14)	/* Determines the replacement strategy for the cache 1:Predictable replacement strategy 0:Normal replacement strategy */
#define CR_L4	(1 << 15)	/* Determines if the T bit is set for PC load instructions 1:Loads to PC do not set the T bit, ARMv4 behavior 0:Loads to PC set the T bit */
#define CR_FI	(1 << 21)	/* Configures low latency features for fast interrupts 1:Low interrupt latency configuration enabled. 0:All performance features enabled */
#define CR_U	(1 << 22)	/* Enables unaligned data access operations for mixed little-endian and big-endian operation 1:enable 0:disable */
#define CR_VE	(1 << 24)	/* Enables the VIC interface to determine interrupt vectors 1:enable 0:disable */
#define CR_EE	(1 << 25)	/* Determines how the E bit in the CPSR bit is set on an exception 1:CPSR E bit is set to 1 on an exception 0:CPSR E bit is set to 0 on an exception */
#define CR_NMI	(1 << 27)	/* Determines the state of the non-maskable bit that is set by a configuration pin FIQISNMI */
#define CR_TE	(1 << 30)	/* Determines the state that the processor enters exceptions 1:Exceptions entered in Thumb state 0:Exceptions entered in ARM state */

/*
 * Domain
 */
#define domain_val(dom,type)	((type) << (2*(dom)))

#define DOMAIN_KERNEL	0
#define DOMAIN_TABLE	0
#define DOMAIN_USER	1
#define DOMAIN_IO	2

/*
 * Domain types
 */
#define DOMAIN_NOACCESS	0
#define DOMAIN_CLIENT	1
#define DOMAIN_MANAGER	3


/*
@ Bad Abort numbers
@ -----------------
@
*/
#define BAD_PREFETCH	0
#define BAD_DATA	1
#define BAD_ADDREXCPTN	2
#define BAD_IRQ		3
#define BAD_UNDEFINSTR	4


#define TTB_SA              (1 << 20)   /* Section base address */
#define TTB_CPTA            (1 << 10)   /* Coarse page table base address */
#define TTB_FPTA            (1 << 12)   /* Fine page table base address */
#define TTB_AP              (3 << 10)   /* Access permission bits */
#define TTB_AP_WR           (0x3 << 10)       /* Read/Write */
#define TTB_AP_R            (0x2 << 10)       /* Read-only */
#define TTB_AP_NC           (0x1)       /* No access */
#define TTB_DOMAIN          (0xF << 5)    /* Domain control bits */


#define TTB_SPECIAL         (1 << 4)    /* Must be 1 */
#define TTB_CACHEABLE       (1 << 3)    /* cacheable */
#define TTB_BUFFERABLE      (1 << 2)    /* bufferable */
#define TTB_CPTD	    (1)         /* Indicates that this is a coarse page table descriptor */
#define TTB_SD              (2)         /* Indicates that this is a section descriptor */
#define TTB_FINE_PGTD       (3)         /* Indicates that this is a fine page table descriptor */
#define TTB_SECTION_SIZE    0x00100000

#define TTB_SPGTD_AP0_MASK	    (3 << 4)
#define TTB_SPGTD_AP1_MASK	    (3 << 6)
#define TTB_SPGTD_AP2_MASK	    (3 << 8)
#define TTB_SPGTD_AP3_MASK	    (3 << 10)
#define TTB_SPGTD_AP0_WR	    (3 << 4)
#define TTB_SPGTD_AP1_WR	    (3 << 6)
#define TTB_SPGTD_AP2_WR	    (3 << 8)
#define TTB_SPGTD_AP3_WR	    (3 << 10)
#define TTB_SPGTD_CACHEABLE	    (1 << 3)
#define TTB_SPGTD_BUFFERABLE	    (1 << 2)
#define TTB_SPGTD_INVALID	    (0x00)
#define TTB_SPGDT_LARGE_PAGE        (0x01)    /* 64K page */
#define TTB_SPGDT_SMALL_PAGE        (0x02)    /* 4K page */
#define TTB_SPGDT_TINY_PAGE         (0x03)    /* 1K page */

struct arm_fault_frame {
	uint32_t spsr;
	uint32_t usp;
	uint32_t ulr;
	uint32_t r[13];
	uint32_t pc;
};

#define MODE_MASK 0x1f
#define MODE_USR 0x10
#define MODE_FIQ 0x11
#define MODE_IRQ 0x12
#define MODE_SVC 0x13
#define MODE_MON 0x16
#define MODE_ABT 0x17
#define MODE_UND 0x1b
#define MODE_SYS 0x1f

struct arm_mode_regs {
	uint32_t fiq_r13, fiq_r14;
	uint32_t irq_r13, irq_r14;
	uint32_t svc_r13, svc_r14;
	uint32_t abt_r13, abt_r14;
	uint32_t und_r13, und_r14;
	uint32_t sys_r13, sys_r14;
};

void arm_save_mode_regs(struct arm_mode_regs *regs);

#endif
