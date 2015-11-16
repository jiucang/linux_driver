# Get the kernel source dir
KVERS = $(shell uname -r)

KBUILD_EXTRA_SYMBOLS=/home/jacob/Linux_driver/driver_demo/chapter5/3th/Module.symvers

# Kernel modules
obj-m += scdriver.o

build: kernel_modules

kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules
clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
