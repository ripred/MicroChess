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
// this stuff changes at runtime during the game, or can be modified

    uint8_t     random : 1,         // add randomness to the game?
           live_update : 1,         // periodically update the LED strip and other external indicators as we choose a move
             profiling : 1,         // are we profiling the engine?
            continuous : 1;         // True if we play games continuously one after another

    uint32_t    seed;               // the starting seed hash for prn's
    print_t     print_level;        // the verbosity setting for the level of output

// this stuff stays the same at runtime during the game, and can't be modified
    static uint8_t const  maxply = 3;                                   // the nominal max ply level
    static uint8_t const  max_quiescent_ply = options_t::maxply + 2;    // the maximum ply level to continue if a piece was taken on a move
    static uint8_t const  max_max_ply = 4;                              // the ultimate maximum ply level
    static uint32_t const move_limit = 200;                             // the maximum number of moves allowed in a full game
    static int const      low_mem_limit = 64;                           // amount of memory we need to exchang sides
    static uint32_t const time_limit = 30000;                           // optional time limit in ms if != 0

    // adjustable multipiers to alter importance of mobility or center proximity
    // during board evaluation. season to taste
    static long  const    mobilityBonus = 3L;
    static long  const    centerBonus = 2L;
    static long  const    kingBonus = 10L;

public:

    options_t() : 
        random(False), 
        live_update(False),
        profiling(False), 
        continuous(False),
        seed(PRN_SEED),
        print_level(Debug1)
    {}

};  // options_t

#endif  // OPTIONS_INCL