#/* **************** LDD:2.0 s_34/lab1_iosched.sh **************** */
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

NMAX=8
NMEGS=100
[[ -n $1 ]] && NMAX=$1
[[ -n $2 ]]  && NMEGS=$2

echo Doing: $NMAX parallel read/writes on: $NMEGS MB size files

TIMEFORMAT="%R   %U   %S"

##############################################################
# simple test of parallel reads
do_read_test(){
    for n in $(seq 1 $NMAX) ; do 
	cat file$n > /dev/null & 
    done
# wait for previous jobs to finish
    wait
}

# simple test of parallel writes
do_write_test(){
    for n in $(seq 1 $NMAX) ; do 
	[[ -f fileout$n ]] && rm -f fileout$n
	(cp file1 fileout$n && sync) &
    done
# wait for previous jobs to finish
    wait
}

# create some files for reading, ok if they are the same
create_input_files(){
    [[ -f file1 ]] || dd if=/dev/urandom of=file1 bs=1M count=$NMEGS
    for n in $(seq 1 $NMAX) ; do
	[[ -f file$n ]] || cp file1 file$n
    done
}

echo -e "\ncreating as needed random input files"
create_input_files

##############################################################
# begin the actual work

# do parallel read test
echo -e "\ndoing timings of parallel reads" 
echo -e " REAL    USER    SYS\n"
for iosched in noop deadline anticipatory cfq ; do
    echo testing IOSCHED = $iosched
    echo $iosched > /sys/block/sda/queue/scheduler
#  cat /sys/block/sda/queue/scheduler
#    echo -e "\nclearing the memory caches\n"
    echo 3 > /proc/sys/vm/drop_caches
    time do_read_test
done
##############################################################
# do parallel write test
echo -e "\ndoing timings of parallel writes" 
echo -e " REAL    USER    SYS\n"
for iosched in noop deadline anticipatory cfq ; do
    echo testing IOSCHED = $iosched
#    echo $iosched > /sys/block/sda/queue/scheduler
#    cat /sys/block/sda/queue/scheduler
    time  do_write_test
done
##############################################################
