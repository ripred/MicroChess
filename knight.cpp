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
 * add the moves for a knight to the proper list (game.moves1 or game.moves2)
 *
 */
void add_knight_moves(index_t from, index_t fwd, Color side) {
    index_t const col = from % 8;
    index_t const row = from / 8;

    for (index_t i = 0; i < NUM_KNIGHT_OFFSETS; i++) {
        offset_t *ptr = (offset_t *)pgm_get_far_address(knight_offsets);
        index_t to_col = col + pgm_read_byte(&ptr[i].x) * fwd;
        index_t to_row = row + pgm_read_byte(&ptr[i].y) * fwd;
        if (isValidPos(to_col, to_row)) {
            index_t to = to_col + (to_row * 8);
            Piece const op = board.get(to);
            if (isEmpty(op) || getSide(op) != side) {
                consider_move(side, from, to);
            }
        }
    }
}