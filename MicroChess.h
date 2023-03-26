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

#pragma pack(0)

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
    MAX_PIECES    = 32,
    MAX_MOVES     = 106,         // R:11 N:8 B:11 Q:22 K:8 B:11 N:8 R:11 P:16 == 106

    NUM_BITS_PT   =   4,        // bits per field in point_t struct
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
// macro to validate a location
#define  isValidPos(col, row) ((col >= 0 && col < 8 && row >= 0 && row < 8))

// print_t is used to set and control the output printing level
enum print_t {
    Error  =  0,        // always display
    Always =  0,
    None   =  0,        // for use in setting print_level
    Debug0,             // less verbose..
    Debug1,             // Normal game output
    Debug2,             // more verbose..
    Debug3,
    Debug4,
    Never      = 99,    // never display
    Everything = 99,    // for use in setting print_level
};

// the global setting that affects the level of output detail
extern print_t print_level;

// macro to return the number of elements in an array of any data type
#define  ARRAYSZ(A) (sizeof(A) / sizeof(*(A)))

// The max and min range for piece values
#define  MAX_VALUE ((long const)( 0b0000100000000000))    // half the value of number of bits the 'value' field in move_t has
#define  MIN_VALUE ((long const)(0 - MAX_VALUE))

// the number of locations on the game board
static unsigned const BOARD_SIZE = 64u;

// The two sides
static Bool const True  = 1u;
static Bool const False = 0u;

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

int debug(print_t const required, char const * const fmt, ...);

#define printf(level, str, ...) \
do { \
    static const char debug_string[] PROGMEM = str; \
    debug(level, debug_string, ##__VA_ARGS__); \
} while(0);

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

struct conv1_t {
    union {
        struct {
             uint8_t  col : 3,
                      row : 3,
                     type : 3,
                     side : 1;
        } pt;
        struct {
            uint8_t index : 6,
                     type : 3,
                     side : 1;
        } ndx;
    } u;

    conv1_t() : u{ 0, 0, Empty, Black } {}
    conv1_t(uint8_t index) { u.ndx = { index, Empty, Black  }; }
    conv1_t(uint8_t index, uint8_t type, uint8_t side) { u.ndx = { index, type, side }; }
    conv1_t(uint8_t col, uint8_t row) : u{ col, row, Empty, Black } {}

};  // conv1_t

struct conv2_t {
    conv1_t from, to;

    conv2_t() {}
    conv2_t(uint8_t from, uint8_t to) : from(from), to(to){}
};

void add_move(Color side, index_t from, index_t to, long value);
void add_pawn_moves(Piece p, index_t from, index_t fwd, Color side);
void add_knight_moves(index_t from, index_t fwd, Color side);
void add_bishop_moves(index_t from, index_t fwd, Color side);
void add_rook_moves(index_t from, index_t fwd, Color side);
void add_queen_moves(index_t from, index_t fwd, Color side);
void add_king_moves(index_t from, index_t fwd, Color side);

#endif // MICROCHESS_INCL
