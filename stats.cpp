/**
 * stats.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * MicroChess statistics functions
 * 
 */

#include <Arduino.h>
#include "MicroChess.h"
#include "stats.h"

// constructor:
stat_t::stat_t() {
    init();
}

// init method
void stat_t::init() {
    max_moves = 0;

    moves_gen_game = 0;

    moves_gen_move_start = 0;
    moves_gen_move_end = 0;
    moves_gen_move_delta = 0;

    game_start = 0;
    game_end = 0;
    game_time = 0;

    move_start = 0;
    move_end = 0;
    move_time = 0;
}

// increase the number of moves evaluated
void stat_t::inc_moves_count() {
    moves_gen_game++;
}

// start the game timers and clear out the game counts
void stat_t::start_game_stats() {
    init();
    game_start = millis();
    game_end = game_start;
    game_time = 0;
    moves_gen_game = 0;
}

// stop the game timers and calc the game stats
void stat_t::stop_game_stats() {
    game_end = millis();
    game_time = game_end - game_start;
}

// start the move timers and clear out the move counts
void stat_t::start_move_stats() {
    move_start = millis();
    move_end = move_start;
    move_time = 0;

    moves_gen_move_start = moves_gen_game;
    moves_gen_move_end = moves_gen_move_start;
    moves_gen_move_delta = 0;
}

uint32_t stat_t::move_count_so_far() const {
    return moves_gen_game - moves_gen_move_start;
}

// stop the move timers and calc the move stats
void stat_t::stop_move_stats() {
    move_end = millis();
    move_time = move_end - move_start;

    moves_gen_move_end = moves_gen_game;
    moves_gen_move_delta = moves_gen_move_end - moves_gen_move_start;
}