#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "cell.h"

//
// C functions that interface with Lisp may have up to 4 arguments
// we have a string to describe the return value (first item) and arguments:
//   n is a number
//   c is a cell that should be evaluated
//   C is a cell to pass in unchanged (same as c for return type)
//

#define MAX_C_ARGS 4
typedef void *voidptr;
typedef voidptr (*GenericFunc)(voidptr, voidptr, voidptr, voidptr);

typedef struct {
    const char *name;
    const char *args;
    GenericFunc func;
} BuiltinFunction;

Cell *globalEnv;
Cell *globalTrue;
Cell *globalQuote;

Cell *Alloc() {
    return (Cell *)malloc(sizeof(Cell));
}

Cell *RawPair(int typ, uint32_t head, uint32_t tail)
{
    Cell *x = Alloc();
    *x = CellPair(typ, head, tail);
    return x;
}
Cell *NewPair(int typ, Cell *head, Cell *tail)
{
    return RawPair(typ, FromPtr(head), FromPtr(tail));
}

// cons two cells
Cell *Cons(Cell *head, Cell *tail)
{
    return NewPair(CELL_PAIR, head, tail);
}

Cell *IsPair(Cell *expr)
{
    int typ;
    typ = GetType(expr);
    switch (typ) {
    case CELL_PAIR:
    case CELL_REF:
        return expr;
    default:
        return NULL;
    }
}

Cell *IsString(Cell *expr)
{
    int typ;
    typ = GetType(expr);
    switch (typ) {
    case CELL_SYMBOL:
    case CELL_STRING:
        return expr;
    default:
        return NULL;
    }
}

Cell *IsNumber(Cell *expr) {
    int typ = GetType(expr);
    if (typ == CELL_NUM) return expr;
    return NULL;
}

Cell *NewCFunc(Cell *name, BuiltinFunction *f) {
    return RawPair(CELL_CFUNC, FromPtr(name), FromPtr(f));
}
void printchar(int c) {
    putchar(c);
}

void printcstr(const char *s) {
    int c;
    while ( (c = *s++) != 0 ) {
        printchar(c);
    }
}

int readchar() {
    return getchar();
}

Cell *CString(const char *str) {
    Cell *x;
    Cell *rest;
    int c;
    
    if (!str || (0 == (c = *str)) ) {
        return NULL;
    }
    rest = CString(str+1);
    x = RawPair(CELL_STRING, c, FromPtr(rest));
    return x;
}

Cell *CSymbol(const char *str) {
    Cell *x = CString(str);
    SetType(x, CELL_SYMBOL);
    return x;
}

Cell *CNum(Num val) {
    Cell *x = Alloc();
    *x = CellNum(val);
    return x;
}

Cell *StringToNum(Cell *str) {
    int typ = GetType(str);
    int c;
    int32_t val = 0;
    int negate = 0;
    if (typ != CELL_STRING) {
        return NULL;
    }
    c = GetHeadVal(str);
    if (c == '-') {
        negate = 1;
        str = GetTail(str);
    }
    while (str) {
        c = GetHeadVal(str);
        if (c >= '0' && c <= '9') {
            c = c-'0';
        } else {
            break;
        }
        val = 10*val + c;
        str = GetTail(str);
    }
    if (negate) val = -val;
    return CNum(val);
}

Cell *Append(Cell *orig, Cell *newtail)
{
    Cell *ptr;
    Cell *next;
    if (!orig) {
        return newtail;
    }
    ptr = orig;
    for(;;) {
        next = GetTail(ptr);
        if (!next) break;
        ptr = next;
    }
    SetTail(ptr, newtail);
    return orig;
}

Cell *AddCharToString(Cell *str, int c)
{
    Cell *onedigit;
    onedigit = RawPair(CELL_STRING, c, 0);
    return Append(str, onedigit);
}

static Cell *doNumToString(UNum x, unsigned base, int prec) {
    int digits = 0;
    Cell *result = NULL;
    Cell *onedigit;
    int c;
    
    if (prec < 0) prec = 1;
    while (x > 0 || digits < prec) {
        c = x % base;
        x = x / base;
        if (c < 10) c += '0';
        else c = (c - 10) + 'A';
        onedigit = RawPair(CELL_STRING, c, 0);
        result = Append(onedigit, result);
        digits++;
    }
    return result;
}

Cell *NumToString(Cell *num) {
    Num val = GetNum(num);
    Cell *result;
    if (val < 0) {
        result = RawPair(CELL_STRING, '-', 0);
        val = -val;
    } else {
        result = NULL;
    }
    return Append(result, doNumToString(val, 10, 1));
}

static void PrintSymbol(Cell *str) {
    int c;
    while (str) {
        c = GetHeadVal(str);
        printchar(c);
        str = GetTail(str);
    }
}

//
// print a cell
// always returns NULL
//
Cell *Print(Cell *str) {
    int typ;

    if (!str) {
        printcstr("()");
        return NULL;
    }
//    printcstr("[");
    typ = GetType(str);
    switch (typ) {
    case CELL_NUM:
        Print(NumToString(str));
        break;
    case CELL_STRING:
    case CELL_SYMBOL:
        PrintSymbol(str);
        break;
    case CELL_REF:
        Print(GetHead(str));
        break;
    case CELL_CFUNC:
        printcstr("#<builtin: ");
        PrintSymbol(GetHead(str));
        printcstr(">");
        break;
    case CELL_FUNC:
        printcstr("#<lambda>");
        break;
    default:
        printchar('(');
        while (str) {
            Print(GetHead(str));
            str = GetTail(str);
            if (str) {
                printchar(' ');
            }
        }
        printchar(')');
    }
//    printcstr("]");
    return NULL;
}

// quote
Cell *Quote(Cell *a)
{
    return a;
}

// head of list
Cell *Head(Cell *x)
{
    if (IsPair(x)) {
        return GetHead(x);
    }
    return NULL;
}
Cell *Tail(Cell *x)
{
    if (IsPair(x) || IsString(x)) {
        return GetTail(x);
    }
    return NULL;
}

// returns the new environment
Cell *Define(Cell *name, Cell *val, Cell *env)
{
    Cell *x = NewPair(CELL_REF, name, val);
    Cell *envtail = GetTail(env);
    Cell *holder = Cons(x, envtail);
    SetTail(env, holder);
    return x;
}

// check for string equality
int
stringCmp(Cell *a, Cell *b)
{
    int ac, bc;

    while (a && b) {
        ac = GetHeadVal(a);
        bc = GetHeadVal(b);
        ac = ac-bc;
        if (ac != 0) return ac;
        a = GetTail(a);
        b = GetTail(b);
    }
    // at this point there are 3 cases:
    // a and b both NULL -> success
    // a is non-NULL, b is NULL -> a is longer, so return positive
    // b is non-NULL, a is NULL -> b is longer, return negative
    if (a) {
        return 1;
    }
    if (b) {
        return -1;
    }
    return 0;
}

// check to see if two cells are equal
// numbers and functions are equal if they have the same values
// strings are equal if they match character for character
// otherwise they cells must be the same
// returns globalTrue if true, NULL if false

Cell *Match(Cell *a, Cell *b)
{
    int atag, btag;
    Num aval, bval;
    if (a == b) {
        return globalTrue;
    }
    atag = GetType(a);
    btag = GetType(b);
    if (atag != btag) {
        return NULL;
    }
    switch (atag) {
    case CELL_NUM:
        return GetNum(a) == GetNum(b) ? globalTrue : NULL;
    case CELL_STRING:
    case CELL_SYMBOL:
        return stringCmp(a, b) == 0 ? globalTrue : NULL;
    default:
        return NULL;
    }
}

// find a symbol ref
Cell *Lookup(Cell *name, Cell *env)
{
    Cell *holder = NULL;
    Cell *hname = NULL;
    env = Tail(env);
    while (env) {
        holder = GetHead(env);
        if (!holder) break;
        hname = GetHead(holder);
        if (Match(hname, name)) {
            return holder;
        }
        env = GetTail(env);
    }
    return NULL;
}

Cell *ReadListFromString(const char **str_p);
    
Cell *ReadQuotedString(const char **str_p) {
    Cell *result = NULL;
    int c;
    const char *str = *str_p;
    for(;;) {
        c = *str++;
        if (!c) break;
        if (c == '"') {
            // two quotes in a row stands for one quote
            if (*str == '"') {
                str++;
            } else {
                break;
            }
        }
        result = AddCharToString(result, c);
    }
    *str_p = str;
    return result;
}

Cell *ReadItemFromString(const char **str_p)
{
    Cell *result = NULL;
    int c;
    const char *str = *str_p;
    int isnum;
    int alldigits = 1;
    
    do {
        c = *str++;
    } while (isspace(c));
    // collect the next token
    if (c == '(') {
        *str_p = str;
        return ReadListFromString(str_p);
    }
    if (c == '"') {
        *str_p = str;
        return ReadQuotedString(str_p);
    }
    if (c == 0 || c == ')') {
        *str_p = str-1;
        return NULL;
    }
    if (c == '-' && isdigit(*str)) {
        result = AddCharToString(result, c);
        c = *str++;
    }
    do {
        result = AddCharToString(result, c);
        alldigits = alldigits && isdigit(c);
        c = *str++;
    } while (c != 0 && !isspace(c) && c != ')');

    if (c) {
        // skip delimiter
        *str_p = str;
    } else {
        *str_p = str-1;
    }
    // test here for numbers, symbols, etc.
    if (alldigits) {
        return StringToNum(result);
    }
    SetType(result, CELL_SYMBOL);
    return result;
}

Cell *ReadListFromString(const char **str_p)
{
    Cell *tail = NULL;
    Cell *x;
    const char *str = *str_p;
    int c;
    while (isspace(*str)) str++;
    *str_p = str;
    if (!*str) return NULL;
    if (*str == ')') {
        str++;
        *str_p = str;
        return NULL;
    }
    x = ReadItemFromString(str_p);
    tail = ReadListFromString(str_p);
    return Cons(x, tail);
}

//
// lambda creates a function as follows:
// ( (args, env) body )
//
Cell *Lambda(Cell *args, Cell *body, Cell *env)
{
    Cell *descrip;
    Cell *oldenv = Tail(env);
    Cell *newenv = Cons(NULL, oldenv);
    Cell *r;

    // fixme some sanity checking here would be nice!
    descrip = Cons(args, newenv);
    r = NewPair(CELL_FUNC, descrip, body);
    return r;
}

Cell *Eval(Cell *expr, Cell *env); // forward declaration

static Cell *argMismatch()
{
    printcstr("argument mismatch\n");
    return NULL;
}
static Cell *applyCfunc(Cell *fn, Cell *args, Cell *env)
{
    BuiltinFunction *B;
    Cell *argv[MAX_C_ARGS];
    Cell *r;
    const char *argstr;
    int rettype;
    int c;
    int i = 0;
        
    if (fn == globalQuote) {
        return GetHead(args);
    }
    B = GetTail(fn);
    argstr = B->args;
    rettype = *argstr++;
    // now extract arguments
    i = 0;
    while ( 0 != (c = *argstr++) && i < MAX_C_ARGS) {
        if (c == 'e') {
            argv[i++] = env;
            continue;
        }
        if (c == 'v') {
            argv[i++] = args;
            args = NULL;
            break;
        }
        argv[i] = args ? GetHead(args) : 0;
        if (islower(c)) {
            argv[i] = Eval(argv[i], env);
        }
        if (c == 'n') {
            if (IsNumber(argv[i])) {
                argv[i] = (Cell *)(intptr_t)GetNum(argv[i]);
            } else {
                return argMismatch();
            }
        }
        args = args? GetTail(args) : 0;
        i++;
    }
    /* are all arguments accounted for? */
    if (args) {
        return argMismatch();
    }
    r = (*B->func)(argv[0], argv[1], argv[2], argv[3]);
    if (rettype == 'n') {
        r = CNum((intptr_t)r);
    }
    return r;
}

static Cell *applyLambda(Cell *fn, Cell *args, Cell *env)
{
    Cell *newenv;
    Cell *body;
    Cell *argdescrip;

    newenv = GetHead(fn);
    body = GetTail(fn);
    argdescrip = GetHead(newenv);
    newenv = GetTail(newenv);

    return Eval(body, newenv);
}

Cell *Apply(Cell *fn, Cell *args, Cell *env)
{
    int typ;

    fn = Eval(fn, env);
    if (!fn) return NULL;
    typ = GetType(fn);
    if (typ == CELL_CFUNC) {
        return applyCfunc(fn, args, env);
    } else if (typ == CELL_FUNC) {
        return applyLambda(fn, args, env);
    } else {
        return NULL;
    }
}

Cell *Eval(Cell *expr, Cell *env)
{
    int typ;
    Cell *r;
    Cell *f, *args;
    
    typ = GetType(expr);
    switch (typ) {
    case CELL_NUM:
    case CELL_STRING:
    case CELL_CFUNC:
    default:
        return expr;
    case CELL_SYMBOL:
        r = Lookup(expr, env);
        if (r) {
            r = GetTail(r);
        } else {
            printcstr("Undefined symbol: ");
            PrintSymbol(expr);
            printcstr("\n");
        }
        return r;
    case CELL_PAIR:
        f = GetHead(expr);
        args = GetTail(expr);
        return Apply(f, args, env);
    }
}

int Plus(int x, int y) { return x+y; }
int Minus(int x, int y) { return x-y; }
int Times(int x, int y) { return x*y; }
int Div(int x, int y) { return x/y; }

BuiltinFunction cdefs[] = {
    // quote must come first
    { "quote", "ccc", (GenericFunc)Quote },

    { "lambda", "cCCe", (GenericFunc)Lambda },
    { "eval", "cce", (GenericFunc)Eval },
    { "define", "cCce", (GenericFunc)Define },
    { "number?", "cc", (GenericFunc)IsNumber },
    { "pair?", "cc", (GenericFunc)IsPair },
    { "eq?", "ccc", (GenericFunc)Match },
    { "append", "ccc", (GenericFunc)Append },
    { "cons", "ccc", (GenericFunc)Cons },
    { "head", "cc", (GenericFunc)Head },
    { "tail", "cc", (GenericFunc)Tail },
    { "/", "nnn", (GenericFunc)Div },
    { "*", "nnn", (GenericFunc)Times },
    { "-", "nnn", (GenericFunc)Minus },
    { "+", "nnn", (GenericFunc)Plus },
    { NULL, NULL, NULL }
};

static Cell *defCFunc(BuiltinFunction *f)
{
    Cell *name, *val;
    name = CSymbol(f->name);
    val = NewCFunc(name, f);
    Define(name, val, globalEnv);
    return val;
}

static void
Init(void)
{
    BuiltinFunction *f;
    Cell *name, *val;
    globalEnv = Cons(NULL, NULL);
    globalTrue = CString("#t");
    Define(globalTrue, globalTrue, globalEnv);
    f = cdefs;
    globalQuote = defCFunc(f);
    f++;
    for (; f->name; f++) {
        defCFunc(f);
    }
}

int
main(int argc, char **argv)
{
    int i;
    static Cell *a, *b;
    Cell *t;
    char buf[512];
    const char *ptr;
    
    Init();
#if 0
    a = Alloc();
    b = Alloc();
    printf("a=%p b=%p\n", a, b);

    t = RawPair(1, FromPtr(a), FromPtr(b));
    printf("head=%p, tail=%p\n", GetHead(t), GetTail(t));
    
    printf("numbers:\n");
    Print(CNum(0)); printchar('\n');
    Print(CNum(1)); printchar('\n');
    Print(CNum(-17)); printchar('\n');

    printf("strings:\n");
    Print(CString("hello, world!\n"));

    printf("check for matches\n");
    printf("7=17: "); Print(Match(CNum(7), CNum(17))); printf("\n");
    printf("-2=-2: "); Print(Match(CNum(-2), CNum(-2))); printf("\n");
#endif
    
    for(;;) {
        Cell *x;
        printf("interactive> "); fflush(stdout);
        ptr = buf;
        fgets(buf, sizeof(buf), stdin);
        x = ReadItemFromString(&ptr);
        x = Eval(x, globalEnv);
        Print(x);
        printf("\n");
    }
    return 0;
}
