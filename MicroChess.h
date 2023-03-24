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

typedef unsigned char Color;
typedef unsigned char Piece;
typedef unsigned char Bool;

////////////////////////////////////////////////////////////////////////////////////////
// limits
enum 
{
    MAX_PIECES    =  32,
    MAX_MOVES     = 106,   // R:11 N:8 B:11 Q:22 K:8 B:11 N:8 R:11 P:16

    NUM_BITS_PT   =   5,
    NUM_BITS_SPOT =   7,
};

typedef   int8_t   index_t;

// structure to hold the offsets that a piece can move to
struct offset_t {
    index_t x, y;
};


////////////////////////////////////////////////////////////////////////////////////////
// the offsets a knight can move to
static offset_t const knight_offsets[8] = {
    { -2, +1 }, // W
    { -2, -1 }, 
    { +2, +1 }, // E
    { +2, -1 }, 
    { +1, +2 }, // N
    { -1, +2 }, 
    { +1, -2 }, // S
    { -1, -2 }  
};

////////////////////////////////////////////////////////////////////////////////////////
// the offsets a rook can move to
static offset_t const rook_offsets[28] = {
    { -1,  0 }, // W
    { -2,  0 },
    { -3,  0 },
    { -4,  0 },
    { -5,  0 },
    { -6,  0 },
    { -7,  0 },
    { +1,  0 }, // E
    { +2,  0 },
    { +3,  0 },
    { +4,  0 },
    { +5,  0 },
    { +6,  0 },
    { +7,  0 },
    {  0, +1 }, // N
    {  0, +2 },
    {  0, +3 },
    {  0, +4 },
    {  0, +5 },
    {  0, +6 },
    {  0, +7 },
    {  0, -1 }, // S
    {  0, -2 },
    {  0, -3 },
    {  0, -4 },
    {  0, -5 },
    {  0, -6 },
    {  0, -7 }
};

////////////////////////////////////////////////////////////////////////////////////////
// the offsets a bishop can move to
static offset_t const bishop_offsets[28] = {
    { -1, +1 }, // NW
    { -2, +2 },
    { -3, +3 },
    { -4, +4 },
    { -5, +5 },
    { -6, +6 },
    { -7, +7 },
    { +1, +1 }, // NE
    { +2, +2 },
    { +3, +3 },
    { +4, +4 },
    { +5, +5 },
    { +6, +6 },
    { +7, +7 },
    { -1, -1 }, // SW
    { -2, -2 },
    { -3, -3 },
    { -4, -4 },
    { -5, -5 },
    { -6, -6 },
    { -7, -7 },
    { -1, +1 }, // SE
    { -2, +2 },
    { -3, +3 },
    { -4, +4 },
    { -5, +5 },
    { -6, +6 },
    { -7, +7 }
};

////////////////////////////////////////////////////////////////////////////////////////
// the offsets a queen can move to
static offset_t const queen_offsets[56] = {
    { -1,  0 }, // W
    { -2,  0 },
    { -3,  0 },
    { -4,  0 },
    { -5,  0 },
    { -6,  0 },
    { -7,  0 },
    { +1,  0 }, // E
    { +2,  0 },
    { +3,  0 },
    { +4,  0 },
    { +5,  0 },
    { +6,  0 },
    { +7,  0 },
    {  0, +1 }, // N
    {  0, +2 },
    {  0, +3 },
    {  0, +4 },
    {  0, +5 },
    {  0, +6 },
    {  0, +7 },
    {  0, -1 }, // S
    {  0, -2 },
    {  0, -3 },
    {  0, -4 },
    {  0, -5 },
    {  0, -6 },
    {  0, -7 },
    { -1, +1 }, // NW
    { -2, +2 },
    { -3, +3 },
    { -4, +4 },
    { -5, +5 },
    { -6, +6 },
    { -7, +7 },
    { +1, +1 }, // NE
    { +2, +2 },
    { +3, +3 },
    { +4, +4 },
    { +5, +5 },
    { +6, +6 },
    { +7, +7 },
    { -1, -1 }, // SW
    { -2, -2 },
    { -3, -3 },
    { -4, -4 },
    { -5, -5 },
    { -6, -6 },
    { -7, -7 },
    { -1, +1 }, // SE
    { -2, +2 },
    { -3, +3 },
    { -4, +4 },
    { -5, +5 },
    { -6, +6 },
    { -7, +7 }
};

////////////////////////////////////////////////////////////////////////////////////////
// the offsets a king can move to
static offset_t const king_offsets[8] = {
    { -1,  0 }, // W
    { -1, +1 }, // NW
    { +1,  0 }, // E
    { +1, +1 }, // NE
    {  0, +1 }, // N
    { -1, -1 }, // SW
    {  0, -1 }, // S
    { +1, -1 }  // SE
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

enum print_t {
    Error  = 0, 
    Always = 0,
    Debug0, 
    Debug1, 
    Debug2, 
    Debug3,
    Debug4,
};

extern print_t const level;

#define  ARRAYSZ(A) (sizeof(A) / sizeof(*(A)))

#define  MAX_VALUE ((long const)( 0b1000000000000)) // half the value of 13 bits - however large the 'value' field in move_t is
#define  MIN_VALUE ((long const)(0 - MAX_VALUE))

static unsigned const BOARD_SIZE = 64u;

static Color const White = 1u;
static Color const Black = 0u;

static Piece const Empty  = 0u;
static Piece const Pawn   = 1u;
static Piece const Knight = 2u;
static Piece const Bishop = 3u;
static Piece const Rook   = 4u;
static Piece const Queen  = 5u;
static Piece const King   = 6u;

static Piece const Type     = 0b00000111u;
static Piece const Side     = 0b00001000u;
static Piece const Moved    = 0b00010000u;
static Piece const Check    = 0b00100000u;

void info();

void show();

// Bool isValidTest();

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

int printf(print_t const required, char const * const fmt, ...);

static long const pieceValues[8] = {
            0,  // empty spot value
        10000,  // pawn value
        30000,  // knight value
        30000,  // bishop value
        50000,  // rook value
        90000,  // queen value
    MAX_VALUE,  // king value
            0   // padded for alignment and increased L1 and L2 cache hit gains
};

// a struct to represent all of the extra data about the 
// starting point and the ending point of a move
struct moveinfo_t {
public:
    index_t     from;
    index_t     col;
    index_t     row;
    Piece       p;
    Piece       type;
    Color       side;

    index_t     to;
    index_t     to_col;
    index_t     to_row;
    Piece       op;
    Piece       otype;
    Color       oside;

public:

};

#include "board.h"
#include "move.h"
#include "game.h"

void show_move(move_t const &move);
void show_pieces();
void show_piece(Piece const p);

extern game_t game;

#endif // MICROCHESS_INCL
