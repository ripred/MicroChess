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
    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    if (freeMemory() < game.options.low_mem_limit) {
        show_low_memory();
        return;
    }

    // See if we can move 1 spot in front of this pawn
    index_t to_col = gen.col;
    index_t to_row = gen.row + (gen.whites_turn ? -1 : +1);
    gen.move.to = to_col + to_row * 8;

    auto check_fwd = [](piece_gen_t &gen, index_t const to_col, index_t const to_row) -> Bool {
        if (isValidPos(to_col, to_row)) {
            // get piece that is 1 or 2 forward of this pawn
            gen.move.to = to_col + to_row * 8;
            Piece op = board.get(gen.move.to);
            if (isEmpty(op)) {
                if (gen.callme(gen)) {
                    // Make forward moves more valuable when we get towards
                    // the end game and there aren't many pieces
                    if (game.piece_count < 18) {
                        gen.best.value += gen.whites_turn ? +100000 : -100000;
                    }
                    return True;
                }
            }
        }

        return False;
    };

    // Check for forward moves
    check_fwd(gen, to_col,to_row);
    if (!hasMoved(board.get(gen.move.from))) {
        to_row += (gen.whites_turn ? -1 : +1);
        check_fwd(gen, to_col,to_row);
    }

    // See if we can capture a piece diagonally
    for (index_t i = -1; i <= 1; i += 2) {
        // See if the turn has timed out
        if ((game.last_was_timeout = timeout())) {
            show_timeout();
            return;
        }

        to_col = gen.col + i;
        to_row = gen.row + (gen.whites_turn ? -1 : 1);
        gen.move.to = to_col + to_row * 8;
        if (isValidPos(to_col, to_row)) {
            // Get piece diagonally
            Piece op = board.get(gen.move.to);
            if (!isEmpty(op) && getSide(op) != gen.side) {
                gen.callme(gen);
            }

            // Check for en-passant
            if ((false)) {
                index_t const last_move_from_row = game.last_move.from / 8;
                index_t const last_move_to_col = game.last_move.to % 8;
                index_t const last_move_to_row = game.last_move.to / 8;

                if (last_move_to_col == to_col && last_move_to_row == gen.row) {
                    if (abs(int(last_move_from_row) - int(last_move_to_row)) > 1) {
                        op = board.get(last_move_to_col + gen.row * 8);
                        if (getType(op) == Pawn && getSide(op) != gen.side) {
                            gen.move.to = to_col + (gen.row + (gen.whites_turn ? -1 : 1)) * 8;
                            gen.callme(gen);
                        }
                    }
                }
            }
        }
    }

} // add_pawn_moves(piece_gen_t &gen)


/*
 * evaluate the moves for a knight against the best move so far
 *
 */
void add_knight_moves(piece_gen_t &gen) {
    static offset_t constexpr offsets[8] PROGMEM = {
        { -2, +1 }, { -2, -1 }, { +2, +1 }, { +2, -1 }, 
        { +1, +2 }, { -1, +2 }, { +1, -2 }, { -1, -2 }  
    };

    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    if (freeMemory() < game.options.low_mem_limit) {
        show_low_memory();
        return;
    }

    offset_t const * const ptr = (offset_t const *) pgm_get_far_address(offsets);
    for (index_t i = 0; i < index_t(ARRAYSZ(offsets)); i++) {
        // See if the turn has timed out
        if ((game.last_was_timeout = timeout())) {
            show_timeout();
            return;
        }

        index_t const to_col = gen.col + pgm_read_byte(&ptr[i].x);
        index_t const to_row = gen.row + pgm_read_byte(&ptr[i].y);
        if (isValidPos(to_col, to_row)) {
            gen.move.to = to_col + to_row * 8;
            Piece const op = board.get(gen.move.to);
            if (isEmpty(op) || getSide(op) != gen.side) {
                gen.callme(gen);
            }
        }
    }

} // add_knight_moves(piece_gen_t &gen)


/*
 * evaluate the moves for a rook against the best move so far
 *
 */
void add_rook_moves(piece_gen_t &gen) {
    static offset_t constexpr dirs[4] PROGMEM = {
        {  0,  1 }, {  0, -1 }, { -1,  0 }, {  1,  0 }
    };

    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    if (freeMemory() < game.options.low_mem_limit) {
        show_low_memory();
        return;
    }

    offset_t const * const ptr = (offset_t const * const) pgm_get_far_address(dirs);
    for (index_t i = 0; i < index_t(ARRAYSZ(dirs)); i++) {
        index_t x = gen.col + pgm_read_byte(&ptr[i].x);
        index_t y = gen.row + pgm_read_byte(&ptr[i].y);

        while (isValidPos(x, y)) {
            // See if the turn has timed out
            if ((game.last_was_timeout = timeout())) {
                show_timeout();
                return;
            }

            gen.move.to = x + y * 8;
            Piece const op = board.get(gen.move.to);

            if (isEmpty(op)) {
                gen.callme(gen);
            }
            else if (getSide(op) != gen.side) {
                gen.callme(gen);
                break;
            }
            else {
                break;
            }

            x += pgm_read_byte(&ptr[i].x);
            y += pgm_read_byte(&ptr[i].y);
        }
    }

} // add_rook_moves(piece_gen_t &gen)


/*
 * evaluate the moves for a bishop against the best move so far
 *
 */
void add_bishop_moves(piece_gen_t &gen) {
    static offset_t const dirs[4] PROGMEM = {
        { -1, -1 }, { -1,  1 }, {  1, -1 }, {  1,  1 }
    };

    // Keep track of memory usage by function level and ply level
    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (freeMemory() < game.options.low_mem_limit) {
        show_low_memory();
        return;
    }

    offset_t const * const ptr = (offset_t const * const) pgm_get_far_address(dirs);
    for (index_t i = 0; i < index_t(ARRAYSZ(dirs)); i++) {
        index_t x = gen.col + pgm_read_byte(&ptr[i].x);
        index_t y = gen.row + pgm_read_byte(&ptr[i].y);

        while (isValidPos(x, y)) {
            // See if the turn has timed out
            if ((game.last_was_timeout = timeout())) {
                show_timeout();
                return;
            }

            gen.move.to = x + y * 8;
            Piece const op = board.get(gen.move.to);

            if (isEmpty(op)) {
                gen.callme(gen);
            }
            else if (getSide(op) != gen.side) {
                gen.callme(gen);
                break;
            }
            else {
                break;
            }

            x += pgm_read_byte(&ptr[i].x);
            y += pgm_read_byte(&ptr[i].y);
        }
    }

} // add_bishop_moves(piece_gen_t &gen)


/*
 * evaluate the moves for a queen against the best move so far
 *
 */
void add_queen_moves(piece_gen_t &gen) {
    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    add_rook_moves(gen);
    add_bishop_moves(gen);

} // add_queen_moves(piece_gen_t &gen)


/*
 * evaluate the moves for a king against the best move so far
 *
 */
void add_king_moves(piece_gen_t &gen) {
    static offset_t constexpr king_offsets[8] PROGMEM = {
        { -1,  0 }, {  0, -1 }, { -1, -1 }, { +1, -1 }, 
        { +1,  0 }, {  0, +1 }, { -1, +1 }, { +1, +1 }
    };

    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    if (freeMemory() < game.options.low_mem_limit) {
        show_low_memory();
        return;
    }

    offset_t const * const ptr = (offset_t *) pgm_get_far_address(king_offsets);
    for (index_t i = 0; i < index_t(ARRAYSZ(king_offsets)); i++) {
        // See if the turn has timed out
        if ((game.last_was_timeout = timeout())) {
            show_timeout();
            return;
        }

        index_t const to_col = gen.col + pgm_read_byte(&ptr[i].x);
        index_t const to_row = gen.row + pgm_read_byte(&ptr[i].y);
        if (isValidPos(to_col, to_row)) {
            gen.move.to = to_col + to_row * 8;
            Piece const op = board.get(gen.move.to);
            if (isEmpty(op) || getSide(op) != gen.side) {
                gen.callme(gen);
            }
        }
    }

    // check for castling
    if (!hasMoved(gen.piece)) {
        // check King's side (right-hand side from white's view)
        index_t rook_loc = 7 + gen.row * 8;
        Piece rook = board.get(rook_loc);
        Bool empty_knight = isEmpty(board.get(1 + gen.row * 8));
        Bool empty_bishop = isEmpty(board.get(2 + gen.row * 8));
        if (!isEmpty(rook) && !hasMoved(rook)) {
            if (empty_knight && empty_bishop) {
                // We can castle on the King's side
                gen.move.to = 2 + gen.row * 8;
                gen.callme(gen);
            }
        }

        // check Queen's side (left-hand side from white's view)
        rook_loc = 0 + gen.row * 8;
        rook = board.get(rook_loc);
        if (!isEmpty(rook) && !hasMoved(rook)) {
            Bool const  empty_queen = isEmpty(board.get(4 + gen.row * 8));
                       empty_knight = isEmpty(board.get(6 + gen.row * 8));
                       empty_bishop = isEmpty(board.get(5 + gen.row * 8));
            if (empty_knight && empty_bishop && empty_queen) {
                // We can castle on the Queens's side
                gen.move.to = 6 + gen.row * 8;
                gen.callme(gen);
            }
        }
    }

} // add_king_moves(piece_gen_t &gen)