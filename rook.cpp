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

/*
 * add the moves for a rook to the proper list (game.moves1 or game.moves2)
 *
 */
void add_rook_moves(index_t from, index_t fwd, Color side) {
    Bool continue_n = True;
    Bool continue_s = True;
    Bool continue_e = True;
    Bool continue_w = True;

    index_t const col = from % 8;
    index_t const row = from / 8;

    for (unsigned i=0; i < NUM_ROOK_OFFSETS; i++) {
        offset_t *ptr = pgm_get_far_address(rook_offsets);
        index_t xoff = pgm_read_byte(&ptr[i].x) * fwd;
        index_t yoff = pgm_read_byte(&ptr[i].y) * fwd;
        index_t to_col = col + xoff;
        index_t to_row = row + yoff;
        if (isValidPos(to_col, to_row)) {
            if (xoff == 0 && yoff > 0 && continue_n) { // going N
                index_t to = to_col + to_row * 8;
                Piece const op = board.get(to);
                if (isEmpty(op)) {
                    // printf(Debug1, "rook offset %d = %d,%d\n", i, to_col, to_row);
                    add_move(side, from, to, 0);
                }
                else {
                    // there is a piece there
                    continue_n = False;

                    // if it is the other side then capture it
                    if (side != getSide(op)) {
                        add_move(side, from, to, 0);
                    }
                }
            }
            else
            if (xoff == 0 && yoff < 0 && continue_s) { // going S
                index_t to = to_col + to_row * 8;
                Piece const op = board.get(to);
                if (isEmpty(op)) {
                    // printf(Debug1, "rook offset %d = %d,%d\n", i, to_col, to_row);
                    add_move(side, from, to, 0);
                }
                else {
                    // there is a piece there
                    continue_s = False;

                    // if it is the other side then capture it
                    if (side != getSide(op)) {
                        add_move(side, from, to, 0);
                    }
                }
            }
            else
            if (xoff < 0 && yoff == 0 && continue_w) { // going W
                index_t to = to_col + to_row * 8;
                Piece const op = board.get(to);
                if (isEmpty(op)) {
                    // printf(Debug1, "rook offset %d = %d,%d\n", i, to_col, to_row);
                    add_move(side, from, to, 0);
                }
                else {
                    // there is a piece there
                    continue_w = False;

                    // if it is the other side then capture it
                    if (side != getSide(op)) {
                        add_move(side, from, to, 0);
                    }
                }
            }
            else
            if (xoff > 0 && yoff == 0 && continue_e) { // going E
                index_t to = to_col + to_row * 8;
                Piece const op = board.get(to);
                if (isEmpty(op)) {
                    // printf(Debug1, "rook offset %d = %d,%d\n", i, to_col, to_row);
                    add_move(side, from, to, 0);
                }
                else {
                    // there is a piece there
                    continue_e = False;

                    // if it is the other side then capture it
                    if (side != getSide(op)) {
                        add_move(side, from, to, 0);
                    }
                }
            }
        }
    }
}