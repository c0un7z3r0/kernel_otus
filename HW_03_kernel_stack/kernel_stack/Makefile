obj-m += kernel_stack.o

kernel_stack-y := src/main.o src/sysfs.o lib/stack.o lib/stack_ops.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)
BUILD_DIR := $(PWD)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) MO=$(BUILD_DIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) MO=$(BUILD_DIR) clean
	rm -rf $(BUILD_DIR)
