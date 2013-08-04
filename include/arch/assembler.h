#ifndef __ASSEMBLER_H__
#define __ASSEMBLER_H__

#ifndef asmlinkage
#define asmlinkage CPP_ASMLINKAGE
#endif

#ifdef __ASSEMBLY__

#define ALIGN __ALIGN
#define ALIGN_STR __ALIGN_STR

#define ENTRY(name) \
  .globl name; \
  ALIGN; \
  name:

#define KPROBE_ENTRY(name) \
  .section .kprobes.text, "ax"; \
  .globl name; \
  ALIGN; \
  name:

#endif


#endif
