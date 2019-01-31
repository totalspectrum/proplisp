#
# Makefile for proplisp
#

# default is to build for Linux so we can easily run tests
# note that we assume sizeof(int) == sizeof(void *) == 4,
# so compile in 32 bit mode

CC=gcc -m32
#CFLAGS= -g -Og -Wall
CFLAGS= -g -Wall

lisp: main.c lisplib.c lisplib.h cell.h
	$(CC) $(CFLAGS) -o lisp main.c lisplib.c

clean:
	rm -f lisp *.o *.elf

test: lisp
	(cd Test; ./runtests.sh)

#
# propeller specific
# the propeller-load -s call creates lisp.binary for users
# who only have old Spin tools
#

MODEL=cmm

lisp.elf: main.c lisplib.c PropSerial/FullDuplexSerial.c lisplib.h cell.h
	propeller-elf-gcc -Os -m$(MODEL) -o lisp.elf main.c lisplib.c PropSerial/FullDuplexSerial.c
	propeller-load -s lisp.elf

fibo.elf: fibo.c fibo.h lisplib.c lisplib.h cell.h
	propeller-elf-gcc -Os -m$(MODEL) -o fibo.elf fibo.c lisplib.c
	propeller-load -s fibo.elf

fibo.h: fibo.lsp
	xxd -i fibo.lsp > fibo.h
