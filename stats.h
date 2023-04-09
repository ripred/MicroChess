/**
 * stats.h
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * header file for MicroChess
 * 
 */
#ifndef STATS_INCL
#define STATS_INCL

////////////////////////////////////////////////////////////////////////////////////////
// the statistics of a game
struct stat_t {
    // move counts
    uint16_t    max_moves;                  // max moves generated during make_all_moves()

    uint32_t    moves_gen_game;             // moves generated entire game

    uint32_t    moves_gen_move_start;       // value of moves_gen_game on move start
    uint32_t    moves_gen_move_end;         // value of moves_gen_game on move end
    uint32_t    moves_gen_move_delta;       // total moves considered for this move

    // time tracking
    uint32_t    game_start;
    uint32_t    game_end;
    uint32_t    game_time;

    uint32_t    move_start;
    uint32_t    move_end;
    uint32_t    move_time;

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
