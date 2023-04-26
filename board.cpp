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
    init();
}


void board_t::clear() {
    for (Piece &entry : board) { entry = Empty; }
}


void board_t::init() {
    clear();

    set( 0, makeSpot(  Rook, Black, False, False));
    set( 1, makeSpot(Knight, Black, False, False));
    set( 2, makeSpot(Bishop, Black, False, False));
    set( 3, makeSpot( Queen, Black, False, False));
    set( 4, makeSpot(  King, Black, False, False));
    set( 5, makeSpot(Bishop, Black, False, False));
    set( 6, makeSpot(Knight, Black, False, False));
    set( 7, makeSpot(  Rook, Black, False, False));
    set( 8, makeSpot(  Pawn, Black, False, False));
    set( 9, makeSpot(  Pawn, Black, False, False));
    set(10, makeSpot(  Pawn, Black, False, False));
    set(11, makeSpot(  Pawn, Black, False, False));
    set(12, makeSpot(  Pawn, Black, False, False));
    set(13, makeSpot(  Pawn, Black, False, False));
    set(14, makeSpot(  Pawn, Black, False, False));
    set(15, makeSpot(  Pawn, Black, False, False));

    set(48, makeSpot(  Pawn, White, False, False));
    set(49, makeSpot(  Pawn, White, False, False));
    set(50, makeSpot(  Pawn, White, False, False));
    set(51, makeSpot(  Pawn, White, False, False));
    set(52, makeSpot(  Pawn, White, False, False));
    set(53, makeSpot(  Pawn, White, False, False));
    set(54, makeSpot(  Pawn, White, False, False));
    set(55, makeSpot(  Pawn, White, False, False));
    set(56, makeSpot(  Rook, White, False, False));
    set(57, makeSpot(Knight, White, False, False));
    set(58, makeSpot(Bishop, White, False, False));
    set(59, makeSpot( Queen, White, False, False));
    set(60, makeSpot(  King, White, False, False));
    set(61, makeSpot(Bishop, White, False, False));
    set(62, makeSpot(Knight, White, False, False));
    set(63, makeSpot(  Rook, White, False, False));
}


Piece board_t::get(index_t const index) const 
{
    return board[index];
}


void board_t::set(index_t const index, Piece const piece)
{
    board[index] = piece;
}

