/**
 * king.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * move generation for kings
 * 
 */

#include <Arduino.h>
#include "MicroChess.h"

/*
 * evaluate the moves for a king against the best move so far
 *
 */
void add_king_moves(piece_gen_t &gen) {
    move_t &move = gen.move;
    move_t &best = gen.best;
    generator_t *callback = gen.callme;

    Piece   const king = board.get(move.from);
    Color   const side = getSide(king);

    index_t const col = move.from % 8;
    index_t const row = move.from / 8;

    for (unsigned i=0; i < NUM_KING_OFFSETS; i++) {
        offset_t const * const ptr = pgm_get_far_address(game.king_offsets);
        index_t  const xoff = pgm_read_byte(&ptr[i].x);
        index_t  const yoff = pgm_read_byte(&ptr[i].y);
        index_t  const to_col = col + xoff;
        index_t  const to_row = row + yoff;
        if (isValidPos(to_col, to_row)) {
            index_t const to = to_col + to_row * 8;
            Piece   const op = board.get(to);
            if (isEmpty(op) || getSide(op) != side) {
                move.to = to;
                callback(move, best);
            }
        }
    }

    // check for castling
    if (!hasMoved(king)) {
        // check King's side
        index_t rook_loc = 0 + row * 8;
        Piece rook = board.get(rook_loc);
        Bool empty_knight = isEmpty(board.get(1 + row * 8));
        Bool empty_bishop = isEmpty(board.get(2 + row * 8));
        if (!isEmpty(rook) && !hasMoved(rook)) {
            if (empty_knight && empty_bishop) {
                // We can castle on the King's side
                move.to = 1 + row * 8;
                callback(move, best);
            }
        }

        // check Queen's side
        rook_loc = 7 + row * 8;
        rook = board.get(rook_loc);
        if (!isEmpty(rook) && !hasMoved(rook)) {
            empty_knight = isEmpty(board.get(6 + row * 8));
            empty_bishop = isEmpty(board.get(5 + row * 8));
            Bool empty_queen = isEmpty(board.get(4 + row * 8));
            if (empty_knight && empty_bishop && empty_queen) {
                // We can castle on the Queens's side
                move.to = 6 + row * 8;
                callback(move, best);
            }
        }
    }
}