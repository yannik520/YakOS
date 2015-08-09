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

#ifndef __TEXT_H
#define __TEXT_H
#include <arch/chip_regs.h>
#include <arch/memory.h>
#include <kernel/types.h>
#include <kernel/reg.h>

//#define BOOTUP_UART_BASE (uint32_t)phys_to_virt(REG_BASE_UART0)

#define UART_DR		(*(volatile unsigned int *)(REG_BASE_UART0 + 0x000))
#define UART_RSR	(*(volatile unsigned int *)(REG_BASE_UART0 + 0x004))
#define UART_FR		(*(volatile unsigned int *)(REG_BASE_UART0 + 0x018))
#define UART_IBRD	(*(volatile unsigned int *)(REG_BASE_UART0 + 0x024))
#define UART_FBRD	(*(volatile unsigned int *)(REG_BASE_UART0 + 0x028))
#define UART_LCR_H	(*(volatile unsigned int *)(REG_BASE_UART0 + 0x02C))
#define UART_CR		(*(volatile unsigned int *)(REG_BASE_UART0 + 0x030))
#define UART_IFLS	(*(volatile unsigned int *)(REG_BASE_UART0 + 0x034))
#define UART_IMSC	(*(volatile unsigned int *)(REG_BASE_UART0 + 0x038))
#define UART_RIS	(*(volatile unsigned int *)(REG_BASE_UART0 + 0x03C))
#define UART_MIS	(*(volatile unsigned int *)(REG_BASE_UART0 + 0x040))
#define UART_ICR	(*(volatile unsigned int *)(REG_BASE_UART0 + 0x044))
#define UART_DMACR	(*(volatile unsigned int *)(REG_BASE_UART0 + 0x048))

#define UART_RSR_OE               0x08
#define UART_RSR_BE               0x04
#define UART_RSR_PE               0x02
#define UART_RSR_FE               0x01

#define UART_FR_TXFE              0x80
#define UART_FR_RXFF              0x40
#define UART_FR_TXFF              0x20
#define UART_FR_RXFE              0x10
#define UART_FR_BUSY              0x08
#define UART_FR_TMSK              (UART_FR_TXFF + UART_FR_BUSY)

#define UART_LCRH_SPS             (1 << 7)
#define UART_LCRH_WLEN_8          (3 << 5)
#define UART_LCRH_WLEN_7          (2 << 5)
#define UART_LCRH_WLEN_6          (1 << 5)
#define UART_LCRH_WLEN_5          (0 << 5)
#define UART_LCRH_FEN             (1 << 4)
#define UART_LCRH_STP2            (1 << 3)
#define UART_LCRH_EPS             (1 << 2)
#define UART_LCRH_PEN             (1 << 1)
#define UART_LCRH_BRK             (1 << 0)

#define UART_CR_CTSEN             (1 << 15)
#define UART_CR_RTSEN             (1 << 14)
#define UART_CR_RTS               (1 << 11)
#define UART_CR_DTR               (1 << 10)
#define UART_CR_RXE               (1 << 9)
#define UART_CR_TXE               (1 << 8)
#define UART_CR_LBE               (1 << 7)
#define UART_CR_UARTEN            (1 << 0)

#define UART_IMSC_OEIM            (1 << 10)
#define UART_IMSC_BEIM            (1 << 9)
#define UART_IMSC_PEIM            (1 << 8)
#define UART_IMSC_FEIM            (1 << 7)
#define UART_IMSC_RTIM            (1 << 6)
#define UART_IMSC_TXIM            (1 << 5)
#define UART_IMSC_RXIM            (1 << 4)

#define UART_BAUD_460800            460800
#define UART_BAUD_230400            230400
#define UART_BAUD_115200            115200
#define UART_BAUD_57600             57600
#define UART_BAUD_38400             38400
#define UART_BAUD_19200             19200
#define UART_BAUD_14400             14400
#define UART_BAUD_9600              9600
#define UART_BAUD_4800              4800
#define UART_BAUD_2400              2400
#define UART_BAUD_1200              1200

#define CONFIG_CLOCK		54000000

#define UART_FIFO_SIZE 1024
typedef struct {
	unsigned char mem[UART_FIFO_SIZE];
	unsigned int wp;
	unsigned int rp;
}UART_FIFO;

typedef struct {
	UART_FIFO fifo_out;
	UART_FIFO fifo_in;
	void (*puts)(const char *);
	unsigned char (*getchar)(void);
}SERIAL_PORT;

void __puts_early(const char *str);
unsigned char  __getchar_early(void);
extern void _console_init(void);

#endif
