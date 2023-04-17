/**
 * options.h
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * The options_t structure holds flags that are used during the game
 * to enable or disable various features of the chess engine.
 * 
 */
#ifndef OPTIONS_INCL
#define OPTIONS_INCL

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////////////
// the settings for a game
struct options_t {
public:
    uint8_t     random : 1,     // add randomness to the game?
           live_update : 1,     // periodically update the LED strip and other external indicators as we choose a move
             profiling : 1,     // are we profiling the engine?
            continuous : 1;     // True if we play games continuously one after another

    uint8_t     maxply;         // the maximum ply level
    uint32_t    move_limit;     // the maximum number of moves allowed in a full game
    uint32_t    seed;           // the starting seed hash for prn's
    print_t     print_level;    // the verbosity setting for the level of output
    int         low_mem_limit;  // amount of memory we need to exchang sides
    uint32_t    time_limit;     // optional time limit in ms if != 0

    // adjustable multipiers to alter importance of mobility or center proximity
    // during board evaluation. season to taste
    static long constexpr mobilityBonus =  3L;
    static long constexpr   centerBonus =  2L;
    static long constexpr    kingBonus  = 10L;

public:

    options_t() : 
        random(False), 
        live_update(False),
        profiling(False), 
        continuous(False),
        maxply(MAX_PLY),
        move_limit(MOVE_LIMIT),
        seed(PRN_SEED),
        print_level(Debug1),
        low_mem_limit(100),
        time_limit(0)
    {}

};  // options_t

#endif  // OPTIONS_INCL