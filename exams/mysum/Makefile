CC=gcc
DEFS=-D_XOPEN_SOURCE=500 -D_BSD_SOURCE
CFLAGS=-Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS=

DIR=src
OFILES=$(DIR)/mysum.o

all: mysum

mysum: $(OFILES) 
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OFILES)
	rm -f mysum
	
