/**
 * game.h
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * header file for MicroChess
 * 
 */
#ifndef GAME_INCL
#define GAME_INCL

#include "Arduino.h"
#include "board.h"

extern board_t board;

////////////////////////////////////////////////////////////////////////////////////////
// board spot by column and row
struct point_t 
{
public:
    index_t  x : NUM_BITS_PT, 
             y : NUM_BITS_PT;

public:
    point_t() : x(0), y(0) {}
    point_t(index_t X, index_t Y) : x(X), y(Y) {}

};  // point_t


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
    stat_t() {
        init();
    }

    // init method
    void init() {
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
    void inc_moves_count() {
        moves_gen_game++;
    }

    // start the game timers and clear out the game counts
    void start_game_stats() {
        game_start = millis();
        game_end = game_start;
        game_time = 0;
        moves_gen_game = 0;
    }

    // stop the game timers and calc the game stats
    void stop_game_stats() {
        game_end = millis();
        game_time = game_end - game_start;
    }

    // start the move timers and clear out the move counts
    void start_move_stats() {
        move_start = millis();
        move_end = move_start;
        move_time = 0;

        moves_gen_move_start = moves_gen_game;
        moves_gen_move_end = moves_gen_move_start;
        moves_gen_move_delta = 0;
    }

    // stop the move timers and calc the move stats
    void stop_move_stats() {
        move_end = millis();
        move_time = move_end - move_start;

        moves_gen_move_end = moves_gen_game;
        moves_gen_move_delta = moves_gen_move_end - moves_gen_move_start;
    }

};  // stat_t


////////////////////////////////////////////////////////////////////////////////////////
// the settings for a game
struct options_t {
public:
    uint8_t
        random : 1,     // add randomness to the game?
     profiling : 1;     // are we profiling the engine?
    uint8_t maxply;     // the maximum ply level

public:

    options_t() : 
        random(False), 
        profiling(False), 
        maxply(MAX_PLY) 
    {}

};  // options_t


////////////////////////////////////////////////////////////////////////////////////////
// the state of a game
struct game_t 
{
public:
    point_t     pieces[MAX_PIECES];
    Piece       taken1[16], taken2[16];
    stat_t      stats;
    Bool        white_king_in_check;
    Bool        black_king_in_check;
    move_t      last_move;
    uint32_t    last_move_time;
    uint32_t    last_moves_evaluated;
    uint8_t     piece_count;
    uint8_t     taken_count1;
    uint8_t     taken_count2;
    uint8_t     turn;           // 0 := Black, 1 := White
    uint8_t     done;           // 1 := game over
    uint16_t    move_num;       // increasing move number

    options_t   options;        // game options

    uint8_t     ply;            // current ply level

    // TODO: track all moves of equal value, not just one
    move_t    best_white_move;
    move_t    best_black_move;

public:
    // constructor
    game_t()
    {
        init();
    }

    void init()
    {
        // initialize list of pieces in the game based of of the board contents:
        piece_count = 0;
        for (uint8_t ndx=0; ndx < BOARD_SIZE; ++ndx) {
            if (Empty == getType(board.get(ndx))) continue;
            if (piece_count < MAX_PIECES) {
                pieces[piece_count++] = point_t( ndx % 8, ndx / 8);
            }
        }

        stats = stat_t();

        taken_count1 = 0;
        taken_count2 = 0;

        last_move = { -1, -1, 0 };
        last_move_time = 0;
        last_moves_evaluated = 0;

        white_king_in_check = 0;
        black_king_in_check = 0;

        turn = White;
        move_num = 0;

        done = 0;

        best_white_move = { -1, -1, 0 };
        best_black_move = { -1, -1, 0 };

        ply = 0;
    }

};  // game_t

#endif // GAME_INCL