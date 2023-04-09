/**
 * board.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * implementation file for the board class
 * 
 */
#include <Arduino.h>
#include "MicroChess.h"
#include "board.h"

board_t::board_t() {
    for (index_t i = 0; i < index_t(BOARD_SIZE); i++) {
        board[i] = Empty;
    }
}


void board_t::init() {
    set( 0, makeSpot(  Rook, Black, 0, 0));
    set( 1, makeSpot(Knight, Black, 0, 0));
    set( 2, makeSpot(Bishop, Black, 0, 0));
    set( 3, makeSpot( Queen, Black, 0, 0));
    set( 4, makeSpot(  King, Black, 0, 0));
    set( 5, makeSpot(Bishop, Black, 0, 0));
    set( 6, makeSpot(Knight, Black, 0, 0));
    set( 7, makeSpot(  Rook, Black, 0, 0));
    set( 8, makeSpot(  Pawn, Black, 0, 0));
    set( 9, makeSpot(  Pawn, Black, 0, 0));
    set(10, makeSpot(  Pawn, Black, 0, 0));
    set(11, makeSpot(  Pawn, Black, 0, 0));
    set(12, makeSpot(  Pawn, Black, 0, 0));
    set(13, makeSpot(  Pawn, Black, 0, 0));
    set(14, makeSpot(  Pawn, Black, 0, 0));
    set(15, makeSpot(  Pawn, Black, 0, 0));

    for (index_t i = 16; i < 48; i++) set(i, Empty);

    set(48, makeSpot(  Pawn, White, 0, 0));
    set(49, makeSpot(  Pawn, White, 0, 0));
    set(50, makeSpot(  Pawn, White, 0, 0));
    set(51, makeSpot(  Pawn, White, 0, 0));
    set(52, makeSpot(  Pawn, White, 0, 0));
    set(53, makeSpot(  Pawn, White, 0, 0));
    set(54, makeSpot(  Pawn, White, 0, 0));
    set(55, makeSpot(  Pawn, White, 0, 0));
    set(56, makeSpot(  Rook, White, 0, 0));
    set(57, makeSpot(Knight, White, 0, 0));
    set(58, makeSpot(Bishop, White, 0, 0));
    set(59, makeSpot( Queen, White, 0, 0));
    set(60, makeSpot(  King, White, 0, 0));
    set(61, makeSpot(Bishop, White, 0, 0));
    set(62, makeSpot(Knight, White, 0, 0));
    set(63, makeSpot(  Rook, White, 0, 0));
}


Piece board_t::get(index_t const index) const 
{
    return board[index];
}


void board_t::set(index_t const index, Piece const piece)
{
    board[index] = piece;
}


Piece board_t::getType(index_t const index) const 
{
    return getType(get(index));
}
