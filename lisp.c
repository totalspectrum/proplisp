#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "cell.h"

Cell *globalEnv;
Cell *globalTrue;

Cell *Alloc() {
    return (Cell *)malloc(sizeof(Cell));
}

Cell *NewPair(int typ, uint32_t head, uint32_t tail)
{
    Cell *x = Alloc();
    *x = CellPair(typ, head, tail);
    return x;
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
    x = NewPair(CELL_STRING, c, FromPtr(rest));
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
    onedigit = NewPair(CELL_STRING, c, 0);
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
        onedigit = NewPair(CELL_STRING, c, 0);
        result = Append(onedigit, result);
        digits++;
    }
    return result;
}

Cell *NumToString(Cell *num) {
    Num val = GetNum(num);
    Cell *result;
    if (val < 0) {
        result = NewPair(CELL_STRING, '-', 0);
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
        PrintSymbol(str);
        break;
    case CELL_REF:
        Print(GetHead(str));
        break;
    case CELL_CFUNC:
        printcstr("#<builtin>");
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

// cons two cells
Cell *Cons(Cell *head, Cell *tail)
{
    return NewPair(CELL_PAIR, FromPtr(head), FromPtr(tail));
}

// returns the new environment
Cell *Define(Cell *name, Cell *val, Cell *env)
{
    Cell *x = NewPair(CELL_REF, FromPtr(name), FromPtr(val));
    Cell *holder = Cons(name, env);
    return holder;
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
        return stringCmp(a, b) == 0 ? globalTrue : NULL;
    default:
        return NULL;
    }
}

// find a symbol ref
Cell *Lookup(Cell *name, Cell *env)
{
    Cell *holder = NULL;
    while (env) {
        holder = GetHead(env);
        if (Match(GetHead(holder), name))
            return holder;
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
    return result;
}

Cell *ReadListFromString(const char **str_p)
{
    Cell *tail;
    Cell *x;
    for(;;) {
        x = ReadItemFromString(str_p);
        if (!x) {
            return x;
        }
        tail = ReadListFromString(str_p);
        return Cons(x, tail);
    }
}

static void
Init(void)
{
    globalEnv = NULL;
    globalTrue = CString("#t");
    globalEnv = Define(globalTrue, globalTrue, globalEnv);
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
    
    a = Alloc();
    b = Alloc();
    printf("a=%p b=%p\n", a, b);

    t = NewPair(1, FromPtr(a), FromPtr(b));
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

    for(;;) {
        printf("interactive> "); fflush(stdout);
        ptr = buf;
        fgets(buf, sizeof(buf), stdin);
        Print(ReadItemFromString(&ptr));
    }
    return 0;
}
