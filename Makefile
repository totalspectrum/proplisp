CC=gcc -m32
CFLAGS= -g -Og -Wall

lisp: main.c lisp.c lisp.h cell.h
	$(CC) $(CFLAGS) -o lisp main.c lisp.c

clean:
	rm -f lisp *.o
