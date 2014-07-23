/*
 * Copyright (c) 2005-2010 Travis Geiselbrecht
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
#ifndef __MEMMAP_H
#define __MEMMAP_H

#define MEMBANK_SIZE (128*1024*1024)

/* some helpful macros */
//#define REG64(x) ((volatile unsigned long long *)(x))
#define REG(x) ((volatile unsigned int *)(x))
#define REG_H(x) ((volatile unsigned short *)(x))
#define REG_B(x) ((volatile unsigned char *)(x))

/* memory map of our generic arm system */
// XXX make more dynamic
#define MAINMEM_BASE 0xc0000000
#define MAINMEM_SIZE (MEMBANK_SIZE)

/* Exception vectors table */
#define EXCEPTION_SIZE		0x100000
#define EXCEPTION_BASE	0xffff0000
//#define EXCEPTION_BASE		0x0

#define REGISTER_BASE 0x10100000
#define REGISTER_SIZE 0x100000

#endif
