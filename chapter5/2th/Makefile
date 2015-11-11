# Get the kernel source dir
KVERS = $(shell uname -r)

# Kernel modules
obj-m += kset_example.o

build: kernel_modules

kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules
clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
