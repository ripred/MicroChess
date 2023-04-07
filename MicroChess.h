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
enum : uint32_t 
{
    PRN_SEED    = 0x232F89A3,   // default hash seed for psuedo random number generator
};

enum 
{
    MAX_PLY            =  0,    // max ply depth

    MOVE_LIMIT         = 100,   // the maximum number of moves allowed in a game

    MAX_PIECES         = 32,    // max number of pieces in game.pieces[]

    NUM_BITS_PT        =  4,    // bits per field in point_t struct
    NUM_BITS_SPOT      =  7,    // bits per field in move_t struct

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

// macro to return the number of elements in an array of any data type
#define  ARRAYSZ(A) (sizeof(A) / sizeof(*(A)))

// The max and min range for piece values
#define  MAX_VALUE ((long const)( 0b010000000000000000))    // half the value of number of bits the 'value' field in move_t has
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

const char* addCommas(long int value);

int debug(char const * const fmt, ...);

#define printf(level, str, ...) \
if (game.options.print_level >= level) { \
    static const char debug_string[] PROGMEM = str; \
    debug(debug_string, ##__VA_ARGS__); \
}

static long constexpr pieceValues[8] = {
            0,      // empty spot value
          100,      // pawn value
          300,      // knight value
          300,      // bishop value
          500,      // rook value
          900,      // queen value
    MAX_VALUE,      // king value
            0       // padded for alignment and increased L1 and L2 cache hit gains
};

// adjustable multipiers to alter importance of mobility or center proximity
// during board evaluation. season to taste
static long constexpr mobilityBonus = 3L;
static long constexpr   centerBonus = 3L;

extern long const center_bonus[8][7][2] PROGMEM;
extern long const material_bonus[7][2] PROGMEM;

extern offset_t const knight_offsets[NUM_KNIGHT_OFFSETS] PROGMEM;
extern offset_t const rook_offsets[NUM_ROOK_OFFSETS] PROGMEM;
extern offset_t const bishop_offsets[NUM_BISHOP_OFFSETS] PROGMEM;
extern offset_t const queen_offsets[NUM_QUEEN_OFFSETS] PROGMEM;
extern offset_t const king_offsets[NUM_KING_OFFSETS] PROGMEM;

#include "board.h"
#include "move.h"
#include "game.h"
#include "conv.h"

extern game_t game;

typedef Bool (generator_t(move_t &, move_t &));

class piece_gen_t {
public:
    move_t      & move;
    move_t      & best;
    generator_t  *callme;

    piece_gen_t(move_t &m, move_t &b, generator_t *cb) :
        move(m), best(b), callme(cb) {}
};

extern void show_move(move_t const &move);
extern void show_pieces();
extern void show_piece(Piece const p);

extern void show_stats();

extern long make_move(move_t const &move, Bool const restore);

Bool consider_move(move_t &move, move_t &best);

extern void choose_best_move(move_t &best_white, move_t &best_black, generator_t callback);

extern void add_pawn_moves(piece_gen_t &gen);
extern void add_knight_moves(piece_gen_t &gen);
extern void add_bishop_moves(piece_gen_t &gen);
extern void add_rook_moves(piece_gen_t &gen);
extern void add_queen_moves(piece_gen_t &gen);
extern void add_king_moves(piece_gen_t &gen);

#endif // MICROCHESS_INCL