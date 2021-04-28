#	Use the following:
#	release build 	- make
#	debug build	- make CFLAGS_MODULE="-DVSDBG"

obj-m += kernel_ds.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules INSTALL_MOD_PATH=$(shell pwd)/build

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
