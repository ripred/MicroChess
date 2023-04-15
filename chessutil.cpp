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
#include <ctype.h>

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

    int start_idx = (buff[0] == '-') ? 1 : 0;

    for (int i = strlen(buff) - 3; i > start_idx; i -= 3) {
        memmove(&buff[i + 1], &buff[i], strlen(buff) - i + 1);
        buff[i] = ',';
    }
    return buff;
}


int debug(char const * const progmem, ...) {
    char fmt[128];
    strcpy_P(fmt, progmem);
    
    char buff[128];
    va_list argList;
    va_start(argList, fmt);
    vsnprintf(buff, ARRAYSZ(buff), fmt, argList);
    va_end(argList);

    return Serial.write(buff, strlen(buff));
}

const char* ftostr(double value, int dec /* = 2 */, char * const buff /* = nullptr */)
{
    static char str[16];
    dtostrf(value, sizeof(str), dec, str);
    char *p = str;
    while (isspace(*p)) p++;
    // char const * const begin = p;
    while (isdigit(*p) || ('-' == *p)) p++;

    char tmp[24];
    strcpy(tmp, addCommas(long(value)));
    strcat(tmp, p);

    if (buff) strcpy(buff, tmp);
    return p;

} // ftostr(double value, int dec = 2)


void show_stats() {
    char str[16]= "";

    // print out the game move counts and time statistics
    printf(Debug1, "======================================================================\n");

    double ftime = game.stats.game_stats.duration() / 1000.0;
    ftostr(ftime, 4, str);
    printf(Debug1, "           total game time: %s seconds\n", str);

    uint32_t move_count = game.stats.move_stats.counter();
    ftostr(move_count, 0, str);
    printf(Debug1, "           number of moves: %s\n", str);

    uint32_t game_count = game.stats.game_stats.counter();
    ftostr(game_count, 0, str);
    printf(Debug1, "total game moves evaluated: %s\n", str);

    uint32_t moves_per_sec = game.stats.game_stats.moveps();
    ftostr(moves_per_sec, 0, str);
    printf(Debug1, "  average moves per second: %s %s\n", str, 
        game.options.profiling ? "" : "(this includes waiting on the serial output)");

    ftostr(game.stats.max_moves, 0, str);
    printf(Debug1, "   max move count per turn: %s\n", str);
    printf(Debug1, "\n");

} // show_stats()


////////////////////////////////////////////////////////////////////////////////////////
// see if a move would violate the 3-move repetition rule
// 
// returns True if the move would violate the rule and end the game, otherwise False
Bool would_repeat(move_t move) 
{
    index_t const total = MAX_REPS * 2 - 1;

    if (game.hist_count < total) {
        return False;
    }

    Bool result = True;

    move_t &m= move;

    for (index_t i = 1; i < total; i += 2) {
        if (game.history[i].to == m.from && game.history[i].from == m.to) {
            m = game.history[i];
        }
        else {
            result = False;
            break;
        }
    }

    return result;

}   // would_repeat(move_t const move)


////////////////////////////////////////////////////////////////////////////////////////
// Add a move to the partial history list and check for 3-move repetition
// 
// returns True if the move violates the rule and end the game, otherwise False
Bool add_to_history(move_t const &move)
{
    Bool result = would_repeat(move);

    memmove(&game.history[1], &game.history[0], sizeof(move_t) * (ARRAYSZ(game.history) - 1));
    game.history[0] = move;
    if (game.hist_count < index_t(ARRAYSZ(game.history))) {
        game.hist_count++;
    }

    return result;

}   // add_to_history()


// display a Piece's color and type
void show_piece(Piece const p) 
{
    Piece const type = getType(p);
    Color const side = getSide(p);

    if (White == side) {
        printf(Debug1, "White");
    }
    else {
        printf(Debug1, "Black");
    }

    switch (type) {
        case  Empty: printf(Debug1, " Empty");  break;
        case   Pawn: printf(Debug1, " Pawn");   break;
        case   Rook: printf(Debug1, " Rook");   break;
        case Knight: printf(Debug1, " Knight"); break;
        case Bishop: printf(Debug1, " Bishop"); break;
        case  Queen: printf(Debug1, " Queen");  break;
        case   King: printf(Debug1, " King");   break;
            default: printf(Debug1, "Error: invalid Piece type: %d\n", type); break;
    }
} // show_piece(Piece const p) 


// debug function to display all of the point_t's in the game.pieces[game.piece_count] list:
void show_pieces()
{
    printf(Debug1, "game.pieces[%2d] = {\n", game.piece_count);
    for (int i = 0; i < game.piece_count; i++) {
        point_t const &loc = game.pieces[i];
        index_t const col = loc.x;
        index_t const row = loc.y;

        if (-1 == col && -1 == row) {
            printf(Debug1, "    game.pieces[%2d] = Empty", i);
        }
        else {
            Piece  const p = board.get(col + row * 8);
            printf(Debug1, "    game.pieces[%2d] = %2d, %2d (%2d): ", i, col, row, col + row * 8);
            show_piece(p);
        }

        printf(Debug1, "\n");
    }
    printf(Debug1, "};\n");

} // show_pieces()


// display a piece being moved
void show_move(move_t const &move)
{
    index_t const    col = move.from % 8;
    index_t const    row = move.from / 8;
    Piece   const      p = board.get(move.from);
    index_t const to_col = move.to % 8;
    index_t const to_row = move.to / 8;
    Piece   const     op = board.get(move.to);

    show_piece(p);

    printf(Debug1, " from: %d,%d (%c%c) to: %d,%d (%c%c)", 
           col,    row,    col + 'A', '8' -    row, 
        to_col, to_row, to_col + 'A', '8' - to_row);

    if (Empty != getType(op)) {
        printf(Debug1, " taking a ");
        show_piece(op);
    }

    char str_value[16] = "";
    strcpy(str_value, addCommas(move.value));
    printf(Debug1, " value: %s%s", (move.value < 0) ? "" : " ", str_value);

} // show_move(move_t const &move)


// functions to convert the board contents to and from a 64-byte ascii string
void to_string(char * const out)
{
    char const icons[] = "pnbrqkPNBRQK";

    for (index_t i = 0; i < index_t(BOARD_SIZE); i++) {
        Piece const piece = board.get(i);
        index_t const x = i % 8;
        index_t const y = i / 8;
        char const c = isEmpty(piece) ? 
            ((y ^ x) & 1 ? '.' : '*') :
            icons[((getSide(piece) * 6) + getType(piece) - 1)];
        out[i] = c;
    }

} // to_string(char * const out)


void from_string(char const * const in)
{
    char const icons[] = "pnbrqkPNBRQK";
    game.piece_count = 0;

    for (index_t i = 0; i < index_t(BOARD_SIZE); i++) {
        char const c = in[i];
        char const * const ptr = strchr(icons, c);
        if (nullptr != ptr) {
            index_t const row = i / 8;
            index_t const ndx = ptr - icons;
            Color const side = ndx < 6 ? Black : White;
            Piece const type = ndx % 6;
            Bool const moved = (Pawn == type && 
                ((White == side && 1 == row) || 
                 (Black == side && 6 == row))) ? True : False;
            board.set(i, makeSpot(type, side, moved, False));
            game.pieces[game.piece_count++] = { index_t(i % 8), index_t(i / 8) };
        }
    }

} // from_string(char const * const in)


////////////////////////////////////////////////////////////////////////////////////////
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
    int freeRam = freeMemory();
    int usedRam = totalRam - freeRam;

    printf(Debug2, "Total SRAM = %d\n", totalRam);
    printf(Debug2, " Free SRAM = %d\n",  freeRam);
    printf(Debug2, " Used SRAM = %d\n",  usedRam);
}


////////////////////////////////////////////////////////////////////////////////////////
// use pre-computed bonus tables for speed!
//                                [col/row][type][side] 
static long constexpr center_bonus[   8   ][  7 ][  2 ] PROGMEM = {
    //                      Black                   ,                      White 
    {
        { 0 *  Empty * game.options.centerBonus * -1,    0 *  Empty * game.options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 0 *   Pawn * game.options.centerBonus * -1,    0 *   Pawn * game.options.centerBonus * +1 },
        { 0 * Knight * game.options.centerBonus * -1,    0 * Knight * game.options.centerBonus * +1 },
        { 0 * Bishop * game.options.centerBonus * -1,    0 * Bishop * game.options.centerBonus * +1 },
        { 0 *   Rook * game.options.centerBonus * -1,    0 *   Rook * game.options.centerBonus * +1 },
        { 0 *  Queen * game.options.centerBonus * -1,    0 *  Queen * game.options.centerBonus * +1 },
        { 0 *   King * game.options.centerBonus * -1,    0 *   King * game.options.centerBonus * +1 },
    }, {
        { 1 *  Empty * game.options.centerBonus * -1,    1 *  Empty * game.options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 1 *   Pawn * game.options.centerBonus * -1,    1 *   Pawn * game.options.centerBonus * +1 },
        { 1 * Knight * game.options.centerBonus * -1,    1 * Knight * game.options.centerBonus * +1 },
        { 1 * Bishop * game.options.centerBonus * -1,    1 * Bishop * game.options.centerBonus * +1 },
        { 1 *   Rook * game.options.centerBonus * -1,    1 *   Rook * game.options.centerBonus * +1 },
        { 1 *  Queen * game.options.centerBonus * -1,    1 *  Queen * game.options.centerBonus * +1 },
        { 1 *   King * game.options.centerBonus * -1,    1 *   King * game.options.centerBonus * +1 },
    }, {
        { 2 *  Empty * game.options.centerBonus * -1,    2 *  Empty * game.options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 2 *   Pawn * game.options.centerBonus * -1,    2 *   Pawn * game.options.centerBonus * +1 },
        { 2 * Knight * game.options.centerBonus * -1,    2 * Knight * game.options.centerBonus * +1 },
        { 2 * Bishop * game.options.centerBonus * -1,    2 * Bishop * game.options.centerBonus * +1 },
        { 2 *   Rook * game.options.centerBonus * -1,    2 *   Rook * game.options.centerBonus * +1 },
        { 2 *  Queen * game.options.centerBonus * -1,    2 *  Queen * game.options.centerBonus * +1 },
        { 2 *   King * game.options.centerBonus * -1,    2 *   King * game.options.centerBonus * +1 },
    }, {
        { 3 *  Empty * game.options.centerBonus * -1,    3 *  Empty * game.options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 3 *   Pawn * game.options.centerBonus * -1,    3 *   Pawn * game.options.centerBonus * +1 },
        { 3 * Knight * game.options.centerBonus * -1,    3 * Knight * game.options.centerBonus * +1 },
        { 3 * Bishop * game.options.centerBonus * -1,    3 * Bishop * game.options.centerBonus * +1 },
        { 3 *   Rook * game.options.centerBonus * -1,    3 *   Rook * game.options.centerBonus * +1 },
        { 3 *  Queen * game.options.centerBonus * -1,    3 *  Queen * game.options.centerBonus * +1 },
        { 3 *   King * game.options.centerBonus * -1,    3 *   King * game.options.centerBonus * +1 },
    }, {
        { 3 *  Empty * game.options.centerBonus * -1,    3 *  Empty * game.options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 3 *   Pawn * game.options.centerBonus * -1,    3 *   Pawn * game.options.centerBonus * +1 },
        { 3 * Knight * game.options.centerBonus * -1,    3 * Knight * game.options.centerBonus * +1 },
        { 3 * Bishop * game.options.centerBonus * -1,    3 * Bishop * game.options.centerBonus * +1 },
        { 3 *   Rook * game.options.centerBonus * -1,    3 *   Rook * game.options.centerBonus * +1 },
        { 3 *  Queen * game.options.centerBonus * -1,    3 *  Queen * game.options.centerBonus * +1 },
        { 3 *   King * game.options.centerBonus * -1,    3 *   King * game.options.centerBonus * +1 },
    }, {
        { 2 *  Empty * game.options.centerBonus * -1,    2 *  Empty * game.options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 2 *   Pawn * game.options.centerBonus * -1,    2 *   Pawn * game.options.centerBonus * +1 },
        { 2 * Knight * game.options.centerBonus * -1,    2 * Knight * game.options.centerBonus * +1 },
        { 2 * Bishop * game.options.centerBonus * -1,    2 * Bishop * game.options.centerBonus * +1 },
        { 2 *   Rook * game.options.centerBonus * -1,    2 *   Rook * game.options.centerBonus * +1 },
        { 2 *  Queen * game.options.centerBonus * -1,    2 *  Queen * game.options.centerBonus * +1 },
        { 2 *   King * game.options.centerBonus * -1,    2 *   King * game.options.centerBonus * +1 },
    }, {
        { 1 *  Empty * game.options.centerBonus * -1,    1 *  Empty * game.options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 1 *   Pawn * game.options.centerBonus * -1,    1 *   Pawn * game.options.centerBonus * +1 },
        { 1 * Knight * game.options.centerBonus * -1,    1 * Knight * game.options.centerBonus * +1 },
        { 1 * Bishop * game.options.centerBonus * -1,    1 * Bishop * game.options.centerBonus * +1 },
        { 1 *   Rook * game.options.centerBonus * -1,    1 *   Rook * game.options.centerBonus * +1 },
        { 1 *  Queen * game.options.centerBonus * -1,    1 *  Queen * game.options.centerBonus * +1 },
        { 1 *   King * game.options.centerBonus * -1,    1 *   King * game.options.centerBonus * +1 },
    }, {
        { 0 *  Empty * game.options.centerBonus * -1,    0 *  Empty * game.options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 0 *   Pawn * game.options.centerBonus * -1,    0 *   Pawn * game.options.centerBonus * +1 },
        { 0 * Knight * game.options.centerBonus * -1,    0 * Knight * game.options.centerBonus * +1 },
        { 0 * Bishop * game.options.centerBonus * -1,    0 * Bishop * game.options.centerBonus * +1 },
        { 0 *   Rook * game.options.centerBonus * -1,    0 *   Rook * game.options.centerBonus * +1 },
        { 0 *  Queen * game.options.centerBonus * -1,    0 *  Queen * game.options.centerBonus * +1 },
        { 0 *   King * game.options.centerBonus * -1,    0 *   King * game.options.centerBonus * +1 },
    }
};

static long constexpr material_bonus[7][2] PROGMEM = {
    //          Black        ,           White  
    { pieceValues[ Empty] * -1,     pieceValues[ Empty] * +1 },   //  Empty
    { pieceValues[  Pawn] * -1,     pieceValues[  Pawn] * +1 },   //   Pawn
    { pieceValues[Knight] * -1,     pieceValues[Knight] * +1 },   // Knight
    { pieceValues[Bishop] * -1,     pieceValues[Bishop] * +1 },   // Bishop
    { pieceValues[  Rook] * -1,     pieceValues[  Rook] * +1 },   //   Rook
    { pieceValues[ Queen] * -1,     pieceValues[ Queen] * +1 },   //  Queen
    { pieceValues[  King] * -1,     pieceValues[  King] * +1 }    //   King
};


////////////////////////////////////////////////////////////////////////////////////////
// transposition tables for move generation
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
    { -1,  0 }, {  0, -1 }, { -1, -1 }, { +1, -1 }, 
    { +1,  0 }, {  0, +1 }, { -1, +1 }, { +1, +1 }
};