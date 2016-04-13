CC=gcc -m32
CFLAGS= -g

lisp: lisp.c cell.h
	$(CC) $(CFLAGS) -o lisp lisp.c

clean:
	rm -f lisp *.o
