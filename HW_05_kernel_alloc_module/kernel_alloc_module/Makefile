obj-m += kernel_alloc.o
kernel_alloc-y := src/main.o src/params.o src/allocator.o src/bitmap.o

ccflags-y += -I$(src)/inc

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)
BUILD_DIR := $(PWD)/build

.PHONY: all clean

all:
	$(MAKE) -C $(KDIR) M=$(PWD) MO=$(BUILD_DIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) MO=$(BUILD_DIR) clean
	rm -rf $(BUILD_DIR)
