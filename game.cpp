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

// The game options
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
Bool game_t::compare_pieces_to_board(board_t &board) const
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
index_t game_t::find_piece(index_t const index) const
{
    for (index_t piece_index = 0; piece_index < piece_count; piece_index++) {
        if (!isValidPos(pieces[piece_index].x, pieces[piece_index].y)) {
            continue;
        }
        else {
            index_t const board_index = pieces[piece_index].x + pieces[piece_index].y * 8u;
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

    stats.init();

    hist_count = 0;

    white_taken_count = 0;
    black_taken_count = 0;

    last_was_pawn_promotion = False;
    last_was_en_passant = False;
    last_was_castle = False;

    timeout1 = False;
    timeout2 = False;

    last_move = { -1, -1, 0 };

    white_king_in_check = False;
    black_king_in_check = False;

    state = PLAYING;

    turn = White;

    move_num = 0;
    book_index = 0;

    ply = 0;

    // Set the location of the two kings
    wking = 7 * 8 + 4;
    bking = 0 * 8 + 4;

    alpha = MIN_VALUE;
    beta  = MAX_VALUE;

    book_supplied = False;
    user_supplied = False;
    supply_valid = False;
    supplied = { -1, -1, 0 };

} // game_t::init()


////////////////////////////////////////////////////////////////////////////////////////
// Sort the game.pieces[] array by player side
void game_t::sort_pieces(Color const side)
{
    if (White == side) {
        // lambda comparator to sort game.pieces[] by White and then Black
        auto compare = [](const void *a, const void *b) -> int {
            point_t const piece_a = *((point_t*) a);
            point_t const piece_b = *((point_t*) b);
            Color   const side_a = getSide(board.get(piece_a.x + piece_a.y * 8));
            Color   const side_b = getSide(board.get(piece_b.x + piece_b.y * 8));
            return (side_a == side_b) ? 0 : ((side_a < side_b) ? +1 : -1);
        };

        qsort(pieces, piece_count, sizeof(point_t), compare);
    }
    else {
        // lambda comparator to sort game.pieces[] by Black and then White
        auto compare = [](const void *a, const void *b) -> int {
            point_t const piece_a = *((point_t*) a);
            point_t const piece_b = *((point_t*) b);
            Color   const side_a = getSide(board.get(piece_a.x + piece_a.y * 8));
            Color   const side_b = getSide(board.get(piece_b.x + piece_b.y * 8));
            return (side_a == side_b) ? 0 : ((side_a > side_b) ? +1 : -1);
        };

        qsort(pieces, piece_count, sizeof(point_t), compare);
    }

} // game_t::sort_pieces(Color const side)


// Shuffle the top side game.pieces[] array
// NOTE: This expects the game.pieces[] array to be sorted
//       with the current player's pieces at the top
void game_t::shuffle_pieces(index_t const shuffle_count)
{
    // Count the number of pieces at the top on the same side and shuffle them
    index_t count = 0;
    for (count = 0; (count + 1) < piece_count; count++) {
        index_t const index1 = pieces[count].x + pieces[count].y * 8u;
        index_t const index2 = pieces[count + 1].x + pieces[count + 1].y * 8u;
        if (getSide(board.get(index1)) != getSide(board.get(index2))) {
            break;
        }
    }

    // Shuffle the pieces
    if (count > 1) {
        for (index_t i = 0; i < shuffle_count; i++) {
            index_t r1 = random(count);
            index_t r2 = random(count);
            if (r1 == r2) { continue; }
            point_t const tmp = pieces[r1];
            pieces[r1] = pieces[r2];
            pieces[r2] = tmp;
        }
    }

} // game_t::shuffle(...)


////////////////////////////////////////////////////////////////////////////////////////
// use pre-computed bonus tables for speed!
// Alias' for experimenting with center bonus strategies:
// static int constexpr EMPTY  = Empty;
// static int constexpr PAWN   = Pawn;
// static int constexpr KNIGHT = Knight;
// static int constexpr ROOK   = Rook;
// static int constexpr BISHOP = Bishop;
// static int constexpr QUEEN  = Queen;

static int constexpr EMPTY  = Empty;
static int constexpr PAWN   = 1;
static int constexpr KNIGHT = 1;
static int constexpr ROOK   = 1;
static int constexpr BISHOP = 1;
static int constexpr QUEEN  = 1;

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