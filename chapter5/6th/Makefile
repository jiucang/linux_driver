# Get the kernel source dir
KVERS = $(shell uname -r)

KBUILD_EXTRA_SYMBOLS=/home/jacob/linux_driver/linux_driver/chapter5/3th/Module.symvers

# Kernel modules
obj-m += scclass.o

build: kernel_modules

kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules
clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
