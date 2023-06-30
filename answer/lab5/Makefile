CFLAG = -Iinclude -Wall -ffreestanding -fno-stack-protector  -nostdlib -nostartfiles -mgeneral-regs-only
K8=kernel8
BL=bootloader
LINKER=linker.ld
START=start
CPIO:=archive/initramfs.cpio
TA_CPIO:=archive/TA/initramfs.cpio

DTB:=archive/bcm2710-rpi-3-b-plus.dtb
USER_DIR:=users/user
USER1:=user1
USER2:=user2
BUILD:=build/

BUILD_ELF:=$(shell find build/ -name '*.elf')
BUILD_IMG:=$(shell find build/ -name '*.img')
ARC_IMG:=$(shell find archive/ -name '*.img')

SRC_C:=$(shell find src/ -name '*.c')
SRC_S:=$(shell find src/ -name '*.s')

KER_C:=$(shell find kernel/ -name '*.c')
KER_S:=$(shell find kernel/ -name '*.s')

BL_C:=$(shell find bootloader/ -name '*.c')
BL_S:=$(shell find bootloader/ -name '*.s')

USER1_C:=$(shell find users/user1/ -name '*.c')
USER1_OBJS:= $(USER1_C:%.c=%.o) 

USER2_C:=$(shell find users/user2/ -name '*.c')
USER2_OBJS:= $(USER2_C:%.c=%.o) 

OBJS:= $(SRC_C:%.c=%.o) \
		$(SRC_S:%.s=%.o) \
		rd.o


KERNEL_OBJS:=$(KER_C:%.c=%.o) \
		$(KER_S:%.s=%.o)
		
		

BL_OBJS:=$(BL_C:%.c=%.o) \
		$(BL_S:%.s=%.o) \
		src/mini_uart.o src/utils_c.o src/utils_s.o src/memzero.o src/sprintf.o




all: $(BUILD)$(USER1).img\
     $(BUILD)$(USER2).img \
     $(BUILD)$(K8).img \
     $(BUILD)$(BL).img 



rd.o:$(CPIO)
	aarch64-linux-gnu-ld -r -b binary -o rd.o $(CPIO)


%.o: %.s
	aarch64-linux-gnu-gcc -g -o $@ -c $<

%.o: %.c
	aarch64-linux-gnu-gcc -g -o $@ $(CFLAG) -c $<


$(BUILD)$(K8).img: $(OBJS) $(KERNEL_OBJS)
	aarch64-linux-gnu-ld -T kernel/$(LINKER) -o $(BUILD)$(K8).elf $^
	aarch64-linux-gnu-objcopy -O binary $(BUILD)$(K8).elf $@

$(BUILD)$(BL).img: $(BL_OBJS) 
	aarch64-linux-gnu-ld -T bootloader/$(LINKER) -o $(BUILD)$(BL).elf $^
	aarch64-linux-gnu-objcopy -O binary $(BUILD)$(BL).elf $@

$(BUILD)$(USER1).img:$(USER1_OBJS)
	aarch64-linux-gnu-ld -T $(USER_DIR)1/$(LINKER) -o $(BUILD)$(USER1).elf $<
	aarch64-linux-gnu-objcopy -O binary $(BUILD)$(USER1).elf $@
	cp $@ archive/rootfs/ 

$(BUILD)$(USER2).img:$(USER2_OBJS)
	aarch64-linux-gnu-ld -T $(USER_DIR)2/$(LINKER) -o $(BUILD)$(USER2).elf $<
	aarch64-linux-gnu-objcopy -O binary $(BUILD)$(USER2).elf $@
	cp $@ archive/rootfs/ 

$(CPIO):
	cd archive/rootfs&&find . | cpio -o -H newc > ../initramfs.cpio

on:
	sudo screen /dev/ttyUSB0 115200
qe: $(BUILD)$(K8).img
	qemu-system-aarch64 -M raspi3b -kernel $< \
	-serial null -serial stdio -s \
	-initrd $(TA_CPIO) \
	-dtb $(DTB) \

clean:
	$(RM) $(BUILD_ELF) $(CPIO) \
	$(OBJS) $(KERNEL_OBJS) $(BL_OBJS) $(USER1_OBJS) $(USER2_OBJS)\
	$(BUILD_IMG) $(ARC_IMG)
	