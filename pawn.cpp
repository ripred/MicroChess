/**
 * pawn.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * move generation for pawns
 * 
 */

#include <Arduino.h>
#include "MicroChess.h"

/*
 * evaluate the moves for a pawn against the best move so far
 *
 */
void add_pawn_moves(piece_gen_t &gen) {
    move_t &move = gen.move;
    move_t &best = gen.best;
    generator_t *callback = gen.callme;

    Piece   const p = board.get(move.from);
    Color   const side = getSide(p);
    index_t const fwd = (White == side) ? -1 : 1;

    // see if we can move 1 spot in front of this pawn
    index_t const col = move.from % 8;
    index_t const row = move.from / 8;

    index_t to_col = col;
    index_t to_row = row + fwd;
    index_t to = to_col + to_row * 8;;
    Piece op = Empty;

    if (isValidPos(to_col, to_row)) {
        // get piece at location 1 spot in front of pawn
        Piece op = board.get(to);
        if (isEmpty(op)) {
            move.to = to;
            callback(move, best);

            // see if we can move 2 spots in front of this pawn
            if (!hasMoved(p)) {
                to_col = col;
                to_row = row + fwd + fwd;
                if (isValidPos(to_col, to_row)) {
                    // get piece at location 2 spots in front of pawn
                    to = to_col + to_row * 8;
                    op = board.get(to);
                    if (isEmpty(op)) {
                        move.to = to;
                        callback(move, best);
                    }
                }
            }
        }
    }

    // see if we can capture a piece diagonally
    for (int i = -1; i <= 1; i += 2) {
        to_col = col + i;
        to_row = row + fwd;
        to = to_col + to_row * 8;
        if (isValidPos(to_col, to_row)) {
            // get piece diagonally
            op = board.get(to);
            if (!isEmpty(op) && getSide(op) != side) {
                move.to = to;
                callback(move, best);
            }

            // check for en-passant
            index_t const last_move_from_row = game.last_move.from / 8;
            index_t const last_move_to_col = game.last_move.to % 8;
            index_t const last_move_to_row = game.last_move.to / 8;
            if (last_move_to_col == to_col && last_move_to_row == row) {
                if (abs(int(last_move_from_row) - int(last_move_to_row)) > 1) {
                    op = board.get(last_move_to_col + last_move_to_row * 8);
                    if (getType(op) == Pawn && getSide(op) != side && side == game.turn) {
                        move.to = to;
                        callback(move, best);
                    }
                }
            }
        }
    }
}