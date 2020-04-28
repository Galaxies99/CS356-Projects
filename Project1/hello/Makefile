obj-m := hello.o
 
all:
	make -C /usr/src/linux-5.5.8/ M=$(shell pwd) modules
clean:
	make -C /usr/src/linux-5.5.8/ M=$(shell pwd) clean
