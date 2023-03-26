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
void add_bishop_moves(index_t from, index_t fwd, Color side) {
    static offset_t const bishop_offsets[NUM_BISHOP_OFFSETS] PROGMEM = {
        { -1, +1 }, { -2, +2 }, { -3, +3 }, { -4, +4 }, { -5, +5 }, { -6, +6 }, { -7, +7 },     // NW
        { +1, +1 }, { +2, +2 }, { +3, +3 }, { +4, +4 }, { +5, +5 }, { +6, +6 }, { +7, +7 },     // NE
        { -1, -1 }, { -2, -2 }, { -3, -3 }, { -4, -4 }, { -5, -5 }, { -6, -6 }, { -7, -7 },     // SW
        { +1, -1 }, { +2, -2 }, { +3, -3 }, { +4, -4 }, { +5, -5 }, { +6, -6 }, { +7, -7 }      // SE
    };

    Bool continue_nw = 1;
    Bool continue_ne = 1;
    Bool continue_sw = 1;
    Bool continue_se = 1;

    index_t const col = from % 8;
    index_t const row = from / 8;

    for (unsigned i=0; i < NUM_BISHOP_OFFSETS; i++) {
        offset_t *ptr = pgm_get_far_address(bishop_offsets);
        index_t xoff = pgm_read_byte(&ptr[i].x) * fwd;
        index_t yoff = pgm_read_byte(&ptr[i].y) * fwd;
        index_t to_col = col + xoff;
        index_t to_row = row + yoff;
        if (isValidPos(to_col, to_row)) {
            if (xoff < 0 && yoff > 0 && continue_nw) { // going NW
                index_t to = to_col + to_row * 8;
                Piece const op = board.get(to);
                if (isEmpty(op)) {
                    add_move(side, from, to, 0);
                }
                else {
                    // there is a piece there
                    continue_nw = False;

                    // if it is the other side then capture it
                    if (side != getSide(op)) {
                        add_move(side, from, to, 0);
                    }
                }
            }
            else
            if (xoff > 0 && yoff > 0 && continue_ne) { // going NE
                index_t to = to_col + to_row * 8;
                Piece const op = board.get(to);
                if (isEmpty(op)) {
                    add_move(side, from, to, 0);
                }
                else {
                    // there is a piece there
                    continue_ne = False;

                    // if it is the other side then capture it
                    if (side != getSide(op)) {
                        add_move(side, from, to, 0);
                    }
                }
            }
            else
            if (xoff < 0 && yoff < 0 && continue_sw) { // going SW
                index_t to = to_col + to_row * 8;
                Piece const op = board.get(to);
                if (isEmpty(op)) {
                    add_move(side, from, to, 0);
                }
                else {
                    // there is a piece there
                    continue_sw = False;

                    // if it is the other side then capture it
                    if (side != getSide(op)) {
                        add_move(side, from, to, 0);
                    }
                }
            }
            else
            if (xoff > 0 && yoff < 0 && continue_se) { // going SE
                index_t to = to_col + to_row * 8;
                Piece const op = board.get(to);
                if (isEmpty(op)) {
                    add_move(side, from, to, 0);
                }
                else {
                    // there is a piece there
                    continue_se = False;

                    // if it is the other side then capture it
                    if (side != getSide(op)) {
                        add_move(side, from, to, 0);
                    }
                }
            }
        }
    }
}