###############################################################
#@file Makefile
#@author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
#@brief This file is used to compile the project
###############################################################

CC=gcc
DEFS=-D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g
CFLAGS=-Wall -std=c99 -pedantic $(DEFS)
LDFLAGS=
DIR=src/
OBJECTFILES=$(DIR)server.o $(DIR)client.o

.PHONY: all clean

all: client server 

server: $(OBJECTFILES) 
	$(CC) $(LDFLAGS) -o $@ $^	

client: $(OBJECTFILES) 
	$(CC) $(LDFLAGS) -o $@ $^	

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTFILES) client server 

