#
# Makefile for LDD4EP(Linux Driver Development for Embedded Processors), 2nd edition
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#

obj-m += helloworld_stm32mp1.o helloworld_stm32mp1_with_parameters.o \
	helloworld_stm32mp1_with_timing.o hellokeys_stm32mp1.o 

obj-m += helloworld_stm32mp1_char_driver.o helloworld_stm32mp1_class_driver.o \
	misc_stm32mp1_driver.o

obj-m += ledRGB_stm32mp1_platform.o ledRGB_stm32mp1_class_platform.o \
	led_stm32mp1_UIO_platform.o

obj-m += int_stm32mp1_key.o int_stm32mp1_key_wait.o keyled_stm32mp1_class.o

obj-m += sdma_stm32mp1_m2m.o sdma_stm32mp1_mmap.o

obj-m += linkedlist_stm32mp1_platform.o

obj-m += io_stm32mp1_expander.o ltc3206_stm32mp1_led_class.o i2c_stm32mp1_accel.o \
	ltc2607_stm32mp1_dual_device.o 

obj-m += ltc2422_stm32mp1_dual.o ltc2422_stm32mp1_trigger.o adxl345_stm32mp1.o \
	adxl345_stm32mp1_iio.o


#FIXME this line for your environments
KERNEL_DIR ?= $(HOME)/STM32MP15-Ecosystem-v1.2.0/Developer-Package/stm32mp1-openstlinux-20-02-19/sources/arm-ostl-linux-gnueabi/linux-stm32mp-4.19-r0/linux-4.19.94

all:
	make -C $(KERNEL_DIR) \
		ARCH=arm CROSS_COMPILE=arm-ostl-linux-gnueabi- \
		SUBDIRS=$(PWD) modules
clean:
	make -C $(KERNEL_DIR) \
		ARCH=arm CROSS_COMPILE=arm-ostl-linux-gnueabi- \
		SUBDIRS=$(PWD) clean
