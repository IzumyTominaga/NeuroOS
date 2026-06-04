CC     = gcc
CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-builtin -O0 -Wall -std=c99
NASM   = nasm
NFLAGS = -f elf32
LD     = ld
LFLAGS = -m elf_i386 -T linker.ld

OBJS = boot.o isr.o gdt.o idt.o vga.o pmm.o vmm.o ctx.o proc.o neuro.o syscall.o usermode.o shell.o keyboard.o kernel.o

all: neuro-os.iso

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(NASM) $(NFLAGS) $< -o $@

neuro-os.bin: $(OBJS)
	$(LD) $(LFLAGS) -o $@ $(OBJS)

neuro-os.iso: neuro-os.bin
	mkdir -p iso/boot/grub
	cp neuro-os.bin iso/boot/
	printf 'set timeout=0\nset default=0\nmenuentry "NeuroOS" {\n  multiboot /boot/neuro-os.bin\n}\n' > iso/boot/grub/grub.cfg
	grub-mkrescue -o neuro-os.iso iso 2>/dev/null

run: neuro-os.iso
	qemu-system-i386 -cdrom neuro-os.iso

clean:
	rm -f *.o *.bin *.iso
	rm -rf iso
