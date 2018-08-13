.PHONY: all
obj-m := rootkit.o
KERNEL_DIR = /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)
all: rootkit client
rootkit:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD)
client:
	gcc -o client main.c ProgramCycle.C OSCommunication.c ServerCommunication.c Utils.c --std=gnu99 -Wall -Wextra -pedantic
clean:
	rm -rf *.o *.ko *.symvers *.mod.* *.order