CC=gcc
CFLAGS = -std=gnu11 -g -O2 -Wall -Wextra -Wno-unused-parameter
LDLIBS = -pthread -lm

build:
	$(CC) $(CFLAGS) -o lab3a lab3a.c $(LDLIBS) 




.PHONY: clean
clean:
	rm  lab3a-204785637.tar.gz 




dist:
	tar -czvf lab3a-204785637.tar.gz *.c *.h README Makefile 

