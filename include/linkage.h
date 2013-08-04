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
#ifndef __LINKAGE_H__
#define __LINKAGE_H__

#ifdef __cplusplus
#define CPP_ASMLINKAGE extern "C"
#else
#define CPP_ASMLINKAGE
#endif

#ifndef __ALIGN
#define __ALIGN		.align 4,0x90
#define __ALIGN_STR	".align 4,0x90"
#endif

#ifndef asmlinkage
#define asmlinkage CPP_ASMLINKAGE
#endif

#ifdef __ASSEMBLY__

#ifndef LINKER_SCRIPT
#define ALIGN __ALIGN
#define ALIGN_STR __ALIGN_STR

#ifndef ENTRY
#define ENTRY(name) \
  .globl name; \
  ALIGN; \
  name:
#endif /* ENTRY */
#endif /* LINKER_SCRIPT */
#endif /* __ASSEMBLY__ */

#endif /* __LINKAGE_H__ */
