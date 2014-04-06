###############################################################
#@file Makefile
#@author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
#@brief This file is used to compile the project
###############################################################

CC=gcc
DEFS=-D_XOPEN_SOURCE=500 -D_BSD_SOURCE 
# use this flag to enable debug info -DENDEBUG
CFLAGS=-Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS=
DIR=src/
SERVER_OBJFILES=$(DIR)server.o
CLIENT_OBJFILES=$(DIR)client.o 
#$(DIR)gamelogic.o
OBJFILES=$(SERVER_OBJFILES) $(CLIENT_OBJFILES)

.PHONY: all clean

all: client server 

server: $(SERVER_OBJFILES) 
	$(CC) $(LDFLAGS) -o $@ $^	

client: $(CLIENT_OBJFILES) 
	$(CC) $(LDFLAGS) -o $@ $^	

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJFILES)
	rm -f client
	rm -f server 

