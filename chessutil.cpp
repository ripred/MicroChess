/**
 * chessutil.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * MicroChess utility functions
 * 
 */
#include "HardwareSerial.h"
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "MicroChess.h"
#include <stdarg.h>
#include <ctype.h>
#include <stdint.h>


extern game_t  game;


////////////////////////////////////////////////////////////////////////////////////////
// Opening book moves (if enabled)
book_t const opening1[] = {
    { 6 * 8 + 4,   5 * 8 + 4 },   //   Pawn from E2 to E3
    { 7 * 8 + 5,   4 * 8 + 2 },   // Bishop from F1 to C4
    { 7 * 8 + 3,   5 * 8 + 5 },   //  Queen from D1 to F3
    { 5 * 8 + 5,   1 * 8 + 5 }    //  Queen from F3 to E7 - checkmate

}; // opening1

Color const book_t::side = White;


piece_gen_t::piece_gen_t(move_t &m) :
    move(m),
    wbest(m),
    bbest(m)
{
    init(board, game);
}


piece_gen_t::piece_gen_t(move_t &m, move_t &wb, move_t &bb, generator_t *cb, Bool const eval) :
    move(m),
    wbest(wb),
    bbest(bb),
    callme(cb),
    evaluating(eval)
{
    init(board, game);
}


void inline piece_gen_t::init(board_t const &board, game_t const &game) {
    piece = board.get(move.from);
    type = getType(piece);
    side = getSide(piece);
    col = move.from % 8;
    row = move.from / 8;
    piece_index = game.find_piece(move.from);
    whites_turn = side;     // same as (White == side)
    cutoff = False;
    num_wmoves = 0;
    num_bmoves = 0;
}


// Utility functions

// get the Type of a Piece
Piece getType(Piece b)
{
    return Type & b;

} // getType(Piece b)


// see if a Piece is Empty
Bool isEmpty(Piece b)
{
    return getType(b) == Empty;

} // isEmpty(Piece b)


// get the value of a piece
int getValue(Piece b)
{
    return pieceValues[getType(b)]; 

} // getValue(Piece b)


// get the side for a Piece
Piece getSide(Piece b)
{ 
    return (Side & b) >> 3u; 

} // getSide(Piece b)


// see if a Piece has moved
Bool hasMoved(Piece b)
{ 
    return (Moved & b) == Moved; 

} // hasMoved(Piece b)


// see if a Piece is in check
Bool inCheck(Piece b)
{ 
    return (Check & b) == Check; 

} // inCheck(Piece b)


// set the Type of a Piece
Piece setType(Piece b, Piece type)
{ 
    return (b & ~Type) | (type & Type); 

} // setType(Piece b, Piece type)


// set the Color of a Piece
Piece setSide(Piece b, Piece side)
{
    return (b & ~Side) | ((side << 3u) & Side); 

} // setSide(Piece b, Piece side)


// set or reset the flag indicating a Piece as moved
Piece setMoved(Piece b, Bool hasMoved)
{ 
    return (b & ~Moved) | (hasMoved ? Moved : 0); 

} // setMoved(Piece b, Bool hasMoved)


// set or reset the flag indicating a Piece is in check
Piece setCheck(Piece b, Bool inCheck)
{ 
    return (b & ~Check) | (inCheck ? Check : 0); 

} // setCheck(Piece b, Bool inCheck)


// construct a Piece value
Piece makeSpot(Piece type, Piece side, unsigned char moved, unsigned char inCheck) {
    return setType(0, type) | setSide(0, side) | setMoved(0, moved) | setCheck(0, inCheck);

} // makeSpot(Piece type, Piece side, unsigned char moved, unsigned char inCheck)


char * addCommas(long int value) {
    static char buff[16];
    snprintf(buff, sizeof(buff), "%ld", value);

    int start_idx = (buff[0] == '-') ? 1 : 0;

    for (int i = strlen(buff) - 3; i > start_idx; i -= 3) {
        memmove(&buff[i + 1], &buff[i], strlen(buff) - i + 1);
        buff[i] = ',';
    }
    return buff;

} // addCommas(long int value)


// repeat printing a character a number of times
void printrep(print_t const level, char const c, index_t repeat) {
    if (game.options.print_level < level) { return; }
    while (repeat--) {
        Serial.write(c);
    }

} // printrep(...)


void printnl(print_t const level, index_t repeat /* = 1 */) {
    printrep(level, '\n', repeat);

} // printnl(...)


int debug(char const * const progmem, ...) {
    char fmt[128];
    strcpy_P(fmt, progmem);
    
    char buff[128];
    va_list argList;
    va_start(argList, fmt);
    vsnprintf(buff, ARRAYSZ(buff), fmt, argList);
    va_end(argList);

    return Serial.write(buff, strlen(buff));

} // debug(char const * const progmem, ...)


char * ftostr(double const value, int const dec, char * const buff)
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


// Check for a timeout during a turn
Bool timeout() {
    if (0 == game.options.time_limit) {
        game.timeout1 = False;
        game.timeout2 = False;
        return False;
    }

    // We have TWO timeout flags; timeout1 and timeout2
    // 2 is set as soon at the timeout happens regardless of ply level
    // 1 is set when then timeout happens only for ply levels > 1 so that
    // we always evaluate all moves for ply level 0 and 1 and only timeout
    // for ply levels >= 2.

    // Set the true timeout flag regardless of ply level
    game.timeout2 = game.stats.move_stats.duration() >= game.options.time_limit;

    // Set the other timeout flag ONLY if we are above ply level 1*
    // NOTE: in order to truly set the game.white_king_in_check or the
    // game.black_king_in_check flags correctly WE CANNOT RELY ON THE FACT
    // THAT BOTH SIDES ARE EVALUATED DURING PLY 0. This only evaluates if
    // the king is in check from the outermost level and stops the responses
    // to any moves from being evaluated and tis is necessary to stop moves
    // from being made that place a king in check. So we must allow both ply
    // level 0 and 1 to complete before we allow a timeout to stop the
    // evaluations:
    game.timeout1 = game.timeout2 && (game.ply > game.options.minply);

    if (game.timeout2) {
        show_timeout();
    }

    return game.timeout1;

} // timeout()


// check for a low memory condition
Bool check_mem(index_t const
    #ifdef ENA_MEM_STATS    // get rid of 'unused' warning when not tracking memory
    level
    #endif
    ) {
    #ifdef ENA_MEM_STATS
    if ((unsigned int)freeMemory() < game.lowest_mem) {
        game.lowest_mem = freeMemory();
        game.lowest_mem_ply = game.ply;
    }

    game.freemem[level][game.ply].mem = freeMemory();
    #endif

    Bool const low_mem = freeMemory() < game.options.low_mem_limit;
    if (low_mem) {
        show_low_memory();
    }
    return low_mem;

} // check_mem(index_t const level)


void direct_write(index_t const pin, Bool const value) {
    if (!value)
    {
        if (pin > 1 && pin < 8 ) {
            bitClear (PORTD, pin);    // == digitalWrite(pin,LOW) for pins 2-6
        }
        else {
            bitClear (PORTB, (pin-8));  // == digitalWrite(pin,LOW) for pins 8-12
        }
    }
    else
    {
        if (pin > 1 && pin < 8 ) {
            bitSet (PORTD, pin);        // == digitalWrite(pin,HIGH) for pins 2-6
        }
        else {
            bitSet (PORTB, (pin-8));   // == digitalWrite(pin,HIGH) for pins 8-12
        }
    }    
}


#if 0

void check_kings() {
    game.white_king_in_check = inCheck(board.get(game.wking));
    game.black_king_in_check = inCheck(board.get(game.bking));
}

#else

// Static function used to implement the visitor-pattern
// when checking for either king's check state
static void visitor(piece_gen_t &gen) {
    Piece const piece = board.get(gen.move.to);
    if (King == getType(piece)) {
        // This is a King; See which side it belongs to
        if (White == getSide(piece)) {
            game.white_king_in_check = True;
        }
        else {
            game.black_king_in_check = True;
        }
    }
}


// Enumerate over all available moves and set the game.white_king_in_check
// and game.black_king_in_check flags accordingly
void check_kings() {
    // Bool const wcheck = inCheck(board.get(game.wking));
    // Bool const bcheck = inCheck(board.get(game.bking));

    game.white_king_in_check = False;
    game.black_king_in_check = False;

    // Walk through the game.pieces[] list and set the king-in-check flags if necessary
    for (index_t i = 0; i < game.piece_count; i++) {
        if (-1 == game.pieces[i].x) { continue; }

        // Construct a move_t object with the starting location
        move_t move = { index_t(game.pieces[i].x + game.pieces[i].y * 8), -1, 0 };
        piece_gen_t gen(move, move, move, visitor, False);
        gen.move.value = gen.whites_turn ? MIN_VALUE : MAX_VALUE;

        if (Empty == gen.type) { continue; }

        // Evaluate the moves for this Piece Type and get the highest value move
        switch (gen.type) {
            case   Pawn:    add_pawn_moves(gen);    break;
            case Knight:    add_knight_moves(gen);  break;
            case Bishop:    add_bishop_moves(gen);  break;
            case   Rook:    add_rook_moves(gen);    break;
            case  Queen:    add_queen_moves(gen);   break;
            case   King:    add_king_moves(gen);    break;
            default: printf(Always, "bad type: line %d\n", __LINE__);   break;
        }

        // If both king's are in check then there's nothing else for us to do
        if (game.white_king_in_check && game.black_king_in_check) {
            // if (wcheck != game.white_king_in_check || bcheck != game.black_king_in_check) {
            //     printf(Debug1, "check mismatch at line %d\n", __LINE__);
            //     while (1) {}
            // }
            return;
        }

    } // for each piece on both sides

    // if (wcheck != game.white_king_in_check || bcheck != game.black_king_in_check) {
    //     printf(Debug1, "check mismatch at line %d\n", __LINE__);
    //     while (1) {}
    // }

} // check_kings()

#endif

void show_low_memory() {
    direct_write(DEBUG1_PIN, HIGH);

} // show_low_memory()


void show_quiescent_search() {
    direct_write(DEBUG2_PIN, HIGH);

} // show_quiescent_search()


void show_timeout() {
    direct_write(DEBUG3_PIN, HIGH);

} // show_timeout()


void show_check() {
    direct_write(DEBUG4_PIN, HIGH);

} // show_check()


#ifdef ENA_MEM_STATS

void show_memory_stats1() {
    // the amount of memory used as reported by the compiler
    int const prg_ram = 938;

    printf(Debug1, "== Memory Usage By Function and Ply Levels ==\n");

    for (index_t i = 0; i <= game.options.max_max_ply; i++) {
        printf(Debug1, "freemem[choose_best_move][ply %d] = %4d\n", i, game.freemem[CHOOSE][i].mem - prg_ram);
        printf(Debug1, "freemem[ piece move gen ][ply %d] = %4d\n", i, game.freemem[ADD_MOVES][i].mem - prg_ram);
        printf(Debug1, "freemem[ consider_move  ][ply %d] = %4d\n", i, game.freemem[CONSIDER][i].mem - prg_ram);
        printf(Debug1, "freemem[    make_move   ][ply %d] = %4d. Diff = %d\n", 
            i, 
            game.freemem[MAKE][i].mem   - prg_ram, 
            game.freemem[CHOOSE][i].mem - game.freemem[MAKE][i].mem);
    
        printnl(Debug1);
    }

    printnl(Debug1);

} // show_memory_stats1()


void show_memory_stats2() {
    // the amount of memory used as reported by the compiler
    int const prg_ram = 933;

    printf(Debug1, "== Memory Usage By Function and Ply Levels ==\n");

    int const choose_best_move_mem = game.freemem[CHOOSE][0].mem    - game.freemem[ADD_MOVES][0].mem;
    int const piece_move_mem       = game.freemem[ADD_MOVES][0].mem - game.freemem[CONSIDER][0].mem;
    int const consider_move_mem    = game.freemem[CONSIDER][0].mem  - game.freemem[MAKE][0].mem;
    int const make_move_mem        = game.freemem[MAKE][0].mem      - game.freemem[CHOOSE][1].mem;

    printf(Debug1, "choose_best_move(...) memory:   %3d\n", choose_best_move_mem);
    printf(Debug1, "      pieces_gen(...) memory: + %3d\n", piece_move_mem);
    printf(Debug1, "   consider_move(...) memory: + %3d\n", consider_move_mem);
    printf(Debug1, "       make_move(...) memory: + %3d\n", make_move_mem);

    int const recurs_mem = 
        choose_best_move_mem +
        piece_move_mem +
        consider_move_mem +
        make_move_mem;

    printrep(Debug1, '=', 35);
    printf(Debug1, "%d\n", recurs_mem);
    printrep(Debug1, ' ', 7);
    printf(Debug1, "Total Recusive Memory: %d\n", recurs_mem);
    printf(Debug1, "    Lowest Memory Registered: %4d at ply level %d\n", game.lowest_mem - prg_ram, game.lowest_mem_ply);
    printnl(Debug1);

} // show_memory_stats2()

#endif


void show_stats() {
    char str[16]= "";

    // print out the game move counts and time statistics
    printrep(Debug1, '=', 70);
    printnl(Debug1);
    printrep(Debug1, ' ', 11);
    printf(Debug1, "total game time: ");
    show_time(game.stats.game_stats.duration());
    printnl(Debug1);

    uint32_t const move_count = game.move_num;
    ftostr(move_count, 0, str);
    printrep(Debug1, ' ', 11);
    printf(Debug1, "number of moves: %s\n", str);

    uint32_t const game_count = game.stats.game_stats.counter();
    ftostr(game_count, 0, str);
    printf(Debug1, "total game moves evaluated: %s\n", str);

    uint32_t const moves_per_sec = game.stats.game_stats.moveps();
    ftostr(moves_per_sec, 0, str);
    printf(Debug1, "  average moves per second: %s %s\n", str, 
        game.options.profiling ? "" : "(this includes waiting on the serial output)");

    #ifdef ENA_MEM_STATS
    show_memory_stats2();
    #endif

} // show_stats()


////////////////////////////////////////////////////////////////////////////////////////
// Check for any received serial data
// 
// Note: Sanitized stack
Bool check_serial()
{
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK
    Bool moved;
    Bool digits;
    char movestr[5];
    index_t i;

    //  Check for low stack space
    if (check_mem(CHOOSE)) { return False; }

    // Now we can alter local variables! 😎 

    moved = False;

    if (Serial.available() == 5) {
        digits = True;
        for (i = 0; i < 5; i++) {
            movestr[i] = Serial.read();
            if (i < 4) {
                if ((movestr[i] >= '0') && (movestr[i] <= '7')) {
                    movestr[i] -= '0';
                }
                else {
                    digits = False;
                }
            }
        }

        if (digits) {
            game.supplied = { index_t(movestr[0] + movestr[1] * 8), index_t(movestr[2] + movestr[3] * 8), 0L };
            game.user_supplied = True;

            printf(Debug1, "User move: ");
            show_move(game.supplied);
            printnl(Debug1);

            moved = True;
        }
    }
    else while (Serial.available()) { Serial.read(); }

    return moved;

} // check_serial()


////////////////////////////////////////////////////////////////////////////////////////
// Fill in the next opening book move if available.
// 
// returns True if there is a move or False otherwise
Bool check_book()
{
    static index_t index = 0;

    if (!game.options.openbook) { return False; }

    if (game.turn != book_t::side) {
        return False;
    }

    if ((index * 2) != game.move_num) {
        return False;
    }

    if (index < index_t(ARRAYSZ(opening1))) {
        game.supplied = { index_t(opening1[index].from), index_t(opening1[index].to), 0L };
        game.book_supplied = True;
        game.supply_valid = False;
        index++;
        return True;
    }

    return False;

} // check_book()


////////////////////////////////////////////////////////////////////////////////////////
// see if a move would violate the move repetition rule
// 
// returns True if the move would violate the rule and end the game, otherwise False
Bool would_repeat(move_t const &move)
{
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK
    game_t::history_t m;
    index_t total, i;
    Bool result;

    //  Check for low stack space
    if (check_mem(ADD_MOVES)) { return False; }

    // Now we can alter local variables! 😎 

    total = MAX_REPS * 2 - 1;

    if (game.hist_count < total) {
        return False;
    }

    result = True;

    m = { move.from, move.to };

    for (i = 1; i < total; i += 2) {
        if (game.history[i].to == m.from && game.history[i].from == m.to) {
            m = game.history[i];
        }
        else {
            result = False;
            break;
        }
    }

    return result;

}   // would_repeat(move_t const &move)


////////////////////////////////////////////////////////////////////////////////////////
// Add a move to the partial history list and check for 3-move repetition
// 
// returns True if the move violates the rule and end the game, otherwise False
Bool add_to_history(move_t const &move)
{
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK
    Bool result;

    //  Check for low stack space
    if (check_mem(ADD_MOVES)) { return False; }

    // Now we can alter local variables! 😎 

    result = would_repeat(move);

    memmove(&game.history[1], &game.history[0], sizeof(game_t::history_t) * (ARRAYSZ(game.history) - 1));
    game.history[0] = { move.from, move.to };
    if (game.hist_count < index_t(ARRAYSZ(game.history))) {
        game.hist_count++;
    }

    return result;

}   // add_to_history(move_t const &move)


void say_check() {
    printf(Debug1, "check");
}


void say_mate() {
    printf(Debug1, "mate");
}


void show_side(Color const side)
{
    if (White == side) { printf(Debug1, "White"); } else { printf(Debug1, "Black"); }
}


void show_check(Color const side, Bool const mate /* = False */)
{
    if (game.options.print_level >= Debug1) {
        show_side(side);

        if (mate) {
            say_mate();
        }
        else {
            Serial.write(" is in ");
            say_check();
        }

        Serial.write("! ");
    }

} // show_check()


void show_check_status() {
    // Announce if either King is in check
    if (game.white_king_in_check) {
        show_check(White);
    }

    if (game.black_king_in_check) {
        show_check(Black);
    }

    printnl(Debug1);
    if (game.white_king_in_check || game.black_king_in_check) { 
        printnl(Debug1);
    }

} // show_check_status()


// display a Piece's color and type
void show_piece(Piece const piece)
{
    show_side(getSide(piece));

    switch (getType(piece)) {
        case  Empty: printf(Debug1, " Empty");  break;
        case   Pawn: printf(Debug1, " Pawn");  break;
        case   Rook: printf(Debug1, " Rook");  break;
        case Knight: printf(Debug1, " Knight");  break;
        case Bishop: printf(Debug1, " Bishop");  break;
        case  Queen: printf(Debug1, " Queen");  break;
        case   King: printf(Debug1, " King");  break;
        default:     
            printf(Debug1, "bad type %d\n", getType(piece)); 
            break;
    }

} // show_piece(Piece const piece)


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
            Piece  const p = board.get(col + row * 8u);
            printf(Debug1, "    game.pieces[%2d] = %2d, %2d (%2d): ", i, col, row, col + row * 8u);
            show_piece(p);
        }

        printnl(Debug1);
    }
    printf(Debug1, "};\n");

} // show_pieces()


// display a piece being moved
void show_move(move_t const &move, Bool const align /* = False */)
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
    if (align) {
        printf(Debug1, " value: %14s", str_value);
    }
    else {
        printf(Debug1, " value: %s", str_value);
    }

} // show_move(move_t const &move, Bool const align)


void show_time(uint32_t ms)
{
    uint32_t minutes = 0;
    uint32_t seconds = 0;
    while (ms >= 1000) {
        ms -= 1000;
        if (++seconds >= 60) {
            seconds = 0;
            minutes++;
        }
    }
    if (0 != minutes) {
        char str[16] {};
        ftostr(minutes, 0, str);
        printf(Debug1, "%s minute%s%s", str, 1 == minutes ? "" : "s", (0 == seconds && 0 == ms) ? "" : ", ");
    }
    if (0 != seconds) {
        printf(Debug1, "%d second%s%s", seconds, (1 == seconds) ? "" : "s", (0 == ms) ? "" : ", ");
    }
    if (0 != ms) {
        if (0 != seconds) {
            printf(Debug1, ", ");
        }
        printf(Debug1, "%ld ms", ms);
    }
}


////////////////////////////////////////////////////////////////////////////////////////
// runtime memory usage functions
#include <unistd.h>


int freeMemory() {
    #ifdef __arm__
    // should use uinstd.h to define sbrk but Due causes a conflict
    extern "C" char* sbrk(int incr);
    #else  // __ARM__
    extern char *__brkval;
    #endif  // __arm__

    char top;
    #ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
    #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
    #else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
    #endif  // __arm__
}