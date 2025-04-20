KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD)/src modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD)/src clean

sign: all
	@tools/sign_module.sh

install: sign
	install -m 644 src/immortal_proc.ko /lib/modules/$(shell uname -r)/extra/
	depmod -a

uninstall:
	rm -f /lib/modules/$(shell uname -r)/extra/immortal_proc.ko
	depmod -a

dkms-install:
	mkdir -p /usr/src/immortal_proc-0.1/
	cp -r * /usr/src/immortal_proc-0.1/
	dkms add -m immortal_proc -v 0.1
	dkms build -m immortal_proc -v 0.1
	dkms install -m immortal_proc -v 0.1

dkms-uninstall:
	dkms remove -m immortal_proc -v 0.1 --all
	rm -rf /usr/src/immortal_proc-0.1/
