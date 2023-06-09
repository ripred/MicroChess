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

typedef uint8_t Color;
typedef uint8_t Piece;
typedef uint8_t Bool;

////////////////////////////////////////////////////////////////////////////////////////
// Magic Numbers
// 
enum : uint32_t {
    PRN_SEED    = 0x232F89A3,   // default hash seed for psuedo random number generator
};

enum {
    VERSION_MAJOR      =  1,    // Major sofware revision number
    VERSION_MINOR      = 72,    // Minor sofware revision number

    SHUFFLE            = 10,    // Number of times we swap entries in the pieces[] array when shuffling

    END_COUNT          = 12,    // Number of pieces we drop below to go into "end game" mode

    MAX_REPS           =  3,    // Max number of times a pair of moves can be repeated

    MAX_PIECES         = 32,    // Max number of pieces in game.pieces[]

    NUM_BITS_PT        =  4,    // Bits per field in point_t struct
    NUM_BITS_SPOT      =  8,    // Bits per field in move_t struct

    LED_STRIP_PIN      =  6,    // Pin used for the LED strip for the board
    DEBUG1_PIN         =  5,    // Output debug LED pins
    DEBUG2_PIN         =  4,
    DEBUG3_PIN         =  3,
    DEBUG4_PIN         =  8,
};

typedef   int8_t   index_t;

enum {
    CHOOSE    = 0,
    ADD_MOVES = 1,
    CONSIDER  = 2,
    MAKE      = 3,
};

// print_t is used to set and control the output printing level
enum print_t : uint8_t {
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


////////////////////////////////////////////////////////////////////////////////////////
// macro to validate a location
#define  isValidPos(col, row) ((col >= 0 && col < 8 && row >= 0 && row < 8))

////////////////////////////////////////////////////////////////////////////////////////
// macro to enable the gathering of memory statistics at different ply levels
// un-comment to enable
// 
// #define   ENA_MEM_STATS

// macro to return the number of elements in an array of any data type
#define     ARRAYSZ(A)         (sizeof((A)) / sizeof(*((A))))

// macros to manipulate a continguous segment of memory as a series of bits
#define     setbit(_A, _B)     ((char*)(_A))[(_B) / 8] |=  (0x80 >> ((_B) % 8))
#define     clrbit(_A, _B)     ((char*)(_A))[(_B) / 8] &= ~(0x80 >> ((_B) % 8))
#define     getbit(_A, _B)     ((char*)(_A))[(_B) / 8] &   (0x80 >> ((_B) % 8))

// The max and min range for piece values
#define     MAX_VALUE       ((long)(LONG_MAX/2))
#define     MIN_VALUE       ((long)(0 - MAX_VALUE))

// The number of locations on the game board
static index_t constexpr BOARD_SIZE = 64u;

// The two sides
static Bool constexpr True  = 1u;
static Bool constexpr False = 0u;

// The two sides
static Color constexpr White = 1u;
static Color constexpr Black = 0u;

// The Piece types
static Piece constexpr Empty  = 0u;
static Piece constexpr Pawn   = 1u;
static Piece constexpr Knight = 2u;
static Piece constexpr Bishop = 3u;
static Piece constexpr Rook   = 4u;
static Piece constexpr Queen  = 5u;
static Piece constexpr King   = 6u;

// The masks for the Piece bit fields
static Piece constexpr Type  = 0b00000111u;
static Piece constexpr Side  = 0b00001000u;
static Piece constexpr Moved = 0b00010000u;
static Piece constexpr Check = 0b00100000u;

// show the game board
extern void     show();

// get the Type of a Piece
extern Piece    getType(Piece b);

// see if a Piece is Empty
extern Bool     isEmpty(Piece b);

// get the value of a piece
extern int      getValue(Piece b);

// get the side for a Piece
extern Piece    getSide(Piece b);

// see if a Piece has moved
extern Bool     hasMoved(Piece b);

// see if a Piece is in check
extern Bool     inCheck(Piece b);

// set the Type of a Piece
extern Piece    setType(Piece b, Piece type);

// set the Color of a Piece
extern Piece    setSide(Piece b, Piece side);

// set or reset the flag indicating a Piece as moved
extern Piece    setMoved(Piece b, Bool hasMoved);

// set or reset the flag indicating a Piece is in check
extern Piece    setCheck(Piece b, Bool inCheck);

// construct a Piece value
extern Piece    makeSpot(Piece type, Piece side, Bool moved, Bool inCheck);

extern void     direct_write(index_t const pin, Bool const value);
extern void     show_low_memory();
extern void     show_quiescent_search();
extern void     show_timeout();
extern void     show_check();
extern void     show_check_status();

extern char    *ftostr(double const value, int const dec, char * const buff);
extern char    *addCommas(long int value);
extern void     printrep(print_t const level, char const c, index_t repeat);
extern void     printnl(print_t const level, index_t repeat= 1);
extern int      debug(char const * const fmt, ...);

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
enum state_t : uint8_t {
    PLAYING = 0,
    STALEMATE,
    WHITE_CHECKMATE,
    BLACK_CHECKMATE,
    WHITE_3_MOVE_REP,
    BLACK_3_MOVE_REP,
    MOVE_LIMIT
};

#include "board.h"
#include "move.h"
#include "game.h"
#include "conv.h"


////////////////////////////////////////////////////////////////////////////////////////
// A structure to represent an opening move or sequences of moves
struct book_t {
    uint8_t const
        from : 6,  // the starting location
          to : 6;  // the ending location
    
    static Color const side;

    book_t(index_t const f, index_t const t) : from(f), to(t) {}
};


// define a data type for a callback move generation handler
typedef void    (generator_t(struct piece_gen_t &gen));


// The piece_gen_t type is a parameter passing structure used
// to speed up the move generation calls for the piece types.
// This is the structure that is passed to each generator function
// for each piece type.
class piece_gen_t {
    public:
    // The move_t structure to use. Initially for each piece generator, 
    // only the 'from' field of the move is valid. The generator function 
    // fills in the 'to' field as it generates moves.
    move_t      & move;

    // The best move found for this piece type so far
    move_t      & wbest;
    move_t      & bbest;

    // The function to call for each move to be evaluated
    generator_t * callme;

    uint8_t 
                 piece : 6,     // The Piece being moved
            evaluating : 1,     // True if we are just evaluating the move
                  side : 1,     // The side the piece is for: White or Black

            num_wmoves : 5,     // The number of white moves available
                  type : 3,     // The Type of the Piece: [Empty|Pawn|Knight|Rook|Bishop|Queen|King]

                   col : 3,     // The column of the piece being moved
           whites_turn : 1,     // True when this move is for White's side
                   row : 3,     // The row of the piece being moved
                cutoff : 1,     // True if we have reached the alpha or beta cutoff

            num_bmoves : 5,     // The number of white moves available
           piece_index : 5;     // The index into the pieces list of the piece being evaluated

    piece_gen_t(move_t &m);

    piece_gen_t(move_t &m, move_t &wb, move_t &bb, generator_t *cb, Bool const eval);

    void init(board_t const &board, game_t const &game);

};  // piece_gen_t


// Display a piece, or a move, the piece list, or a time duration
extern void     show_side(Color const side);
extern void     show_check(Color const side, Bool const mate = False);
extern void     show_piece(Piece const piece);
extern void     show_move(move_t const &move, Bool const align = False);
extern void     show_pieces();
extern void     show_time(uint32_t ms);

// show the game time and move statistics
extern void     show_stats();

// Show the current memory statistics
extern Bool     check_mem(index_t const level);
extern int      freeMemory();

// Control an external LED strip to display the board state
extern void     init_led_strip();
extern void     set_led_strip(index_t const flash = -1);

extern Bool     timeout();

extern Bool     check_serial();
extern Bool     check_book();

extern void     check_kings();
extern void     consider_move(piece_gen_t &gen);
extern long     make_move(piece_gen_t &gen);
extern long     evaluate(piece_gen_t &gen);
extern Bool     would_repeat(move_t const &move);
extern Bool     add_to_history(move_t const &move);
extern void     choose_best_moves(move_t &wbest, move_t &bbest, generator_t const callback);
extern void     check_kings();

extern index_t  add_pawn_moves(piece_gen_t &gen);
extern index_t  add_knight_moves(piece_gen_t &gen);
extern index_t  add_bishop_moves(piece_gen_t &gen);
extern index_t  add_rook_moves(piece_gen_t &gen);
extern index_t  add_queen_moves(piece_gen_t &gen);
extern index_t  add_king_moves(piece_gen_t &gen);

#endif // MICROCHESS_INCL