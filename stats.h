/**
 * stats.h
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * The stat_t structure holds the move counts for a game and
 * the starting end ending times for the game.
 * 
 * The structure also holds the same information for 
 * 
 * 
 */
#ifndef STATS_INCL
#define STATS_INCL

#include <stdint.h>

// the movetime_t structure holds a start time and an end time
// and a count of how many moves were evaluated during that time.
// This is used to calculate the average number of moves evaluated
// per second.
struct movetime_t {
private:
    uint32_t    start;
    uint32_t    stop;
    uint32_t    dur;
    uint32_t    count;
    Bool        running;
    double      moves_per_sec;

public:

    index_t     depth;

    // Constructor
    movetime_t();

    // init method
    void init();

    // Begin the timer
    void begin();

    // End the timer
    void end();

    // Get the time so far or the total time depending
    // on whether the timer is running or not.
    uint32_t duration() const;

    // Increment the counter
    uint32_t increment();

    // Get the counter
    uint32_t counter() const;

    // Get the moves per second
    double moveps() const;

};  // movetime_t


////////////////////////////////////////////////////////////////////////////////////////
// the statistics of a game
struct stat_t {
    movetime_t  game_stats;
    movetime_t  move_stats;

    // constructor:
    stat_t();

    // init method
    void init();

    // increase the number of moves evaluated
    void inc_moves_count();

    // start the game timers and clear out the game counts
    void start_game_stats();

    // stop the game timers and calc the game stats
    void stop_game_stats();

    // start the move timers and clear out the move counts
    void start_move_stats();

    uint32_t move_count_so_far() const;

    // stop the move timers and calc the move stats
    void stop_move_stats();

};  // stat_t

#endif  // STATS_INCL
