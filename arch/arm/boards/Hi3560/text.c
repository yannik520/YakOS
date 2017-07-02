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
#include <arch/text.h>
#include <kernel/semaphore.h>
#include <kernel/task.h>
#include <arch/interrupts.h>
#include <arch/platform.h>
#include <string.h>

volatile SERIAL_PORT uart = {
	.fifo_out = {0},
	.fifo_in  = {0},
	.puts	  = __puts_early,
	.getchar  = __getchar_early,
};
DEFINE_SEMAPHORE(serial_sem);

void __puts_early(const char *str)
{
	while (*str) {
		while (UART_FR & UART_FR_TXFF);

		UART_DR = *str;

		if (*str == '\n') {
			while (UART_FR & UART_FR_TXFF);

			UART_DR = '\r';
		}
		str++;
	}
	while (UART_FR & UART_FR_BUSY);
}

void __puts(const char *str)
{
	down(&serial_sem);
	while (*str) {
		uart.fifo_out.mem[uart.fifo_out.wp] = *str;
		uart.fifo_out.wp = (uart.fifo_out.wp + 1) % UART_FIFO_SIZE;
		if (*str == '\n') {
			uart.fifo_out.mem[uart.fifo_out.wp] = '\r';
			uart.fifo_out.wp = (uart.fifo_out.wp + 1) % UART_FIFO_SIZE;
		}
		str++;
	}

	while (0 == (UART_FR & UART_FR_TXFF)) {
		if (uart.fifo_out.rp != uart.fifo_out.wp)
		{
			UART_DR = (unsigned int)uart.fifo_out.mem[uart.fifo_out.rp];
			uart.fifo_out.rp = (uart.fifo_out.rp + 1) % UART_FIFO_SIZE;
		}
		else {
			break;
		}
	}
	up(&serial_sem);
}

void putchar(char c)
{
	while (UART_FR & UART_FR_TXFF);
	UART_DR = c;
	while (UART_FR & UART_FR_BUSY);
}

unsigned char  __getchar_early(void)
{
	unsigned int data = 0;

	while (UART_FR & UART_FR_RXFE);
	
	data = UART_DR;

	if (data & 0xFFFFFF00) {
		UART_RSR = (unsigned int)0xFFFFFFFF;
		return -1;
	}

	return (unsigned char)(data & 0xFF);
}

unsigned char __getchar(void)
{
	unsigned int data;

	while(uart.fifo_in.rp == uart.fifo_in.wp);
	
	down(&serial_sem);
	data = uart.fifo_in.mem[uart.fifo_in.rp];
	uart.fifo_in.rp = (uart.fifo_in.rp + 1) % UART_FIFO_SIZE;
	up(&serial_sem);

	return data;
}

static handler_return platform_uart(void *arg)
{
	unsigned int reg;
	unsigned int data;
	
	reg = UART_MIS;
	
	if (reg & UART_IMSC_TXIM)
	{
		while (0 == (UART_FR & UART_FR_TXFF)) {
			if (uart.fifo_out.rp != uart.fifo_out.wp)
			{
				UART_DR = (unsigned int)uart.fifo_out.mem[uart.fifo_out.rp];
				uart.fifo_out.rp = (uart.fifo_out.rp + 1) % UART_FIFO_SIZE;
			}
			else {
				break;
			}
		}
		UART_ICR = UART_IMSC_TXIM;
	}

	if (reg & UART_IMSC_RXIM)
	{
		while (0 == (UART_FR & UART_FR_RXFE)) {
			data = UART_DR;
			if (data & 0xFFFFFF00) {
				UART_RSR = (unsigned int)0xFFFFFFFF;
				uart.fifo_in.mem[uart.fifo_in.wp] =  -1;
			}
			else {
				uart.fifo_in.mem[uart.fifo_in.wp] = (unsigned char)(data & 0xFF);
			}
			uart.fifo_in.wp = (uart.fifo_in.wp + 1) % UART_FIFO_SIZE;
		}
		UART_ICR = UART_IMSC_RXIM;
	}

	unmask_interrupt(INTNR_UART0);

	return INT_NO_RESCHEDULE;
}

static void serial_init(void)
{
	unsigned int temp;
	unsigned int divider;
	unsigned int remainder;
	unsigned int fraction;

	/*
	 ** First, disable everything.
	 */
	UART_CR = 0x0;
	/*
	 ** Set baud rate
	 **
	 ** IBRD = UART_CLK / (16 * BAUD_RATE)
	 ** FBRD = ROUND((64 * MOD(UART_CLK,(16 * BAUD_RATE))) / (16 * BAUD_RATE))
	 */
	temp = 16 * UART_BAUD_115200;
	divider = CONFIG_CLOCK / temp;
	remainder = CONFIG_CLOCK % temp;
	temp = (8 * remainder) / UART_BAUD_115200;
	fraction = (temp >> 1) + (temp & 1);

	UART_IBRD = divider;
	UART_FBRD = fraction;

	/*
	 ** Set the UART to be 8 bits, 1 stop bit, no parity, fifo enabled.
	 */
	//UART_LCR_H = UART_LCRH_WLEN_8 | UART_LCRH_FEN;
	UART_LCR_H = UART_LCRH_WLEN_8;

	/*
	** Enable Interrupt
	*/
	UART_IMSC = UART_IMSC_RXIM | UART_IMSC_TXIM;
	
	/*
	 ** Finally, enable the UART
	 */
	UART_CR = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
}

void _console_init(void)
{
	enter_critical_section();

	uart.puts = __puts;
	uart.getchar = __getchar;
	register_int_handler(INTNR_UART0, &platform_uart, 0);
	unmask_interrupt(INTNR_UART0);
	serial_init();

	exit_critical_section();
}
