# Makefile Generation and running for SOLUTIONS sets
# Jerry Cooperstein, Linux Foundation 2/2003 - 2012
# License: GPLv2

# where the kernel tree is that you want
export KROOT
#ifneq ($(ARCH), "")
#	 export ARCH
#endif
export CONFIG_SHELL=/bin/bash
export MAKEFLAGS += --no-print-directory

# go into all subdirectories
# otherwise can do make SDIRS="s_07 s_12" etc.
SDIRS= $(wildcard s_??)

all:  
	@for dirs in $(SDIRS); do \
	   cd $$dirs > /dev/null && echo ' ' \
	&& echo DOING $$dirs ..........>&2  && \
	   if [ -f nomake.sh ] ; then \
		./nomake.sh ; \
	   else \
		../genmake ; \
		if [ -f Makefile ] ; then $(MAKE) ; fi ; \
	   fi ; \
	cd .. ; \
	done
clean:
	@for dirs in $(SDIRS); 	do \
	if [ ! -f $$dirs/nomake.sh ] && [ -f $$dirs/Makefile ] ; then \
		$(MAKE) -C $$dirs clean ; fi ;\
	done

help:
	@echo " "
	@echo "       "\"make\" does it all
	@echo " "
	@echo "       "\"make KROOT=/usr/src/linux-2.5.59\"  picks the kernel
	@echo " "
	@echo "       "\"make SDIRS=\"s_07 s_12 s_3*\"\"  picks which subdirectories
	@echo " "
