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

} // game_t::game_t()


////////////////////////////////////////////////////////////////////////////////////////
// Set the pieces[] array based off of the board contents
void game_t::set_pieces_from_board(board_t &board)
{
    // Initialize the game.pieces[] list based off of the current board contents:
    piece_count = 0;

    for (index_t index = 0; index < index_t(BOARD_SIZE); index++) {
        if (Empty == getType(board.get(index))) continue;
        if (piece_count < MAX_PIECES) {
            pieces[piece_count++] = point_t(index % 8, index / 8);
        }
    }

} // game_t::set_pieces_from_board(board_t &board)


////////////////////////////////////////////////////////////////////////////////////////
// Compare the pieces[] array to the board contents and return False
// if there are differences or return True if they are the same.
Bool game_t::compare_pieces_to_board(board_t &board)
{
    for (index_t index = 0; index < index_t(BOARD_SIZE); index++) {
        Piece   const piece = board.get(index);
        index_t const piece_index = find_piece(index);

        if (Empty == getType(piece)) {
            // Since this spot is empty, make sure the spot
            // IS NOT in our pieces[] list:
            if (-1 != piece_index) {
                // ERROR - we shouldn't have found this spot
                return False;
            }

            continue;
        }

        if (-1 == piece_index) {
            // ERROR - we should have found this spot
            return False;
        }
    }

    return True;

} // game_t::compare_pieces_to_board(board_t &board)


////////////////////////////////////////////////////////////////////////////////////////
// Find the piece index for a given board index.
// 
// returns the index into the game.pieces[] array for the specified piece
index_t game_t::find_piece(index_t const index)
{
    for (index_t piece_index = 0; piece_index < piece_count; piece_index++) {
        if (!isValidPos(pieces[piece_index].x, pieces[piece_index].y)) {
            continue;
        }
        else {
            index_t const board_index = pieces[piece_index].x + pieces[piece_index].y * 8;
            if (board_index == index) {
                return piece_index;
            }
        }
    }

    return -1;

} // game_t::find_piece(int const index)


////////////////////////////////////////////////////////////////////////////////////////
// Initialize for a new game
void game_t::init()
{
    set_pieces_from_board(board);

    #ifdef ENA_MEM_STATS
    lowest_mem = 0xFFFF;
    lowest_mem_ply = -1;
    #endif

    complete = 0;

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
    // last_move_time = 0;
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

} // game_t::init()


////////////////////////////////////////////////////////////////////////////////////////
// use pre-computed bonus tables for speed!
// Alias' for experimenting with center bonus strategies:
static int constexpr EMPTY  = Empty;
static int constexpr PAWN   = Pawn;
static int constexpr KNIGHT = Knight;
static int constexpr ROOK   = Rook;
static int constexpr BISHOP = Bishop;
static int constexpr QUEEN  = Queen;
// static int constexpr PAWN   = 1;
// static int constexpr KNIGHT = 1;
// static int constexpr ROOK   = 1;
// static int constexpr BISHOP = 1;
// static int constexpr QUEEN  = 1;

long const game_t::center_bonus[ 8 ][ 7 ][ 2 ] PROGMEM = {
    //                      Black                   ,                      White 
    {
        { 0 *  EMPTY * options.centerBonus * -1,    0 *  EMPTY * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 7 *   PAWN * options.centerBonus * -1,    0 *   PAWN * options.centerBonus * +1 },
        { 0 * KNIGHT * options.centerBonus * -1,    0 * KNIGHT * options.centerBonus * +1 },
        { 0 * BISHOP * options.centerBonus * -1,    0 * BISHOP * options.centerBonus * +1 },
        { 0 *   ROOK * options.centerBonus * -1,    0 *   ROOK * options.centerBonus * +1 },
        { 0 *  QUEEN * options.centerBonus * -1,    0 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 1 *  EMPTY * options.centerBonus * -1,    1 *  EMPTY * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 6 *   PAWN * options.centerBonus * -1,    1 *   PAWN * options.centerBonus * +1 },
        { 1 * KNIGHT * options.centerBonus * -1,    1 * KNIGHT * options.centerBonus * +1 },
        { 1 * BISHOP * options.centerBonus * -1,    1 * BISHOP * options.centerBonus * +1 },
        { 1 *   ROOK * options.centerBonus * -1,    1 *   ROOK * options.centerBonus * +1 },
        { 1 *  QUEEN * options.centerBonus * -1,    1 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 2 *  EMPTY * options.centerBonus * -1,    2 *  EMPTY * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 5 *   PAWN * options.centerBonus * -1,    2 *   PAWN * options.centerBonus * +1 },
        { 2 * KNIGHT * options.centerBonus * -1,    2 * KNIGHT * options.centerBonus * +1 },
        { 2 * BISHOP * options.centerBonus * -1,    2 * BISHOP * options.centerBonus * +1 },
        { 2 *   ROOK * options.centerBonus * -1,    2 *   ROOK * options.centerBonus * +1 },
        { 2 *  QUEEN * options.centerBonus * -1,    2 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 3 *  EMPTY * options.centerBonus * -1,    3 *  EMPTY * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 4 *   PAWN * options.centerBonus * -1,    3 *   PAWN * options.centerBonus * +1 },
        { 3 * KNIGHT * options.centerBonus * -1,    3 * KNIGHT * options.centerBonus * +1 },
        { 3 * BISHOP * options.centerBonus * -1,    3 * BISHOP * options.centerBonus * +1 },
        { 3 *   ROOK * options.centerBonus * -1,    3 *   ROOK * options.centerBonus * +1 },
        { 3 *  QUEEN * options.centerBonus * -1,    3 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 3 *  EMPTY * options.centerBonus * -1,    3 *  EMPTY * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 3 *   PAWN * options.centerBonus * -1,    4 *   PAWN * options.centerBonus * +1 },
        { 3 * KNIGHT * options.centerBonus * -1,    3 * KNIGHT * options.centerBonus * +1 },
        { 3 * BISHOP * options.centerBonus * -1,    3 * BISHOP * options.centerBonus * +1 },
        { 3 *   ROOK * options.centerBonus * -1,    3 *   ROOK * options.centerBonus * +1 },
        { 3 *  QUEEN * options.centerBonus * -1,    3 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 2 *  EMPTY * options.centerBonus * -1,    2 *  EMPTY * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 2 *   PAWN * options.centerBonus * -1,    5 *   PAWN * options.centerBonus * +1 },
        { 2 * KNIGHT * options.centerBonus * -1,    2 * KNIGHT * options.centerBonus * +1 },
        { 2 * BISHOP * options.centerBonus * -1,    2 * BISHOP * options.centerBonus * +1 },
        { 2 *   ROOK * options.centerBonus * -1,    2 *   ROOK * options.centerBonus * +1 },
        { 2 *  QUEEN * options.centerBonus * -1,    2 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 1 *  EMPTY * options.centerBonus * -1,    1 *  EMPTY * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 1 *   PAWN * options.centerBonus * -1,    6 *   PAWN * options.centerBonus * +1 },
        { 1 * KNIGHT * options.centerBonus * -1,    1 * KNIGHT * options.centerBonus * +1 },
        { 1 * BISHOP * options.centerBonus * -1,    1 * BISHOP * options.centerBonus * +1 },
        { 1 *   ROOK * options.centerBonus * -1,    1 *   ROOK * options.centerBonus * +1 },
        { 1 *  QUEEN * options.centerBonus * -1,    1 *  QUEEN * options.centerBonus * +1 },
        {                             MAX_VALUE,                                MIN_VALUE },
    }, {
        { 0 *  EMPTY * options.centerBonus * -1,    0 *  EMPTY * options.centerBonus * +1 },       // col/row offset 0; 3 from center
        { 0 *   PAWN * options.centerBonus * -1,    7 *   PAWN * options.centerBonus * +1 },
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
