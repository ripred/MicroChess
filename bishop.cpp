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

////////////////////////////////////////////////////////////////////////////////////////
// add the moves for a bishop to the proper list (game.moves1 or game.moves2)
void add_bishop_moves(index_t from, index_t col, index_t row, index_t fwd, Color side) {
    Bool continue_nw = 1;
    Bool continue_ne = 1;
    Bool continue_sw = 1;
    Bool continue_se = 1;

    for (unsigned i=0; i < NUM_BISHOP_OFFSETS; i++) {
        offset_t *ptr = pgm_get_far_address(bishop_offsets);
        index_t xoff = ptr[i].x * fwd;
        index_t yoff = ptr[i].y * fwd;
        index_t to_col = col + xoff;
        index_t to_row = row + yoff;
        if (isValidPos(to_col, to_row)) {
            if (xoff < 0 && yoff > 0 && continue_nw) { // going NW
                index_t to = to_col + (to_row * 8);
                Piece const op = board.get(to);
                if (Empty == getType(op)) {
                    add_move(side, from, to, 0);
                }
                else {
                    // there is a piece there
                    continue_nw = 0;

                    // if it is the other side then capture it
                    if (side != getType(op)) {
                        add_move(side, from, to, 0);
                    }
                }
            }
            else
            if (xoff > 0 && yoff > 0 && continue_ne) { // going NE
                index_t to = to_col + (to_row * 8);
                Piece const op = board.get(to);
                if (Empty == getType(op)) {
                    add_move(side, from, to, 0);
                }
                else {
                    // there is a piece there
                    continue_ne = 0;

                    // if it is the other side then capture it
                    if (side != getType(op)) {
                        add_move(side, from, to, 0);
                    }
                }
            }
            else
            if (xoff < 0 && yoff < 0 && continue_sw) { // going SW
                index_t to = to_col + (to_row * 8);
                Piece const op = board.get(to);
                if (Empty == getType(op)) {
                    add_move(side, from, to, 0);
                }
                else {
                    // there is a piece there
                    continue_sw = 0;

                    // if it is the other side then capture it
                    if (side != getType(op)) {
                        add_move(side, from, to, 0);
                    }
                }
            }
            else
            if (xoff > 0 && yoff < 0 && continue_se) { // going SE
                index_t to = to_col + (to_row * 8);
                Piece const op = board.get(to);
                if (Empty == getType(op)) {
                    add_move(side, from, to, 0);
                }
                else {
                    // there is a piece there
                    continue_se = 0;

                    // if it is the other side then capture it
                    if (side != getType(op)) {
                        add_move(side, from, to, 0);
                    }
                }
            }
        }
    }
}