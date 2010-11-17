CFLAGS= -W -Wall -Wstrict-prototypes -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -I. -std=gnu99 -O3 -g
OBJECTS= loader.o kernel.o libC.o tty.o paging.o rmm.o panic.o elfkernel.o
LIBGCC=$(shell locate libgcc.a | head -n 1)
LDFLAGS= -L$(shell dirname $(LIBGCC)) -lgcc
M= Makefile
LS= linker.ld
GRUBDIR=.
PADSIZE=750
HDASIZE=1000000

QEMU_FLAGS ?= -no-kvm
QEMU_RAM ?= 512

.s.o: $< $(M)
	nasm -f elf -o $@ $<

.c.o: $< $(M)
	gcc $(CFLAGS) -o $@ -c $<

kernel.bin: $(OBJECTS) $(M) $(LS)
	ld -T $(LS) -o $@ $(OBJECTS) $(LDFLAGS)

pad: $(M)
	dd if=/dev/zero of=pad bs=$(PADSIZE) count=1

pad2: $(M)
	dd if=/dev/zero of=pad2 bs=$(HDASIZE) count=1

boot.img: kernel.bin pad pad2
	cat $(GRUBDIR)/stage1 $(GRUBDIR)/stage2 pad kernel.bin pad2 > $@

test: boot.img
	(sleep 1; cat bootscript) | qemu $(QEMU_FLAGS) -m $(QEMU_RAM) -hda boot.img -monitor stdio > /dev/null

fclean:
	rm -f $(OBJECTS) kernel.bin boot.img pad pad2

all: boot.img

re: fclean all

retest: re test

.PHONY: fclean test all re retest
.DEFAULT: all
