obj-m += hbp.o
hbp-objs := hbp_main.o hbp_hw.o hbp_hook.o hbp_task.o hbp_ioctl.o

PWD := $(shell pwd)
KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm64 LLVM=1 CROSS_COMPILE=aarch64-linux-gnu- modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
