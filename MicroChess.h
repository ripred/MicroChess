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

#pragma pack(0)

#include <Arduino.h>
#include <inttypes.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

typedef unsigned char Color;
typedef unsigned char Piece;
typedef unsigned char Bool;

////////////////////////////////////////////////////////////////////////////////////////
// Magic Numbers
// 
enum : uint32_t {
    PRN_SEED    = 0x232F89A3,   // default hash seed for psuedo random number generator
};

enum {
    MAX_PLY            =  3,    // max ply depth

    DEF_TIME_LIMIT     = 30000, // default time limit per move

    MAX_REPS           =  3,    // the max number of times a pair of moves can be repeated

    MAX_PIECES         = 32,    // max number of pieces in game.pieces[]

    NUM_BITS_PT        =  4,    // bits per field in point_t struct
    NUM_BITS_SPOT      =  7,    // bits per field in move_t struct

    SHOW_DURATION      = 20,    // Duration that LED indicator are shown

    DEBUG1_PIN         =  5,    // output debug LED pins
    DEBUG2_PIN         =  4,
    DEBUG3_PIN         =  3,
};

typedef   int8_t   index_t;

// structure to hold the offsets that a piece can move to
struct offset_t {
    index_t x, y;
};

////////////////////////////////////////////////////////////////////////////////////////
// macro to validate a location
#define  isValidPos(col, row) ((col >= 0 && col < 8 && row >= 0 && row < 8))

////////////////////////////////////////////////////////////////////////////////////////
// macro to enable the gathering of memory statistics at different ply levels
// un-comment to enable
// 
// #define   ENA_MEM_STATS

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

// Macro to return the number of elements in an array of any data type
#define  ARRAYSZ(A) (sizeof(A) / sizeof(*(A)))

// Macros to manipulate a continguous segment of memory as a series of bits
#define     _setbit(_A, _B)     ((char*)(_A))[(_B) / 8] |=  (0x80 >> ((_B) % 8))
#define     _clrbit(_A, _B)     ((char*)(_A))[(_B) / 8] &= ~(0x80 >> ((_B) % 8))
#define     _getbit(_A, _B)     ((char*)(_A))[(_B) / 8] &   (0x80 >> ((_B) % 8))

// The max and min range for piece values
#define  MAX_VALUE ((long const)(LONG_MAX / 8))
#define  MIN_VALUE ((long const)(0 - MAX_VALUE))

// The number of locations on the game board
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
extern void show();

// get the Type of a Piece
extern Piece getType(Piece b);

// see if a Piece is Empty
extern Bool  isEmpty(Piece b);

// get the value of a piece
extern int   getValue(Piece b);

// get the side for a Piece
extern Piece getSide(Piece b);

// see if a Piece has moved
extern Bool  hasMoved(Piece b);

// see if a Piece is in check
extern Bool  inCheck(Piece b);

// set the Type of a Piece
extern Piece setType(Piece b, Piece type);

// set the Color of a Piece
extern Piece setSide(Piece b, Piece side);

// set or reset the flag indicating a Piece as moved
extern Piece setMoved(Piece b, Bool hasMoved);

// set or reset the flag indicating a Piece is in check
extern Piece setCheck(Piece b, Bool inCheck);

// construct a Piece value
extern Piece makeSpot(Piece type, Piece side, Bool moved, Bool inCheck);

extern void show_low_memory();
extern void show_quiescent_search();
extern void show_timeout();

extern const char* ftostr(double const value, int const dec, char * const buff);

extern const char* addCommas(long int value);

extern int debug(char const * const fmt, ...);

#define printf(__level, __str, ...) \
if (game.options.print_level >= __level) { \
    static const char debug_string[] PROGMEM = __str; \
    debug(debug_string, ##__VA_ARGS__); \
}

static long constexpr pieceValues[8] = {
             0,      // empty spot value
          1000,      // pawn value
          3000,      // knight value
          3000,      // bishop value
          5000,      // rook value
          9000,      // queen value
     MAX_VALUE,      // king value
             0       // padded for alignment and increased L1 and L2 cache hit gains
};

// Alias' for the current game state
enum state_t {
    PLAYING,
    STALEMATE,
    WHITE_CHECKMATE,
    BLACK_CHECKMATE,
    WHITE_3_MOVE_REP,
    BLACK_3_MOVE_REP,
    FIFTY_MOVES
};

#include "board.h"
#include "move.h"
#include "game.h"
#include "conv.h"

extern game_t game;


// define a data type for a callback move generation handler
typedef Bool (generator_t(struct piece_gen_t &gen));

// The piece_gen_t type is a parameter passing structure used
// to speed up the move generation calls for the piece types.
// This is the structure that is passed to each generator function
// for each piece type.
struct piece_gen_t {
    // The move_t structure to use. Initially for each piece generator, 
    // only the 'from' field of the move is valid. The generator function 
    // fills in the 'to' field as it generates moves.
    move_t      & move;

    // The best move found for this piece type so far
    move_t      & best;

    // The function to call for each move to be evaluated
    generator_t * callme;

    uint8_t evaluating : 1,     // True if we are just evaluating the move
                  side : 1,     // The side the piece is for: White or Black
           whites_turn : 1,     // True when this move is for White's side
                cutoff : 1,     // True if we have reached the alpha or beta cutoff
                   col : 4,
                   row : 4;

    // The index into the pieces list of the piece being evaluated
    index_t     piece_index;

    // The Piece beig moved
    Piece       piece;

    // The Type of the Piece: [Empty | Pawn | Knight | Rook | Bishop | Queen | King]
    Piece       type;

    piece_gen_t(move_t &m, move_t &b, generator_t *cb, Bool const eval) :
        move(m), best(b), callme(cb), evaluating(eval), cutoff(False)
    {
        piece = board.get(move.from);
        type = getType(piece);
        side = getSide(piece);
        col = move.from % 8;
        row = move.from / 8;
    }

}; // piece_gen_t


// display a piece, or a move, or the piece list
extern void show_piece(Piece const p);
extern void show_move(move_t const &move);
extern void show_pieces();
extern void show_time(uint32_t ms);

// show the game time and move statistics
extern void show_stats();

// show the current memory statistics
extern Bool check_mem();
extern int freeMemory();

// control an external LED strip to display the board state
extern void init_led_strip();
extern void set_led_strip();

extern Bool timeout();

extern long make_move(piece_gen_t & gen);

extern Bool would_repeat(move_t const move);

extern Bool add_to_history(move_t const &move);

extern index_t choose_best_move(Color const who, move_t &best, generator_t callback);

extern Bool consider_move(piece_gen_t &gen);

extern index_t add_pawn_moves(piece_gen_t &gen);
extern index_t add_knight_moves(piece_gen_t &gen);
extern index_t add_bishop_moves(piece_gen_t &gen);
extern index_t add_rook_moves(piece_gen_t &gen);
extern index_t add_queen_moves(piece_gen_t &gen);
extern index_t add_king_moves(piece_gen_t &gen);

#endif // MICROCHESS_INCL