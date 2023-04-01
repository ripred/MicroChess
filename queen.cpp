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

void add_queen_moves(index_t from, index_t fwd, Color side) {
    index_t dirs[8][2] = { {0,1}, {0,-1}, {-1,0}, {1,0}, {-1,1}, {1,1}, {-1,-1}, {1,-1} };
    Bool continue_dir[8] = { True, True, True, True, True, True, True, True };
    int8_t const col = from % 8;
    int8_t const row = from / 8;

    for (auto& dir : dirs) {
        int8_t x = col + dir[0] * fwd;
        int8_t y = row + dir[1] * fwd;

        index_t offset = 0;
        while (isValidPos(x, y) && continue_dir[offset]) {
            index_t to = x + y * 8;
            Piece const op = board.get(to);

            if (isEmpty(op)) {
                consider_move(side, from, to);
            }
            else if (side != getSide(op)) {
                continue_dir[offset] = False;
                consider_move(side, from, to);
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
