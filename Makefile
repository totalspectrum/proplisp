CC=gcc -m32
CFLAGS= -g -Og -Wall

lisp: lisp.c lisp.h cell.h
	$(CC) $(CFLAGS) -o lisp lisp.c

clean:
	rm -f lisp *.o
