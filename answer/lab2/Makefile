CFLAG = -Iinclude -fno-stack-protector
K8=kernel8
BL=bootloader
LINKER=linker.ld
K=kernel
START=start
CPIO:=archive/initramfs.cpio
DTB:=archive/bcm2710-rpi-3-b-plus.dtb

SRC_C:=$(shell find src/ -name '*.c')
SRC_S:=$(shell find src/ -name '*.s')

KER_C:=$(shell find kernel/ -name '*.c')
KER_S:=$(shell find kernel/ -name '*.s')

BL_C:=$(shell find bootloader/ -name '*.c')
BL_S:=$(shell find bootloader/ -name '*.s')

OBJS:= $(SRC_C:%.c=%.o) \
		$(SRC_S:%.s=%.o)\


KERNEL_OBJS:=$(KER_C:%.c=%.o) \
		$(KER_S:%.s=%.o)\
		rd.o

BL_OBJS:=$(BL_C:%.c=%.o) \
		$(BL_S:%.s=%.o) \
		src/mini_uart.o src/utils_c.o src/utils_assembly.o src/memzero.o


all:$(K8).img $(BL).img



rd.o:$(CPIO)
	aarch64-linux-gnu-ld -r -b binary -o rd.o $(CPIO)

qe:$(K8).img
	qemu-system-aarch64 -M raspi3 -kernel $< -serial null -serial stdio -initrd $(CPIO) -dtb $(DTB)


%.o: %.s
	aarch64-linux-gnu-gcc -o $@ -c $<

%.o: %.c
	aarch64-linux-gnu-gcc -o $@ $(CFLAG) -c $<

$(CPIO):
	cd archive/rootfs&&find . | cpio -o -H newc > ../initramfs.cpio

$(K8).img: $(OBJS) $(KERNEL_OBJS)
	aarch64-linux-gnu-ld -T kernel/linker.ld -o $(K8).elf $^
	aarch64-linux-gnu-objcopy -O binary $(K8).elf $@

$(BL).img: $(BL_OBJS) 
	aarch64-linux-gnu-ld -T bootloader/linker.ld -o $(BL).elf $^
	aarch64-linux-gnu-objcopy -O binary $(BL).elf $@

on:
	sudo screen /dev/ttyUSB0 115200
clean:
	$(RM) $(K8:=.elf) $(BL:=.elf) $(OBJS) $(KERNEL_OBJS) $(BL_OBJS) $(K8:=.img) $(BL:=.img) 