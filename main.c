//
// lisp interpreter REPL (read-evaluate-print loop)
// 
#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

#ifdef __propeller__
#include <propeller.h>
#define ARENA_SIZE 4096
#else
#define ARENA_SIZE 32768
#define MAX_SCRIPT_SIZE 100000
#endif

// make these whatever you need to switch terminal to
// raw or cooked mode

#ifdef __linux__
#include <termios.h>
#include <unistd.h>
struct termios origt, rawt;

void setraw() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    tcgetattr(fileno(stdin), &origt);
    rawt = origt;
    cfmakeraw(&rawt);
    tcsetattr(fileno(stdin), TCSAFLUSH, &rawt);
}
void setcooked() {
    tcsetattr(fileno(stdin), TCSAFLUSH, &origt);
}
#elif defined(__propeller__)
void setraw() {
    stdin->_flag &= ~_IOCOOKED;
}
void setcooked() {
    stdin->_flag |= _IOCOOKED;
}
#else
// make these whatever you need to switch terminal to
// raw or cooked mode
void setraw() {
}
void setcooked() {
}
#endif

int inchar() {
    return getchar();
}
void outchar(int c) {
    if (c == '\n') {
        putchar('\r');
    }
    putchar(c);
}

#ifdef MAX_SCRIPT_SIZE
char script[MAX_SCRIPT_SIZE];

void
runscript(const char *filename)
{
    FILE *f = fopen(filename, "r");
    int r;
    if (!f) {
        perror(filename);
        return;
    }
    r=fread(script, 1, MAX_SCRIPT_SIZE, f);
    fclose(f);
    if (r <= 0) {
        fprintf(stderr, "File read error on %s\n", filename);
        return;
    }
    script[r] = 0;
    Lisp_Run(script, 1);
}
#endif

#ifdef __propeller__
#include <unistd.h>

static intptr_t getcnt_fn()
{
    return CNT;
}

// wait for ms millisconds
static intptr_t waitms_fn(intptr_t ms)
{
    usleep(ms * 1000);
    return ms;
}
static intptr_t pinout_fn(intptr_t pin, intptr_t onoff)
{
    unsigned mask = 1<<pin;
    DIRA |= mask;
    if (onoff) {
        OUTA |= mask;
    } else {
        OUTA &= ~mask;
    }
    return OUTA;
}
static intptr_t pinin_fn(intptr_t pin)
{
    unsigned mask=1<<pin;
    DIRA &= ~mask;
    return (INA & mask) ? 1 : 0;
}

#else
// compute a function of two variables
// used for testing scripts
static intptr_t testfunc(intptr_t x, intptr_t y, intptr_t a, intptr_t b)
{
    (void)a;
    (void)b;
    return x*x + y*y;
}
#endif

LispCFunction defs[] = {
#ifdef __propeller__
    { "getcnt",    "n",   (GenericFunc)getcnt_fn },
    { "pinout",    "nnn", (GenericFunc)pinout_fn },
    { "pinin",     "nn",  (GenericFunc)pinin_fn },
    { "waitms",    "nn",  (GenericFunc)waitms_fn },
#else
    { "dsqr",      "nnn", (GenericFunc)testfunc },
#endif
    { NULL, 0 }
};

//
// an attempt to provide a "nice" read-evaluate-print loop
// we count ( and prompt the user based on those,
// only evaluating when the expression is done
//
#define SIZE 256

void prompt(int n) {
    if (n < 0) {
        outchar('?');
    }
    while (n > 0) {
        outchar('>');
        --n;
    }
    outchar(' ');
}

void outstr(const char *s) {
    int c;
    while (0 != (c = *s++)) outchar(c);
}

char *
getOneLine()
{
    static char buf[SIZE];
    int strcount = 0;
    int instring = 0;
    int c, i;
    int parencount = 0;
    int firstprompt = 1;
    
    prompt(firstprompt);
    for(;;) {
        buf[strcount] = 0;
        c = inchar();
        switch (c) {
        case 12: // ^L means refresh
            outchar('\n');
            outstr(buf);
            break;
        case 3:  // ^C means terminate
            outchar('\n');
            return NULL;
        case 8: // ^H is backspace
        case 127:
            if (strcount > 0) {
                c = buf[--strcount]; // the character we are about to erase
                if (c == '"') instring = !instring;
                else if (c == '(') --parencount;
                else if (c == ')') ++parencount;
                if (c == '\n') {
                    // OK, this is cute, we're going to back up a line
                    outchar('\n');
                    buf[strcount] = 0;
                    outstr(buf);
                } else {
                    c = 8;
                    outchar(c); outchar(' '); outchar(c);
                }
            }
            break;
        case '"':
            instring = !instring;
            goto output;
        case '(':
            if (!instring) parencount++;
            goto output;
        case ')':
            if (!instring) --parencount;
            goto output;
        case '\n':
        case '\r':
            c = '\n';
            outchar(c);
            buf[strcount++] = c;
            buf[strcount] = 0;
            if (parencount > 0) {
                if (firstprompt) {
                    outstr(buf);
                    firstprompt = 0;
                }
                for (i = 0; i < parencount; i++) {
                    outchar(' ');
                    buf[strcount++] = ' ';
                }
                buf[strcount] = 0;
            } else {
                return buf;
            }
            break;
        output:
        default:
            outchar(c);
            buf[strcount++] = c;
        }
    }

    return buf;
}

void
REPL()
{
    char *ptr;
    Cell *result;

    for(;;) {
        setraw();
        ptr = getOneLine();
        setcooked();
        if (!ptr) {
            break;
        }
        result = Lisp_Run(ptr, 0);
        Lisp_Print(result);
        outchar('\n');
    }
}

char arena[ARENA_SIZE];

int
main(int argc, char **argv)
{
    Cell *err;
    int i;

#ifndef __propeller__
#endif
    err = Lisp_Init(arena, sizeof(arena));
    for (i = 0; err && defs[i].name; i++) {
        err = Lisp_DefineCFunc(&defs[i]);
    }
    if (err == NULL) {
        printf("Initialization of interpreter failed!\n");
        return 1;
    }
#ifdef __propeller__
    REPL();
#else
    if (argc > 2) {
        printf("Usage: proplisp [file]\n");
    }
    if (argv[1]) {
        runscript(argv[1]);
    } else {
        REPL();
    }
#endif
    return 0;
}
