/**
 * stats.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * MicroChess statistics and timing functions
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
    maxply = 0;
}


// Begin the timer
void movetime_t::begin() 
{
    if (!running) {
        start = millis();
        stop = start;
        dur = 0;
        count = 0;
        moves_per_sec = 0.0;
        running = True;
    }
}


// End the timer
void movetime_t::end() 
{
    if (running) {
        stop = millis();
        dur = stop - start;
        running = False;

        if (0 != dur && 0 != count) {
            moves_per_sec = double(count) / (double(dur) / 1000.0);
        }
    }
}


// Get the time so far in milliseconds, or the total time,
// depending on whether the timer is running or not.
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
    // return moves_per_sec;
    return double(count) / (duration() / 1000.0);
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