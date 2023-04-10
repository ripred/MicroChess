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
#include "stats.h"
#include "options.h"

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

    // the current state of the game
    state_t     state;

    // increasing move number
    uint16_t    move_num;

    // the game options
    options_t   options;

    // the current ply level
    uint8_t     ply;

    // the location of the two kings
    index_t     white_king;
    index_t     black_king;

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

        stats.init();

        taken_count1 = 0;
        taken_count2 = 0;

        last_move = { -1, -1, 0 };
        last_was_en_passant = False;
        last_move_time = 0;
        last_moves_evaluated = 0;

        white_king_in_check = False;
        black_king_in_check = False;

        state = PLAYING;

        turn = White;

        move_num = 0;

        ply = 0;

        white_king = 60;
        black_king = 4;

    }

};  // game_t

#endif // GAME_INCL