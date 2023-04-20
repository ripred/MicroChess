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
#include <stdint.h>
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
    // the game options
    static options_t options;

    static long const center_bonus[8][7][2] PROGMEM;
    static long const material_bonus[7][2] PROGMEM;

    // the last 5 moves are kept to recognize 3-move repetition
    move_t      history[MAX_REPS * 2 - 1];
    index_t     hist_count;

    // the pieces on the board
    point_t     pieces[MAX_PIECES];
    uint8_t     piece_count;

    // the pieces that have been taken
    Piece       taken_by_white[16];
    Piece       taken_by_black[16];
    uint8_t     white_taken_count;
    uint8_t     black_taken_count;

    // the statistics of the game
    stat_t      stats;

    // the check state of the two kings
    Bool        white_king_in_check;
    Bool        black_king_in_check;

    // the last move made
    long        last_value;
    move_t      last_move;
    uint8_t     last_was_en_passant     : 1,    // true when last move was an en-passaant capture
                last_was_castle         : 1,    // true when last move was a castling of a king and a rook
                last_was_timeout        : 1,    // true when last move timed out
                last_was_pawn_promotion : 1;    // true when last move promoted a Pawn to a Queen

    // last move statistics
    uint32_t    last_move_time;
    uint32_t    last_moves_evaluated;

    // whose turn it is
    uint8_t     turn;           // 0 := Black, 1 := White

    // the current state of the game
    state_t     state;

    // increasing move number
    uint16_t    move_num;

    // the current ply level
    uint8_t     ply;

    // the location of both kings
    index_t     wking;
    index_t     bking;

    // the alpha and beta edges of our search envelope
    long        alpha;
    long        beta;
public:
    // constructor
    game_t();

    // init function
    void init();

};  // game_t

#endif // GAME_INCL