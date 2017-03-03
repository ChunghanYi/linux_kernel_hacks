### Automatic Makefile generation by 'genmake' script        ####
### Copyright, Jerry Cooperstein, coop@linuxfoundation.org 2/2003 - 1/2012 ####
### License: GPLv2 ###

obj-m +=  lab1_char.o lab2_priv.o lab3_seek.o lab4_dynamic.o lab5_dynamic_udev.o lab5_with_header.o lab6_misc.o lab6_misc_with_header.o

export KROOT=/home/chyi/IoT/UDOO/src/buildroot/output/build/linux-4.9

export ARCH=arm

allofit:  userprogs modules
modules:
	@$(MAKE) -C $(KROOT) M=$(PWD) modules
modules_install:
	@$(MAKE) -C $(KROOT) M=$(PWD) modules_install
kernel_clean:
	@$(MAKE) -C $(KROOT) M=$(PWD) clean

userprogs:
	@$(MAKE) \
		CFLAGS="-O2 -Wall -pedantic" \
	 lab1_char_test lab3_seek_test

clean: kernel_clean
	rm -rf  lab1_char_test lab3_seek_test  Module.symvers modules.order
