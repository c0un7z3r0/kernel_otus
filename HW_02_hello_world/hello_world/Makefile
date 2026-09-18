ifneq ($(KERNELRELEASE),)

obj-m += hello_world.o

hello_world-y := src/hello_world.o

else

KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

SRC_DIR := $(PWD)/src
BUILD_DIR := $(PWD)/build

MODULE := hello_world

CLANG_FORMAT := clang-format
CLANG_FORMAT_FLAGS := -i

FORMAT_FILES := $(SRC_DIR)/*.c

.PHONY: all kbuild clean format check

all: kbuild

kbuild:
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) \
		MO=$(BUILD_DIR) \
		modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) \
		MO=$(BUILD_DIR) \
		clean
	rm -rf $(BUILD_DIR)

format:
	$(CLANG_FORMAT) $(CLANG_FORMAT_FLAGS) $(FORMAT_FILES)

check:
	./check.sh

endif
