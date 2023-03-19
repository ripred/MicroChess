#include <avr/pgmspace.h>
/**
 * chessutil.cpp
 * 
 * MicroChess utility functions
 * 
 */
#include "MicroChess.h"

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


char *getCoords(index_t index) 
{
    static const char *const coords[BOARD_SIZE] PROGMEM = {
        "0,0", "1,0", "2,0", "3,0", "4,0", "5,0", "6,0", "7,0",
        "0,1", "1,1", "2,1", "3,1", "4,1", "5,1", "6,1", "7,1",
        "0,2", "1,2", "2,2", "3,2", "4,2", "5,2", "6,2", "7,2",
        "0,3", "1,3", "2,3", "3,3", "4,3", "5,3", "6,3", "7,3",
        "0,4", "1,4", "2,4", "3,4", "4,4", "5,4", "6,4", "7,4",
        "0,5", "1,5", "2,5", "3,5", "4,5", "5,5", "6,5", "7,5",
        "0,6", "1,6", "2,6", "3,6", "4,6", "5,6", "6,6", "7,6",
        "0,7", "1,7", "2,7", "3,7", "4,7", "5,7", "6,7", "7,7"
    };

    return (char *) pgm_read_word(&(coords[index]));
}


// [[maybe_unused]] 
char *getCoords(index_t file, index_t rank) 
{ 
    return getCoords(file + rank * 8); 
}

char const *getNotate(index_t const index) {
    static char const PROGMEM notations[BOARD_SIZE][3] = {
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
    };
    return &notations[index][0];
}


// [[maybe_unused]] 
char const *getNotate(index_t file, index_t rank) 
{ 
    return getNotate(file + rank * 8);
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

int printf(print_t const required, print_t const level, char const * const progmem, ...) {
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