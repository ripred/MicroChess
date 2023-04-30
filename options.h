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
// The settings for a game
struct options_t {
public:

    // This stuff changes at runtime during the game and can be modified
    uint8_t          random : 1,    // Add randomness to the game?
                live_update : 1,    // Periodically update the LED strip and other external indicators as we choose a move
                  profiling : 1,    // Are we profiling the engine?
                 continuous : 1,    // True if we play games continuously one after another
             shuffle_pieces : 1,    // True if we want to process the pieces in a random order
                white_human : 1,    // Flags indicating if white player is human or not
                black_human : 1,    // Flags indicating if black player is human or not
         alpha_beta_pruning : 1,    // Use alpha-beta pruning when True
                max_max_ply : 3,    // The ultimate maximum ply level
          max_quiescent_ply : 3,    // The maximum ply level to continue if a piece was taken on a move
                     maxply : 3;    // The nominal max ply level

    uint32_t    seed;               // The starting seed hash for prn's
    print_t     print_level;        // The verbosity setting for the level of output
    uint32_t    time_limit;         // Optional time limit in ms if != 0
    index_t     mistakes;           // The percentage of times the engine will make a mistake

    // This stuff stays the same at runtime during the game, and can't be modified during the game

    static uint32_t const move_limit    = 100;  // The maximum number of moves allowed in a full game
    static int      const low_mem_limit = 873;  // The amount of memory used as reported by the compiler

    // adjustable multipiers to alter importance of mobility or center proximity
    // during board evaluation. season to taste
    static long  constexpr  materialBonus =  1L;
    static long  constexpr  centerBonus   =  1L;
    static long  constexpr  kingBonus     =  1L;
    static long  constexpr  mobilityBonus =  3L;

public:

    options_t() : 
        random(False), 
        live_update(False),
        profiling(False), 
        continuous(False),
        shuffle_pieces(True),
        white_human(False),
        black_human(False),
        alpha_beta_pruning(True),
        max_max_ply(4),
        max_quiescent_ply(4),
        maxply(2),
        seed(PRN_SEED),
        print_level(Debug1),
        time_limit(0)
    {}

};  // options_t

#endif  // OPTIONS_INCL