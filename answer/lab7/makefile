tool_chain = aarch64-linux-gnu-
SIMU_ENV = qemu-system-aarch64
CC = gcc
CPP = g++

SRC_DIR = src
DUM_DIR = out
USER_SRC_DIR = user/src
USER_DUM_DIR = user/out

SRCS = $(wildcard $(SRC_DIR)/*.c)
SRCSS = $(wildcard $(SRC_DIR)/*.S)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(DUM_DIR)/%.o)
OBJSS = $(SRCSS:$(SRC_DIR)/%.S=$(DUM_DIR)/%.o)

USER_SRCS = $(wildcard $(USER_SRC_DIR)/*.c)
USER_SRCSS = $(wildcard $(USER_SRC_DIR)/*.S)
USER_OBJS = $(USER_SRCS:$(USER_SRC_DIR)/%.c=$(USER_DUM_DIR)/%.o)
USER_OBJSS = $(USER_SRCSS:$(USER_SRC_DIR)/%.S=$(USER_DUM_DIR)/%.o)

.PHONY: all clean run debug tty

all: kernel8.img sender 

kernel8.img: $(DUM_DIR) $(OBJS) $(OBJSS) $(SRC_DIR)/linker.ld
	$(tool_chain)ld -T $(SRC_DIR)/linker.ld -o kernel8.elf $(OBJS) $(OBJSS)
	$(tool_chain)objcopy -O binary kernel8.elf kernel8.img

$(DUM_DIR)/%.o: $(SRC_DIR)/%.S
	$(tool_chain)$(CC) -g -Wall -I include -c $< -o $@

$(DUM_DIR)/%.o: $(SRC_DIR)/%.c
	$(tool_chain)$(CC) -g -Wall -I include -c $< -o $@ -fno-stack-protector -mstrict-align

$(DUM_DIR): 
	mkdir -p $(DUM_DIR)

sender: sendfile.cpp
	$(CPP) sendfile.cpp -o sender

usercode.img: $(USER_DUM_DIR) $(USER_OBJS) $(USER_OBJSS)
	$(tool_chain)$(CC) -o usercode.img $(USER_OBJS) $(USER_OBJSS)

$(USER_DUM_DIR)/%.o: $(USER_SRC_DIR)/%.S
	$(tool_chain)$(CC) -g -Wall -I user/include -c $< -o $@

$(USER_DUM_DIR)/%.o: $(USER_SRC_DIR)/%.c
	$(tool_chain)$(CC) -g -Wall -I user/include -c $< -o $@ -fno-stack-protector

$(USER_DUM_DIR): 
	mkdir -p $(USER_DUM_DIR)

clean:
	rm -rf kernel8.*
	rm -rf out
	rm sender

run: all
	$(SIMU_ENV) -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb  bcm2710-rpi-3-b-plus.dtb 
tty: all
	$(SIMU_ENV) -M raspi3b -kernel kernel8.img -display none -serial null -serial pty
debug: all
	$(SIMU_ENV) -M raspi3b -kernel kernel8.img -display none -S -s -initrd initramfs.cpio -d int -dtb  bcm2710-rpi-3-b-plus.dtb
dev: all
	$(SIMU_ENV) -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb 

super: all
	$(SIMU_ENV) -M raspi3b -kernel kernel8.img -serial null -serial pty -initrd initramfs.cpio -dtb  bcm2710-rpi-3-b-plus.dtb -d int -S -s

screen: all
	$(SIMU_ENV) -M raspi3b -kernel kernel8.img -serial null -serial stdio -initrd initramfs.cpio -dtb  bcm2710-rpi-3-b-plus.dtb 

