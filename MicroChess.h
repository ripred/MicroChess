/**
 * MicroChess.h
 * 
 */
#ifndef MICROCHESS_INCL
#define MICROCHESS_INCL

#include <stdio.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h>

typedef unsigned char Color;
typedef unsigned char Piece;
typedef unsigned char Bool;

#define MAX_VALUE ((long const)(LONG_MAX / 2))
#define MIN_VALUE ((long const)(0 - MAX_VALUE))

static unsigned const BOARD_SIZE = 64u;

static Piece const Type = 0b00000111u;
//static Piece const Unused   = 0b00001000u;
static Piece const Side     = 0b00010000u;
static Piece const Moved    = 0b00100000u;
static Piece const Check    = 0b01000000u;
static Piece const Promoted = 0b10000000u;

struct Spot {
    uint8_t  type : 3,  // one of the Piece types above
           unused : 1,
             side : 1,  // one of the Color types above
            moved : 1,  // 1 if moved
            check : 1,  // 1 if in check
         promoted : 1;
};

static Color const White = 1u;
static Color const Black = 0u;

static Piece const Empty  = 0u;
static Piece const Pawn   = 1u;
static Piece const Knight = 2u;
static Piece const Bishop = 3u;
static Piece const Rook   = 4u;
static Piece const Queen  = 5u;
static Piece const King   = 6u;

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

Piece getType(Piece b);
unsigned char isEmpty(Piece b);
int getValue(Piece b);
Piece getSide(Piece b);
unsigned char hasMoved(Piece b);
unsigned char inCheck(Piece b);
unsigned char isPromoted(Piece b);
Piece setType(Piece b, Piece type);
Piece setSide(Piece b, Piece side);
Piece setMoved(Piece b, unsigned char hasMoved);
Piece setCheck(Piece b, unsigned char inCheck);
Piece setPromoted(Piece b, unsigned char promoted);
Piece makeSpot(Piece type, Piece side, unsigned char moved, unsigned char inCheck);
char *getCoords(int index);
char *getCoords(int file, int rank);
char *getNotate(int const index);
char *getNotate(int file, int rank);
char *getName(Piece b);
char *getColor(Piece b);
char *addCommas(long int value);

#endif // MICROCHESS_INCL
