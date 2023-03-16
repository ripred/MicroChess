/**
 * MicroChess.h
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

typedef unsigned char Color;
typedef unsigned char Piece;
typedef unsigned char Bool;

enum PrintType { Debug1, Debug2, Debug3, None  };

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

    spot_t() :
        type(Empty), 
        side(Black), 
        moved(0), 
        check(0)
    {
    }

    spot_t(Piece p) 
    {
        *this = *((spot_t *) &p);
    }

    spot_t(uint8_t t, uint8_t s, uint8_t m, uint8_t c) :
        type(t), 
        side(s), 
        moved(m), 
        check(c)
    {
    }
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

char *getCoords(int index);
char *getCoords(int file, int rank);
char const *getNotate(int const index);
char const *getNotate(int file, int rank);
char *getName(Piece b);
char *getColor(Piece b);
const char* addCommas(long int value);

int printf(PrintType const level, PrintType const mode, char const * const fmt, ...);

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
