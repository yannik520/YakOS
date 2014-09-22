HOST ?= macosx
ARCH ?= arm
TARGET ?= yakOS
TARGET_BIN ?= $(TARGET).bin
TARGET_ELF ?= $(TARGET).elf
TARGET_SYM ?= $(TARGETE).sym

export TOPDIR=$(shell pwd)
TOOLCHAIN_PATH += $(TOPDIR)/tools/arm-elf-toolchain-$(HOST)/bin/
TOOLCHAIN_PREFIX := $(TOOLCHAIN_PATH)arm-elf-
CC := $(TOOLCHAIN_PREFIX)gcc
LD := $(TOOLCHAIN_PREFIX)ld
OBJDUMP := $(TOOLCHAIN_PREFIX)objdump
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
STRIP := $(TOOLCHAIN_PREFIX)strip
CPPFILT := $(TOOLCHAIN_PREFIX)c++filt
SIZE := $(TOOLCHAIN_PREFIX)size
NM := $(TOOLCHAIN_PREFIX)nm

SHELL    = /bin/bash
HOSTCC   = cc
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
                else if [ -x /bin/bash ]; then echo /bin/bash; \
                else echo sh; fi ; fi)

ifeq (.config,$(wildcard .config))
    include .config
else
    $(warning You have not .config file, please run make menuconfig first!)
    $(shell exit 1)
endif

define ask_to_save
	@if [ ! -f .config ]; then \
		echo; \
		echo "You have not saved your config, please re-run make config"; \
		echo; \
		exit 1; \
	else \
		PNAME="yakOS_config" && \
		echo -n "Save a copy to profiles/$${PNAME}.list [YES/NO] ? " && \
		read YESNO && \
		if [ "a$${YESNO}" = "aYES" ]; then \
			echo "Save to profiles/$${PNAME}.list "; \
			cp -f .config ./profiles/$${PNAME}.list; \
		else \
			echo "NOT save to profiles/$${PNAME}.list, only .config exist "; \
		fi; \
		echo ""; \
		echo "Done, pls do \"make all\" to build all images"; \
	fi
endef


PLATFORM_LIBGCC := -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc
CFLAGS := -O2 -g -Iinclude -fno-builtin -finline -W -Wall -Wno-multichar -Wno-unused-parameter -Wno-unused-function
#CPPFLAGS := -fno-exceptions -fno-rtti -fno-threadsafe-statics
ASFLAGS := -DASSEMBLY -D__ASSEMBLY__ -Iinclude
LDFLAGS := 
LDFLAGS += -gc-sections

NOECHO ?= @

ifeq ("x$(CONFIG_BUILD_BOARD_DEMO)", "xy")
	BOARD ?= demo
else
	BOARD ?= qemu_mini2440
endif


ALLOBJS-y := \
	lib/string.o

include arch/$(ARCH)/Makefile
include arch/$(ARCH)/boot/Makefile
include arch/$(ARCH)/boards/$(BOARD)/Makefile
include kernel/Makefile
include mm/Makefile
include modules/Makefile
include fs/Makefile
include init/Makefile

LINKER_SCRIPT_TEMPLETE := arch/$(ARCH)/boot/build.ld
LINKER_SCRIPT := build.ld

ALL_INCLUDE_FILES := $(wildcard ./include/arch/$(ARCH)/boards/$(BOARD)/*.h)

ALLOBJS-m := $(ALLOBJS-m:%.o=%.ko)

.PHONY: all
all: prepare $(TARGET_BIN) $(TARGET_ELF) $(TARGET_SYM) $(ALLOBJS-m)

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

$(TARGET_ELF): $(ALLOBJS-y) $(LINKER_SCRIPT)
	@echo linking $@
	$(noecho)$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) $(ALLOBJS-y) -o $@ $(PLATFORM_LIBGCC)
$(TARGET_SYM): $(TARGET_ELF)
	@echo generating listing: $@
	$(NOECHO)$(OBJDUMP) -Mreg-names-raw -S $< | $(CPPFILT) > $@


$(ALLOBJS-m): %.ko: %.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(STRIP) --strip-unneeded -g -x $@


.PHONY: menuconfig
menuconfig:
	scripts/mkconfig > Config.in
	@srctree=`pwd` HOSTCC=gcc make -f scripts/Makefile.build obj=scripts/basic
	@srctree=`pwd` HOSTCC=gcc make -f scripts/Makefile.build obj=scripts/kconfig/lxdialog
	@srctree=`pwd` HOSTCC=gcc make -f scripts/Makefile.build obj=scripts/kconfig/ menuconfig

	@$(call ask_to_save)

.PHONY: clean distclean
clean:
	@rm -rf *.bin *.elf .sym $(ALLOBJS-y) $(ALLOBJS-m)

distclean: clean
	@rm -rf .config Config.in .kconfig.d ./include/arch/*.h
