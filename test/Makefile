CFLAGS= -W -Wall -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -I. -std=gnu99
OBJECTS= loader.o kernel.o libC.o tty.o paging.o rmm.o panic.o elfkernel.o
LIBGCC=$(shell locate libgcc.a | head -n 1)
LDFLAGS= -L$(shell dirname $(LIBGCC)) -lgcc
M= Makefile
LS= linker.ld
GRUBDIR=.
PADSIZE=750

.s.o: $< $(M)
	nasm -f elf -o $@ $<

.c.o: $< $(M)
	gcc $(CFLAGS) -o $@ -c $<

kernel.bin: $(OBJECTS) $(M) $(LS)
	ld -T $(LS) -o $@ $(OBJECTS) $(LDFLAGS)

pad: $(M)
	dd if=/dev/zero of=pad bs=1 count=$(PADSIZE)

boot.img: kernel.bin pad
	cat $(GRUBDIR)/stage1 $(GRUBDIR)/stage2 pad kernel.bin > $@

test: boot.img
	qemu -no-kvm -fda boot.img

fclean:
	rm -f $(OBJECTS) kernel.bin boot.img pad

all: boot.img

re: fclean all

.PHONY: fclean test all re
.DEFAULT: all
