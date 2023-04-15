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

    white_taken_count = 0;
    black_taken_count = 0;

    last_value = 0;
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


////////////////////////////////////////////////////////////////////////////////////////
// use pre-computed bonus tables for speed!
//                                [col/row][type][side] 
long constexpr game_t::center_bonus[   8   ][  7 ][  2 ] PROGMEM = {
    //                      Black                   ,                      White 
    {
        { 0 *  Empty * options.centerBonus * -1,    0 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 0 *   Pawn * options.centerBonus * -1,    0 *   Pawn * options.centerBonus * +1 },
        { 0 * Knight * options.centerBonus * -1,    0 * Knight * options.centerBonus * +1 },
        { 0 * Bishop * options.centerBonus * -1,    0 * Bishop * options.centerBonus * +1 },
        { 0 *   Rook * options.centerBonus * -1,    0 *   Rook * options.centerBonus * +1 },
        { 0 *  Queen * options.centerBonus * -1,    0 *  Queen * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 1 *  Empty * options.centerBonus * -1,    1 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 1 *   Pawn * options.centerBonus * -1,    1 *   Pawn * options.centerBonus * +1 },
        { 1 * Knight * options.centerBonus * -1,    1 * Knight * options.centerBonus * +1 },
        { 1 * Bishop * options.centerBonus * -1,    1 * Bishop * options.centerBonus * +1 },
        { 1 *   Rook * options.centerBonus * -1,    1 *   Rook * options.centerBonus * +1 },
        { 1 *  Queen * options.centerBonus * -1,    1 *  Queen * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 2 *  Empty * options.centerBonus * -1,    2 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 2 *   Pawn * options.centerBonus * -1,    2 *   Pawn * options.centerBonus * +1 },
        { 2 * Knight * options.centerBonus * -1,    2 * Knight * options.centerBonus * +1 },
        { 2 * Bishop * options.centerBonus * -1,    2 * Bishop * options.centerBonus * +1 },
        { 2 *   Rook * options.centerBonus * -1,    2 *   Rook * options.centerBonus * +1 },
        { 2 *  Queen * options.centerBonus * -1,    2 *  Queen * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 3 *  Empty * options.centerBonus * -1,    3 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 3 *   Pawn * options.centerBonus * -1,    3 *   Pawn * options.centerBonus * +1 },
        { 3 * Knight * options.centerBonus * -1,    3 * Knight * options.centerBonus * +1 },
        { 3 * Bishop * options.centerBonus * -1,    3 * Bishop * options.centerBonus * +1 },
        { 3 *   Rook * options.centerBonus * -1,    3 *   Rook * options.centerBonus * +1 },
        { 3 *  Queen * options.centerBonus * -1,    3 *  Queen * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 3 *  Empty * options.centerBonus * -1,    3 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 3 *   Pawn * options.centerBonus * -1,    3 *   Pawn * options.centerBonus * +1 },
        { 3 * Knight * options.centerBonus * -1,    3 * Knight * options.centerBonus * +1 },
        { 3 * Bishop * options.centerBonus * -1,    3 * Bishop * options.centerBonus * +1 },
        { 3 *   Rook * options.centerBonus * -1,    3 *   Rook * options.centerBonus * +1 },
        { 3 *  Queen * options.centerBonus * -1,    3 *  Queen * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 2 *  Empty * options.centerBonus * -1,    2 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 2 *   Pawn * options.centerBonus * -1,    2 *   Pawn * options.centerBonus * +1 },
        { 2 * Knight * options.centerBonus * -1,    2 * Knight * options.centerBonus * +1 },
        { 2 * Bishop * options.centerBonus * -1,    2 * Bishop * options.centerBonus * +1 },
        { 2 *   Rook * options.centerBonus * -1,    2 *   Rook * options.centerBonus * +1 },
        { 2 *  Queen * options.centerBonus * -1,    2 *  Queen * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 1 *  Empty * options.centerBonus * -1,    1 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 1 *   Pawn * options.centerBonus * -1,    1 *   Pawn * options.centerBonus * +1 },
        { 1 * Knight * options.centerBonus * -1,    1 * Knight * options.centerBonus * +1 },
        { 1 * Bishop * options.centerBonus * -1,    1 * Bishop * options.centerBonus * +1 },
        { 1 *   Rook * options.centerBonus * -1,    1 *   Rook * options.centerBonus * +1 },
        { 1 *  Queen * options.centerBonus * -1,    1 *  Queen * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 0 *  Empty * options.centerBonus * -1,    0 *  Empty * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 0 *   Pawn * options.centerBonus * -1,    0 *   Pawn * options.centerBonus * +1 },
        { 0 * Knight * options.centerBonus * -1,    0 * Knight * options.centerBonus * +1 },
        { 0 * Bishop * options.centerBonus * -1,    0 * Bishop * options.centerBonus * +1 },
        { 0 *   Rook * options.centerBonus * -1,    0 *   Rook * options.centerBonus * +1 },
        { 0 *  Queen * options.centerBonus * -1,    0 *  Queen * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }
};

long constexpr game_t::material_bonus[7][2] PROGMEM = {
    //          Black        ,           White  
    { pieceValues[ Empty] * -1,     pieceValues[ Empty] * +1 },   //  Empty
    { pieceValues[  Pawn] * -1,     pieceValues[  Pawn] * +1 },   //   Pawn
    { pieceValues[Knight] * -1,     pieceValues[Knight] * +1 },   // Knight
    { pieceValues[Bishop] * -1,     pieceValues[Bishop] * +1 },   // Bishop
    { pieceValues[  Rook] * -1,     pieceValues[  Rook] * +1 },   //   Rook
    { pieceValues[ Queen] * -1,     pieceValues[ Queen] * +1 },   //  Queen
    { pieceValues[  King] * -1,     pieceValues[  King] * +1 }    //   King
};


////////////////////////////////////////////////////////////////////////////////////////
// transposition tables for move generation
offset_t constexpr game_t::knight_offsets[NUM_KNIGHT_OFFSETS] PROGMEM = {
    { -2, +1 }, { -2, -1 }, { +2, +1 }, { +2, -1 }, 
    { +1, +2 }, { -1, +2 }, { +1, -2 }, { -1, -2 }  
};

offset_t constexpr game_t::rook_offsets[NUM_ROOK_OFFSETS] PROGMEM = {
    { -1,  0 }, { -2,  0 }, { -3,  0 }, { -4,  0 }, { -5,  0 }, { -6,  0 }, { -7,  0 },     // W
    { +1,  0 }, { +2,  0 }, { +3,  0 }, { +4,  0 }, { +5,  0 }, { +6,  0 }, { +7,  0 },     // E
    {  0, +1 }, {  0, +2 }, {  0, +3 }, {  0, +4 }, {  0, +5 }, {  0, +6 }, {  0, +7 },     // N
    {  0, -1 }, {  0, -2 }, {  0, -3 }, {  0, -4 }, {  0, -5 }, {  0, -6 }, {  0, -7 }      // S
};

offset_t constexpr game_t::bishop_offsets[NUM_BISHOP_OFFSETS] PROGMEM = {
    { -1, +1 }, { -2, +2 }, { -3, +3 }, { -4, +4 }, { -5, +5 }, { -6, +6 }, { -7, +7 },     // NW
    { +1, +1 }, { +2, +2 }, { +3, +3 }, { +4, +4 }, { +5, +5 }, { +6, +6 }, { +7, +7 },     // NE
    { -1, -1 }, { -2, -2 }, { -3, -3 }, { -4, -4 }, { -5, -5 }, { -6, -6 }, { -7, -7 },     // SW
    { +1, -1 }, { +2, -2 }, { +3, -3 }, { +4, -4 }, { +5, -5 }, { +6, -6 }, { +7, -7 }      // SE
};

offset_t constexpr game_t::queen_offsets[NUM_QUEEN_OFFSETS] PROGMEM = {
    { -1,  0 }, { -2,  0 }, { -3,  0 }, { -4,  0 }, { -5,  0 }, { -6,  0 }, { -7,  0 },     // W
    { +1,  0 }, { +2,  0 }, { +3,  0 }, { +4,  0 }, { +5,  0 }, { +6,  0 }, { +7,  0 },     // E
    {  0, +1 }, {  0, +2 }, {  0, +3 }, {  0, +4 }, {  0, +5 }, {  0, +6 }, {  0, +7 },     // N
    {  0, -1 }, {  0, -2 }, {  0, -3 }, {  0, -4 }, {  0, -5 }, {  0, -6 }, {  0, -7 },     // S
    { -1, +1 }, { -2, +2 }, { -3, +3 }, { -4, +4 }, { -5, +5 }, { -6, +6 }, { -7, +7 },     // NW
    { +1, +1 }, { +2, +2 }, { +3, +3 }, { +4, +4 }, { +5, +5 }, { +6, +6 }, { +7, +7 },     // NE
    { -1, -1 }, { -2, -2 }, { -3, -3 }, { -4, -4 }, { -5, -5 }, { -6, -6 }, { -7, -7 },     // SW
    { +1, -1 }, { +2, -2 }, { +3, -3 }, { +4, -4 }, { +5, -5 }, { +6, -6 }, { +7, -7 }      // SE
};

offset_t constexpr game_t::king_offsets[NUM_KING_OFFSETS] PROGMEM = {
    { -1,  0 }, {  0, -1 }, { -1, -1 }, { +1, -1 }, 
    { +1,  0 }, {  0, +1 }, { -1, +1 }, { +1, +1 }
};
