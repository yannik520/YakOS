TOOLCHAIN_PREFIX := arm-none-eabi-
CC := $(TOOLCHAIN_PREFIX)gcc
LD := $(TOOLCHAIN_PREFIX)ld
OBJDUMP := $(TOOLCHAIN_PREFIX)objdump
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
CPPFILT := $(TOOLCHAIN_PREFIX)c++filt
SIZE := $(TOOLCHAIN_PREFIX)size
NM := $(TOOLCHAIN_PREFIX)nm

CFLAGS := -O2 -g -fno-builtin -finline -W -Wall -Wno-multichar -Wno-unused-parameter -Wno-unused-function
CPPFLAGS := -fno-exceptions -fno-rtti -fno-threadsafe-statics
ASMFLAGS := -DASSEMBLY
LDFLAGS := 
LDFLAGS += -gc-sections


NOECHO ?= @

ALLOBJS := main.o setup.o task.o printf.o malloc.o
LINKER_SCRIPT := build.ld

all: bigeye.bin bigeye.elf bigeye.sym

bigeye.bin: bigeye.elf
	@echo generation image: $@
	$(NOECHO)$(SIZE) $<
	$(NOCOPY)$(OBJCOPY) -O binary $< $@

bigeye.elf: $(ALLOBJS) $(LINKER_SCRIPT)
	@echo linking $@
	$(noecho)$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) $(ALLOBJS) -o $@
bigeye.sym: bigeye.elf
	@echo generating listing: $@
	$(NOECHO)$(OBJDUMP) -Mreg-names-raw -S $< | $(CPPFILT) > $@

clean:
	@rm -rf *.bin *.elf *.sym *.o