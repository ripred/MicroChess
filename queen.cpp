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

// void add_queen_moves(index_t from, index_t fwd, Color side) {
void add_queen_moves(move_t &move) {
    Piece   const p = board.get(move.from);
    Color   const side = getSide(p);
    index_t const  fwd = (White == side) ? -1 : 1;

    index_t dirs[8][2] = { {0,1}, {0,-1}, {-1,0}, {1,0}, {-1,1}, {1,1}, {-1,-1}, {1,-1} };
    Bool continue_dir[8] = { True, True, True, True, True, True, True, True };
    index_t const col = move.from % 8;
    index_t const row = move.from / 8;

    for (auto const &dir : dirs) {
        index_t x = col + dir[0] * fwd;
        index_t y = row + dir[1] * fwd;

        index_t offset = 0;
        while (isValidPos(x, y) && continue_dir[offset]) {
            index_t to = x + y * 8;
            Piece const op = board.get(to);

            if (isEmpty(op)) {
                consider_move(side, move.from, to);
            }
            else if (side != getSide(op)) {
                continue_dir[offset] = False;
                consider_move(side, move.from, to);
                break;
            }
            else {
                continue_dir[offset] = False;
                break;
            }

            x += dir[0] * fwd;
            y += dir[1] * fwd;
        }
        offset++;
    }
}
