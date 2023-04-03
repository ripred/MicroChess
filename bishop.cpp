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

// void add_bishop_moves(index_t from, index_t fwd, Color side) {
void add_bishop_moves(move_t &move) {
    Piece   const p = board.get(move.from);
    Color   const side = getSide(p);
    index_t const  fwd = (White == side) ? -1 : 1;

    bool continue_dir[4] = {true, true, true, true};

    index_t const col = move.from % 8;
    index_t const row = move.from / 8;

    for (unsigned i=0; i < NUM_BISHOP_OFFSETS; i++) {
        offset_t *ptr = pgm_get_far_address(bishop_offsets);
        index_t xoff = pgm_read_byte(&ptr[i].x) * fwd;
        index_t yoff = pgm_read_byte(&ptr[i].y) * fwd;
        index_t to_col = col + xoff;
        index_t to_row = row + yoff;
        if (isValidPos(to_col, to_row)) {
            index_t to = to_col + to_row * 8;
            Piece const op = board.get(to);

            if (xoff == 0) {
                int index = yoff > 0 ? 0 : 1; // NW or SW
                if (continue_dir[index]) {
                    if (isEmpty(op)) {
                        consider_move(side, move.from, to);
                    } else {
                        continue_dir[index] = false;
                        if (side != getSide(op)) {
                            consider_move(side, move.from, to);
                        }
                    }
                }
            } else if (yoff == 0) {
                int index = xoff > 0 ? 2 : 3; // NE or SE
                if (continue_dir[index]) {
                    if (isEmpty(op)) {
                        consider_move(side, move.from, to);
                    } else {
                        continue_dir[index] = false;
                        if (side != getSide(op)) {
                            consider_move(side, move.from, to);
                        }
                    }
                }
            }
        }
    }
}