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

static intptr_t getcnt_fn()
{
    return CNT;
}
static intptr_t wait_fn(intptr_t when)
{
    waitcnt(CNT + when);
    return when;
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
    { "wait",      "nn",  (GenericFunc)wait_fn },
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
    char *ptr = buf;
    int strcount = 0;
    int instring = 0;
    int c;
    int parencount = 0;
    
    prompt(1);
    for(;;) {
        c = inchar();
        switch (c) {
        case 3:  // ^C means terminate
            outchar('\n');
            return NULL;
        case 8: // ^H is backspace
        case 127:
            if (strcount > 0) {
                --ptr;
                --strcount;
                c = buf[strcount]; // the character we are about to erase
                if (c == '"') instring = !instring;
                else if (c == '(') --parencount;
                else if (c == ')') ++parencount;
                else if (c == '\n') {
                    // OK, this is cute, we're going to back up a line
                    outchar('\n');
                    buf[strcount+1] = 0;
                    prompt(parencount);
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
            *ptr++ = c;
            if (parencount > 0) {
                prompt(parencount+1);
            } else {
                *ptr++ = 0;
                return buf;
            }
            break;
        output:
        default:
            outchar(c);
            *ptr++ = c;
            strcount++;
        }
    }

    return buf;
}

void
REPL()
{
    char *ptr;
    Cell *result;

    setraw();
    for(;;) {
        ptr = getOneLine();
        if (!ptr) {
            break;
        }
        result = Lisp_Run(ptr, 0);
        Lisp_Print(result);
        outchar('\n');
    }
    setcooked();
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
