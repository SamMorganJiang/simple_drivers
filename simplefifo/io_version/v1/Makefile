KVERS = $(shell uname -r)

CONFIG_DEBUG_MUTEX=y
CONFIG_DETECT_HUNG_TASKS=y

# Kernel modules
obj-m += simplefifo.o simplefifo_dev.o

# Specify flags for the module compilation.
EXTRA_CFLAGS=-g -O1

build: kernel_modules

kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
