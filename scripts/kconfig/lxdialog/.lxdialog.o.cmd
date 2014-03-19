cmd_scripts/kconfig/lxdialog/lxdialog.o := gcc -Wp,-MD,scripts/kconfig/lxdialog/.lxdialog.o.d    -DCURSES_LOC="<ncurses.h>" -DLOCALE    -c -o scripts/kconfig/lxdialog/lxdialog.o scripts/kconfig/lxdialog/lxdialog.c

deps_scripts/kconfig/lxdialog/lxdialog.o := \
  scripts/kconfig/lxdialog/lxdialog.c \
  /usr/include/stdc-predef.h \
  /usr/include/i386-linux-gnu/bits/predefs.h \
  scripts/kconfig/lxdialog/dialog.h \
  /usr/include/i386-linux-gnu/sys/types.h \
    $(wildcard include/config/.h) \
    $(wildcard include/config/e/offset64.h) \
    $(wildcard include/config/gefile64.h) \
    $(wildcard include/config/d.h) \
    $(wildcard include/config/en.h) \
    $(wildcard include/config/en2k8.h) \
    $(wildcard include/config/c.h) \
    $(wildcard include/config/x98.h) \
    $(wildcard include/config/ix199506.h) \
  /usr/include/features.h \
    $(wildcard include/config/c11.h) \
    $(wildcard include/config/c99.h) \
    $(wildcard include/config/c95.h) \
    $(wildcard include/config/ix.h) \
    $(wildcard include/config/ix2.h) \
    $(wildcard include/config/ix199309.h) \
    $(wildcard include/config/en/extended.h) \
    $(wildcard include/config/en2k.h) \
    $(wildcard include/config/en2kxsi.h) \
    $(wildcard include/config/en2k8xsi.h) \
    $(wildcard include/config/gefile.h) \
    $(wildcard include/config/ile.h) \
    $(wildcard include/config/ntrant.h) \
    $(wildcard include/config/tify/level.h) \
    $(wildcard include/config/cxx11.h) \
    $(wildcard include/config/i.h) \
    $(wildcard include/config/ix/implicitly.h) \
    $(wildcard include/config/ern/inlines.h) \
  /usr/include/i386-linux-gnu/sys/cdefs.h \
    $(wildcard include/config/espaces.h) \
  /usr/include/i386-linux-gnu/bits/wordsize.h \
  /usr/include/i386-linux-gnu/gnu/stubs.h \
  /usr/include/i386-linux-gnu/gnu/stubs-32.h \
  /usr/include/i386-linux-gnu/bits/types.h \
  /usr/include/i386-linux-gnu/bits/typesizes.h \
  /usr/include/time.h \
  /usr/lib/gcc/i686-linux-gnu/4.8/include/stddef.h \
  /usr/include/endian.h \
  /usr/include/i386-linux-gnu/bits/endian.h \
  /usr/include/i386-linux-gnu/bits/byteswap.h \
  /usr/include/i386-linux-gnu/bits/byteswap-16.h \
  /usr/include/i386-linux-gnu/sys/select.h \
  /usr/include/i386-linux-gnu/bits/select.h \
  /usr/include/i386-linux-gnu/bits/sigset.h \
  /usr/include/i386-linux-gnu/bits/time.h \
  /usr/include/i386-linux-gnu/sys/sysmacros.h \
  /usr/include/i386-linux-gnu/bits/pthreadtypes.h \
  /usr/include/fcntl.h \
  /usr/include/i386-linux-gnu/bits/fcntl.h \
  /usr/include/i386-linux-gnu/bits/fcntl-linux.h \
  /usr/include/i386-linux-gnu/bits/stat.h \
  /usr/include/unistd.h \
  /usr/include/i386-linux-gnu/bits/posix_opt.h \
  /usr/include/i386-linux-gnu/bits/environments.h \
  /usr/include/i386-linux-gnu/bits/confname.h \
  /usr/include/getopt.h \
  /usr/include/ctype.h \
  /usr/include/xlocale.h \
  /usr/include/stdlib.h \
  /usr/include/i386-linux-gnu/bits/waitflags.h \
  /usr/include/i386-linux-gnu/bits/waitstatus.h \
  /usr/include/alloca.h \
  /usr/include/i386-linux-gnu/bits/stdlib-float.h \
  /usr/include/string.h \
    $(wildcard include/config/ing/inlines.h) \
  /usr/include/curses.h \
    $(wildcard include/config/sion.h) \
    $(wildcard include/config/stdbool/h.h) \
    $(wildcard include/config/k.h) \
    $(wildcard include/config/ition.h) \
    $(wildcard include/config/nts.h) \
  /usr/include/ncurses_dll.h \
  /usr/include/stdio.h \
  /usr/include/libio.h \
    $(wildcard include/config/ar/t.h) \
    $(wildcard include/config//io/file.h) \
  /usr/include/_G_config.h \
  /usr/include/wchar.h \
  /usr/lib/gcc/i686-linux-gnu/4.8/include/stdarg.h \
  /usr/include/i386-linux-gnu/bits/stdio_lim.h \
  /usr/include/i386-linux-gnu/bits/sys_errlist.h \
  /usr/lib/gcc/i686-linux-gnu/4.8/include/stdbool.h \
  /usr/include/unctrl.h \
  /usr/include/curses.h \
  /usr/include/locale.h \
  /usr/include/i386-linux-gnu/bits/locale.h \

scripts/kconfig/lxdialog/lxdialog.o: $(deps_scripts/kconfig/lxdialog/lxdialog.o)

$(deps_scripts/kconfig/lxdialog/lxdialog.o):
