default:build 

build: 
	gcc -Wall -Wextra -std=c99 -g -lmraa -lm lab4b.c -o lab4b

clean:
	rm -f lab4b-804796478.tar.gz lab4b log.txt

dist: build
	tar -cvzf lab4b-804796478.tar.gz lab4b.c Makefile README

check: build
	echo "SCALE=C\nPERIOD=5\nSCALE=F\nSTART\nSTOP\nLOG test\nLOG jiba\nLOG zhanghaitao\nOFF" | ./lab4b --period=2 --scale=F --log="log.txt"; \
	EXIT_CODE=$$?;\
    echo "program exited with $$EXIT_CODE";\
    exit $$EXIT_CODE
