# Get the kernel source dir
KVERS = $(shell uname -r)

# Kernel modules
obj-m += globalmem.o
# For multi file compilation.
# modulename-objs := file1.o file2.o

# Specify flags for the module compilation/debug.
# EXTRA_CFLAGS=-g -O0


build: kernel_modules

kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(KVERS)build M=$(CURDIR) clean
