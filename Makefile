obj-m += pa4_input.o
obj-m += pa4_output.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(shell pwd) modules
	$(CC) testpa4.c -o testpa4

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(shell pwd) clean
	rm testpa4
