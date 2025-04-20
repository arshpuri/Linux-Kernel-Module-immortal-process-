# src/Makefile
obj-m += immortal_proc.o
immortal_proc-objs := immortal_main.o immortal_hooks.o immortal_proc_fs.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
