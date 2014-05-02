ifneq ($(KERNELRELEASE), )
	obj-m := oint_leds.o
else
	KERNELDIR ?= ~/board/mini6410/linux-2.6.38
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

.PHONY: clean
clean:
	rm -rf *.o *.ko *.mod.c modules.* Module.* .*.cmd .tmp_versions
