#include <avr/pgmspace.h>
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

////////////////////////////////////////////////////////////////////////////////////////
// the offsets a knight can move to
////////////////////////////////////////////////////////////////////////////////////////
// add the moves for a knight to the proper list (game.moves1 or game.moves2)
void add_knight_moves(index_t from, index_t fwd, Color side) {
    static offset_t const knight_offsets[NUM_KNIGHT_OFFSETS] PROGMEM = {
        { -2, +1 }, { -2, -1 }, { +2, +1 }, { +2, -1 }, 
        { +1, +2 }, { -1, +2 }, { +1, -2 }, { -1, -2 }  
    };

    index_t const col = from % 8;
    index_t const row = from / 8;

    for (unsigned i=0; i < NUM_KNIGHT_OFFSETS; i++) {
        offset_t *ptr = (offset_t *) pgm_get_far_address(knight_offsets);
        index_t to_col = col + pgm_read_byte(&ptr[i].x) * fwd;
        index_t to_row = row + pgm_read_byte(&ptr[i].y) * fwd;
        // printf(Debug1, "knight offset %d = %d,%d\n", i, to_col, to_row);
        if (isValidPos(to_col, to_row)) {
            index_t to = to_col + (to_row * 8);
            Piece const op = board.get(to);
            if (Empty == getType(op) || getSide(op) != side) {
                add_move(side, from, to, 0);
            }
        }
    }
}