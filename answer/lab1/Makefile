CFLAG = -Iinclude -fno-stack-protector
K8=kernel8
LINKER=linker.ld
K=kernel
START=start

SRC_C:=$(shell find src/ -name '*.c')
SRC_S:=$(shell find src/ -name '*.s')

OBJS:= $(SRC_C:%.c=%.o) \
		$(SRC_S:%.s=%.o) \
		kernel.o\
		start.o


all:$(K8).img

qe:$(K8).img
	qemu-system-aarch64 -M raspi3 -kernel $< -serial null -serial stdio

start.o: start.s
	aarch64-linux-gnu-gcc -o $@ -c -O0 $< 

%.o: %.s
	aarch64-linux-gnu-gcc -o $@ -c $<

%.o: %.c
	aarch64-linux-gnu-gcc -o $@ $(CFLAG) -c $<


$(K8).img: $(OBJS)
	aarch64-linux-gnu-ld -T $(LINKER) -o $(K8).elf $^
	aarch64-linux-gnu-objcopy -O binary $(K8).elf $@

clean:
	$(RM) $(K8:=.elf) $(OBJS)