/**
 * bishop.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * move generation for bishops
 * 
 */

#include <Arduino.h>
#include "MicroChess.h"

/*
 * evaluate the moves for a bishop against the best move so far
 *
 */
void add_bishop_moves(piece_gen_t &gen) {
    move_t &move = gen.move;
    move_t &best = gen.best;
    generator_t *callback = gen.callme;

    bool continue_dir[4] = { true, true, true, true };
    Piece   const p = board.get(move.from);
    Color   const side = getSide(p);
    index_t const fwd = (White == side) ? -1 : 1;

    index_t const col = move.from % 8;
    index_t const row = move.from / 8;

    for (unsigned i=0; i < NUM_BISHOP_OFFSETS; i++) {
        offset_t const * const ptr = pgm_get_far_address(bishop_offsets);
        index_t  const xoff = pgm_read_byte(&ptr[i].x) * fwd;
        index_t  const yoff = pgm_read_byte(&ptr[i].y) * fwd;
        index_t  const to_col = col + xoff;
        index_t  const to_row = row + yoff;
        if (isValidPos(to_col, to_row)) {
            index_t const to = to_col + to_row * 8;
            Piece   const op = board.get(to);

            if (xoff == 0) {
                int const index = yoff > 0 ? 0 : 1; // NW or SW
                if (continue_dir[index]) {
                    if (isEmpty(op)) {
                        move.to = to;
                        callback(move, best);
                    } else {
                        continue_dir[index] = false;
                        if (side != getSide(op)) {
                            move.to = to;
                            callback(move, best);
                        }
                    }
                }
            } else if (yoff == 0) {
                int const index = xoff > 0 ? 2 : 3; // NE or SE
                if (continue_dir[index]) {
                    if (isEmpty(op)) {
                        move.to = to;
                        callback(move, best);
                    } else {
                        continue_dir[index] = false;
                        if (side != getSide(op)) {
                            move.to = to;
                            callback(move, best);
                        }
                    }
                }
            }
        }
    }
}