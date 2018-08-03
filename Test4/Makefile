ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: build clean

build: maliciousProgram
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

sneaky_process:
	gcc -pedantic -std=gnu99 -ggdb -Wall -Werror -o maliciousProgram maliciousProgram.c

clean:
	rm -rf *.o *~ core .depend .*.cmd *.order *.symvers *.ko *.mod.c maliciousProgram
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

else

$(info Building with KERNELRELEASE = ${KERNELRELEASE})
obj-m :=    rootkit.o

endif