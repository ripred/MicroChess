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

/*
 ******************************************************************************************
 * movetime_t objects
 * 
 */

// Constructor
movetime_t::movetime_t()
{
    init();
}


// init method
void movetime_t::init()
{
    start = 0;
    stop = 0;
    dur = 0;
    count = 0;
    running = False;
    moves_per_sec = 0.0;
}



// Begin the timer
void movetime_t::begin() 
{
    start = millis();
    running = True;
    stop = start;
    dur = 0;
    count = 0;
    moves_per_sec = 0.0;
}


// End the timer
void movetime_t::end() 
{
    if (!running) {
        stop = millis();
        dur = stop - start;
        if (dur != 0 && count != 0) {
            moves_per_sec = double(count) / (double(dur) / 1000.0);
        }
        running = False;
    }
}


// Get the time so far, or the total time, depending on
// whether the timer is running or not.
uint32_t movetime_t::duration() const 
{
    if (running) {
        return millis() - start;
    }
    return dur;
}


// Increment the counter
uint32_t movetime_t::increment() 
{
    if (running) count++;
    return count;
}


// Get the counter
uint32_t movetime_t::counter() const 
{
    return count;
}


// Get the moves per second
double movetime_t::moveps() const 
{
    return moves_per_sec;
}


/*
 ******************************************************************************************
 * stat_t objects
 * 
 */

// constructor:
stat_t::stat_t() {
    init();
}

// init method
void stat_t::init() {
    game_stats.init();
    move_stats.init();

    max_moves = 0;
}

// increase the number of moves evaluated
void stat_t::inc_moves_count() {
    game_stats.increment();
    move_stats.increment();
}

// start the game timers and clear out the game counts
void stat_t::start_game_stats() {
    game_stats.begin();
}

// stop the game timers and calc the game stats
void stat_t::stop_game_stats() {
    game_stats.end();
}

// start the move timers and clear out the move counts
void stat_t::start_move_stats() {
    move_stats.begin();
}

uint32_t stat_t::move_count_so_far() const {
    return move_stats.counter();
}

// stop the move timers and calc the move stats
void stat_t::stop_move_stats() {
    move_stats.end();
}