# 模块最终名字叫 hbp.ko
obj-m += hbp.o

# 依赖的源文件
hbp-objs := \
    hbp_main.o \
    hbp_hw.o \
    hbp_hook.o \
    hbp_task.o \
    hbp_ioctl.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# 强制开启 LLVM 支持 (适配 GKI 6.1 / 6.6)
LLVM ?= 1
CC := clang

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm64 LLVM=$(LLVM) CROSS_COMPILE=aarch64-linux-gnu- modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
