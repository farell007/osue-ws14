###############################################################
#@file Makefile
#@author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
#@brief This file is used to compile the project "calc"
#@detail compiles the output files and linkes them
###############################################################

CC=gcc
DEFS=-D_XOPEN_SOURCE=500 -D_BSD_SOURCE -DENDEBUG 
# use this flag to enable debug info -DENDEBUG
CFLAGS=-Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS=
DIR=src/
OBJFILES=$(DIR)calculator.o

all: calculator doxygen 

calculator: $(OBJFILES) 
	$(CC) $(LDFLAGS) -o $@ $^	

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

doxygen:
	doxygen ./doc/Doxyfile

clean:
	rm -f $(OBJFILES)
	rm -f calculator 
