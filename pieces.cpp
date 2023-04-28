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
index_t add_pawn_moves(piece_gen_t &gen) {
    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) { return 0; }

    // See if we can move 1 spot in front of this pawn
    index_t to_col = gen.col;
    index_t to_row = gen.row + (gen.whites_turn ? -1 : +1);
    gen.move.to = to_col + to_row * 8;

    // Count the number of available moves
    index_t count = 0;

    // local lambda function to check for forward moves
    auto check_fwd = [](piece_gen_t &gen, index_t const col, index_t const row) -> index_t {
        if (!isValidPos(col, row)) {
            return 0;
        }

        gen.move.to = col + row * 8;

        if (!isEmpty(board.get(gen.move.to))) {
            return 0;
        }

        gen.callme(gen);

        return 1;
    };

    // Check 1 row ahead
    count += check_fwd(gen, to_col, to_row);

    // Check 2 rows ahead
    if (!hasMoved(board.get(gen.move.from))) {
        to_row += gen.whites_turn ? -1 : +1;
        count += check_fwd(gen, to_col, to_row);
    }

    // See if we can capture a piece diagonally
    for (index_t i = -1; i <= 1; i += 2) {
        // See if the turn has timed out
        if ((game.last_was_timeout = timeout()) && (game.ply > 1)) {
            show_timeout();
            return count;
        }

        to_col = gen.col + i;
        to_row = gen.row + (gen.whites_turn ? -1 : +1);
        gen.move.to = to_col + to_row * 8;
        if (isValidPos(to_col, to_row)) {
            // Check diagonal piece
            Piece op = board.get(gen.move.to);
            if (!isEmpty(op) && getSide(op) != gen.side) {
                gen.callme(gen);
                count++;
            }

            // Check for en-passant
            // index_t const last_move_from_row = game.last_move.from / 8;
            // index_t const last_move_to_col = game.last_move.to % 8;
            // index_t const last_move_to_row = game.last_move.to / 8;

            // if (last_move_to_col == to_col && last_move_to_row == gen.row) {
            //     if (abs(int(last_move_from_row) - int(last_move_to_row)) > 1) {
            //         op = board.get(last_move_to_col + gen.row * 8);
            //         if (getType(op) == Pawn && getSide(op) != gen.side) {
            //             gen.move.to = to_col + (gen.row + (gen.whites_turn ? -1 : 1)) * 8;
            //             gen.callme(gen);
            //             count++;
            //         }
            //     }
            // }
        }
    }

    return count;

} // add_pawn_moves(piece_gen_t &gen)


static offset_t constexpr knight_offsets[8] PROGMEM = {
    { -1, +2 }, { -1, -2 }, { -2, +1 }, { -2, -1 }, 
    { +1, +2 }, { +1, -2 }, { +2, +1 }, { +2, -1 }  
};

static offset_t constexpr rook_offsets[4] PROGMEM = {
    {  0,  1 }, {  0, -1 }, { -1,  0 }, {  1,  0 }
};

static offset_t const bishop_offsets[4] PROGMEM = {
    { -1, -1 }, { -1,  1 }, {  1, -1 }, {  1,  1 }
};


// #define  USE_NEW

#ifdef USE_NEW
index_t gen_moves(piece_gen_t &gen, offset_t const * const ptr, index_t const num_dirs, index_t const num_iter) {
    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    if (freeMemory() < game.options.low_mem_limit) {
        show_low_memory();
        return 0;
    }

    // Count the number of available moves
    index_t count = 0;

    for (index_t i = 0; i < num_dirs; i++) {
        index_t x = gen.col + pgm_read_byte(&ptr[i].x);
        index_t y = gen.row + pgm_read_byte(&ptr[i].y);

        for (index_t iter = 0; iter < num_iter && isValidPos(x, y); iter++) {
            // See if the turn has timed out
            if ((game.last_was_timeout = timeout()) && (game.ply > 1)) {
                show_timeout();
                return count;
            }

            gen.move.to = x + y * 8;
            Piece const other_piece = board.get(gen.move.to);

            if (isEmpty(other_piece)) {
                gen.callme(gen);
                count++;
            }
            else if (getSide(other_piece) != gen.side) {
                gen.callme(gen);
                count++;
                break;
            }
            else {
                break;
            }

            x += pgm_read_byte(&ptr[i].x);
            y += pgm_read_byte(&ptr[i].y);
        }
    }

    return count;

} // gen_moves(piece_gen_t &gen, offset_t const * const dirs, index_t const num_dirs, index_t const num_iter)


index_t add_knight_moves(piece_gen_t &gen) {
    return gen_moves(gen, pgm_get_far_address(knight_offsets), ARRAYSZ(knight_offsets), 1);
}


index_t add_rook_moves(piece_gen_t &gen) {
    return gen_moves(gen, pgm_get_far_address(rook_offsets), ARRAYSZ(rook_offsets), 7);
}


index_t add_bishop_moves(piece_gen_t &gen) {
    return gen_moves(gen, pgm_get_far_address(bishop_offsets), ARRAYSZ(bishop_offsets), 7);
}


index_t add_king_moves(piece_gen_t &gen) {
    // Count the number of available moves
    index_t count = 0;

    count += gen_moves(gen, pgm_get_far_address(rook_offsets), ARRAYSZ(rook_offsets), 1);
    count += gen_moves(gen, pgm_get_far_address(rook_offsets), ARRAYSZ(rook_offsets), 1);

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
                count++;
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
                count++;
            }
        }
    }

    return count;

} // add_king_moves(piece_gen_t &gen)

#else

/*
 * evaluate the moves for a knight against the best move so far
 *
 */
index_t add_knight_moves(piece_gen_t &gen) {
    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) { return 0; }

    // Count the number of available moves
    index_t count = 0;

    offset_t const * const ptr = (offset_t const *) pgm_get_far_address(knight_offsets);

    for (index_t i = 0; i < index_t(ARRAYSZ(knight_offsets)); i++) {
        // See if the turn has timed out
        if ((game.last_was_timeout = timeout()) && (game.ply > 1)) {
            show_timeout();
            return count;
        }

        index_t const to_col = gen.col + pgm_read_byte(&ptr[i].x);
        index_t const to_row = gen.row + pgm_read_byte(&ptr[i].y);
        if (isValidPos(to_col, to_row)) {
            gen.move.to = to_col + to_row * 8;
            Piece const op = board.get(gen.move.to);
            if (isEmpty(op) || getSide(op) != gen.side) {
                gen.callme(gen);
                count++;
            }
        }
    }

    return count;

} // add_knight_moves(piece_gen_t &gen)


/*
 * evaluate the moves for a rook against the best move so far
 *
 */
index_t add_rook_moves(piece_gen_t &gen) {
    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) { return 0; }

    // Count the number of available moves
    index_t count = 0;

    offset_t const * const ptr = (offset_t const * const) pgm_get_far_address(rook_offsets);

    for (index_t i = 0; i < index_t(ARRAYSZ(rook_offsets)); i++) {
        index_t x = gen.col + pgm_read_byte(&ptr[i].x);
        index_t y = gen.row + pgm_read_byte(&ptr[i].y);

        while (isValidPos(x, y)) {
            // See if the turn has timed out
            if ((game.last_was_timeout = timeout()) && (game.ply > 1)) {
                show_timeout();
                return count;
            }

            gen.move.to = x + y * 8;
            Piece const op = board.get(gen.move.to);

            if (isEmpty(op)) {
                gen.callme(gen);
                count++;
            }
            else if (getSide(op) != gen.side) {
                gen.callme(gen);
                count++;
                break;
            }
            else {
                break;
            }

            x += pgm_read_byte(&ptr[i].x);
            y += pgm_read_byte(&ptr[i].y);
        }
    }

    return count;

} // add_rook_moves(piece_gen_t &gen)


/*
 * evaluate the moves for a bishop against the best move so far
 *
 */
index_t add_bishop_moves(piece_gen_t &gen) {
    // Keep track of memory usage by function level and ply level
    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) { return 0; }

    // Count the number of available moves
    index_t count = 0;

    offset_t const * const ptr = (offset_t const * const) pgm_get_far_address(bishop_offsets);

    for (index_t i = 0; i < index_t(ARRAYSZ(bishop_offsets)); i++) {
        index_t x = gen.col + pgm_read_byte(&ptr[i].x);
        index_t y = gen.row + pgm_read_byte(&ptr[i].y);

        while (isValidPos(x, y)) {
            // See if the turn has timed out
            if ((game.last_was_timeout = timeout()) && (game.ply > 1)) {
                show_timeout();
                return count;
            }

            gen.move.to = x + y * 8;
            Piece const op = board.get(gen.move.to);

            if (isEmpty(op)) {
                gen.callme(gen);
                count++;
            }
            else if (getSide(op) != gen.side) {
                gen.callme(gen);
                count++;
                break;
            }
            else {
                break;
            }

            x += pgm_read_byte(&ptr[i].x);
            y += pgm_read_byte(&ptr[i].y);
        }
    }

    return count;

} // add_bishop_moves(piece_gen_t &gen)


/*
 * evaluate the moves for a king against the best move so far
 *
 */
index_t add_king_moves(piece_gen_t &gen) {
    static offset_t constexpr king_offsets[8] PROGMEM = {
        { -1,  0 }, {  0, -1 }, { -1, -1 }, { +1, -1 }, 
        { +1,  0 }, {  0, +1 }, { -1, +1 }, { +1, +1 }
    };

    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) { return 0; }

    // Count the number of available moves
    index_t count = 0;

    offset_t const * const ptr = (offset_t *) pgm_get_far_address(king_offsets);

    for (index_t i = 0; i < index_t(ARRAYSZ(king_offsets)); i++) {
        // See if the turn has timed out
        if ((game.last_was_timeout = timeout()) && (game.ply > 1)) {
            show_timeout();
            return count;
        }

        index_t const to_col = gen.col + pgm_read_byte(&ptr[i].x);
        index_t const to_row = gen.row + pgm_read_byte(&ptr[i].y);
        if (isValidPos(to_col, to_row)) {
            gen.move.to = to_col + to_row * 8;
            Piece const op = board.get(gen.move.to);
            if (isEmpty(op) || getSide(op) != gen.side) {
                gen.callme(gen);
                count++;
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
                count++;
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
                count++;
            }
        }
    }

    return count;
} // add_king_moves(piece_gen_t &gen)

#endif


/*
 * evaluate the moves for a queen against the best move so far
 *
 */
index_t add_queen_moves(piece_gen_t &gen) {
    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    return add_rook_moves(gen) + add_bishop_moves(gen);

} // add_queen_moves(piece_gen_t &gen)
