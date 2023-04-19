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
    Bool        alpha_beta_pruning; // use alpha-beta pruning when True
    uint8_t     max_max_ply;        // the ultimate maximum ply level
    uint8_t     max_quiescent_ply;  // the maximum ply level to continue if a piece was taken on a move
    uint8_t     maxply;             // the nominal max ply level


// this stuff stays the same at runtime during the game, and can't be modified
    static uint32_t const move_limit = 200;                             // the maximum number of moves allowed in a full game
    static int      const low_mem_limit = 1024;                           // amount of memory we need to exchang sides
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
        print_level(Debug1),
        alpha_beta_pruning(False),
        max_max_ply(4),
        max_quiescent_ply(4),
        maxply(2)
    {}

};  // options_t

#endif  // OPTIONS_INCL