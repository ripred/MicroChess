/**
 * rook.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * move generation for rooks
 * 
 */

#include <Arduino.h>
#include "MicroChess.h"

void add_rook_moves(index_t from, index_t fwd, Color side) {
    bool continue_dir[4] = {true, true, true, true};

    index_t const col = from % 8;
    index_t const row = from / 8;

    for (unsigned i=0; i < NUM_ROOK_OFFSETS; i++) {
        offset_t *ptr = pgm_get_far_address(rook_offsets);
        index_t xoff = pgm_read_byte(&ptr[i].x) * fwd;
        index_t yoff = pgm_read_byte(&ptr[i].y) * fwd;
        index_t to_col = col + xoff;
        index_t to_row = row + yoff;
        if (isValidPos(to_col, to_row)) {
            index_t to = to_col + to_row * 8;
            Piece const op = board.get(to);

            if (xoff == 0) {
                int index = yoff > 0 ? 0 : 1; // N or S
                if (continue_dir[index]) {
                    if (isEmpty(op)) {
                        consider_move(side, from, to);
                    } else {
                        continue_dir[index] = false;
                        if (side != getSide(op)) {
                            consider_move(side, from, to);
                        }
                    }
                }
            } else if (yoff == 0) {
                int index = xoff > 0 ? 2 : 3; // E or W
                if (continue_dir[index]) {
                    if (isEmpty(op)) {
                        consider_move(side, from, to);
                    } else {
                        continue_dir[index] = false;
                        if (side != getSide(op)) {
                            consider_move(side, from, to);
                        }
                    }
                }
            }
        }
    }
}