CC=gcc
DEFS=-D_XOPEN_SOURCE=500 -D_BSD_SOURCE
CFLAGS=-Wall -g -std=c99 -pedantic $(DEFS)
DIR=src/
OBJECTFILES=$(DIR)myexpand.o

.PHONY: all clean

all: myexpand 

myexpand: $(OBJECTFILES) 
	$(CC) $(LDFLAGS) -o $@ $^	

%.o: %.c
	$(CC) $(LDFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTFILES) myexpand 

tt:
	./tests/test.sh

bp:
	./backup/backup.sh
