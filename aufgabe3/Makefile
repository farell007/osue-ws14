###############################################################
#@file Makefile
#@author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
#@brief This file is used to compile the project "2048"
#@detail compiles the output files and linkes them and generates documentation
###############################################################

CC=gcc
DEFS=-D_XOPEN_SOURCE=500 -D_BSD_SOURCE -DENDEBUG 
# use this flag to enable debug info -DENDEBUG
CFLAGS=-Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS=-lsem182
DIR=src/
OBJSERV=$(DIR)2048-server.o $(DIR)shared.o
OBJCLIENT=$(DIR)2048-client.o $(DIR)shared.o

all: 2048-server 2048-client

2048-server: $(OBJSERV) 
	$(CC) -o $@ $^ $(LDFLAGS)

2048-client: $(OBJCLIENT) 
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

doxygen:
	doxygen ./doc/Doxyfile

clean:
	rm -f $(OBJSERV)
	rm -f $(OBJCLIENT)
	rm -f 2048-server
	rm -f 2048-client
