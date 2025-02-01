/**
 * options.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * MicroChess options_t implementation
 * 
 */
#include <Arduino.h>
#include "MicroChess.h"
#include "options.h"

options_t::options_t() : 
    max_max_ply(4),
    max_quiescent_ply(4),
    maxply(2),
    minply(1),
    random(False), 
    live_update(False),

    profiling(False), 
    continuous(False),
    integrate(True),
    openbook(False),
    shuffle_pieces(True),

    white_human(False),
    black_human(False),
    alpha_beta_pruning(True),
    seed(PRN_SEED),
    print_level(Debug1),
    time_limit(0),
    mistakes(0),
    randskip(0)
{

}
