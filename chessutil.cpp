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


// const char* addCommas(long int value) {
//     static char buff[32];
//     snprintf(buff, 32, "%ld", value);
//     for (int i = strlen(buff) - 3; i > 0; i -= 3) {
//         memmove(&buff[i + 1], &buff[i], strlen(buff) - i);
//         buff[i] = ',';
//     }
//     return buff;
// }

int printf(print_t const required, char const * const progmem, ...) {
    if (print_level >= required) {
        char fmt[100] = "";
        for (int i = 0; fmt[i] = pgm_read_byte_near(progmem + i), fmt[i] != 0; i++) {}
        
        char buff[100] = "";
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


// display a Piece's color and type
void show_piece(Piece const p) 
{
    Piece const type = getType(p);
    Color const side = getSide(p);

    static char const fmt[] PROGMEM = "%s %s";
    printf(Debug1, fmt, 
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


// display a piece being moved
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
           col,    row,    col + 'A', '8' -    row, 
        to_col, to_row, to_col + 'A', '8' - to_row);
}