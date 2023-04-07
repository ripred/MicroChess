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

    Piece   const p = board.get(move.from);
    Color   const side = getSide(p);
    index_t const fwd = (White == side) ? -1 : 1;

    index_t const col = move.from % 8;
    index_t const row = move.from / 8;

    for (unsigned i=0; i < NUM_KING_OFFSETS; i++) {
        offset_t const * const ptr = pgm_get_far_address(king_offsets);
        index_t  const xoff = pgm_read_byte(&ptr[i].x) * fwd;
        index_t  const yoff = pgm_read_byte(&ptr[i].y) * fwd;
        index_t  const to_col = col + xoff * fwd;
        index_t  const to_row = row + yoff * fwd;
        if (isValidPos(to_col, to_row)) {
            index_t const to = to_col + to_row * 8;
            Piece   const op = board.get(to);
            if (isEmpty(op) || getSide(op) != side) {
                move.to = to;
                callback(move, best);
            }
        }
    }
}