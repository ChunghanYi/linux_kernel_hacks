obj-m +=  hello_world.o

export KERNEL_SRC=~/yocto/fsl-community-bsp/imx6dl-riotboard/tmp/work/imx6dl_riotboard-poky-linux-gnueabi/linux-fslc/4.4+gitAUTOINC+928f8d55df-r0/build

export ARCH=arm

allofit:  modules
modules:
	@$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules
modules_install:
	@$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install
kernel_clean:
	@$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean

clean: kernel_clean
	rm -rf   Module.symvers modules.order
