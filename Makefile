#
# Makefile for proplisp
#

# default is to build for Linux so we can easily run tests
# note that we assume sizeof(int) == sizeof(void *) == 4,
# so compile in 32 bit mode

CC=gcc -m32
#CFLAGS= -g -Og -Wall
CFLAGS= -g -Wall

lisp: main.c lisp.c lisp.h cell.h
	$(CC) $(CFLAGS) -o lisp main.c lisp.c

clean:
	rm -f lisp *.o

test: lisp
	(cd Test; ./runtests.sh)

#
# propeller specific
# the propeller-load -s call creates lisp.binary for users
# who only have old Spin tools
#

lisp.elf: main.c lisp.c PropSerial/FullDuplexSerial.c lisp.h cell.h
	propeller-elf-gcc -Os -mcmm -o lisp.elf main.c lisp.c PropSerial/FullDuplexSerial.c
	propeller-load -s lisp.elf
