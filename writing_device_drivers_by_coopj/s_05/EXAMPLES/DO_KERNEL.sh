#/* **************** LDD:2.0 s_05/DO_KERNEL.sh **************** */
#/*
# * The code herein is: Copyright Jerry Cooperstein, 2012
# *
# * This Copyright is retained for the purpose of protecting free
# * redistribution of source.
# *
# *     URL:    http://www.coopj.com
# *     email:  coop@coopj.com
# *
# * The primary maintainer for this code is Jerry Cooperstein
# * The CONTRIBUTORS file (distributed with this
# * file) lists those known to have contributed to the source.
# *
# * This code is distributed under Version 2 of the GNU General Public
# * License, which you should have received with the source.
# *
# */
#!/bin/bash

# Script to compile and install the Linux kernel and modules.
# written by Jerry Cooperstein (2002-2009)
# Copyright GPL blah blah blah

get_SOURCE(){
    KERNEL=$1
    TARFILE=$2
    CONFIGFILE=$3
    LKV=linux-$KERNEL
    KCONFIG=$LKV/.config
    [[ ! -f $TARFILE ]] && echo no  "$TARFILE, aborting" && exit
    [[ ! -f $CONFIGFILE ]] && echo no "$CONFIGFILE, aborting" && exit
    echo -e "\nbuilding: Linux $KERNEL kernel from $TARFILE
               Using: $CONFIGFILE as the configuration file\n"
    set -x
# Decompress kernel, assumed in bz2 form
    tar jxf $TARFILE
# Copy over the .config file
    cp $CONFIGFILE $KCONFIG
# Change to the main source directory
    cd $LKV
    set +x
}

# determine which distribution we are on
get_SYSTEM(){
    [[ -n $SYSTEM ]] && return
    SYSTEM=
    [[ -n $(grep -i Red\ Hat /proc/version) ]] && SYSTEM=REDHAT && return
    [[ -n $(grep -i Ubuntu   /proc/version) ]] && SYSTEM=UBUNTU && return
    [[ -n $(grep -i debian   /proc/version) ]] && SYSTEM=DEBIAN && return
    [[ -n $(grep -i suse     /proc/version) ]] && SYSTEM=SUSE   && return
    [[ -n $(grep -i gentoo   /proc/version) ]] && SYSTEM=GENTOO && return
}

# find out what kernel version this is
get_KERNELVERSION(){
    for FIELD in VERSION PATCHLEVEL SUBLEVEL EXTRAVERSION ; do
	eval $(sed -ne "/^$FIELD/s/ //gp" Makefile)
    done
# is there a local version file?
    [[ -f ./localversion-tip ]] && \
	EXTRAVERSION="$EXTRAVERSION$(cat localversion-tip)"
    KERNEL=$VERSION.$PATCHLEVEL.$SUBLEVEL$EXTRAVERSION
}

# determine where to place vmlinuz, System.map, initrd image, config file
get_BOOT(){
    if [[ -n $BOOT ]] ; then 
	[[ -d $BOOT ]] && return
	echo $BOOT does not exist
    fi
    BOOT=/boot
}

# parallelize, speed up for multiple CPU's
get_MAKE(){
    NCPUS=$(grep ^processor /proc/cpuinfo | wc -l)
    JOBS=$(( 3*($NCPUS-1)/2 + 2 ))
    MAKE="make -j $JOBS"
}

# if it is a NVIDIA'able  kernel, compile nvidia.ko
 dealwith_NVIDIA(){
     [[ $(lsmod | grep nvidia) ]] && \
     [[ $(which --skip-alias dealwith_nvidia) ]] && \
	 dealwith_nvidia $KERNEL
}

makeinitrd_REDHAT(){
# Construct mkinitrd image
# Fedora 13+ and RHEL6
if [[ -f /sbin/dracut ]] ; then
    dracut -v -f $BOOT/initrd-$KERNEL.img $KERNEL
else
    /sbin/mkinitrd -v -f $BOOT/initrd-$KERNEL.img $KERNEL 
fi
# Update /boot/grub/grub.conf
    cp /boot/grub/grub.conf /boot/grub/grub.conf.BACKUP
    /sbin/grubby --copy-default  \
	--make-default \
	--remove-kernel=$BOOT/vmlinuz-$KERNEL \
	--add-kernel=$BOOT/vmlinuz-$KERNEL \
	--initrd=$BOOT/initrd-$KERNEL.img \
	--title=$BOOT/$KERNEL
# if it is a NVIDIA'able  kernel, compile nvidia.ko
    dealwith_NVIDIA 
}

makeinitrd_DEBIAN(){
    make install
    update-initramfs -ct -k $KERNEL
    update-grub
}

makeinitrd_UBUNTU(){
    makeinitrd_DEBIAN
}

makeinitrd_SUSE(){
    make install
}

makeinitrd_GENTOO(){
    make install
    genkernel 	ramdisk --kerneldir=$PWD   
}
makeinitrd_(){
    echo System $SYSTEM is not something I understand, can not finish
    exit
}

##########################################################################
# Start of the work

NARGS="$#"
[[ $NARGS == "3" ]]  && get_SOURCE $1 $2 $3

get_KERNELVERSION
get_BOOT
get_SYSTEM
get_MAKE

echo building: Linux $KERNEL kernel,  and placing in: $BOOT on a $SYSTEM system

# set shell to abort on any failure and echo commands
set -e -x

# Do the main compilation work, kernel and modules
$MAKE

# Install the modules
$MAKE modules_install 

# Install the compressed kernel, System.map file, config file, 
cp arch/x86/boot/bzImage   $BOOT/vmlinuz-$KERNEL 
cp System.map              $BOOT/System.map-$KERNEL 
cp .config                 $BOOT/config-$KERNEL 

# making initrd and updating grub is very distribution dependendent:

echo I am building the initrd image and modifying grub config on $SYSTEM

makeinitrd_"$SYSTEM"
