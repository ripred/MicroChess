/**
 * knight.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * move generation for knights
 * 
 */

#include <Arduino.h>
#include "MicroChess.h"

/*
 * evaluate the moves for a knight against the best move so far
 *
 */
void add_knight_moves(piece_gen_t &gen) {
    move_t &move = gen.move;
    move_t &best = gen.best;
    generator_t *callback = gen.callme;

    Piece   const p = board.get(move.from);
    Color   const side = getSide(p);

    index_t const col = move.from % 8;
    index_t const row = move.from / 8;

    for (index_t i = 0; i < NUM_KNIGHT_OFFSETS; i++) {
        offset_t const * const ptr = (offset_t *)pgm_get_far_address(knight_offsets);
        index_t  const to_col = col + pgm_read_byte(&ptr[i].x);
        index_t  const to_row = row + pgm_read_byte(&ptr[i].y);
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