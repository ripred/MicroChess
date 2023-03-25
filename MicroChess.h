/**
 * MicroChess.h
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
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
    MAX_MOVES     = 106,        // R:11 N:8 B:11 Q:22 K:8 B:11 N:8 R:11 P:16

    NUM_BITS_PT   =   5,        // bits per field in point_t struct
    NUM_BITS_SPOT =   7,        // bits per field in move_t struct

    NUM_KNIGHT_OFFSETS =  8,    // number of offsets to check for knight moves
    NUM_BISHOP_OFFSETS = 28,    // number of offsets to check for bishop moves
    NUM_ROOK_OFFSETS   = 28,    // number of offsets to check for rook moves
    NUM_QUEEN_OFFSETS  = 56,    // number of offsets to check for queen moves
    NUM_KING_OFFSETS   =  8,    // number of offsets to check for king moves
};

typedef   int8_t   index_t;

// structure to hold the offsets that a piece can move to
struct offset_t {
    index_t x, y;
};

////////////////////////////////////////////////////////////////////////////////////////
// the offsets a knight can move to
static offset_t const knight_offsets[NUM_KNIGHT_OFFSETS] PROGMEM = {
    { -2, +1 }, { -2, -1 }, { +2, +1 }, { +2, -1 }, 
    { +1, +2 }, { -1, +2 }, { +1, -2 }, { -1, -2 }  
};

////////////////////////////////////////////////////////////////////////////////////////
// the offsets a rook can move to
static offset_t const rook_offsets[NUM_ROOK_OFFSETS] PROGMEM = {
    { -1,  0 }, { -2,  0 }, { -3,  0 }, { -4,  0 }, { -5,  0 }, { -6,  0 }, { -7,  0 },     // W
    { +1,  0 }, { +2,  0 }, { +3,  0 }, { +4,  0 }, { +5,  0 }, { +6,  0 }, { +7,  0 },     // E
    {  0, +1 }, {  0, +2 }, {  0, +3 }, {  0, +4 }, {  0, +5 }, {  0, +6 }, {  0, +7 },     // N
    {  0, -1 }, {  0, -2 }, {  0, -3 }, {  0, -4 }, {  0, -5 }, {  0, -6 }, {  0, -7 }      // S
};

////////////////////////////////////////////////////////////////////////////////////////
// the offsets a bishop can move to
static offset_t const bishop_offsets[NUM_BISHOP_OFFSETS] PROGMEM = {
    { -1, +1 }, { -2, +2 }, { -3, +3 }, { -4, +4 }, { -5, +5 }, { -6, +6 }, { -7, +7 },     // NW
    { +1, +1 }, { +2, +2 }, { +3, +3 }, { +4, +4 }, { +5, +5 }, { +6, +6 }, { +7, +7 },     // NE
    { -1, -1 }, { -2, -2 }, { -3, -3 }, { -4, -4 }, { -5, -5 }, { -6, -6 }, { -7, -7 },     // SW
    { +1, -1 }, { +2, -2 }, { +3, -3 }, { +4, -4 }, { +5, -5 }, { +6, -6 }, { +7, -7 }      // SE
};

////////////////////////////////////////////////////////////////////////////////////////
// the offsets a queen can move to
static offset_t const queen_offsets[NUM_QUEEN_OFFSETS] = {
    { -1,  0 }, { -2,  0 }, { -3,  0 }, { -4,  0 }, { -5,  0 }, { -6,  0 }, { -7,  0 },     // W
    { +1,  0 }, { +2,  0 }, { +3,  0 }, { +4,  0 }, { +5,  0 }, { +6,  0 }, { +7,  0 },     // E
    {  0, +1 }, {  0, +2 }, {  0, +3 }, {  0, +4 }, {  0, +5 }, {  0, +6 }, {  0, +7 },     // N
    {  0, -1 }, {  0, -2 }, {  0, -3 }, {  0, -4 }, {  0, -5 }, {  0, -6 }, {  0, -7 },     // S
    { -1, +1 }, { -2, +2 }, { -3, +3 }, { -4, +4 }, { -5, +5 }, { -6, +6 }, { -7, +7 },     // NW
    { +1, +1 }, { +2, +2 }, { +3, +3 }, { +4, +4 }, { +5, +5 }, { +6, +6 }, { +7, +7 },     // NE
    { -1, -1 }, { -2, -2 }, { -3, -3 }, { -4, -4 }, { -5, -5 }, { -6, -6 }, { -7, -7 },     // SW
    { +1, -1 }, { +2, -2 }, { +3, -3 }, { +4, -4 }, { +5, -5 }, { +6, -6 }, { +7, -7 }      // SE
};

////////////////////////////////////////////////////////////////////////////////////////
// the offsets a king can move to
static offset_t const king_offsets[NUM_KING_OFFSETS] PROGMEM = {
    { -1,  0 }, { -1, +1 }, { +1,  0 }, { +1, +1 }, 
    {  0, +1 }, { -1, -1 }, {  0, -1 }, { +1, -1 }
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

// print_t is used to set and control the output printing level
enum print_t {
    Never  =  0,        // never display
    None   =  0,
    Debug0, 
    Debug1,             // Normal game output
    Debug2,             // more verbose ..
    Debug3,
    Debug4,
    Error      = 99,    // always display
    Always     = 99,
    Everything = 99,
};

// the global setting that affects the level of output detail
extern print_t print_level;

// macro to return the number of elements in an array of any data type
#define  ARRAYSZ(A) (sizeof(A) / sizeof(*(A)))

// The max and min range for piece values
#define  MAX_VALUE ((long const)( 0b00000010000000))    // half the value of number of bits the 'value' field in move_t has
#define  MIN_VALUE ((long const)(0 - MAX_VALUE))

// the number of locations on the game board
static unsigned const BOARD_SIZE = 64u;

// The two sides
static Color const White = 1u;
static Color const Black = 0u;

// The Piece types
static Piece const Empty  = 0u;
static Piece const Pawn   = 1u;
static Piece const Knight = 2u;
static Piece const Bishop = 3u;
static Piece const Rook   = 4u;
static Piece const Queen  = 5u;
static Piece const King   = 6u;

// the masks for the Piece bit fields
static Piece const Type     = 0b00000111u;
static Piece const Side     = 0b00001000u;
static Piece const Moved    = 0b00010000u;
static Piece const Check    = 0b00100000u;

// show the game board
void show();

// test the validation macro
// Bool isValidTest();

// get the Type of a Piece
Piece getType(Piece b);

// see if a Piece is Empty
Bool  isEmpty(Piece b);

// get the value of a piece
int   getValue(Piece b);

// get the side for a Piece
Piece getSide(Piece b);

// see if a Piece has moved
Bool  hasMoved(Piece b);

// see if a Piece is in check
Bool  inCheck(Piece b);

// set the Type of a Piece
Piece setType(Piece b, Piece type);

// set the Color of a Piece
Piece setSide(Piece b, Piece side);

// set or reset the flag indicating a Piece as moved
Piece setMoved(Piece b, Bool hasMoved);

// set or reset the flag indicating a Piece is in check
Piece setCheck(Piece b, Bool inCheck);

// construct a Piece value
Piece makeSpot(Piece type, Piece side, Bool moved, Bool inCheck);

char *getName(Piece b);

char *getColor(Piece b);

// const char* addCommas(long int value);

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

#include "board.h"
#include "move.h"
#include "game.h"

void show_move(move_t const &move);
void show_pieces();
void show_piece(Piece const p);

extern game_t game;

void add_move(Color side, index_t from, index_t to, long value);
void add_pawn_moves(Piece p, index_t from, index_t col, index_t row, index_t fwd, Color side);
void add_knight_moves(index_t from, index_t col, index_t row, index_t fwd, Color side);
void add_bishop_moves(index_t from, index_t col, index_t row, index_t fwd, Color side);
void add_rook_moves(index_t from, index_t col, index_t row, index_t fwd, Color side);
void add_queen_moves(index_t from, index_t col, index_t row, index_t fwd, Color side);
void add_king_moves(index_t from, index_t col, index_t row, index_t fwd, Color side);

#endif // MICROCHESS_INCL
