/**
 * MicroChess.h
 * 
 * header file for MicroChess
 * 
 */
#ifndef MICROCHESS_INCL
#define MICROCHESS_INCL

#include <Arduino.h>

#pragma pack(1)

#include <stdio.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h>

////////////////////////////////////////////////////////////////////////////////////////
// limits
enum 
{
    MAX_PIECES    = 32,
    MAX_MOVES     = 64,

    NUM_BITS_PT   = 5,
    NUM_BITS_SPOT = 7,
};

typedef   int8_t   index_t;

////////////////////////////////////////////////////////////////////////////////////////
// the offsets a knight can move to
static struct knight_offset_t 
{
    index_t x, y;
} const knight_offsets[8] = {
    { -2, +1 }, // left
    { -2, -1 }, 
    { +2, +1 }, // right
    { +2, -1 }, 
    { +1, +2 }, // forward
    { -1, +2 }, 
    { +1, -2 }, // backward
    { -1, -2 }  

    // { 0, 0 }, // BUGBUG - REMOVE AFTER TESTING!
    // { 0, 0 }, // BUGBUG - REMOVE AFTER TESTING!
    // { 0, 0 }, // BUGBUG - REMOVE AFTER TESTING!
    // { 0, 0 }, // BUGBUG - REMOVE AFTER TESTING!
    // { 0, 0 }, // BUGBUG - REMOVE AFTER TESTING!
    // { 0, 0 }, // BUGBUG - REMOVE AFTER TESTING!
    // { 0, 0 }, // BUGBUG - REMOVE AFTER TESTING!
};

////////////////////////////////////////////////////////////////////////////////////////
// macro to validate a location
// #define DEBUG_TRACE
#ifdef DEBUG_TRACE
#define TRACE_FAIL (printf(Debug3, level, "Failed isValidPos(...) at line: %d in %s\n", __LINE__, __FILE__),0)
#else
#define TRACE_FAIL 1
#endif
#define  isValidPos(col, row) ((col >= 0 && col < 8 && row >= 0 && row < 8) ? 1 : (TRACE_FAIL,0))

typedef unsigned char Color;
typedef unsigned char Piece;
typedef unsigned char Bool;

enum print_t {
    None,
    Debug0, 
    Debug1, 
    Debug2, 
    Debug3, 
    Always = 100
};

#define  ARRAYSZ(A) (sizeof(A) / sizeof(*(A)))

#define  MAX_VALUE ((long const)( 0b1000000000000)) // half the value of 13 bits - however large the 'value' field in move_t is
#define  MIN_VALUE ((long const)(0 - MAX_VALUE))

static unsigned const BOARD_SIZE = 64u;

static Piece const Type     = 0b00000111u;
static Piece const Side     = 0b00001000u;
static Piece const Moved    = 0b00010000u;
static Piece const Check    = 0b00100000u;

static Color const White = 1u;
static Color const Black = 0u;

static Piece const Empty  = 0u;
static Piece const Pawn   = 1u;
static Piece const Knight = 2u;
static Piece const Bishop = 3u;
static Piece const Rook   = 4u;
static Piece const Queen  = 5u;
static Piece const King   = 6u;

struct spot_t {
    uint8_t  type : 3,     // one of the Piece types above
             side : 1,     // one of the Color types above
            moved : 1,     // 1 if moved
            check : 1;     // 1 if in check

    spot_t() : type(Empty), side(Black), moved(0), check(0) {}
    spot_t(Piece p) { *this = *((spot_t *) &p); }
    spot_t(uint8_t t, uint8_t s, uint8_t m, uint8_t c) : type(t), side(s), moved(m), check(c){}
};

Piece getType(Piece b);

Bool  isEmpty(Piece b);

int   getValue(Piece b);

Piece getSide(Piece b);

Bool  hasMoved(Piece b);

Bool  inCheck(Piece b);

Piece setType(Piece b, Piece type);

Piece setSide(Piece b, Piece side);

Piece setMoved(Piece b, Bool hasMoved);

Piece setCheck(Piece b, Bool inCheck);

Piece makeSpot(Piece type, Piece side, Bool moved, Bool inCheck);

char *getCoords(index_t index);
char *getCoords(index_t file, index_t rank);
char const *getNotate(index_t const index);
char const *getNotate(index_t file, index_t rank);
char *getName(Piece b);
char *getColor(Piece b);
const char* addCommas(long int value);

int printf(print_t const required, print_t const level, char const * const fmt, ...);

static long const pieceValues[8] = {
    0,          // empty spot value
    10000,      // pawn value
    30000,      // knight value
    30000,      // bishop value
    50000,      // rook value
    90000,      // queen value
    MAX_VALUE,  // king value
    0           // padded for alignment and increased L1 and L2 cache hit gains
};

#endif // MICROCHESS_INCL
