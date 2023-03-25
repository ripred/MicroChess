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
// the state of a game
struct game_t 
{
public:
    point_t   pieces[MAX_PIECES];
    move_t    moves1[MAX_MOVES];
    move_t    moves2[MAX_MOVES];
    Piece     taken1[16];
    Piece     taken2[16];
    Bool      white_king_in_check;
    Bool      black_king_in_check;
    move_t    last_move;
    uint8_t   piece_count;
    uint8_t   move_count1;
    uint8_t   move_count2;
    uint8_t   max_moves;
    uint8_t   taken_count1;
    uint8_t   taken_count2;
    uint8_t   eval_ndx,     // board index being currently evaluated
                  turn,     // 0 := Black, 1 := White
              move_num,     // increasing move number
                  done;     // 1 := game over
public:
    // constructor
    game_t()
    {
        init();
    }

    void init()
    {
        // initialize list of pieces in the game
        piece_count = 0;
        for (uint8_t ndx=0; ndx < BOARD_SIZE; ++ndx) {
            if (Empty == getType(board.get(ndx))) continue;
            if (piece_count < MAX_PIECES) {
                pieces[piece_count++] = point_t( ndx % 8, ndx / 8);
            }
        }

        move_count1 = 0;
        move_count2 = 0;

        max_moves = 0;

        taken_count1 = 0;
        taken_count2 = 0;

        last_move = { -1, -1, 0 };

        white_king_in_check = 0;
        black_king_in_check = 0;

        turn = White;
        move_num = 0;

        done = 0;
    }

};  // game_t

#endif // GAME_INCL