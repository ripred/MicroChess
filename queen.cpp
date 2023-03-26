/**
 * queen.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * move generation for queens
 * 
 */

#include <Arduino.h>
#include "MicroChess.h"

////////////////////////////////////////////////////////////////////////////////////////
// add the moves for a queen to the proper list (game.moves1 or game.moves2)
void add_queen_moves(index_t from, index_t fwd, Color side) {
    static offset_t const queen_offsets[NUM_QUEEN_OFFSETS] PROGMEM = {
        { -1,  0 }, { -2,  0 }, { -3,  0 }, { -4,  0 }, { -5,  0 }, { -6,  0 }, { -7,  0 },     // W
        { +1,  0 }, { +2,  0 }, { +3,  0 }, { +4,  0 }, { +5,  0 }, { +6,  0 }, { +7,  0 },     // E
        {  0, +1 }, {  0, +2 }, {  0, +3 }, {  0, +4 }, {  0, +5 }, {  0, +6 }, {  0, +7 },     // N
        {  0, -1 }, {  0, -2 }, {  0, -3 }, {  0, -4 }, {  0, -5 }, {  0, -6 }, {  0, -7 },     // S
        { -1, +1 }, { -2, +2 }, { -3, +3 }, { -4, +4 }, { -5, +5 }, { -6, +6 }, { -7, +7 },     // NW
        { +1, +1 }, { +2, +2 }, { +3, +3 }, { +4, +4 }, { +5, +5 }, { +6, +6 }, { +7, +7 },     // NE
        { -1, -1 }, { -2, -2 }, { -3, -3 }, { -4, -4 }, { -5, -5 }, { -6, -6 }, { -7, -7 },     // SW
        { +1, -1 }, { +2, -2 }, { +3, -3 }, { +4, -4 }, { +5, -5 }, { +6, -6 }, { +7, -7 }      // SE
    };

    Bool continue_n  = True;
    Bool continue_s  = True;
    Bool continue_e  = True;
    Bool continue_w  = True;
    Bool continue_nw = True;
    Bool continue_ne = True;
    Bool continue_sw = True;
    Bool continue_se = True;

    index_t const col = from % 8;
    index_t const row = from / 8;

    for (unsigned i=0; i < NUM_QUEEN_OFFSETS; i++) {
        offset_t *ptr = pgm_get_far_address(queen_offsets);
        index_t xoff = pgm_read_byte(&ptr[i].x) * fwd;
        index_t yoff = pgm_read_byte(&ptr[i].y) * fwd;
        index_t to_col = col + xoff;
        index_t to_row = row + yoff;
        if (isValidPos(to_col, to_row)) {
            if (xoff == 0 && yoff > 0 && continue_n) { // going N
                index_t to = to_col + to_row * 8;
                Piece const op = board.get(to);
                if (isEmpty(op)) {
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
            else
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