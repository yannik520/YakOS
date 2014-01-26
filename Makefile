ARCH ?= arm
BOARD ?= demo
TARGET ?= lynx_kimage
TARGET_BIN ?= $(TARGET).bin
TARGET_ELF ?= $(TARGET).elf
TARGET_SYM ?= $(TARGETE).sym

export TOPDIR=$(shell pwd)
TOOLCHAIN_PATH += $(TOPDIR)/tools/arm-elf-toolchain/bin/
TOOLCHAIN_PREFIX := $(TOOLCHAIN_PATH)arm-elf-
CC := $(TOOLCHAIN_PREFIX)gcc
LD := $(TOOLCHAIN_PREFIX)ld
OBJDUMP := $(TOOLCHAIN_PREFIX)objdump
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
CPPFILT := $(TOOLCHAIN_PREFIX)c++filt
SIZE := $(TOOLCHAIN_PREFIX)size
NM := $(TOOLCHAIN_PREFIX)nm

PLATFORM_LIBGCC := -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc
CFLAGS := -O2 -g -Iinclude -fno-builtin -finline -W -Wall -Wno-multichar -Wno-unused-parameter -Wno-unused-function
#CPPFLAGS := -fno-exceptions -fno-rtti -fno-threadsafe-statics
ASFLAGS := -DASSEMBLY -D__ASSEMBLY__ -Iinclude
LDFLAGS := 
LDFLAGS += -gc-sections

NOECHO ?= @

ALLOBJS := \
	kernel/main.o \
	kernel/sched_fifo.o \
	kernel/sched.o \
	kernel/task.o \
	kernel/printk.o \
	kernel/malloc.o \
	kernel/timer.o \
	kernel/semaphore.o \
	init/init_shell.o \
	lib/string.o

include arch/$(ARCH)/Makefile
include arch/$(ARCH)/boot/Makefile
include arch/$(ARCH)/boards/$(BOARD)/Makefile

LINKER_SCRIPT_TEMPLETE := arch/$(ARCH)/boot/build.ld
LINKER_SCRIPT := build.ld

ALL_INCLUDE_FILES := $(wildcard ./include/arch/$(ARCH)/boards/$(BOARD)/*.h)

all: prepare $(TARGET_BIN) $(TARGET_ELF) $(TARGET_SYM)

prepare: prepare_include build.ld

.PHONY: prepare_env prepare_include build.ld
prepare_env:
	$(shell source ./env_setting.sh)

prepare_include:
	echo $(ALL_INCLUDE_FILES)
	@cp $(wildcard ./include/arch/$(ARCH)/boards/$(BOARD)/*.h) ./include/arch
	@cp $(wildcard ./include/arch/$(ARCH)/*.h) ./include/arch

build.ld: $(LINKER_SCRIPT_TEMPLETE)
	@sed "s/%MEMBASE%/$(MEMBASE)/;s/%STACKSIZE%/$(STACKSIZE)/" < $< > $@

$(TARGET_BIN): $(TARGET_ELF)
	@echo generation image: $@
	$(NOECHO)$(SIZE) $<
	$(NOCOPY)$(OBJCOPY) -O binary $< $@

$(TARGET_ELF): $(ALLOBJS) $(LINKER_SCRIPT)
	@echo linking $@
	$(noecho)$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) $(ALLOBJS) -o $@ $(PLATFORM_LIBGCC)
$(TARGET_SYM): $(TARGET_ELF)
	@echo generating listing: $@
	$(NOECHO)$(OBJDUMP) -Mreg-names-raw -S $< | $(CPPFILT) > $@

clean:
	@rm -rf *.bin *.elf *.sym $(ALLOBJS)

distclean: clean
	@rm -rf ./include/arch/*.h
