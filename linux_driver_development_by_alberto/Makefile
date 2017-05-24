# Linux driver development for embedded processors
# Chunghan Yi, chunghan.yi@gmail.com, 2017
# License: GPLv2

MAKE	= make
CD		= cd
RM		= rm
LABDIRS	= lab03 lab04 lab05 lab06 lab07 lab08 lab09 lab10

all:	labs

labs:
		@for n in $(LABDIRS); do \
			$(CD) $$n; \
			../genmake; \
			$(MAKE) || exit 1; \
			$(CD) ..; \
		done

clean:
		@for n in $(LABDIRS); do \
			$(CD) $$n; \
			$(MAKE) clean || exit 1; \
			$(CD) ..; \
		done

install:
		@for n in $(LABDIRS); do \
			$(CD) $$n; \
			$(MAKE) install || exit 1; \
			$(CD) ..; \
		done
