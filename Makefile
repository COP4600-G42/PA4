obj-m += pa3_input.o
obj-m += pa3_output.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(shell pwd) modules
	$(CC) testpa3.c -o testpa3

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(shell pwd) clean
	rm testpa3
