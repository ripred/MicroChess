/**
 * board.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * implementation file for the board class
 * 
 */
#include <Arduino.h>
#include <stdint.h>
#include "MicroChess.h"
#include "board.h"

board_t::board_t() {
    init();

} // board_t::board_t()


void board_t::clear() {
    for (index_t i = 0; i < index_t(BOARD_SIZE); i++) {
        board.set(i, Empty);
    }

} // board_t::clear()


void board_t::init() {
    clear();

    uint8_t pieces[8] = { Rook, Knight, Bishop, Queen, King, Bishop, Knight, Rook };
    for (int piece = 0; piece < 8; piece++) {
        board.set(piece, pieces[piece]);
        board.set(8 + piece, Pawn);
        board.set(48 + piece, Pawn);
        board.set(48 + piece, setSide(board.get(48 + piece), White));
        board.set(56 + piece, pieces[piece]);
        board.set(56 + piece, setSide(board.get(56 + piece), White));
    }

} // board_t::init()



Piece board_t::get(unsigned char index) const
{
    return board.get(index);
}

void board_t::set(unsigned char index, Piece const piece) 
{
    board.set(index, piece);
}    





// Piece board_t::get(index_t const index) const 
// {
//     return board[index];

// } // board_t::get(index_t const index)


// void board_t::set(index_t const index, Piece const piece)
// {
//     board[index] = piece;

// } // board_t::set(index_t const index, Piece const piece)

