all: one.img

one.o: one.S
	aarch64-linux-gnu-gcc -c one.S -o one.o

one.img: one.o
	aarch64-linux-gnu-ld one.o -T linker.ld -o one.elf
	aarch64-linux-gnu-objcopy -O binary one.elf one.img

clean:
	rm one.o one.elf > /dev/null 2>/dev/null || true
