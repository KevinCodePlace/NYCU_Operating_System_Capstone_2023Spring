SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -Wextra -Wpedantic -Werror -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles 

all: kernel8.img

start.o: start.S
	aarch64-linux-gnu-gcc $(CFLAGS) -c start.S -o start.o

%.o: %.c
	aarch64-linux-gnu-gcc $(CFLAGS) -c $< -o $@

kernel8.img: start.o $(OBJS)
	aarch64-linux-gnu-ld start.o $(OBJS) -T linker.ld -o kernel8.elf
	aarch64-linux-gnu-objcopy -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf *.o *~>/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio  -initrd initramfs.cpio -display none  -kernel kernel8.img
