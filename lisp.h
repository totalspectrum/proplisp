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
} BuiltinFunction;

#endif
