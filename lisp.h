#ifndef LISP_H
#define LISP_H

#include "cell.h"

//
// C functions that interface with Lisp may have up to 4 arguments
// we have a string to describe the return value (first item) and arguments:
//   n is a number
//   c is a cell that should be evaluated
//   C is a cell to pass in unchanged (same as c for return type)
//   e indicates that the current execution environment should be passed
//   v indicates "varargs"; a list holding all remaining arguments is passed
//     this must come last in the list
// The first character (to indicate the return type) must be either c or n
//

#define MAX_C_ARGS 4
typedef void *voidptr;
typedef voidptr (*GenericFunc)(voidptr, voidptr, voidptr, voidptr);

typedef struct {
    const char *name;
    const char *args;
    GenericFunc func;
} LispCFunction;

//
// external interface
//
// define a new function
Cell *Lisp_DefineCFunc(LispCFunction *f);

// evaluate in a global environment
Cell *Lisp_Eval(Cell *x);

// print an expression
Cell *Lisp_Print(Cell *expr);

// run a string script
// returns last expression in script
// if printIt is 1, prints the result
Cell *Lisp_Run(const char *buffer, int printIt);

// initialize everything
// returns a pointer to the global environment
// or NULL on failure
Cell *Lisp_Init(void *arena, size_t arenasize);

#endif
