obj-m := mycalc.o

default:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
	$(CC) testmycalc.c -o testmycalc

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	rm testmycalc
