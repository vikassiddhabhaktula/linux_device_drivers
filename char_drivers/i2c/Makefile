obj-m += i2c_bus_driver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules INSTALL_MOD_PATH=$(shell pwd)/build

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
