TARGET_MODULE=audio
CC=gcc
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

# If KERNELRELEASE is defined, we have been invoked from the kernel build system
# and can use its language.
# This runs on the second run of make.
ifneq ($(KERNELRELEASE),)
	obj-m := $(TARGET_MODULE).o
# Otherwise, we were invoked from the command line - invoke the kernel build system.
# This runs on the first run of make.
else
	BUILDSYSTEM_DIR:=/lib/modules/$(shell uname -r)/build
	PWD:=$(shell pwd)


all :
	# run kernel build system to make module
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules

clean:
	# run kernel build system to cleanup in current directory
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean

install:
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules_install
add:
	sudo insmod $(TARGET_MODULE).ko
remove:
	sudo rmmod $(TARGET_MODULE)
endif
