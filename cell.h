#ifndef CELL_H
#define CELL_H

#include <stdint.h>

#ifdef __propeller__
#define SMALL
#endif

#ifdef SMALL
// a Cell is a 32 bit integer, holding
// two 14 bit pointers (low bits assumed 0)
// and a 4 bit tag
// or, a 28 bit signed integer with 4 bit tag
// the tag is in the lowest 4 bits for ease of
// extraction
// the tag is further divided into a 3 bit type
// and 1 used bit for garbage collection: gttt
//

typedef uint32_t Cell;
typedef int32_t Num;
typedef uint32_t UNum;
#define INLINE

#else
// a Cell is a 64 bit integer, holding
// two 30 bit pointers (low bits assumed 0)
// and a 4 bit tag
// or, a 60 bit signed integer with 4 bit tag
// the tag is in the lowest 4 bits for ease of// extraction
// the tag is further divided into a 3 bit type
// and 1 used bit for garbage collection: gttt
//

typedef uint64_t Cell;
typedef int64_t Num;
typedef uint64_t UNum;

#define INLINE inline

#endif

enum CellType {
    CELL_NUM = 0,
    CELL_CFUNC = 1,  // tail is ptr to C function, head is ???
    CELL_STRING = 2, // head is first char, tail points to rest of string
    CELL_PAIR = 3,   // basic building block for lists and such
    CELL_FUNC = 4,   // a lambda expression

    CELL_REF = 5,    // a variable reference: head is var name, tail is value
    CELL_SYMBOL = 6, // like a string, but will be dereferenced

    CELL_NIL = -1,  // not actually stored
};

static INLINE uint32_t FromPtr(void *ptr) {
    uint32_t v = (uint32_t)ptr;
    v = v>>2;
    return v;
}
static INLINE void *ToPtr(uint32_t v) {
    v = v<<2;
    return (void *)v;
}

static INLINE int GetUsed(Cell *ptr) { return (*ptr) & 0x08; }
static INLINE int GetType(Cell *ptr) {
    if (!ptr) return CELL_NIL;
    return (*ptr) & 0x07;
}

#ifdef SMALL
#define PTRMASK 0x3FFF
#define HEADSHIFT 18
#else
#define PTRMASK 0x3FFFFFFF
#define HEADSHIFT 34
#endif

static INLINE uint32_t GetTailVal(Cell *ptr) {
    Cell v = *ptr;
    v = v>>4;
    return ((uint32_t)v) & PTRMASK;
}
static INLINE uint32_t GetHeadVal(Cell *ptr) {
    Cell v = *ptr;
    v = v>>HEADSHIFT;
    return ((uint32_t)v) & PTRMASK;
}
static INLINE void *GetTail(Cell *ptr) {
    return ToPtr(GetTailVal(ptr));
}
static INLINE void *GetHead(Cell *ptr) {
    return ToPtr(GetHeadVal(ptr));
}

static INLINE void SetUsed(Cell *ptr, int x) {
    if (x) {
        *ptr |= 0x8;
    } else {
        *ptr &= ~(Num)0x8;
    }
}
static INLINE void SetType(Cell *ptr, int x) {
    *ptr &= ~0x7;
    *ptr |= x & 7;
}
static INLINE Num GetNum(Cell *ptr) {
    return (Num)(*ptr)>>4;
}
       
static void SetTail(Cell *ptr, Cell *val) {
    Cell r = *ptr;
    uint32_t v = FromPtr(val);
    r &= ~ ((Cell)PTRMASK<<4);
    r |= v<<4;
    *ptr = r;
}

static inline Cell CellNum(Num val) {
    return (val << 4) | CELL_NUM;
}

static inline Cell CellPair(int typ, uint32_t head, uint32_t tail) {
    uint32_t h, t;
    return (((Cell)head) <<HEADSHIFT) | (((Cell)tail)<<4) | (typ & 0x7);
}

#endif