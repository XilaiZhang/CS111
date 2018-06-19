default: build-client build-server

build-client:
	gcc -g -std=c99 -Wall -Wextra -lz lab1b-client.c -o lab1b-client
build-server:
	gcc -g -std=c99 -Wall -Wextra -lz lab1b-server.c -o lab1b-server
clean:
	rm -f lab1b-client lab1b-server *.tar.gz *.txt
dist:
	tar -czvf lab1b-804796478.tar.gz lab1b-client.c lab1b-server.c Makefile README 
