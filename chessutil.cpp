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

print_t level = Debug1;

// Bool isValidTest()
// {
// #ifdef TRACE_FAIL
//     auto trace = []() -> int {
//         static char const fmt[] PROGMEM = "\nerror: failed isValidPos(x,y) test 1!\n\n";
//         printf(Always, fmt);
//         return 0;
//     };
// #else
//     auto trace = []() -> int { return 0; };
// #endif
//     for (index_t y=0; y < 8; y++) {
//         for (index_t x=0; x < 8; x++) {
//             if (!isValidPos(x, y)) {
//                 return trace();
//             }
//         }
//     }

//     if (isValidPos(-1, 0) || isValidPos(0, -1) || isValidPos(8, 0) || isValidPos(0, 8)) {
//         return trace();
//     }

//     static char const fmt[] PROGMEM = "passed isValidPos(x,y) tests\n\n";
//     printf(Debug2, fmt);

//     return 1;
// }


Piece getType(Piece b) 
{
    return Type & b;
}

Bool isEmpty(Piece b) 
{
    return getType(b) == Empty;
}


int getValue(Piece b) 
{
    return pieceValues[getType(b)]; 
}


Piece getSide(Piece b) 
{ 
    return (Side & b) >> 3u; 
}


Bool hasMoved(Piece b) 
{ 
    return (Moved & b) == Moved; 
}


Bool inCheck(Piece b) 
{ 
    return (Check & b) == Check; 
}


Piece setType(Piece b, Piece type) 
{ 
    return (b & ~Type) | (type & Type); 
}


Piece setSide(Piece b, Piece side) {
    return (b & ~Side) | ((side << 3u) & Side); 
}


Piece setMoved(Piece b, Bool hasMoved) 
{ 
    return (b & ~Moved) | (hasMoved ? Moved : 0); 
}


Piece setCheck(Piece b, Bool inCheck) 
{ 
    return (b & ~Check) | (inCheck ? Check : 0); 
}


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
    static char buff[16];
    snprintf(buff, 16, "%ld", value);
    for (int i = strlen(buff) - 3; i > 0; i -= 3) {
        memmove(&buff[i + 1], &buff[i], strlen(buff) - i);
        buff[i] = ',';
    }
    return buff;
}

int printf(print_t const required, char const * const progmem, ...) {
    char fmt[100] = "";
    for (int i = 0; fmt[i] = pgm_read_byte_near(progmem + i), fmt[i] != 0; i++) {}
    if (level >= required) {
        char buff[100] = "";
        va_list argList;
        va_start(argList, fmt);
        vsnprintf(buff, ARRAYSZ(buff), fmt, argList);
        va_end(argList);
        return Serial.write(buff, strlen(buff));
    }

    return 0;
}

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

int freeMem() {
    extern int __heap_start/*, *__brkval */;
    int v;

    return (int)&v - (__brkval == 0  ? (int)&__heap_start : (int) __brkval);
}

void printMemoryStats() {
    // ============================================================
    // startup memory
    int totalRam = 2048;
    // int freeRam = freeMem();
    int freeRam = freeMemory();
    int usedRam = totalRam - freeRam;

    static char const fmt1[] PROGMEM = "Total SRAM = %d\n";
    printf(Debug2, fmt1, totalRam);
    static char const fmt2[] PROGMEM = "Free SRAM = %d\n";
    printf(Debug2, fmt2, freeRam);
    static char const fmt3[] PROGMEM = "Used SRAM = %d\n";
    printf(Debug2, fmt3, usedRam);

    static char const fmt4[] PROGMEM = "sizeof(move_t) = %d\n";
    printf(Debug2, fmt4, sizeof(move_t));
    static char const fmt5[] PROGMEM = "meaning there is room for %d more move_t entries.\n";
    printf(Debug2, fmt5, freeRam / sizeof(move_t) );
    static char const fmt6[] PROGMEM = "or %d more move_t entries per move list.\n";
    printf(Debug2, fmt6, ((freeRam / sizeof(move_t)) / 2) );
}


void show_piece(Piece const p) 
{
    Piece const type = getType(p);
    Color const side = getSide(p);

    static char const fmt[] PROGMEM = "%5s %s";
    printf(Debug1, fmt, 
        (Empty == type ?  "" : (White == side ?  "White" : "Black")), 
        (Empty == type ?  "Empty" :
          Pawn == type ?   "Pawn" :
        Knight == type ? "Knight" :
        Bishop == type ? "Bishop" :
          Rook == type ?   "Rook" :
         Queen == type ?  "Queen" : "King"));
}

void show_pieces() 
{
    static char const fmt1[] PROGMEM = "game.pieces[%2d] = {\n";
    printf(Debug1, fmt1, game.piece_count);
    for (int i = 0; i < game.piece_count; i++) {
        point_t const &loc = game.pieces[i];
        index_t const col = loc.x;
        index_t const row = loc.y;
        Piece  const p = board.get(col + row * 8);
        static char const fmt1[] PROGMEM = "    game.pieces[%2d] = %d, %d: ";
        printf(Debug1, fmt1, i, col, row);
        show_piece(p);
        static char const fmt2[] PROGMEM = "\n";
        printf(Debug1, fmt2);
    }
    static char const fmt2[] PROGMEM = "};\n";
    printf(Debug1, fmt2);
}


void show_move(move_t const &move) {
    index_t const    col = move.from % 8;
    index_t const    row = move.from / 8;
    index_t const   from = col + row * 8;
    Piece   const      p = board.get(from);
    index_t const to_col = move.to % 8;
    index_t const to_row = move.to / 8;

    show_piece(p);

    static char const fmt[] PROGMEM = " from: %d,%d (%c%c) to: %d,%d (%c%c)";
    printf(Debug1, fmt, 
        col,    row,       col + 'A', '8' -    row, 
        to_col, to_row, to_col + 'A', '8' - to_row);
}
