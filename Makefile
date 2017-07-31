#
#
# author: mhl
# create: 2017-07-31
# description:
# 
# 

DRIVER_VERSION  := "0.0.1"
DRIVER_AUTHOR   := "mhl8002 @ 126.com"
DRIVER_DESC     := "linux first driver hello world"
DRIVER_LICENSE  := "DUAL BSD/GPL"

MODULE_NAME  := scull

ifneq ($(KERNELRELEASE),)

obj-m := $(MODULE_NAME).o

else

LINUX_KERNEL  := $(shell uname -r)
LINUX_KERNEL_PATH := /lib/modules/$(LINUX_KERNEL)/build

CURRENT_PATH := $(shell pwd)
CFG_INC := $(CURRENT_PATH)
MODCFLAGS := -O2 -Wall -DMODULE -D__KERNEL__ -DLINUX -std=c99
EXTRA_CFLAGS += $(MODULE_FLAGS) -i $(CFG_INC)

modules:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules

modules_install:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules_install

insmod:
	sudo insmod $(MODULE_NAME).ko

reinsmod:
	sudo rmmod $(MODULE_NAME)
	sudo insmod $(MODULE_NAME).ko

#github:
#	cd $(ROOT) && make github

rmmod:
	sudo rmmod $(MODULE_NAME)

#test :
	

clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
	rm -f modules.order Module.symvers Module.markers

.PHNOY:
	modules modules_install clean

endif
