all: myexpand 

myexpand: myexpand.o
	gcc -o $@ $^	

%.o: %.c
	gcc -std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -c -o $@ $^

clean:
	rm -f myexpand 
	rm -f myexpand.o

tt:
	./tests/test.sh

bp:
	./backup/backup.sh
