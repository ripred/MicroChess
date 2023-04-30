/**
 * pieces.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * move generation for all pieces
 * 
 */
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "MicroChess.h"

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


// Function to check for forward moves
index_t check_fwd(piece_gen_t &gen, index_t const col, index_t const row) {
    if (!isValidPos(col, row)) { return 0; }
    gen.move.to = col + row * 8;
    if (!isEmpty(board.get(gen.move.to))) { return 0; }
    gen.callme(gen);
    return 1;
};


index_t add_pawn_moves(piece_gen_t &gen) {
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK
    index_t to_col, to_row, count, i, last_move_from_row;
    index_t last_move_to_col, last_move_to_row;
    Piece op;

    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) { return 0; }

    // Now we can alter local variables! 😎 

    // See if we can move 1 spot in front of this pawn
    to_col = gen.col;
    to_row = gen.row + (gen.whites_turn ? -1 : +1);
    gen.move.to = to_col + to_row * 8;

    // Count the number of available moves
    count = 0;

    // Check 1 row ahead
    count += check_fwd(gen, to_col, to_row);

    // Check 2 rows ahead
    if (!hasMoved(board.get(gen.move.from))) {
        to_row += gen.whites_turn ? -1 : +1;
        count += check_fwd(gen, to_col, to_row);
    }

    // See if we can capture a piece diagonally
    for (i = -1; i <= 1; i += 2) {
        // See if the turn has timed out
        if (timeout()) {
            return count;
        }

        to_col = gen.col + i;
        to_row = gen.row + (gen.whites_turn ? -1 : +1);
        gen.move.to = to_col + to_row * 8;
        if (isValidPos(to_col, to_row)) {
            // Check diagonal piece
            op = board.get(gen.move.to);
            if (!isEmpty(op) && getSide(op) != gen.side) {
                gen.callme(gen);
                count++;
            }

            // Check for en-passant
            last_move_from_row = game.last_move.from / 8;
            last_move_to_col = game.last_move.to % 8;
            last_move_to_row = game.last_move.to / 8;

            if (last_move_to_col == to_col && last_move_to_row == gen.row) {
                if (abs(int(last_move_from_row) - int(last_move_to_row)) > 1) {
                    op = board.get(last_move_to_col + gen.row * 8);
                    if (getType(op) == Pawn && getSide(op) != gen.side) {
                        gen.move.to = to_col + (gen.row + (gen.whites_turn ? -1 : 1)) * 8;
                        gen.callme(gen);
                        count++;
                    }
                }
            }
        }
    }

    return count;

} // add_pawn_moves(piece_gen_t &gen)


index_t gen_moves(piece_gen_t &gen, offset_t const * const ptr, index_t const num_dirs, index_t const num_iter) {
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK
    index_t count;
    index_t i;
    index_t x;
    index_t y;
    index_t iter;
    Piece other_piece;

    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) { return 0; }

    // Now we can alter local variables! 😎 

    // Count the number of available moves
    count = 0;

    for (i = 0; i < num_dirs; i++) {
        x = gen.col + pgm_read_byte(&ptr[i].x);
        y = gen.row + pgm_read_byte(&ptr[i].y);

        for (iter = 0; iter < num_iter && isValidPos(x, y); iter++) {
            // See if the turn has timed out
            if (timeout()) {
                return count;
            }

            gen.move.to = x + y * 8;
            other_piece = board.get(gen.move.to);

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

} // gen_moves(...)


index_t add_knight_moves(piece_gen_t &gen) {
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK

    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) { return 0; }

    // Now we can alter local variables! 😎 

    return gen_moves(gen, pgm_get_far_address(knight_offsets), ARRAYSZ(knight_offsets), 1);

} // add_knight_moves(piece_gen_t &gen)


index_t add_rook_moves(piece_gen_t &gen) {
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK

    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) { return 0; }

    // Now we can alter local variables! 😎 

    return gen_moves(gen, pgm_get_far_address(rook_offsets), ARRAYSZ(rook_offsets), 7);

} // add_rook_moves(piece_gen_t &gen)


index_t add_bishop_moves(piece_gen_t &gen) {
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK

    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) { return 0; }

    // Now we can alter local variables! 😎 

    return gen_moves(gen, pgm_get_far_address(bishop_offsets), ARRAYSZ(bishop_offsets), 7);

} // add_bishop_moves(piece_gen_t &gen)


index_t add_king_moves(piece_gen_t &gen) {
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK
    index_t count;
    Piece rook;
    Bool empty_knight;
    Bool empty_bishop;
    Bool empty_queen;

    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) { return 0; }

    // Now we can alter local variables! 😎 

    // Count the number of available moves
    count = 0;

    count += gen_moves(gen, pgm_get_far_address(rook_offsets), ARRAYSZ(rook_offsets), 1);
    count += gen_moves(gen, pgm_get_far_address(bishop_offsets), ARRAYSZ(bishop_offsets), 1);

    // check for castling
    if (!hasMoved(gen.piece)) {
        // check King's side (right-hand side from white's view)
        rook = board.get(7 + gen.row * 8);
        empty_knight = isEmpty(board.get(1 + gen.row * 8));
        empty_bishop = isEmpty(board.get(2 + gen.row * 8));
        if (!isEmpty(rook) && !hasMoved(rook)) {
            if (empty_knight && empty_bishop) {
                // We can castle on the King's side
                gen.move.to = 2 + gen.row * 8;
                gen.callme(gen);
                count++;
            }
        }

        // check Queen's side (left-hand side from white's view)
        rook = board.get(0 + gen.row * 8);
        if (!isEmpty(rook) && !hasMoved(rook)) {
             empty_queen = isEmpty(board.get(4 + gen.row * 8));
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


index_t add_queen_moves(piece_gen_t &gen) {
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK

    #ifdef ENA_MEM_STATS
    game.freemem[1][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) { return 0; }

    // Now we can alter local variables! 😎 

    return add_rook_moves(gen) + add_bishop_moves(gen);

} // add_queen_moves(piece_gen_t &gen)
