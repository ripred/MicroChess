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

    uint8_t          
                max_max_ply : 3,    // The ultimate maximum ply level
                     random : 1,    // Add randomness to the game when True
          max_quiescent_ply : 3,    // The maximum ply level to continue if a piece was taken on a move
                live_update : 1,    // Periodically update the LED strip and other external indicators as we choose a move

                     maxply : 3,    // The nominal max ply level
                  profiling : 1,    // We profiling the engine when True
                 continuous : 1,    // True if we play games continuously one after another
                  integrate : 1,    // Integrate recursive return values when True
                   openbook : 1,    // Use opening book when True
             shuffle_pieces : 1,    // True if we want to process the pieces in a random order

                     minply : 3,    // The minumum ply level to complete for a turn
                white_human : 1,    // Flags indicating if white player is human or not
                black_human : 1,    // Flags indicating if black player is human or not
         alpha_beta_pruning : 1;    // Use alpha-beta pruning when True

    uint32_t    seed;               // The starting seed hash for prn's
    print_t     print_level;        // The verbosity setting for the level of output
    uint32_t    time_limit;         // Optional time limit in ms if != 0
    index_t     mistakes;           // The percentage of times the engine will make a mistake
    index_t     randskip;           // Randomly skip ply depths percentage


    // This stuff stays the same at runtime during the game, and can't be modified during the game

    static uint32_t constexpr move_limit    = 100;  // The maximum number of moves allowed in a full game
    static int      constexpr low_mem_limit = 810;  // The amount of memory used as reported by the compiler

    // Adjustable multipiers to alter the importance of mobility, center proximity,
    // material, and king bonus metrics during board evaluation. Season to taste.
    static long  constexpr  materialBonus =  1L;
    static long  constexpr  centerBonus   =  1L;
    static long  constexpr  kingBonus     =  1L;
    static long  constexpr  mobilityBonus =  1L;

public:

    options_t();

};  // options_t

#endif  // OPTIONS_INCL