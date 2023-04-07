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

    uint32_t move_count_so_far() const {
        return moves_gen_game - moves_gen_move_start;
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
           random : 1,          // add randomness to the game?
        profiling : 1;          // are we profiling the engine?
    uint8_t     maxply;         // the maximum ply level
    uint8_t     move_limit;     // the maximum number of moves allowed in a full game
    uint32_t    seed;           // the starting seed hash for prn's
    print_t     print_level;    // the verbosity setting for the level of output

public:

    options_t() : 
        random(False), 
        profiling(False), 
        maxply(MAX_PLY),
        move_limit(MOVE_LIMIT),
        seed(PRN_SEED),
        print_level(Debug1)
    {}

};  // options_t


////////////////////////////////////////////////////////////////////////////////////////
// the state of a game
struct game_t 
{
public:
    // the last 5 moves are kept to recognize 3-move repetition
    move_t      history[5];
    index_t     hist_count;

    // the pieces on the board
    point_t     pieces[MAX_PIECES];
    uint8_t     piece_count;

    // the pieces that have been taken
    Piece       taken1[16], taken2[16];
    uint8_t     taken_count1;
    uint8_t     taken_count2;

    // the statistics of the game
    stat_t      stats;

    // the check state of the two kings
    Bool        white_king_in_check;
    Bool        black_king_in_check;

    // the last move made
    move_t      last_move;
    uint8_t     last_was_en_passant : 1;    // true when last move was an en-passaant capture
    uint32_t    last_move_time;
    uint32_t    last_moves_evaluated;

    // whose turn it is
    uint8_t     turn;           // 0 := Black, 1 := White

    // flag indicating the end of the game
    Bool        done;

    // increasing move number
    uint16_t    move_num;

    // the game options
    options_t   options;

    // the current ply level
    uint8_t     ply;

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

        hist_count = 0;

        stats = stat_t();

        taken_count1 = 0;
        taken_count2 = 0;

        last_move = { -1, -1, 0 };
        last_was_en_passant = False;
        last_move_time = 0;
        last_moves_evaluated = 0;

        white_king_in_check = False;
        black_king_in_check = False;

        turn = White;
        move_num = 0;

        done = False;

        ply = 0;
    }

};  // game_t

#endif // GAME_INCL