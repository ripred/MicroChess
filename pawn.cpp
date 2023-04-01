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

void add_pawn_moves(Piece p, index_t from, index_t fwd, Color side) {
    // see if we can move 1 spot in front of this pawn
    index_t const col = from % 8;
    index_t const row = from / 8;

    index_t to_col = col;
    index_t to_row = row + fwd;
    index_t to;
    Piece op = Empty;

    if (isValidPos(to_col, to_row)) {
        // get piece at location 1 spot in front of pawn
        index_t to = to_col + to_row * 8;
        Piece op = board.get(to);
        if (isEmpty(op)) {
            consider_move(side, from, to);

            // see if we can move 2 spots in front of this pawn
            if (!hasMoved(p)) {
                to_col = col;
                to_row = row + fwd + fwd;
                if (isValidPos(to_col, to_row)) {
                    // get piece at location 2 spots in front of pawn
                    to = to_col + to_row * 8;
                    op = board.get(to);
                    if (isEmpty(op)) {
                        consider_move(side, from, to);
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
                consider_move(side, from, to);
            }

            // check for en-passant
            if (isValidPos(to_col, to_row)) {
                op = board.get(to);
                if (!isEmpty(op) && getSide(op) != side) {
                    index_t last_move_from_row = game.last_move.from / 8;
                    index_t last_move_to_col = game.last_move.to % 8;
                    index_t last_move_to_row = game.last_move.to / 8;
                    if (last_move_to_col == to_col && last_move_to_row == row) {
                        if (abs(int(last_move_from_row) - int(last_move_to_row)) > 1) {
                            if (getType(op) == Pawn) {
                                consider_move(side, from, to);
                            }
                        }
                    }
                }
            }
        }
    }
}