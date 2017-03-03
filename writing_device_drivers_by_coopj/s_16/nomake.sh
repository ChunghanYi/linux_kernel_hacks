#/* **************** LDD:2.0 s_16/nomake.sh **************** */
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

PATH=../:$PATH
cat *.c > /lib/firmware/my_fwfile
genmake $KROOT
make
