/**
 * pawn.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * move generation for pawns
 * 
 */
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "MicroChess.h"

/*
 * evaluate the moves for a pawn against the best move so far
 *
 */
void add_pawn_moves(piece_gen_t &gen) {
    if (freeMemory() < game.options.low_mem_limit) {
        digitalWrite(DEBUG1_PIN, HIGH);
        delayMicroseconds(200);
        digitalWrite(DEBUG1_PIN, LOW);
        return;
    }

    // see if we can move 1 spot in front of this pawn
    index_t to_col = (gen.move.from % 8);
    index_t to_row = (gen.move.from / 8) + ((White == getSide(board.get(gen.move.from))) ? -1 : 1);
    gen.move.to = to_col + to_row * 8;
    Piece op = Empty;

    if (isValidPos(to_col, to_row)) {
        // get piece at location 1 spot in front of pawn
        Piece op = board.get(gen.move.to);
        if (isEmpty(op)) {
            gen.callme(gen.move, gen.best);

            // see if we can move 2 spots in front of this pawn
            if (!hasMoved(board.get(gen.move.from))) {
                to_row += ((White == getSide(board.get(gen.move.from))) ? -1 : 1);
                if (isValidPos(to_col, to_row)) {
                    // get piece at location 2 spots in front of pawn
                    gen.move.to = to_col + to_row * 8;
                    op = board.get(gen.move.to);
                    if (isEmpty(op)) {
                        gen.callme(gen.move, gen.best);
                    }
                }
            }
        }
    }

    // see if we can capture a piece diagonally
    for (index_t i = -1; i <= 1; i += 2) {
        to_col = (gen.move.from % 8) + i;
        to_row = (gen.move.from / 8) + ((White == getSide(board.get(gen.move.from))) ? -1 : 1);
        gen.move.to = to_col + to_row * 8;
        if (isValidPos(to_col, to_row)) {
            // get piece diagonally
            op = board.get(gen.move.to);
            if (!isEmpty(op) && getSide(op) != getSide(board.get(gen.move.from))) {
                gen.callme(gen.move, gen.best);
            }

            // BUGBUG: TODO: Fix this and the companion part in make_move() to recognize and implement it
            // 
            // check for en-passant
            // index_t const last_move_from_row = game.last_move.from / 8;
            // index_t const last_move_to_col = game.last_move.to % 8;
            // index_t const last_move_to_row = game.last_move.to / 8;

            // if (last_move_to_col == to_col && last_move_to_row == row) {
            //     if (abs(int(last_move_from_row) - int(last_move_to_row)) > 1) {
            //         op = board.get(last_move_to_col + row * 8);
            //         if (getType(op) == Pawn && getSide(op) != getSide(board.get(gen.move.from))) {
            //             move.to = to_col + (row + ((White == getSide(board.get(gen.move.from))) ? -1 : 1)) * 8;
            //             gen.callme(move, gen.best);
            //         }
            //     }
            // }
        }
    }
}


/*
 * evaluate the moves for a knight against the best move so far
 *
 */
void add_knight_moves(piece_gen_t &gen) {
    static offset_t constexpr offsets[] PROGMEM = {
        { -2, +1 }, { -2, -1 }, { +2, +1 }, { +2, -1 }, 
        { +1, +2 }, { -1, +2 }, { +1, -2 }, { -1, -2 }  
    };

    if (freeMemory() < game.options.low_mem_limit) {
        digitalWrite(DEBUG1_PIN, HIGH);
        delayMicroseconds(200);
        digitalWrite(DEBUG1_PIN, LOW);
        return;
    }

    offset_t const * const ptr = (offset_t const *) pgm_get_far_address(offsets);
    for (index_t i = 0; i < index_t(ARRAYSZ(offsets)); i++) {
        index_t  const to_col = (gen.move.from % 8) + pgm_read_byte(&ptr[i].x);
        index_t  const to_row = (gen.move.from / 8) + pgm_read_byte(&ptr[i].y);
        if (isValidPos(to_col, to_row)) {
            gen.move.to = to_col + to_row * 8;
            Piece   const op = board.get(gen.move.to);
            if (isEmpty(op) || getSide(op) != getSide(board.get(gen.move.from))) {
                gen.callme(gen.move, gen.best);
            }
        }
    }
}


/*
 * evaluate the moves for a rook against the best move so far
 *
 */
void add_rook_moves(piece_gen_t &gen) {
    static offset_t constexpr dirs[] PROGMEM = {
        {  0,  1 }, {  0, -1 }, { -1,  0 }, {  1,  0 }
    };

    if (freeMemory() < game.options.low_mem_limit) {
        digitalWrite(DEBUG1_PIN, HIGH);
        delayMicroseconds(200);
        digitalWrite(DEBUG1_PIN, LOW);
        return;
    }

    offset_t const * ptr = (offset_t const *) pgm_get_far_address(dirs);
    for (index_t i = 0; i < index_t(ARRAYSZ(dirs)); i++) {
        index_t x = (gen.move.from % 8) + pgm_read_byte(&ptr[i].x);
        index_t y = (gen.move.from / 8) + pgm_read_byte(&ptr[i].y);

        while (isValidPos(x, y)) {
            gen.move.to = x + y * 8;
            Piece const op = board.get(gen.move.to);

            if (isEmpty(op)) {
                gen.callme(gen.move, gen.best);
            }
            else if (getSide(board.get(gen.move.from)) != getSide(op)) {
                gen.callme(gen.move, gen.best);
                break;
            }
            else {
                break;
            }

            x += pgm_read_byte(&ptr[i].x);
            y += pgm_read_byte(&ptr[i].y);
        }
    }
}


/*
 * evaluate the moves for a bishop against the best move so far
 *
 */
void add_bishop_moves(piece_gen_t &gen) {
    //                                     NW      SW      NE      SE
    static index_t const dirs[4][2] PROGMEM = {
        { -1, -1 }, { -1,  1 }, {  1, -1 }, {  1,  1 }
    };

    if (freeMemory() < game.options.low_mem_limit) {
        digitalWrite(DEBUG1_PIN, HIGH);
        delayMicroseconds(200);
        digitalWrite(DEBUG1_PIN, LOW);
        return;
    }

    offset_t const * ptr = (offset_t const *) pgm_get_far_address(dirs);
    for (index_t i = 0; i < index_t(ARRAYSZ(dirs)); i++) {
        index_t x = (gen.move.from % 8) + pgm_read_byte(&ptr[i].x);
        index_t y = (gen.move.from / 8) + pgm_read_byte(&ptr[i].y);

        while (isValidPos(x, y)) {
            gen.move.to = x + y * 8;
            Piece const op = board.get(gen.move.to);

            if (isEmpty(op)) {
                gen.callme(gen.move, gen.best);
            }
            else if (getSide(board.get(gen.move.from)) != getSide(op)) {
                gen.callme(gen.move, gen.best);
                break;
            }
            else {
                break;
            }

            x += pgm_read_byte(&ptr[i].x);
            y += pgm_read_byte(&ptr[i].y);
        }
    }
}


/*
 * evaluate the moves for a queen against the best move so far
 *
 */
void add_queen_moves(piece_gen_t &gen) {
    add_rook_moves(gen);
    add_bishop_moves(gen);
}


/*
 * evaluate the moves for a king against the best move so far
 *
 */
void add_king_moves(piece_gen_t &gen) {
    static offset_t constexpr king_offsets[] PROGMEM = {
        { -1,  0 }, {  0, -1 }, { -1, -1 }, { +1, -1 }, 
        { +1,  0 }, {  0, +1 }, { -1, +1 }, { +1, +1 }
    };

    if (freeMemory() < game.options.low_mem_limit) {
        digitalWrite(DEBUG1_PIN, HIGH);
        delayMicroseconds(200);
        digitalWrite(DEBUG1_PIN, LOW);
        return;
    }

    offset_t const * const ptr = (offset_t *)pgm_get_far_address(king_offsets);
    for (index_t i = 0; i < index_t(ARRAYSZ(king_offsets)); i++) {
        index_t  const to_col = (gen.move.from % 8) + pgm_read_byte(&ptr[i].x);
        index_t  const to_row = (gen.move.from / 8) + pgm_read_byte(&ptr[i].y);
        if (isValidPos(to_col, to_row)) {
            gen.move.to = to_col + to_row * 8;
            Piece const op = board.get(gen.move.to);
            if (isEmpty(op) || getSide(op) != getSide(board.get(gen.move.from))) {
                gen.callme(gen.move, gen.best);
            }
        }
    }

    // check for castling
    if (!hasMoved(board.get(gen.move.from))) {
        // check King's side (right-hand side from white's view)
        index_t rook_loc = 0 + (gen.move.from / 8) * 8;
        Piece rook = board.get(rook_loc);
        Bool empty_knight = isEmpty(board.get(1 + (gen.move.from / 8) * 8));
        Bool empty_bishop = isEmpty(board.get(2 + (gen.move.from / 8) * 8));
        if (!isEmpty(rook) && !hasMoved(rook)) {
            if (empty_knight && empty_bishop) {
                // We can castle on the King's side
                gen.move.to = 1 + (gen.move.from / 8) * 8;
                gen.callme(gen.move, gen.move);
            }
        }

        // check Queen's side (left-hand side from white's view)
        rook_loc = 7 + (gen.move.from / 8) * 8;
        rook = board.get(rook_loc);
        if (!isEmpty(rook) && !hasMoved(rook)) {
            Bool empty_queen = isEmpty(board.get(4 + (gen.move.from / 8) * 8));
                empty_knight = isEmpty(board.get(6 + (gen.move.from / 8) * 8));
                empty_bishop = isEmpty(board.get(5 + (gen.move.from / 8) * 8));
            if (empty_knight && empty_bishop && empty_queen) {
                // We can castle on the Queens's side
                gen.move.to = 6 + (gen.move.from / 8) * 8;
                gen.callme(gen.move, gen.move);
            }
        }
    }

} // add_king_moves(piece_gen_t &gen)