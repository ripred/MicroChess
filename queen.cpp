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

/*
 * evaluate the moves for a queen against the best move so far
 *
 */
void add_queen_moves(piece_gen_t &gen) {
    move_t &move = gen.move;
    move_t &best = gen.best;
    generator_t *callback = gen.callme;

    index_t dirs[8][2] = { {0,1}, {0,-1}, {-1,0}, {1,0}, {-1,1}, {1,1}, {-1,-1}, {1,-1} };
    Bool continue_dir[8] = { True, True, True, True, True, True, True, True };
    Piece   const p = board.get(move.from);
    Color   const side = getSide(p);

    index_t const col = move.from % 8;
    index_t const row = move.from / 8;

    for (auto const &dir : dirs) {
        index_t x = col + dir[0];
        index_t y = row + dir[1];

        index_t offset = 0;
        while (isValidPos(x, y) && continue_dir[offset]) {
            index_t const to = x + y * 8;
            Piece   const op = board.get(to);

            if (isEmpty(op)) {
                move.to = to;
                callback(move, best);
            }
            else if (side != getSide(op)) {
                continue_dir[offset] = False;
                move.to = to;
                callback(move, best);
                break;
            }
            else {
                continue_dir[offset] = False;
                break;
            }

            x += dir[0];
            y += dir[1];
        }
        offset++;
    }
}