# NAME: Xilai Zhang
# EMAIL: xilaizhang@g.ucla.edu
# ID: 804796478

.SILENT:
default:

	gcc -Wall -Wextra -std=c99 -g -lm -lmraa lab4c_tcp.c -o lab4c_tcp 
	gcc -Wall -Wextra -std=c99 -g -lm -lssl -lcrypto -lmraa lab4c_tls.c -o lab4c_tls 

clean:
	rm -f *.tar.gz *.txt lab4c_tcp lab4c_tls
dist:
	tar -czvf lab4c-804796478.tar.gz Makefile README lab4c_tcp.c lab4c_tls.c
