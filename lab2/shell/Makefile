SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

ASMS = $(wildcard *.S)
ASM_OBJS = $(ASMS:.S=.o)


CFLAGS = -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -I../../../opt/lib/gcc/aarch64-linux-gnu/12.2.0/include

all: kernel8.img

%.o: %.S
	aarch64-linux-gnu-gcc $(CFLAGS) -c $< -o $@

%.o: %.c
	aarch64-linux-gnu-gcc $(CFLAGS) -c $< -o $@

kernel8.img: $(ASM_OBJS) $(OBJS)
	aarch64-linux-gnu-ld $(ASM_OBJS) $(OBJS) -T linker.ld -o kernel8.elf
	aarch64-linux-gnu-objcopy -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf kernel8.img *.o *~>/dev/null 2>/dev/null || true
#
run:
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -display none  -kernel kernel8.img
