/**
 * chessutil.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * MicroChess utility functions
 * 
 */

#include <Arduino.h>
#include <avr/pgmspace.h>
#include "MicroChess.h"
#include <stdarg.h>

print_t print_level = Debug1;

// get the Type of a Piece
Piece getType(Piece b) 
{
    return Type & b;
}


// see if a Piece is Empty
Bool isEmpty(Piece b) 
{
    return getType(b) == Empty;
}


// get the value of a piece
int getValue(Piece b) 
{
    return pieceValues[getType(b)]; 
}


// get the side for a Piece
Piece getSide(Piece b) 
{ 
    return (Side & b) >> 3u; 
}


// see if a Piece has moved
Bool hasMoved(Piece b) 
{ 
    return (Moved & b) == Moved; 
}


// see if a Piece is in check
Bool inCheck(Piece b) 
{ 
    return (Check & b) == Check; 
}


// set the Type of a Piece
Piece setType(Piece b, Piece type) 
{ 
    return (b & ~Type) | (type & Type); 
}


// set the Color of a Piece
Piece setSide(Piece b, Piece side) {
    return (b & ~Side) | ((side << 3u) & Side); 
}


// set or reset the flag indicating a Piece as moved
Piece setMoved(Piece b, Bool hasMoved) 
{ 
    return (b & ~Moved) | (hasMoved ? Moved : 0); 
}


// set or reset the flag indicating a Piece is in check
Piece setCheck(Piece b, Bool inCheck) 
{ 
    return (b & ~Check) | (inCheck ? Check : 0); 
}


// construct a Piece value
Piece makeSpot(Piece type, Piece side, unsigned char moved, unsigned char inCheck) {
    return setType(0, type) | setSide(0, side) | setMoved(0, moved) | setCheck(0, inCheck);
}


char *getName(Piece b) {
    static char const * names[] = {"Empty", "Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};
    static const int num_names = ARRAYSZ(names);
    static const int type_offset = 0;

    int const type = getType(b);

    if (type < type_offset || type >= type_offset + num_names) {
        return nullptr;
    }

    return (char*) names[type + type_offset];
}


char *getColor(Piece b) 
{ 
    return getSide(b) == White ? (char*) "White" : (char*) "Black"; 
}


const char* addCommas(long int value) {
    static char buff[32];
    snprintf(buff, 32, "%ld", value);
    for (int i = strlen(buff) - 3; i > 0; i -= 3) {
        memmove(&buff[i + 1], &buff[i], strlen(buff) - i);
        buff[i] = ',';
    }
    return buff;
}

int debug(print_t const required, char const * const progmem, ...) {
    if (print_level >= required) {
        char fmt[128];
        strcpy_P(fmt, progmem);
        
        char buff[128];
        va_list argList;
        va_start(argList, fmt);
        vsnprintf(buff, ARRAYSZ(buff), fmt, argList);
        va_end(argList);

        return Serial.write(buff, strlen(buff));
    }

    return 0;
}

// runtime memory usage functions
#include <unistd.h>

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
    char top;
#ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
#else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

void printMemoryStats() {
    // ============================================================
    // startup memory
    int totalRam = 2048;
    // int freeRam = freeMem();
    int freeRam = freeMemory();
    int usedRam = totalRam - freeRam;

    printf(Debug2, "Total SRAM = %d\n", totalRam);
    printf(Debug2, "Free SRAM = %d\n", freeRam);
    printf(Debug2, "Used SRAM = %d\n", usedRam);

    printf(Debug2, "sizeof(move_t) = %d\n", sizeof(move_t));
    printf(Debug2, 
        "meaning there is room for %d more move_t entries.\n", 
        freeRam / sizeof(move_t) );
    printf(Debug2, 
        "or %d more move_t entries per move list.\n", 
        ((freeRam / sizeof(move_t)) / 2) );
}


// display a Piece's color and type
void show_piece(Piece const p) 
{
    Piece const type = getType(p);
    Color const side = getSide(p);

    printf(Debug1, "%s %s", 
        (Empty == type ?  "" : (White == side ?  "White" : "Black")), 
        (Empty == type ?  "Empty" :
          Pawn == type ?   "Pawn" :
        Knight == type ? "Knight" :
        Bishop == type ? "Bishop" :
          Rook == type ?   "Rook" :
         Queen == type ?  "Queen" : "King"));
}


// debug function to display all of the point_t's in the game.pieces[game.piece_count] list:
void show_pieces() 
{
    printf(Debug1, "game.pieces[%2d] = {\n", game.piece_count);
    for (int i = 0; i < game.piece_count; i++) {
        point_t const &loc = game.pieces[i];
        index_t const col = loc.x;
        index_t const row = loc.y;
        Piece  const p = board.get(col + row * 8);
        printf(Debug1, "    game.pieces[%2d] = %d, %d: ", i, col, row);
        show_piece(p);
        printf(Debug1, "\n");
    }
    printf(Debug1, "};\n");
}


// display a piece being moved
void show_move(move_t const &move) {
    index_t const    col = move.from % 8;
    index_t const    row = move.from / 8;
    index_t const   from = col + row * 8;
    Piece   const      p = board.get(from);
    index_t const to_col = move.to % 8;
    index_t const to_row = move.to / 8;

    show_piece(p);

    printf(Debug1, " from: %d,%d (%c%c) to: %d,%d (%c%c)", 
           col,    row,    col + 'A', '8' -    row, 
        to_col, to_row, to_col + 'A', '8' - to_row);
}

// use pre-computed bonus tables for speed!
static long constexpr center_bonus[8][7][2] PROGMEM = {
    //                   Black             ,            White 
    {
        { 0 *  Empty * centerBonus * -1, 0 *  Empty * centerBonus * +1 },       // col/row offset 0
        { 0 *   Pawn * centerBonus * -1, 0 *   Pawn * centerBonus * +1 },
        { 0 * Knight * centerBonus * -1, 0 * Knight * centerBonus * +1 },
        { 0 * Bishop * centerBonus * -1, 0 * Bishop * centerBonus * +1 },
        { 0 *   Rook * centerBonus * -1, 0 *   Rook * centerBonus * +1 },
        { 0 *  Queen * centerBonus * -1, 0 *  Queen * centerBonus * +1 },
        { 0 *   King * centerBonus * -1, 0 *   King * centerBonus * +1 },
    }, {
        { 1 *  Empty * centerBonus * -1, 1 *  Empty * centerBonus * +1 },       // col/row offset 1
        { 1 *   Pawn * centerBonus * -1, 1 *   Pawn * centerBonus * +1 },
        { 1 * Knight * centerBonus * -1, 1 * Knight * centerBonus * +1 },
        { 1 * Bishop * centerBonus * -1, 1 * Bishop * centerBonus * +1 },
        { 1 *   Rook * centerBonus * -1, 1 *   Rook * centerBonus * +1 },
        { 1 *  Queen * centerBonus * -1, 1 *  Queen * centerBonus * +1 },
        { 1 *   King * centerBonus * -1, 1 *   King * centerBonus * +1 },
    }, {
        { 2 *  Empty * centerBonus * -1, 2 *  Empty * centerBonus * +1 },       // col/row offset 2
        { 2 *   Pawn * centerBonus * -1, 2 *   Pawn * centerBonus * +1 },
        { 2 * Knight * centerBonus * -1, 2 * Knight * centerBonus * +1 },
        { 2 * Bishop * centerBonus * -1, 2 * Bishop * centerBonus * +1 },
        { 2 *   Rook * centerBonus * -1, 2 *   Rook * centerBonus * +1 },
        { 2 *  Queen * centerBonus * -1, 2 *  Queen * centerBonus * +1 },
        { 2 *   King * centerBonus * -1, 2 *   King * centerBonus * +1 },
    }, {
        { 3 *  Empty * centerBonus * -1, 3 *  Empty * centerBonus * +1 },       // col/row offset 3
        { 3 *   Pawn * centerBonus * -1, 3 *   Pawn * centerBonus * +1 },
        { 3 * Knight * centerBonus * -1, 3 * Knight * centerBonus * +1 },
        { 3 * Bishop * centerBonus * -1, 3 * Bishop * centerBonus * +1 },
        { 3 *   Rook * centerBonus * -1, 3 *   Rook * centerBonus * +1 },
        { 3 *  Queen * centerBonus * -1, 3 *  Queen * centerBonus * +1 },
        { 3 *   King * centerBonus * -1, 3 *   King * centerBonus * +1 },
    }, {
        { 4 *  Empty * centerBonus * -1, 4 *  Empty * centerBonus * +1 },       // col/row offset 4
        { 4 *   Pawn * centerBonus * -1, 4 *   Pawn * centerBonus * +1 },
        { 4 * Knight * centerBonus * -1, 4 * Knight * centerBonus * +1 },
        { 4 * Bishop * centerBonus * -1, 4 * Bishop * centerBonus * +1 },
        { 4 *   Rook * centerBonus * -1, 4 *   Rook * centerBonus * +1 },
        { 4 *  Queen * centerBonus * -1, 4 *  Queen * centerBonus * +1 },
        { 4 *   King * centerBonus * -1, 4 *   King * centerBonus * +1 },
    }, {
        { 5 *  Empty * centerBonus * -1, 5 *  Empty * centerBonus * +1 },       // col/row offset 5
        { 5 *   Pawn * centerBonus * -1, 5 *   Pawn * centerBonus * +1 },
        { 5 * Knight * centerBonus * -1, 5 * Knight * centerBonus * +1 },
        { 5 * Bishop * centerBonus * -1, 5 * Bishop * centerBonus * +1 },
        { 5 *   Rook * centerBonus * -1, 5 *   Rook * centerBonus * +1 },
        { 5 *  Queen * centerBonus * -1, 5 *  Queen * centerBonus * +1 },
        { 5 *   King * centerBonus * -1, 5 *   King * centerBonus * +1 },
    }, {
        { 6 *  Empty * centerBonus * -1, 6 *  Empty * centerBonus * +1 },       // col/row offset 6
        { 6 *   Pawn * centerBonus * -1, 6 *   Pawn * centerBonus * +1 },
        { 6 * Knight * centerBonus * -1, 6 * Knight * centerBonus * +1 },
        { 6 * Bishop * centerBonus * -1, 6 * Bishop * centerBonus * +1 },
        { 6 *   Rook * centerBonus * -1, 6 *   Rook * centerBonus * +1 },
        { 6 *  Queen * centerBonus * -1, 6 *  Queen * centerBonus * +1 },
        { 6 *   King * centerBonus * -1, 6 *   King * centerBonus * +1 },
    }, {
        { 7 *  Empty * centerBonus * -1, 7 *  Empty * centerBonus * +1 },       // col/row offset 7
        { 7 *   Pawn * centerBonus * -1, 7 *   Pawn * centerBonus * +1 },
        { 7 * Knight * centerBonus * -1, 7 * Knight * centerBonus * +1 },
        { 7 * Bishop * centerBonus * -1, 7 * Bishop * centerBonus * +1 },
        { 7 *   Rook * centerBonus * -1, 7 *   Rook * centerBonus * +1 },
        { 7 *  Queen * centerBonus * -1, 7 *  Queen * centerBonus * +1 },
        { 7 *   King * centerBonus * -1, 7 *   King * centerBonus * +1 }
    }
};

static long constexpr material_bonus[7][2] PROGMEM = {
    //          Black        ,           White  
    { pieceValues[Empty] * +1, pieceValues[Empty] * -1 },   //  Empty
    { pieceValues[Empty] * +1, pieceValues[Empty] * -1 },   //   Pawn
    { pieceValues[Empty] * +1, pieceValues[Empty] * -1 },   // Knight
    { pieceValues[Empty] * +1, pieceValues[Empty] * -1 },   // Bishop
    { pieceValues[Empty] * +1, pieceValues[Empty] * -1 },   //   Rook
    { pieceValues[Empty] * +1, pieceValues[Empty] * -1 },   //  Queen
    { pieceValues[Empty] * +1, pieceValues[Empty] * -1 }    //   King
};

static offset_t constexpr knight_offsets[NUM_KNIGHT_OFFSETS] PROGMEM = {
    { -2, +1 }, { -2, -1 }, { +2, +1 }, { +2, -1 }, 
    { +1, +2 }, { -1, +2 }, { +1, -2 }, { -1, -2 }  
};

static offset_t constexpr rook_offsets[NUM_ROOK_OFFSETS] PROGMEM = {
    { -1,  0 }, { -2,  0 }, { -3,  0 }, { -4,  0 }, { -5,  0 }, { -6,  0 }, { -7,  0 },     // W
    { +1,  0 }, { +2,  0 }, { +3,  0 }, { +4,  0 }, { +5,  0 }, { +6,  0 }, { +7,  0 },     // E
    {  0, +1 }, {  0, +2 }, {  0, +3 }, {  0, +4 }, {  0, +5 }, {  0, +6 }, {  0, +7 },     // N
    {  0, -1 }, {  0, -2 }, {  0, -3 }, {  0, -4 }, {  0, -5 }, {  0, -6 }, {  0, -7 }      // S
};

static offset_t constexpr bishop_offsets[NUM_BISHOP_OFFSETS] PROGMEM = {
    { -1, +1 }, { -2, +2 }, { -3, +3 }, { -4, +4 }, { -5, +5 }, { -6, +6 }, { -7, +7 },     // NW
    { +1, +1 }, { +2, +2 }, { +3, +3 }, { +4, +4 }, { +5, +5 }, { +6, +6 }, { +7, +7 },     // NE
    { -1, -1 }, { -2, -2 }, { -3, -3 }, { -4, -4 }, { -5, -5 }, { -6, -6 }, { -7, -7 },     // SW
    { +1, -1 }, { +2, -2 }, { +3, -3 }, { +4, -4 }, { +5, -5 }, { +6, -6 }, { +7, -7 }      // SE
};

static offset_t constexpr queen_offsets[NUM_QUEEN_OFFSETS] PROGMEM = {
    { -1,  0 }, { -2,  0 }, { -3,  0 }, { -4,  0 }, { -5,  0 }, { -6,  0 }, { -7,  0 },     // W
    { +1,  0 }, { +2,  0 }, { +3,  0 }, { +4,  0 }, { +5,  0 }, { +6,  0 }, { +7,  0 },     // E
    {  0, +1 }, {  0, +2 }, {  0, +3 }, {  0, +4 }, {  0, +5 }, {  0, +6 }, {  0, +7 },     // N
    {  0, -1 }, {  0, -2 }, {  0, -3 }, {  0, -4 }, {  0, -5 }, {  0, -6 }, {  0, -7 },     // S
    { -1, +1 }, { -2, +2 }, { -3, +3 }, { -4, +4 }, { -5, +5 }, { -6, +6 }, { -7, +7 },     // NW
    { +1, +1 }, { +2, +2 }, { +3, +3 }, { +4, +4 }, { +5, +5 }, { +6, +6 }, { +7, +7 },     // NE
    { -1, -1 }, { -2, -2 }, { -3, -3 }, { -4, -4 }, { -5, -5 }, { -6, -6 }, { -7, -7 },     // SW
    { +1, -1 }, { +2, -2 }, { +3, -3 }, { +4, -4 }, { +5, -5 }, { +6, -6 }, { +7, -7 }      // SE
};

static offset_t constexpr king_offsets[NUM_KING_OFFSETS] PROGMEM = {
    { -1,  0 }, { -1, +1 }, { +1,  0 }, { +1, +1 }, 
    {  0, +1 }, { -1, -1 }, {  0, -1 }, { +1, -1 }
};