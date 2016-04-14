#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

#ifdef __propeller__
#include <propeller.h>
#define ARENA_SIZE 2048
#else
#define ARENA_SIZE 8192
#define MAX_SCRIPT_SIZE 100000
#endif

int inchar() {
    return getchar();
}
void outchar(int c) {
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
    Lisp_Run(script);
}
#endif

#ifdef __propeller__

static intptr_t getcnt_fn()
{
    return CNT;
}
static intptr_t waitcnt_fn(intptr_t when)
{
    waitcnt(when);
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
    { "waitcnt",   "nnn", (GenericFunc)waitcnt_fn },
#else
    { "dsqr",      "nnn", (GenericFunc)testfunc },
#endif
    { NULL, 0 }
};

void
REPL()
{
    char buf[128];
    Cell *r;
    
    for(;;) {
        printf("> "); fflush(stdout);
        fgets(buf, sizeof(buf), stdin);
        r = Lisp_Run(buf);
        Lisp_Print(r);
        printf("\n"); fflush(stdout);
    }
}

char arena[ARENA_SIZE];

int
main(int argc, char **argv)
{
    Cell *err;
    int i;
    
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
