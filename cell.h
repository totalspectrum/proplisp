#ifndef CELL_H
#define CELL_H

#include <stdint.h>

// a Cell is a 64 bit integer, holding
// two 30 bit pointers (low bits assumed 0)
// and a 4 bit tag
// or, a 60 bit signed integer with 4 bit tag
// the tag is in the lowest 4 bits for ease of
// extraction
// the tag is further divided into a 3 bit type
// and 1 used bit for garbage collection: gttt
//

typedef uint64_t Cell;
typedef int64_t Num;
typedef uint64_t UNum;

static inline uint32_t FromPtr(void *ptr) {
    uint32_t v = (uint32_t)ptr;
    return v>>2;
}
static inline void *ToPtr(uint32_t v) {
    v = v<<2;
    return (void *)v;
}

static inline int GetUsed(Cell *ptr) { return (*ptr) & 0x08; }
static inline int GetType(Cell *ptr) { return (*ptr) & 0x07; }

#define PTRMASK 0x3FFFFFFF

static inline uint32_t GetTailVal(Cell *ptr) {
    Cell v = *ptr;
    v = v>>4;
    return ((uint32_t)v) & PTRMASK;
}
static inline uint32_t GetHeadVal(Cell *ptr) {
    Cell v = *ptr;
    v = v>>34LL;
    return ((uint32_t)v) & PTRMASK;
}
static inline void *GetTail(Cell *ptr) {
    return ToPtr(GetTailVal(ptr));
}
static inline void *GetHead(Cell *ptr) {
    return ToPtr(GetHeadVal(ptr));
}

static inline void SetUsed(Cell *ptr, int x) {
    if (x) {
        *ptr |= 0x8;
    } else {
        *ptr &= ~(Num)0x8;
    }
}
static inline void SetType(Cell *ptr, int x) {
    *ptr &= ~0x7;
    *ptr |= x & 7;
}
static inline Num GetNum(Cell *ptr) {
    return (Num)(*ptr)>>4;
}
       
// setters
static void SetTail(Cell *ptr, Cell *val) {
    Cell r = *ptr;
    uint32_t v = FromPtr(val);
    r &= ~ ((Cell)0x3FFFFFFF<<4);
    r |= v<<4;
    *ptr = r;
}

enum CellType {
    CELL_NUM = 0,
    CELL_CFUNC = 1,  // tail is ptr to C function, head 
    CELL_STRING = 2, // head is first char, tail points to rest of string
    CELL_PAIR = 3,   // basic building block for lists and such
    CELL_FUNC = 4,   // a lambda expression

    CELL_REF = 5,    // a variable reference: head is var name, tail is value
    CELL_SYMBOL = 6, // like a string, but will be dereferenced
};

static inline Cell CellNum(Num val) {
    return (val << 4) | CELL_NUM;
}

static inline Cell CellPair(int typ, uint32_t head, uint32_t tail) {
    uint32_t h, t;
    return (((Cell)head) <<34) | (((Cell)tail)<<4) | (typ & 0x7);
}

#endif
