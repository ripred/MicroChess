/*********************************************************************
 * MicroChess.h
 *
 * the MicroChess project: https://github.com/ripred/MicroChess
 *
 * Global definitions, macros, and forward declarations for the
 * MicroChess engine.
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
    PRN_SEED = 0x232F89A3,   // default hash seed for pseudo random number generator
};

enum {
    VERSION_MAJOR = 1,    // Major software revision number
    VERSION_MINOR = 90,    // Minor software revision number

    SHUFFLE = 10,    // Number of times we swap entries in the pieces[] array when shuffling

    END_COUNT = 12,    // Number of pieces we drop below to go into "end game" mode

    MAX_REPS = 3,    // Max number of times a pair of moves can be repeated

    MAX_PIECES = 32,    // Max number of pieces in game.pieces[]

    NUM_BITS_PT = 4,    // Bits per field in point_t struct
    NUM_BITS_SPOT = 8,  // Bits per field in move_t struct

#if not TEENSYDUINO
    LED_STRIP_PIN = 6,    // Pin used for the LED strip for the board
    DEBUG1_PIN = 5,    // Output debug LED pins
    DEBUG2_PIN = 4,
    DEBUG3_PIN = 3,
    DEBUG4_PIN = 8,
#else
    LED_STRIP_PIN = 0xFF,    // Pin used for the LED strip for the board
    DEBUG1_PIN = 0xFF,    // Output debug LED pins
    DEBUG2_PIN = 0xFF,
    DEBUG3_PIN = 0xFF,
    DEBUG4_PIN = 0xFF,
#endif
};

typedef int8_t index_t;

enum {
    CHOOSE = 0,
    ADD_MOVES = 1,
    CONSIDER = 2,
    MAKE = 3,
};

// print_t is used to set and control the output printing level
enum print_t : uint8_t {
    Error = 0,        // always display
    Always = 0,
    None = 0,        // for use in setting print_level
    Debug0,             // less verbose..
    Debug1,             // Normal game output
    Debug2,             // more verbose..
    Debug3,
    Debug4,
    Never = 99,    // never display
    Everything = 99,    // for use in setting print_level
};

////////////////////////////////////////////////////////////////////////////////////////
// macro to validate a location
#define isValidPos(col, row) ((col >= 0 && col < 8 && row >= 0 && row < 8))

// macro to enable the gathering of memory statistics at different ply levels
// #define ENA_MEM_STATS

// macro to return the number of elements in an array of any data type
#define ARRAYSZ(A) (sizeof((A)) / sizeof(*((A))))

// macros to manipulate a contiguous segment of memory as a series of bits
#define setbit(_A, _B) ((char*)(_A))[(_B) / 8] |=  (0x80 >> ((_B) % 8))
#define clrbit(_A, _B) ((char*)(_A))[(_B) / 8] &= ~(0x80 >> ((_B) % 8))
#define getbit(_A, _B) ((char*)(_A))[(_B) / 8] &   (0x80 >> ((_B) % 8))

// ---------------------------------------------------------------------
//  MAX_VALUE / MIN_VALUE – compile‑time constants derived from LONG_MAX
// ---------------------------------------------------------------------
constexpr long MAX_VALUE = LONG_MAX / 2L;   // same semantics as the original macro
constexpr long MIN_VALUE = -MAX_VALUE;      // negative of MAX_VALUE

// The number of locations on the game board
static index_t constexpr BOARD_SIZE = 64u;

// The two sides
static Bool constexpr True = 1u;
static Bool constexpr False = 0u;

// The two sides
static Color constexpr White = 1u;
static Color constexpr Black = 0u;

// The Piece types
static Piece constexpr Empty = 0u;
static Piece constexpr Pawn = 1u;
static Piece constexpr Knight = 2u;
static Piece constexpr Bishop = 3u;
static Piece constexpr Rook = 4u;
static Piece constexpr Queen = 5u;
static Piece constexpr King = 6u;

// The masks for the Piece bit fields
static Piece constexpr Type = 0b00000111u;
static Piece constexpr Side = 0b00001000u;
static Piece constexpr Moved = 0b00010000u;
static Piece constexpr Check = 0b00100000u;

// show the game board
extern void show();

// get the Type of a Piece
extern Piece getType(Piece b);

// see if a Piece is Empty
extern Bool isEmpty(Piece b);

// get the value of a piece
extern int getValue(Piece b);

// get the side for a Piece
extern Piece getSide(Piece b);

// see if a Piece has moved
extern Bool hasMoved(Piece b);

// see if a Piece is in check
extern Bool inCheck(Piece b);

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

extern void direct_write(index_t const pin, Bool const value);
extern void show_low_memory();
extern void show_quiescent_search();
extern void show_timeout();
extern void show_check();
extern void show_check_status();

extern char *ftostr(double const value, int const dec, char * const buff);
extern char *addCommas(long int value);
extern void printrep(print_t const level, char const c, index_t repeat);
extern void printnl(print_t const level, index_t repeat = 1);
extern int debug(char const * const progmem, ...);

#define printf(__level, __str, ...)                                          \
    do {                                                                    \
        if (game.options.print_level >= __level) {                           \
            static const char debug_string[] PROGMEM = __str;                \
            debug(debug_string, ##__VA_ARGS__);                             \
        }                                                                   \
    } while (0)

static long constexpr pieceValues[8] = {
    0,          // empty spot value
    1000,       // pawn value
    3000,       // knight value
    3000,       // bishop value
    5000,       // rook value
    9000,       // queen value
    MAX_VALUE,  // king value
    0           // padded for alignment and increased L1 and L2 cache hit gains
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

// Add for non‑AVR builds
#ifndef pgm_get_far_address
#define pgm_get_far_address(x) ((uintptr_t)(&(x)))
#endif

#endif // MICROCHESS_INCL
