tool_chain = aarch64-linux-gnu-
SIMU_ENV = qemu-system-aarch64

SRC_DIR = src
DUM_DIR = out

SRCS = $(wildcard $(SRC_DIR)/*.c)
SRCSS = $(wildcard $(SRC_DIR)/*.S)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(DUM_DIR)/%.o)
OBJSS = $(SRCSS:$(SRC_DIR)/%.S=$(DUM_DIR)/%.o)

.PHONY: all clean run debug tty

all: kernel8.img 

kernel8.img: $(DUM_DIR) $(OBJS) $(OBJSS) $(SRC_DIR)/linker.ld
	$(tool_chain)ld -T $(SRC_DIR)/linker.ld -o kernel8.elf $(OBJS) $(DUM_DIR)/start.o
	$(tool_chain)objcopy -O binary kernel8.elf kernel8.img

$(DUM_DIR)/%.o: $(SRC_DIR)/%.S
	$(tool_chain)gcc -c $< -o $@

$(DUM_DIR)/%.o: $(SRC_DIR)/%.c
	$(tool_chain)gcc -Wall -I include -c $< -o $@ -fno-stack-protector

$(DUM_DIR): 
	mkdir -p $(DUM_DIR)

clean:
	rm -rf kernel8.*
	rm -rf out

run: all
	$(SIMU_ENV) -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -d int 
tty:
	$(SIMU_ENV) -M raspi3 -kernel kernel8.img -display none -serial null -serial pty -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -d int
debug: 
	$(SIMU_ENV) -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio
dev:
	$(SIMU_ENV) -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb 


