/**
 * game.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * MicroChess game_t implementation
 * 
 */

#include <Arduino.h>
#include "MicroChess.h"
#include "game.h"

// the game options
options_t game_t::options;

game_t::game_t()
{
    init();
}


void game_t::init()
{
    // Initialize the game.pieces[] list based off of the current board contents:
    piece_count = 0;
    for (uint8_t ndx=0; ndx < BOARD_SIZE; ++ndx) {
        if (Empty == getType(board.get(ndx))) continue;
        if (piece_count < MAX_PIECES) {
            pieces[piece_count++] = point_t( ndx % 8, ndx / 8);
        }
    }

#ifdef ENA_MEM_STATS
    lowest_mem = 0xFFFF;
    lowest_mem_ply = -1;
#endif

    hist_count = 0;

    stats.init();

    white_taken_count = 0;
    black_taken_count = 0;

    last_value = 0;
    last_move = { -1, -1, 0 };
    last_was_en_passant = False;
    last_was_castle = False;
    last_was_timeout = False;
    last_was_pawn_promotion = False;
    last_move_time = 0;
    last_moves_evaluated = 0;

    white_king_in_check = False;
    black_king_in_check = False;

    state = PLAYING;

    turn = White;

    move_num = 0;

    ply = 0;

    wking = 60;
    bking =  4;

    alpha = MIN_VALUE;
    beta  = MAX_VALUE;

} // init()


////////////////////////////////////////////////////////////////////////////////////////
// use pre-computed bonus tables for speed!
//                                [col/row][type][side] 
// Alias' for experimenting with center bonus strategies:
static int constexpr PAWN   = Pawn;
static int constexpr KNIGHT = Knight;
static int constexpr ROOK   = Rook;
static int constexpr BISHOP = Bishop;
static int constexpr QUEEN  = Queen;
// static int constexpr PAWN   = 1;
// static int constexpr ROOK   = 1;
// static int constexpr BISHOP = 1;
// static int constexpr QUEEN  = 1;

long const game_t::center_bonus[   8   ][  7 ][  2 ] PROGMEM = {
    //                      Black                   ,                      White 
    {
        { 0 *  Empty * options.centerBonus * -1,    0 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 0 *   PAWN * options.centerBonus * -1,    0 *   PAWN * options.centerBonus * +1 },
        { 0 * KNIGHT * options.centerBonus * -1,    0 * KNIGHT * options.centerBonus * +1 },
        { 0 * BISHOP * options.centerBonus * -1,    0 * BISHOP * options.centerBonus * +1 },
        { 0 *   ROOK * options.centerBonus * -1,    0 *   ROOK * options.centerBonus * +1 },
        { 0 *  QUEEN * options.centerBonus * -1,    0 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 1 *  Empty * options.centerBonus * -1,    1 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 1 *   PAWN * options.centerBonus * -1,    1 *   PAWN * options.centerBonus * +1 },
        { 1 * KNIGHT * options.centerBonus * -1,    1 * KNIGHT * options.centerBonus * +1 },
        { 1 * BISHOP * options.centerBonus * -1,    1 * BISHOP * options.centerBonus * +1 },
        { 1 *   ROOK * options.centerBonus * -1,    1 *   ROOK * options.centerBonus * +1 },
        { 1 *  QUEEN * options.centerBonus * -1,    1 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 2 *  Empty * options.centerBonus * -1,    2 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 2 *   PAWN * options.centerBonus * -1,    2 *   PAWN * options.centerBonus * +1 },
        { 2 * KNIGHT * options.centerBonus * -1,    2 * KNIGHT * options.centerBonus * +1 },
        { 2 * BISHOP * options.centerBonus * -1,    2 * BISHOP * options.centerBonus * +1 },
        { 2 *   ROOK * options.centerBonus * -1,    2 *   ROOK * options.centerBonus * +1 },
        { 2 *  QUEEN * options.centerBonus * -1,    2 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 3 *  Empty * options.centerBonus * -1,    3 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 3 *   PAWN * options.centerBonus * -1,    3 *   PAWN * options.centerBonus * +1 },
        { 3 * KNIGHT * options.centerBonus * -1,    3 * KNIGHT * options.centerBonus * +1 },
        { 3 * BISHOP * options.centerBonus * -1,    3 * BISHOP * options.centerBonus * +1 },
        { 3 *   ROOK * options.centerBonus * -1,    3 *   ROOK * options.centerBonus * +1 },
        { 3 *  QUEEN * options.centerBonus * -1,    3 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 3 *  Empty * options.centerBonus * -1,    3 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 3 *   PAWN * options.centerBonus * -1,    3 *   PAWN * options.centerBonus * +1 },
        { 3 * KNIGHT * options.centerBonus * -1,    3 * KNIGHT * options.centerBonus * +1 },
        { 3 * BISHOP * options.centerBonus * -1,    3 * BISHOP * options.centerBonus * +1 },
        { 3 *   ROOK * options.centerBonus * -1,    3 *   ROOK * options.centerBonus * +1 },
        { 3 *  QUEEN * options.centerBonus * -1,    3 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 2 *  Empty * options.centerBonus * -1,    2 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 2 *   PAWN * options.centerBonus * -1,    2 *   PAWN * options.centerBonus * +1 },
        { 2 * KNIGHT * options.centerBonus * -1,    2 * KNIGHT * options.centerBonus * +1 },
        { 2 * BISHOP * options.centerBonus * -1,    2 * BISHOP * options.centerBonus * +1 },
        { 2 *   ROOK * options.centerBonus * -1,    2 *   ROOK * options.centerBonus * +1 },
        { 2 *  QUEEN * options.centerBonus * -1,    2 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 1 *  Empty * options.centerBonus * -1,    1 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 1 *   PAWN * options.centerBonus * -1,    1 *   PAWN * options.centerBonus * +1 },
        { 1 * KNIGHT * options.centerBonus * -1,    1 * KNIGHT * options.centerBonus * +1 },
        { 1 * BISHOP * options.centerBonus * -1,    1 * BISHOP * options.centerBonus * +1 },
        { 1 *   ROOK * options.centerBonus * -1,    1 *   ROOK * options.centerBonus * +1 },
        { 1 *  QUEEN * options.centerBonus * -1,    1 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 0 *  Empty * options.centerBonus * -1,    0 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 0 *   PAWN * options.centerBonus * -1,    0 *   PAWN * options.centerBonus * +1 },
        { 0 * KNIGHT * options.centerBonus * -1,    0 * KNIGHT * options.centerBonus * +1 },
        { 0 * BISHOP * options.centerBonus * -1,    0 * BISHOP * options.centerBonus * +1 },
        { 0 *   ROOK * options.centerBonus * -1,    0 *   ROOK * options.centerBonus * +1 },
        { 0 *  QUEEN * options.centerBonus * -1,    0 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }
};

long const game_t::material_bonus[7][2] PROGMEM = {
    //          Black        ,           White  
    { pieceValues[ Empty] * -1,     pieceValues[ Empty] * +1 },   //  Empty
    { pieceValues[  Pawn] * -1,     pieceValues[  Pawn] * +1 },   //   Pawn
    { pieceValues[Knight] * -1,     pieceValues[Knight] * +1 },   // Knight
    { pieceValues[Bishop] * -1,     pieceValues[Bishop] * +1 },   // Bishop
    { pieceValues[  Rook] * -1,     pieceValues[  Rook] * +1 },   //   Rook
    { pieceValues[ Queen] * -1,     pieceValues[ Queen] * +1 },   //  Queen
    { pieceValues[  King] * -1,     pieceValues[  King] * +1 }    //   King
};
