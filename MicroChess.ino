/**
 * ArduinoChess.ino
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * version 1.0.0
 * written March thru May 2023 - Trent M. Wyatt
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * TODO:
 * 
 *  [ ] 
 *  [ ] Add awareness of other Arduino's on the I2C bus acting as slave devices
 *      and add the ability to parallel process to moves between all available
 *      CPU's!
 *  [ ] Change to use true printf, stdin, stdout and stderr so we don't copy the fmt buffer
 *  [ ] Add tests that prove that the same moves are chosen when alpha-beta
 *      pruning is enabled that are chosen when it is disabled and running in brute force?
 *  [ ] Add optional use of I2C serial RAM to create transposition tables for
 *      moves that have already been searced during this turn.
 *  [+] Fix problems with:
 *      [+] King's in check not being detected or reported correctly
 *      [+] King's not reacting when in check
 *  [+] Change skip logic to use the same percentage based logic as the mistakes logic
 *  [+] Fix alpha-beta bug!
 *  [+] Finish enabling the en-passant pawn move generation.
 *  [+] Add a function to display times over 1000 ms as minutes, seconds, and ms
 *  [+] Add flags to the game to indicate whether each side is human or not
 *  [+] Add the display of the full game time so far to each show() update
 *  [+] Enhance to pause the game when a newline is received
 *  [+] Enhance the game to allow moves to be entered by a human via the serial port
 *  [+] Change make_move(...) to return MIN_VALUE only if the ply level is 0
 *      so we don't actually choose it. Otherwise consider taking the King
 *      worth MAX_VALUE.
 *  [+] Change to have two sets of option_t in the game, one for each player in order to test
 *      option settings against each other
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 */
#include <Arduino.h>
// #include <Wire.h>
#include "MicroChess.h"

////////////////////////////////////////////////////////////////////////////////////////
// The game board
board_t board;


////////////////////////////////////////////////////////////////////////////////////////
// The currently running game states and flags
game_t game;


// Un-comment the following line to display each move as it is evaluated
// #define SHOW1


////////////////////////////////////////////////////////////////////////////////////////
// Consider a move against the best white move or the best black move
// depending on the color of the piece and set this as the best move
// if it has a higher value (or an equal value when we're using random)
// 
// Note: Sanitized stack
void consider_move(piece_gen_t &gen)
{
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK
    Bool dont_move;

    //  Check for low stack space
    if (check_mem(CONSIDER)) { return; }

    dont_move = False;

    if (PLAYING != game.state) {
        return;
    }

    // If this is a supplied move that has already been validated then just return:
    if (game.supply_valid) {
        return;
    }

    // See if the destination is a king, and if so then don't really make the move
    // or evaluate it; Just return MAX or MIN value depending on whose side it is:
    if (getType(board.get(gen.move.to)) == King) {
        if (gen.whites_turn) {
            game.white_king_in_check = True;
            gen.move.value = MAX_VALUE;
        }
        else {
            game.black_king_in_check = True;
            gen.move.value = MIN_VALUE;
        }

        show_check();

        dont_move = True;
    }

    // See if the move came from the user or from an opening book:
    if (game.book_supplied || game.user_supplied) {
        if ((gen.move.from == game.supplied.from) && (gen.move.to == game.supplied.to)) {
            game.supply_valid = True;
            return;
        }
    }

    // Check for alpha or beta cutoff
    if (gen.cutoff) {
        #ifdef SHOW1
        if (0 == game.ply) {
            printf(Debug1, "** ");
        }
        #endif
        dont_move = True;
    }

    // Recursively generate the move's value
    if (!dont_move) {
        make_move(gen);
    }

    if (Pawn == gen.piece) {
        // Reward any moves involving a Pawn slightly
        gen.move.value += (gen.whites_turn ? +10 : -10);
    }

    // See if we are in the end game
    if (game.piece_count <= END_COUNT) {
        if (Pawn == gen.piece) {
            // Reward any moves involving a Pawn at this point
            gen.move.value += (gen.whites_turn ? +5000 : -5000);
        }
        else {
            // Otherwise make all pieces converge on the opponent's King.

            // Get the location of the opponent's King
            index_t kingloc = (gen.whites_turn ? game.bking : game.wking);
            index_t king_col = kingloc % 8;
            index_t king_row = kingloc / 8;

            // Get the difference between our destination and the opponent's king
            index_t delta_col = abs((gen.move.to % 8) - king_col);
            index_t delta_row = abs((gen.move.to / 8) - king_row);

            gen.move.value = (14 - (delta_col + delta_row)) + 
                (gen.whites_turn ? game.black_king_in_check : game.white_king_in_check) * 10;
        }
    }

    if (gen.whites_turn && game.white_king_in_check) {
        gen.move.value = MIN_VALUE;
    }
    
    if (!gen.whites_turn && game.black_king_in_check) {
        gen.move.value = MAX_VALUE;
    }
    
    // Penalize the move if it would cause us to lose by move repetition
    if (would_repeat(gen.move)) {
        gen.move.value = gen.whites_turn ? MIN_VALUE : MAX_VALUE;
    }

    // See if this move is equal to OR greater than the best move we've seen so far
    if (gen.whites_turn) {
        if ((gen.move.value == gen.wbest.value) && random(2)) {
            gen.wbest = gen.move;
        }
        else if (gen.move.value > gen.wbest.value) {
            gen.wbest = gen.move;
        }
    }
    else {
        if ((gen.move.value == gen.bbest.value) && random(2)) {
            gen.bbest = gen.move;
        }
        else if (gen.move.value < gen.bbest.value) {
            gen.bbest = gen.move;
        }
    }

    // Debugging output
    #ifdef SHOW1
    if (0 == game.ply) {
        show_move(gen.move, True);
    }
    #endif

}   // consider_move(piece_gen_t &gen)


////////////////////////////////////////////////////////////////////////////////////////
// Move a piece on the board, taking a piece if necessary. Evaluate the value of the 
// board after the move. Optionally restore the board back to it's original state after
// evaluating the value of the move.
// 
// This is a big and complicated function.
// It performs 5 major steps:
// 
//  1) Identify the piece being moved
//  2) Identify any piece being captured and remove it if so
//  3) Place the piece being moved at the destination
//  4) Evaluate the board score after making the move
//  5) If we are just considering the move then put everything back
// 
// returns the value of the board after the move was made
// 
// Note: Sanitized stack
long make_move(piece_gen_t & gen)
{
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK
    struct local_t {
        uint8_t
                                 op : 6,
                      book_supplied : 1,
                      user_supplied : 1,    //  8

                         board_rook : 6,
            last_was_pawn_promotion : 1,
                last_was_en_passant : 1,    // 16

                        place_piece : 6,
                white_king_in_check : 1,
                black_king_in_check : 1,    // 24

                     captured_piece : 6,
                    last_was_castle : 1,
                          quiescent : 1,    // 32

                             to_col : 3,
                       supply_valid : 1,
                             to_row : 3,
                              oside : 1,    // 40

                              wking : 6,
                              bking : 6,
                  white_taken_count : 5, 
                  black_taken_count : 5,
                              otype : 3;    // 65 bits (9 bytes)
    } vars;

    index_t taken_index, captured, castly_rook, hist_count;
    game_t::history_t history[MAX_REPS * 2 - 1];
    move_t  last_move, wbest, bbest;
    int32_t recurse_value;

    //  Check for low stack space
    if (check_mem(MAKE)) { return gen.whites_turn ? MIN_VALUE : MAX_VALUE; }

    // Now we can alter local variables! 😎 

    // The value of the board.
    // Default to the worst value for our side.
    // i.e: Don't make the move whatever it is
    gen.move.value = gen.whites_turn ? MIN_VALUE : MAX_VALUE;


    /// Step 1: Identify the piece being moved

    vars.to_col = uint8_t(gen.move.to % 8);
    vars.to_row = uint8_t(gen.move.to / 8);
    vars.op = board.get(gen.move.to);
    vars.otype = getType(vars.op);
    vars.oside = getSide(vars.op);        

    // Save the current last 5 moves in the game history
    hist_count = game.hist_count;

    // Save the current king locations
    vars.wking = game.wking;
    vars.bking = game.bking;

    // Save the current number of taken pieces
    vars.white_taken_count = game.white_taken_count;
    vars.black_taken_count = game.black_taken_count;

    // Save the current last move and move flags
    vars.last_was_pawn_promotion = game.last_was_pawn_promotion;
    vars.last_was_en_passant = game.last_was_en_passant;
    vars.last_was_castle = game.last_was_castle;
    last_move = game.last_move;

    // Save the user supplied  and book supplied flags
    vars.book_supplied = game.book_supplied;
    vars.user_supplied = game.user_supplied;
    vars.supply_valid = game.supply_valid;

    if (gen.evaluating) {
        memmove(history, game.history, sizeof(history));
        game.stats.inc_moves_count();
    }

    // Save the state of whether or not the kings are in check.
    // We do this AFTER we've had a chance to set the 'king-in-check'
    // flags above so that this move leaves the flags behind after evaluation
    vars.white_king_in_check = game.white_king_in_check;
    vars.black_king_in_check = game.black_king_in_check;


    /// Step 2: Identify any piece being captured and remove it if so.
    // 
    // * NOTE BELOW *
    // Once any changes have been made to the board or game state we MUST NOT return
    // without passing through the "if (gen.evaluating) { ... }" restoration logic.

    // The game.pieces[] index being captured (if any, -1 if none)
    taken_index = -1;

    // The board index being captured (if any, -1 if none)
    captured = -1;
    vars.captured_piece = Empty;

    // Check for en-passant capture
    if (Pawn == gen.type && isEmpty(vars.otype) && gen.col != vars.to_col) {
        game.last_was_en_passant = True;
        captured = vars.to_col + gen.row * 8u;
        vars.captured_piece = board.get(captured);
    }
    else {
        // See if the destination is not empty and not a piece on our side.
        // i.e. an opponent's piece.
        if (Empty != vars.otype && gen.side != vars.oside) {
            captured = gen.move.to;
            vars.captured_piece = board.get(captured);
        }
    }

    // If a piece was taken, make the change on the board and to the game.pieces[] list
    if (-1 != captured) {
        // Remember the piece index of the piece being taken
        taken_index = game.find_piece(captured);

        // Change the spot on the board for the taken piece to Empty
        board.set(captured, Empty);

        // Soft-delete the piece taken in the piece list!
        game.pieces[taken_index] = { -1, -1 };

        // Add the piece to the list of taken pieces
        if (gen.whites_turn) {
            game.taken_by_white[game.white_taken_count++].piece = vars.captured_piece;
        }
        else {
            game.taken_by_black[game.black_taken_count++].piece = vars.captured_piece;
        }
    }


    /// Step 3: Place the piece being moved at the destination

    // Set the 'moved' flag on the piece that we place on the board
    vars.place_piece = setMoved(gen.piece, True);

    // Promote a Pawn to a Queen if it reaches the back row
    if (Pawn == gen.type && (vars.to_row == (gen.whites_turn ? index_t(0) : index_t(7)))) {
        vars.place_piece = setType(vars.place_piece, Queen);
        game.last_was_pawn_promotion = True;
    }

    // Move the piece to the destination on the board
    board.set(gen.move.from, Empty);
    board.set(gen.move.to, vars.place_piece);

    // Update the piece list to reflect the piece's new location
    game.pieces[gen.piece_index] = { index_t(vars.to_col), index_t(vars.to_row) };

    // Check for castling
    castly_rook = -1;

    // If the piece being moved is a King
    if (King == gen.type) {
        // Update this side's king location
        ((White == gen.side) ? game.wking : game.bking) = gen.move.to;

        // Get the horizontal distance the king is
        // moving and see if it is a Castling move
        if (abs(vars.to_col - (gen.move.from % 8)) > 1) {
            // see which side we're castling on
            if (2 == abs(vars.to_col - (gen.move.from % 8))) {
                // Castle on the King's side
                vars.board_rook = 7 + gen.row * 8u;
                castly_rook = game.find_piece(vars.board_rook);
                board.set(vars.board_rook, setMoved(board.get(vars.board_rook), True));
                game.pieces[castly_rook].x = 5;
                game.last_was_castle = True;
            }
            else if (3 == abs(vars.to_col - (gen.move.from % 8))) {
                // Castle on the Queen's side
                vars.board_rook = 0 + gen.row * 8u;
                castly_rook = game.find_piece(vars.board_rook);
                board.set(vars.board_rook, setMoved(board.get(vars.board_rook), True));
                game.pieces[castly_rook].x = 3;
                game.last_was_castle = True;
            }
        }
    }


    /// Step 4: Evaluate the board score after making the move

    // Get the value of the current board
    gen.move.value = evaluate(gen);

    // Control the percentage of moves that the engine makes a mistake on
    if (0 != game.options.mistakes) {
        if (random(100) <= game.options.mistakes) {
            gen.move.value -= gen.whites_turn ? +5000 : -5000;
        }
    }

    // set our move as the last move
    game.last_move = gen.move;

    ////////////////////////////////////////////////////////////////////////////////////////
    // The move has been made and we have the value for the updated board.
    // Recursively look-ahead and accumulatively update the value here.
    // 
    if (gen.evaluating) {
        // flag indicating whether we are traversing into quiescent moves
        vars.quiescent = ((-1 != captured) && (game.ply < (game.options.max_quiescent_ply)) && (game.ply < game.options.max_max_ply));

        if (((game.ply < game.options.maxply) || vars.quiescent)) {
            if (!timeout()) {
                // Indicate whether we are on a quiescent search or not
                if (vars.quiescent) {
                    show_quiescent_search();
                }
                else {
                    direct_write(DEBUG2_PIN, LOW);
                }

                if ((0 == game.options.randskip) || (random(100) > game.options.randskip)) {
                    // Explore The Future! (plies)
                    game.ply++;
                    game.turn = !game.turn;
                    if (game.ply > game.stats.move_stats.depth) {
                        game.stats.move_stats.depth = game.ply;
                    }
                    wbest = { -1, -1, game.alpha };
                    bbest = { -1, -1, game.beta  };
                    reset_turn_flags();
                    choose_best_moves(wbest, bbest, consider_move);
                    game.turn = !game.turn;
                    game.ply--;

                    if (gen.whites_turn) {
                        if (-1 != gen.wbest.from && -1 != gen.wbest.to) {
                            if (game.options.alpha_beta_pruning) {
                                recurse_value = max(gen.move.value, gen.wbest.value);
                                gen.move.value = game.options.integrate ? (gen.move.value + recurse_value) : recurse_value;
                                if (gen.move.value > game.beta) {
                                    gen.cutoff = True;
                                }
                                else {
                                    game.alpha = max(game.alpha, gen.move.value);
                                }
                            }
                            else {
                                gen.move.value = game.options.integrate ? (gen.move.value + gen.wbest.value) : gen.wbest.value;
                            }
                        }
                    }
                    else {
                        if (-1 != gen.bbest.from && -1 != gen.bbest.to) {
                            if (game.options.alpha_beta_pruning) {
                                recurse_value = min(gen.move.value, gen.bbest.value);
                                gen.move.value = game.options.integrate ? (gen.move.value + recurse_value) : recurse_value;
                                if (gen.move.value < game.alpha) {
                                    gen.cutoff = True;
                                }
                                else {
                                    game.beta = min(game.beta, gen.move.value);
                                }
                            }
                            else {
                                gen.move.value = game.options.integrate ? (gen.move.value + gen.bbest.value) : gen.bbest.value;
                            }
                        }
                    }

                    // We're finished calling into the future moves, and setting the new alpha and
                    // beta boundaries. Now make sure this move didn't place our king in check

                    // if (gen.whites_turn) {
                    //     if (game.white_king_in_check) {
                    //         gen.move.value = MIN_VALUE;
                    //     }
                    // }
                    // else {
                    //     if (game.black_king_in_check) {
                    //         gen.move.value = MAX_VALUE;
                    //     }
                    // }

                    if (game.ply > 0) {
                        game.white_king_in_check = vars.white_king_in_check;
                        game.black_king_in_check = vars.black_king_in_check;
                    }
                }
            }
        }

    } // if (gen.evaluating)


    /// Step 5: If we are just considering the move then put everything back

    if (gen.evaluating) {
        if (-1 == captured) {
            board.set(gen.move.to, vars.op);
        } else {
            // restore the captured board changes and
            // set it's "in-check" flag
            vars.captured_piece = setCheck(vars.captured_piece, True);
            board.set(captured, vars.captured_piece);

            // restore the captured piece list changes
            game.pieces[taken_index] = { index_t(captured % 8), index_t(captured / 8) };
        }

        // restore the taken pieces list changes
        game.white_taken_count = vars.white_taken_count;
        game.black_taken_count = vars.black_taken_count;

        // restore the changes made to the moves history
        memmove(game.history, history, sizeof(history));
        game.hist_count = hist_count;

        // restore the moved piece board changes
        board.set(gen.move.from, gen.piece);

        // restore the moved piece pieces list changes
        game.pieces[gen.piece_index] = { index_t(gen.col), index_t(gen.row) };

        // restore the last move made
        game.last_move = last_move;

        // restore the last move flags
        game.last_was_en_passant = vars.last_was_en_passant;
        game.last_was_castle = vars.last_was_castle;
        game.last_was_pawn_promotion = vars.last_was_pawn_promotion;

        // game.white_king_in_check = vars.white_king_in_check;
        // game.black_king_in_check = vars.black_king_in_check;

        game.book_supplied = vars.book_supplied;
        game.user_supplied = vars.user_supplied;
        game.supply_valid = vars.supply_valid;

        // Don't take the move if it leaves us in check
        if (gen.whites_turn) {
            if (game.white_king_in_check) {
                gen.move.value = MIN_VALUE;
            }
        }
        else {
            if (game.black_king_in_check) {
                gen.move.value = MAX_VALUE;
            }
        }

        // restore the king's locations
        game.wking = vars.wking;
        game.bking = vars.bking;

        // restore any rook moved during a castle move
        game.last_was_castle = vars.last_was_castle;
        if (-1 != castly_rook) {
            if (3 == game.pieces[castly_rook].x) {
                game.pieces[castly_rook].x = 0;
            }
            else {
                game.pieces[castly_rook].x = 7;
            }

            board.set((game.pieces[castly_rook].x + game.pieces[castly_rook].y * 8), 
                setMoved(board.get((game.pieces[castly_rook].x + game.pieces[castly_rook].y * 8)), False));
        }

    } // if (gen.evaluating)

    return gen.move.value;

}   // make_move(piece_gen_t & gen)


////////////////////////////////////////////////////////////////////////////////////////
// Evaluate the identity (score) of the board state.
// Positive scores indicate an advantage for white and
// Negative scores indicate an advantage for black.
// Uses pre-computed material bonus tables for speed.
// 
// returns the score/value of the current board
// 
// Note: Sanitized stack
long evaluate(piece_gen_t &gen)
{
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK
    long materialTotal, mobilityTotal, centerTotal, kingTotal, score;
    index_t col, row, piece_index, kloc, col_dist, row_dist, proximity;
    Piece p, ptype;
    Color pside;

    //  Check for low stack space
    if (check_mem(MAKE)) { return 0; }

    // Now we can alter local variables! 😎 

    // Calculate the value of the board:
    materialTotal = 0L;
    mobilityTotal = 0L;
    centerTotal = 0L;
    kingTotal = 0L;
    score = 0L;

    for (piece_index = 0; piece_index < game.piece_count; piece_index++) {
        col = game.pieces[piece_index].x;
        row = game.pieces[piece_index].y;
        if (-1 == col || -1 == row) continue;

        p = board.get(col + row * 8);
        ptype = getType(p);
        pside = getSide(p);

        if (Empty == ptype) continue;

        // Material Bonus
        materialTotal += pgm_read_dword(&game.material_bonus[ptype][pside]) * game.options.materialBonus;

        // In-Check Penalty
        if (inCheck(p)) {
            if (White == ptype) {
                materialTotal -= ptype;
            }
            else {
                materialTotal += ptype;
            }
        }

        // Let's not encourage the King to wander to
        // the center of the board mmkay?
        if (King == ptype) {
            continue;
        }

        // Center Bonus
        centerTotal +=
            pgm_read_dword(&game.center_bonus[col][ptype][pside]) +
            pgm_read_dword(&game.center_bonus[row][ptype][pside]);

        // Proximity to opponent's King Bonus
        kloc = (White == pside) ? game.bking : game.wking;
        col_dist = (col > (kloc % 8)) ? (col - (kloc % 8)) : ((kloc % 8) - col);
        row_dist = (row > (kloc / 8)) ? (row - (kloc / 8)) : ((kloc / 8) - row);
        proximity = 14 - (col_dist + row_dist);
        if (White == pside) {
            kingTotal += proximity;
        }
        else {
            kingTotal -= proximity;
        }
    }

    kingTotal *= game.options.kingBonus;

    // Mobility Bonus
    if (gen.whites_turn) {
        mobilityTotal += static_cast<long>(gen.num_wmoves * game.options.mobilityBonus);
    }
    else {
        mobilityTotal -= static_cast<long>(gen.num_bmoves * game.options.mobilityBonus);
    }

    score = kingTotal + materialTotal + centerTotal + mobilityTotal;

    // printf(Debug4, 
    //     "evaluation: %ld = centerTotal: %ld  materialTotal: %ld  mobilityTotal: %ld\n", 
    //     score, centerTotal, materialTotal, mobilityTotal);

    return score;

}   // evaluate(...)


////////////////////////////////////////////////////////////////////////////////////////
// Evaluate all of the available moves for both sides.
// The best moves are stored in wbest and bbest.
// The callback is called for each move, implementing the visitor pattern.
// 
// This function is the top of the recursive call chain:
// 
//  choose_best_move(...)
//      add_xxxx_moves(...)
//          consider_move(...)
//              make_move(...)
//                  choose_best_move(...)
// 
// Note: Sanitized stack
void choose_best_moves(move_t &wbest, move_t &bbest, generator_t const callback)
{
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK

    //  Check for low stack space
    if (check_mem(CHOOSE)) { return; }

    // Now we can alter local variables!
    else {
        static uint32_t last_led_update;
        index_t move_count;
        move_t move = { -1, -1, 0 };
        piece_gen_t gen(move, wbest, bbest, callback, True);

        gen.num_wmoves = 0;
        gen.num_bmoves = 0;

        // Turn off the 'King in check' LED
        direct_write(DEBUG4_PIN, LOW);

        // Walk through the game.pieces[] list and evaluate the moves for each one
        for (gen.piece_index = 0; gen.piece_index < game.piece_count; gen.piece_index++) {
            if (game.supply_valid || (PLAYING != game.state)) {
                return;
            }

            if (check_serial()) {
                return;
            }
            else {
                gen.col = game.pieces[gen.piece_index].x;
                if (-1 == gen.col) { continue; }
    
                // Construct a move_t object with the starting location
                gen.row = game.pieces[gen.piece_index].y;
                gen.move.from = gen.col + gen.row * 8u;
                gen.move.to = -1;
                gen.piece = board.get(gen.move.from);
                gen.type = getType(gen.piece);
                gen.side = getSide(gen.piece);
                gen.whites_turn = gen.side; // same as White == gen.side
                gen.move.value = gen.whites_turn ? MIN_VALUE : MAX_VALUE;
    
                if (Empty == gen.type) {
                    continue;
                }
    
                // Periodically update the LED strip display and progress indicator if enabled
                if (game.options.live_update 
                // && (game.ply < game.options.max_max_ply)
                // && (game.ply <= game.options.maxply)
                ) {
                    if ((millis() - last_led_update) >= 10)
                    {
                        last_led_update = millis();
                        set_led_strip(gen.move.from);
                    }
                }

                // Keep track of the location of the Kings
                if (King == gen.type) {
                    (gen.whites_turn ? game.wking : game.bking) = gen.move.from;
                }

                // Check for move timeout (only if we're at ply level 2 or above, 
                // this happens internally in the timeout() function)
                if (timeout()) {
                    break;
                }
    
                move_count = 0;
    
                // Evaluate the moves for this Piece Type and get the highest value move
                switch (gen.type) {
                    default: printf(Always, "bad type: line %d\n", __LINE__);   break;
                    case   Pawn:    move_count = add_pawn_moves(gen);           break;
                    case Knight:    move_count = add_knight_moves(gen);         break;
                    case Bishop:    move_count = add_bishop_moves(gen);         break;
                    case   Rook:    move_count = add_rook_moves(gen);           break;
                    case  Queen:    move_count = add_queen_moves(gen);          break;
                    case   King:    
                        move_count = add_king_moves(gen);
                        // if (0 == move_count) {
                        //     if (White == gen.side && game.white_king_in_check) {
                        //         game.state = BLACK_CHECKMATE;
                        //         return;
                        //     }

                        //     if (Black == gen.side && game.black_king_in_check) {
                        //         game.state = WHITE_CHECKMATE;
                        //         return;
                        //     }
                        // }
                        break;
                }

                // Keep track of the total number of moves for this side
                (gen.whites_turn ? gen.num_wmoves : gen.num_bmoves) += move_count;
    
                // Check for alpha or beta cuttoff
                if (gen.cutoff) {
                    break;
                }
    
                // Check for move timeout if we've finished ply level 1
                if (game.timeout1) {
                    break;
                }
            }
    
        } // for each piece on both sides
    
        // See if the game is over
        if (0 == game.ply) {
            // See if we only have the two kings on either side:
            if (2 == game.piece_count) {
                game.state = STALEMATE;
            }

            if ((0 == gen.num_wmoves) && (0 == gen.num_bmoves)) {
                game.state = STALEMATE;
            }

            if ((0 == gen.num_wmoves) && game.white_king_in_check) {
                game.state = BLACK_CHECKMATE;
            }

            if ((0 == gen.num_bmoves) && game.black_king_in_check) {
                game.state = WHITE_CHECKMATE;
            }
        }
    }


}   // choose_best_moves(...)


////////////////////////////////////////////////////////////////////////////////////////
// Set the per-side options. This allows testing feature choices against each other
void set_per_side_options() {
    if (game.turn) {
        // White's turn
    }
    else {
        // Black's turn
    }
}


////////////////////////////////////////////////////////////////////////////////////////
// reset the various move tracking flags
// 
// Note: Sanitized stack
void reset_turn_flags()
{
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK
    index_t index;

    //  Check for low stack space
    if (check_mem(CHOOSE)) { return; }

    // Now we can alter local variables! 😎 

    for (index = 0; index < game.piece_count; index++) {
        if (-1 == game.pieces[index].x) { continue; }
        board.set(         game.pieces[index].x + game.pieces[index].y * 8, 
        setCheck(board.get(game.pieces[index].x + game.pieces[index].y * 8), False));
    }

    // reset the king-in-check flags
    game.white_king_in_check = False;
    game.black_king_in_check = False;

    game.last_was_en_passant = False;
    game.last_was_castle = False;
    game.timeout1 = False;
    game.timeout2 = False;
    game.last_was_pawn_promotion = False;

    game.book_supplied = False;
    game.user_supplied = False;
    game.supply_valid = False;

    set_per_side_options();

}   // reset_turn_flags()


////////////////////////////////////////////////////////////////////////////////////////
// Make the next move in the game
void take_turn()
{
    // Turn off the LED move indicators
    direct_write(DEBUG1_PIN, LOW);
    direct_write(DEBUG2_PIN, LOW);
    direct_write(DEBUG3_PIN, LOW);
    direct_write(DEBUG4_PIN, LOW);

    // See if we've hit the move limit and return if so
    if (game.move_num >= game.options.move_limit) {
        game.state = MOVE_LIMIT;
        return;
    }

    // The best white and black moves
    move_t wmove = { -1, -1, MIN_VALUE };
    move_t bmove = { -1, -1, MAX_VALUE };

    // Reset the flags for this turn
    game.stats.start_move_stats();
    game.stats.move_stats.depth = 0;

    reset_turn_flags();

    // Set the alpha and beta edges to the worst case (brute force)
    // O(N) based on whose turn it is. Math is so freakin cool..
    game.alpha = wmove.value;
    game.beta  = bmove.value;

    Bool const whites_turn = game.turn; // same as (White == game.turn) ? True : False;
    move_t move = { -1, -1, whites_turn ? MIN_VALUE : MAX_VALUE };

    // See if we have an opening book move
    check_book();

    if (game.options.shuffle_pieces) {
        game.sort_pieces(game.turn);
        game.shuffle_pieces(SHUFFLE);
    }

    // Choose the best moves for both sides
    choose_best_moves(wmove, bmove, consider_move);

    // Gather the move statistics for this turn
    game.stats.stop_move_stats();

    printf(Debug1, "\nMove #%d: ", game.move_num + 1);

    // If we have a user or a book move that's been validated then use it
    if (game.supply_valid) {
        (whites_turn ? wmove : bmove) = game.supplied;
        game.last_move = game.supplied;

        if (game.book_supplied) {
            printf(Debug1, "Book: ");
        }

        if (game.user_supplied) {
            printf(Debug1, "User: ");
        }
    }

    move = (whites_turn ? wmove : bmove);

    // Display the move that we chose * Before Modifying the Board *
    show_move(move);

    // Save the number of pieces in the game before we make the move
    // in order to see if any pieces were taken
    index_t const piece_count = game.piece_count;

    // Make the move:
    piece_gen_t gen(move, wmove, bmove, consider_move, False);
    gen.move = move;
    gen.init(board, game);

    make_move(gen);

    // Set the 'king-in-check' flags
    check_kings();

    // Check for move repetition
    if ((PLAYING == game.state) && add_to_history(gen.move)) {
        game.state = whites_turn ? WHITE_3_MOVE_REP : BLACK_3_MOVE_REP;
    }

    if (game.last_was_en_passant) {
        printf(Debug1, " - en passant capture ")
    }

    if (game.last_was_pawn_promotion) {
        printf(Debug1, " - pawn promoted ")
    }

    if (game.last_was_castle) {
        printf(Debug1, " - castling ")
    }

    printnl(Debug1);

    if (whites_turn && game.white_king_in_check) {
        game.state = BLACK_CHECKMATE;
    }

    if (!whites_turn && game.black_king_in_check) {
        game.state = WHITE_CHECKMATE;
    }

    // Toggle whose turn it is
    game.turn = !game.turn;

    // Increase the game move counter
    game.move_num++;

    // Delete any soft-deleted pieces for real
    if (piece_count != game.piece_count) {
        for (index_t i = 0; i < game.piece_count; i++) {
            if (-1 == game.pieces[i].x) {
                game.pieces[i] = game.pieces[--game.piece_count];
                break;
            }
        }
    }

}   // take_turn()


void show_game_options() {
    printf(Always, "-= MicroChess ver %d.%02d =-\n\n", VERSION_MAJOR, VERSION_MINOR);

    char str1[16];
    char str2[16];
    ftostr(MAX_VALUE, 0, str1);
    ftostr(MIN_VALUE, 0, str2);
    printf(Always, "MIN/MAX: %s to %s\n", str2, str1);

    printf(Always, "Time limit: ");
    if (0 == game.options.time_limit) {
        printf(Always, "none\n");
    }
    else {
        show_time(game.options.time_limit);
        printnl(Always);
    }

    printf(Always, "PRNG Seed: 0x%04X%04X\n",
        (game.options.seed >> 16),
        word(game.options.seed));

    printf(Always, "Plies: M: %d, N: %d, Q: %d, X: %d\n", 
        game.options.minply,
        game.options.maxply,
        game.options.max_quiescent_ply,
        game.options.max_max_ply);

    printf(Always, "Max moves: %d\n", game.options.move_limit);

    printf(Always, "Alpha-Beta: ");
    if (game.options.alpha_beta_pruning) {
        printf(Always, "y\n");
    }
    else {
        printf(Always, "n\n");
    }

    printf(Always, "Mistakes: %d%%\n", game.options.mistakes);

    printf(Always, "Integrate: ");
    if (game.options.integrate) {
        printf(Always, "y\n");
    }
    else {
        printf(Always, "n\n");
    }

    printf(Always, "Openings: ");
    if (game.options.openbook) {
        printf(Always, "y\n");
    }
    else {
        printf(Always, "n\n");
    }

    #ifdef ENA_MEM_STATS
    printf(Always, "RAM tbl: y\n");
    #else
    printf(Always, "RAM tbl: n\n");
    #endif

    printf(Always, "Shuffle: ");
    if (game.options.shuffle_pieces) {
        printf(Always, "y\n");
    }
    else {
        printf(Always, "n\n");
    }

    printf(Always, "Random: ");
    if (game.options.random) {
        printf(Always, "y\n");
    }
    else {
        printf(Always, "n\n");
    }

    printf(Always, "Skip: %d%%\n", game.options.randskip);

    // Enable random seed when program is debugged.
    // Disable random seed to reproduce issues or to profile.
    if (game.options.profiling) {
        printf(Always, "\nProfiling:\n");

        // Turn off output if we are profiling
        game.options.print_level = None;
    } 
    printnl(Always);

    randomSeed(game.options.seed);

}   // show_game_options()


////////////////////////////////////////////////////////////////////////////////////////
// Set all of the options for the game
void set_game_options()
{
    // Set game.options.profiling to True to disable output and profile the engine
    game.options.profiling = False;
    // game.options.profiling = True;

    // Set the ultimate maximum ply level (incl)
    game.options.max_max_ply = 4;

    // Set the max ply level (inclusive) for normal moves
    game.options.maxply = 2;

    // Set the minimum ply level required to complete for a turn
    game.options.minply = 1;

    // Set the percentage of moves that might be a mistake
    game.options.mistakes = 0;

    // Set whether weintegrate ply values or assume them
    game.options.integrate = False;
    // game.options.integrate = True;

    // Set game.options.random to True to use randomness in the game decisions
    // game.options.random = False;
    game.options.random = True;

    // Set the time limit per turn in milliseconds
    // game.options.time_limit = 0;     // for no time limit
    game.options.time_limit = 3000;

    // Set whether we play continuously or not
    // game.options.continuous = False;
    game.options.continuous = True;

    // Enable or disable alpha-beta pruning
    // game.options.alpha_beta_pruning = False;
    game.options.alpha_beta_pruning = True;

    // When shuffle_pieces is True we shuffle the pieces[] array before each turn
    // so that we process the current side's pieces in random order.
    // game.options.shuffle_pieces = False;
    game.options.shuffle_pieces = True;

    // Set the percentage of moves we randomly skip at ply depths > 1
    // game.options.randskip = 0;
    game.options.randskip = 75;

    // Enable or disable opening book moves
    // game.options.openbook = False;
    game.options.openbook = True;

    // Set the maximum ply level to continue if a move takes a piece
    // The quiescent search depth is based off of the max ply level
    game.options.max_quiescent_ply = min(game.options.maxply + 1, game.options.max_max_ply);

    // set the 'live update' flag
    // game.options.live_update = False;
    game.options.live_update = True;

    // game seed hash for PRN generator - default to 4 hex prime numbers
    game.options.seed = 0x232F89A3;

    // Salt the psuedo-random number generator seed if enabled:
    if (game.options.random) {
        // Add salt to the psuedo random number generator seed
        // from the physical environment
        uint8_t const pins[] = { 2, 7, 9, 10, 11, 12 };
        uint8_t const total_passes = random(23, 87);
        uint32_t some_bits = 1234567890;

        randomSeed(random(0, LONG_MAX));

        for (uint8_t pass = 0; pass < total_passes; pass++) {
            for (uint8_t pin = 0; pin < ARRAYSZ(pins); pin++) {
                pinMode(pins[pin], INPUT);
                some_bits ^= digitalRead(pins[pin]) << (analogRead(A2) % 42u);                
            }
        }
        uint8_t bits = (game.options.seed >> 11) & 0xFF;
        game.options.seed += 
            bits +
            (uint32_t(analogRead(A0)) << 24) +
            (uint32_t(analogRead(A1)) << 16) +
            (uint32_t(analogRead(A2)) << 17) +
            (uint32_t(analogRead(A3)) << 11) +
            uint32_t(analogRead(A4)) +
            uint32_t(micros());

        game.options.seed += some_bits;        
    }

}   // set_game_options()


////////////////////////////////////////////////////////////////////////////////////////
// Continually call take_turn() until we reach the end of the game.
// Display the statistics for the game and start another game.
void setup()
{
    // The baud rate we will be using
    static long constexpr baud_rate = 1000000;

    // Send out a message telling the user what baud rate to set their console to
    // using each baud rate one at a time, so that our message will be seen no
    // matter what baud rate setting they are currently set to!
    // {
    //     // The baud rates we support
    //     static uint32_t const baud_rates[] PROGMEM = {
    //         300, 600, 750, 1200, 2400, 4800, 9600, 19200, 31250, 38400,
    //         57600, 74480, 115200, 230400, 250000, 460800, 500000, 921600
    //     };

    //     // Get the value returned when nothing is waiting in the Serial output buffer
    //     int const empty_size = Serial.availableForWrite();

    //     for (index_t i = 0; i < index_t(ARRAYSZ(baud_rates)); i++) {
    //         Serial.begin(long(pgm_read_dword(&baud_rates[i])));

    //         while (!Serial) {}

    //         printf(Always, "\n%ld bps", baud_rate);

    //         // wait for the bytes to all be sent
    //         while (empty_size != Serial.availableForWrite()) {}

    //         Serial.end();
    //     }
    // }

    // Initialize the Serial output
    Serial.begin(baud_rate); while (!Serial);
    printnl(Debug1, 40);

    // Initialize the LED strip
    init_led_strip();

    // Initialize the LED indicators
    static uint8_t constexpr pins[] = { DEBUG4_PIN, DEBUG1_PIN, DEBUG2_PIN, DEBUG3_PIN };
    for (uint8_t pin : pins) {
        pinMode(pin, OUTPUT);
        direct_write(pin, HIGH);
        delay(200);
        direct_write(pin,  LOW);
    }

    // Initialize the continuous game statistics
    uint32_t state_totals[6] = { 0, 0, 0, 0, 0, 0 };
    uint32_t white_wins = 0;
    uint32_t black_wins = 0;

    set_game_options();
    show_game_options();

    // Play a game until it is over
    do {
        set_game_options();

        // set up a particular game board to test:
        // board.clear();
        // board.set(7 + 0 * 8u, King);
        // board.set(3 + 3 * 8u, Queen);
        // board.set(4 + 4 * 8u, King | Side);
        // board.set(0 + 7 * 8u, Queen | Side);
        // game.init();

        // initialize the board and the game:
         board.init();
         game.init();

        // Shuffle our pieces really well so we evaluate them in a random order
        game.sort_pieces(game.turn);
        game.shuffle_pieces(SHUFFLE);

        show_check_status();
        show();

        game.stats.start_game_stats();

        do {
            take_turn();
            if (PLAYING == game.state) {
                show_check_status();
                show();
            }

            if (!game.compare_pieces_to_board(board)) {
                // printf(Debug1, "Error: game.pieces[] contents are different from the board contents\n");
                game.set_pieces_from_board(board);
            }

        } while (PLAYING == game.state);

        // Calculate the game statistics
        game.stats.stop_game_stats();

        // Return the output to normal
        game.options.print_level = Debug1;

        // Display the end game reason
        switch (game.state) {
            case STALEMATE:         printf(Debug1, "Stalemate\n\n");                                        break;
            case WHITE_CHECKMATE:   printf(Debug1, "Checkmate! White wins!\n\n");                           break;
            case BLACK_CHECKMATE:   printf(Debug1, "Checkmate! Black wins!\n\n");                           break;
            case WHITE_3_MOVE_REP:  printf(Debug1, "%d-move repetition! Black wins!\n\n", MAX_REPS);        break;
            case BLACK_3_MOVE_REP:  printf(Debug1, "%d-move repetition! White wins!\n\n", MAX_REPS);        break;
            case MOVE_LIMIT:        printf(Debug1, "%d-move limit reached!\n\n", game.options.move_limit);  break;
            default: 
            case PLAYING:           break;
        }

        // Show the final board
        show();

        // Show the game move and game counts and time statistics
        show_stats();

        // Keep track of the game end reasons when playing continuously
        state_totals[game.state - 1]++;
        char str[16] = "";

        printf(Debug1, "         Stalemate   White Checkmate   Black Checkmate  White %d-Move Rep  Black %d-Move Rep        Move Limit\n", 
            MAX_REPS, MAX_REPS);
        
        ftostr(state_totals[       STALEMATE - 1], 0, str);
        printf(Debug1, "%18s", str);
        ftostr(state_totals[ WHITE_CHECKMATE - 1], 0, str);
        printf(Debug1, "%18s", str);
        ftostr(state_totals[ BLACK_CHECKMATE - 1], 0, str);
        printf(Debug1, "%18s", str);
        ftostr(state_totals[WHITE_3_MOVE_REP - 1], 0, str);
        printf(Debug1, "%18s", str);
        ftostr(state_totals[BLACK_3_MOVE_REP - 1], 0, str);
        printf(Debug1, "%18s", str);
        ftostr(state_totals[     MOVE_LIMIT - 1], 0, str);
        printf(Debug1, "%18s", str);

        printnl(Debug1);

        switch (game.state) {
            default:
            case PLAYING:
            case STALEMATE:
            case MOVE_LIMIT:
                break;

            case WHITE_CHECKMATE:
            case BLACK_3_MOVE_REP:
                white_wins++;
                break;

            case BLACK_CHECKMATE:
            case WHITE_3_MOVE_REP:
                black_wins++;
                break;
        }

        printf(Debug1, "   White wins: %3ld   Black wins: %3ld\n\n", white_wins, black_wins)
    
        if (game.options.profiling) {
            // Return to no output when profiling
            game.options.print_level = None;
        }

    } while (game.options.continuous);

}   // setup()


void loop() {}


/// Board display functions

void show_header(Bool const dev) {
    if (Debug1 >= game.options.print_level) {
        Serial.write((game.piece_count <= END_COUNT) ? '+' : ' ');
        char const base = (dev ? '0' : 'A');
        for (index_t i = 0; i < 8; i++) {
            printrep(Debug1, ' ', 2);
            Serial.write(base + i);
        }
    }

} // show_header(Bool const dev)


static char const icons[] PROGMEM = "pnbrqkPNBRQK";


void show_panel1(index_t const y) {
    index_t constexpr row_offset = 0;
    index_t constexpr col_offset = 5;
    Piece piece, ptype;
    char str[16];
    Color pside;

    printrep(Debug1, ' ', col_offset);

    // Display the extra status info on certain lines:
    switch (y) {
        // display the last move made if available
        case row_offset + 0:
            if (game.last_move.from != -1 && game.last_move.to != -1) {
                printf(Debug1, "Last Move: %c%c to %c%c", 
                    (game.last_move.from % 8) + 'A', 
                    '8' - (game.last_move.from / 8), 
                    (game.last_move.to   % 8) + 'A', 
                    '8' - (game.last_move.to   / 8) );
            }
            break;

        // Display the time spent on the last move
        case row_offset + 1:
            if (0 == game.stats.move_stats.duration()) break;
            if (0 != game.stats.move_stats.counter()) {
                ftostr(game.stats.move_stats.counter(), 0, str);
                printf(Debug1, "%s moves in ", str);

                show_time(game.stats.move_stats.duration());

                ftostr(game.stats.move_stats.moveps(), 2, str);
                printf(Debug1, " (%s moves/sec)", str);
            }
            break;

        // Display the total game time so far
        case row_offset + 2:
            if (game.move_num > 0) {
                printf(Debug1, "Game time: ");
                show_time(game.stats.game_stats.duration());
            }
            break;

        // Display the max ply depth we were able to reach
        case row_offset + 3:
            if (game.move_num > 0) {
                printf(Debug1, "Max ply depth reached: %d", game.stats.move_stats.depth);
            }
            break;

        // Display the check state for White
        case row_offset + 4:
            if (game.white_king_in_check) {
                show_check(White);
            }
            break;

        // Display the check state for Black
        case row_offset + 5:
            if (game.black_king_in_check) {
                show_check(Black);
            }
            break;

        // Display the pieces taken by White
        case row_offset + 6:
            printf(Debug1, "Taken 1: ");
            for (index_t i = 0; i < game.white_taken_count; i++) {
                piece = game.taken_by_white[i].piece;
                ptype = getType(piece);
                pside = getSide(piece);
                printf(Debug1, "%c ", pgm_read_byte(&icons[(pside * 6) + ptype - 1]));
            }
            break;

        // Display the pieces taken by Black
        case row_offset + 7:
            printf(Debug1, "Taken 2: ");
            for (index_t i = 0; i < game.black_taken_count; i++) {
                piece = game.taken_by_black[i].piece;
                ptype = getType(piece);
                pside = getSide(piece);
                printf(Debug1, "%c ", pgm_read_byte(&icons[(pside * 6) + ptype - 1]));
            }
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// display the game board
void show()
{
    static Bool constexpr dev = True;

    if (game.options.print_level < Debug1) { return; }

    show_header(!dev);
    printnl(Debug1);

    for (unsigned char y = 0; y < 8u; ++y) {
        printf(Debug1, "%c ", dev ? ('0' + y) : ('8' - y));
        for (unsigned char x = 0; x < 8u; ++x) {
            Piece const piece = board.get(y * 8u + x);
            printf(Debug1, " %c ", 
                isEmpty(piece) ? ((y ^ x) & 1 ? '*' : '.') :
                pgm_read_byte(&icons[((getSide(piece) * 6) + getType(piece) - 1)]));
        }

        printf(Debug1, " %c ", !dev ? ('0' + y) : ('8' - y));

        show_panel1(y);

        printnl(Debug1);
    }

    show_header(dev);

    char str_score[16] = "";
    ftostr(game.last_move.value, 0, str_score);
    printrep(Debug1, ' ', 9);
    printf(Debug1, "Board value: %s ", str_score);
    if (0 != game.last_move.value) {
        show_side(game.last_move.value > 0);
        printf(Debug1, "'s favor");
    }

    printnl(Debug1, 2);

    set_led_strip();

}   // show()