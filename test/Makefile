CFLAGS= -W -Wall -std=c99 -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -I.
OBJECTS= loader.o kernel.o libC.o
M= Makefile
LS= linker.ld
GRUBDIR=.
PADSIZE=750

.s.o: $< $(M)
	nasm -f elf -o $@ $<

.c.o: $< $(M)
	gcc $(CFLAGS) -o $@ -c $<

kernel.bin: $(OBJECTS) $(M) $(LS)
	ld -T $(LS) -o $@ $(OBJECTS)

pad: $(M)
	dd if=/dev/zero of=pad bs=1 count=$(PADSIZE)

boot.img: kernel.bin pad
	cat $(GRUBDIR)/stage1 $(GRUBDIR)/stage2 pad kernel.bin > $@

test: boot.img
	qemu -no-kvm -fda boot.img

fclean:
	rm -f $(OBJECTS) kernel.bin boot.img pad

.PHONY: fclean test
