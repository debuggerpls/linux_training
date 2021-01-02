ifneq ($(KERNELRELEASE),)
obj-m := serial.o
else
KDIR := $(HOME)/projects/linux_training/linux-kernel-labs/src/linux
all:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
endif
