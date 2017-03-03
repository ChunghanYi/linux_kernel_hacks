#/* **************** LDD:2.0 s_04/lab4_loadit.sh **************** */
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
#!/bin/sh

# 1/2005 J. Cooperstein (coop@linuxfoundation.org) License:GPLv2

module=lab4_dynamic.ko
node=/dev/mycdrv
minor=0

[[ -n $1 ]] && module="$1"
[[ -n $2 ]] && node="$2"

echo loading $module
insmod $module

major=$(awk "\$2==\"mycdrv\" {print \$1}" /proc/devices)
echo major number is: $major

echo creating the device node at $node with minor number=0

mknod $node c $major $minor
