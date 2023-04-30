/**
 * ArduinoChess.ino
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * version 1.0.0
 * written March 2023 - Trent M. Wyatt
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * TODO:
 * 
 *  [ ] 
 *  [+] Finish enabling the en-passant pawn move generation.
 *  [+] Add a function to display times over 1000 ms as minutes, seconds, and ms
 *  [+] Change make_move(...) to return MIN_VALUE only if the ply level is 0
 *      so we don't actually choose it. Otherwise consider taking the King
 *      worth MAX_VALUE.
 * 
 *  [+] Add flags to the game to indicate whether each side is human or not
 *  [+] Add the display of the full game time so far to each show() update
 * 
 * 
 * 
 *  [ ] Change to use true printf, stdin, stdout and stderr so we don't copy the fmt buffer
 *  [ ] Add tests that prove that the same moves are chosen when alpha-beta
 *      pruning is enabled that are chosen when it is disabled and running in brute force?
 *  [ ] Add optional use of I2C serial RAM to create transposition tables for
 *      moves that have already been searced during this turn.
 *  [ ] Enhance to pause the game when a newline is received
 *  [ ] Enhance the game to allow moves to be entered by a human via the serial port
 *  [ ] Change to have two sets of option_t in the game, one for each player in order to test
 *      option settings against each other
 *  [ ] Change all of the white and black options, game states and stats into two value arrays
 *  [ ] add reading and writing of FEN notation.
 *  [ ] 
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 */
#include <Arduino.h>
#include "MicroChess.h"

////////////////////////////////////////////////////////////////////////////////////////
// The game board
board_t board;

////////////////////////////////////////////////////////////////////////////////////////
// The currently running game states and flags
game_t game;


// #define SHOW1


////////////////////////////////////////////////////////////////////////////////////////
// Consider a move against the best white move or the best black move
// depending on the color of the piece and set this as the best move
// if it has a higher value (or an equal value when we're using random)
// 
// returns True if the move is the new best move, False otherwise
// 
// Note: Sanitized stack
// 
Bool consider_move(piece_gen_t &gen)
{
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK
    #ifdef SHOW1
    Bool repeat;
    #endif
    Bool better;

    #ifdef ENA_MEM_STATS
    game.freemem[2][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) {
        #ifdef SHOW1
        if (0 == game.ply) {
            printf(Debug1, "** ");
            show_move(gen.move, True);
            printf(Debug1, " returning early at line %d\n", __LINE__);
        }
        #endif
        return False;
    }


    // Check for alpha-beta cutoff
    if (gen.cutoff) {
        #ifdef SHOW1
        if (0 == game.ply) {
            printf(Debug1, "** ");
            show_move(gen.move);
            printf(Debug1, " returning early at line %d\n", __LINE__);
        }
        #endif
        return False;
    }

    // Recursively generate the move's value
    make_move(gen);

    #ifdef SHOW1
    repeat = False;
    #endif

    better = False;

    // Penalize the move if it would cause us to lose by move repetition
    if (would_repeat(gen.move)) {
        gen.move.value = gen.whites_turn ? MIN_VALUE : MAX_VALUE;
        #ifdef SHOW1
        repeat = True;
        #endif
    }

    if (gen.whites_turn) {
        if (-1 == gen.wbest.to) {
            gen.wbest = gen.move;
            better = True;
        }
    }
    else {
        if (-1 == gen.bbest.to) {
            gen.bbest = gen.move;
            better = True;
        }
    }

    // Check to see if this move is equal to the best move we've seen so far
    if (!better) {
        if (gen.whites_turn) {
            if ((gen.move.value == gen.wbest.value) && random(2)) {
                gen.wbest = gen.move;
                better = True;
            }
            else if (gen.move.value > gen.wbest.value) {
                gen.wbest = gen.move;
                better = True;
            }
        }
        else {
            if ((gen.move.value == gen.bbest.value) && random(2)) {
                gen.bbest = gen.move;
                better = True;
            }
            else if (gen.move.value < gen.bbest.value) {
                gen.bbest = gen.move;
                better = True;
            }
        }
    }

    // Debugging
    #ifdef SHOW1
    if (0 == game.ply) {
        show_move(gen.move, True);
        printf(Debug1, "%s%s\n", (repeat ? "+" : " "), (better ? "*" : " "));
    }
    #endif

    return better;

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
// 
long make_move(piece_gen_t & gen)
{
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK
    struct local_t {
        local_t() {}
        ~local_t() {}

        // The attributes for the destination location
        uint8_t     to_col : 3,
                    to_row : 3,
        // The attributes for any possible piece being taken at the destination location
                        op : 8,
                     otype : 3,
                     oside : 1;
    } vars;
    move_t history[MAX_REPS * 2 - 1];
    index_t hist_count;
    index_t wking;
    index_t bking;
    index_t white_taken_count;
    index_t black_taken_count;
    Bool last_was_pawn_promotion;
    Bool last_was_en_passant;
    Bool last_was_castle;
    move_t last_move;
    uint32_t move_count;
    Bool white_king_in_check;
    Bool black_king_in_check;
    index_t taken_index;
    index_t captured;
    Piece captured_piece;
    Piece place_piece;
    index_t castly_rook;
    index_t dist;
    index_t board_rook;
    Bool quiescent;
    move_t wbest;
    move_t bbest;
    static move_t last_led_update;
    index_t captured_col;
    index_t captured_row;
    index_t rook;

    //  Check for low stack space
    if (check_mem()) { return gen.whites_turn ? MIN_VALUE : MAX_VALUE; }

    // Now we can alter local variables! 😎 

    #ifdef ENA_MEM_STATS
    game.freemem[3][game.ply].mem = freeMemory();
    #endif

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
    wking = game.wking;
    bking = game.bking;

    // Save the current number of taken pieces
    white_taken_count = game.white_taken_count;
    black_taken_count = game.black_taken_count;

    // Save the current last move and move flags
    last_was_pawn_promotion = game.last_was_pawn_promotion;
    last_was_en_passant = game.last_was_en_passant;
    last_was_castle = game.last_was_castle;
    last_move = game.last_move;

    if (gen.evaluating) {
        memmove(history, game.history, sizeof(history));

        // Track the number of moves considered so far
        game.stats.inc_moves_count();

        // Track the max number of moves generated during a turn
        move_count = game.stats.move_count_so_far();

        if (gen.side == game.turn) {
            if (move_count > game.stats.max_moves) {
                game.stats.max_moves = move_count;
            }
        }
    }

    // See if this move places the opponent King in check
    // (attempts to take the opponent's King)
    if (King == vars.otype && (gen.side != vars.oside)) {
        if (gen.whites_turn) {
            game.black_king_in_check = True;
            gen.move.value = MAX_VALUE;
            if (0 == game.ply) {
                // return MIN_VALUE;
                return gen.move.value;
            }
        }
        else {
            game.white_king_in_check = True;
            gen.move.value = MIN_VALUE;
            if (0 == game.ply) {
                // return MAX_VALUE;
                return gen.move.value;
            }
        }
    }

    // Save the state of whether or not the kings are in check.
    // We do this AFTER we've had a chance to set the 'king-in-check'
    // flags above so that this move leaves the flags behind after evaluation
    white_king_in_check = game.white_king_in_check;
    black_king_in_check = game.black_king_in_check;


    /// Step 2: Identify any piece being captured and remove it if so.
    // 
    // * NOTE BELOW *
    // Once any changes have been made to the board or game state we MUST NOT return
    // without passing through the "if (gen.evaluating) { ... }" restoration logic.

    // The game.pieces[] index being captured (if any, -1 if none)
    taken_index = -1;

    // The board index being captured (if any, -1 if none)
    captured = -1;
    captured_piece = Empty;

    // Check for en-passant capture
    if (Pawn == gen.type && isEmpty(vars.otype) && gen.col != vars.to_col) {
        game.last_was_en_passant = True;
        captured = vars.to_col + gen.row * 8;
        captured_piece = board.get(captured);
    }
    else {
        // See if the destination is not empty and not a piece on our side.
        // i.e. an opponent's piece.
        if (Empty != vars.otype && gen.side != vars.oside) {
            captured = gen.move.to;
            captured_piece = board.get(captured);
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
            game.taken_by_white[game.white_taken_count++] = captured_piece;
        }
        else {
            game.taken_by_black[game.black_taken_count++] = captured_piece;
        }
    }


    /// Step 3: Place the piece being moved at the destination

    // Set the 'moved' flag on the piece that we place on the board
    place_piece = setMoved(gen.piece, True);

    // Promote a pawn to a queen if it reaches the back row
    if (Pawn == gen.type && (vars.to_row == (gen.whites_turn ? index_t(0) : index_t(7)))) {
        place_piece = setType(place_piece, Queen);
        game.last_was_pawn_promotion = True;
    }

    // Move the piece to the destination on the board
    board.set(gen.move.from, Empty);
    board.set(gen.move.to, place_piece);

    // Update the piece list to reflect the piece's new location
    game.pieces[gen.piece_index] = { index_t(vars.to_col), index_t(vars.to_row) };

    // Check for castling
    castly_rook = -1;

    // If the piece being moved is a King
    if (King == gen.type) {
        // if (White == gen.side) {
        //     game.wking = gen.move.to;
        // }
        // else {
        //     game.bking = gen.move.to;
        // }

        // Get the horizontal distance the king is moving
        dist = abs(vars.to_col - (gen.move.from % 8));

        // See if it is a castling move
        if (dist >= 2) {
            // see which side we're castling on
            if (2 == dist) {
                // Castle on the King's side
                board_rook = 7 + gen.row * 8;
                castly_rook = game.find_piece(board_rook);
                board.set(board_rook, setMoved(board.get(board_rook), True));
                game.pieces[castly_rook].x = 5;
                game.last_was_castle = True;
            }
            else if (3 == dist) {
                // Castle on the Queen's side
                board_rook = 0 + gen.row * 8;
                castly_rook = game.find_piece(board_rook);
                board.set(board_rook, setMoved(board.get(board_rook), True));
                game.pieces[castly_rook].x = 3;
                game.last_was_castle = True;
            }
            else {
                // error
                printf(Debug1, "Error: Invalid King move at line %d\n", __LINE__);
                while ((true)) {}
            }
        }
    }


    // Step 4: Evaluate the board score after making the move

    // get the value of the current board
    gen.move.value = evaluate();

    // Control the percentage of moves that the engine makes a mistake on
    if (0 != game.options.mistakes) {
        if (random(100) <= game.options.mistakes) {
            gen.move.value -= gen.whites_turn ? +5000 : -5000;
        }
    }

    game.last_value = gen.move.value;

    // set our move as the last move
    game.last_move = gen.move;

    // ------------------------------------------------------------------------------------------
    // The move has been made and we have the value for the updated board.
    // Recursively look-ahead and accumulatively update the value here.
    // This is known as the Minimax (Maximin) algorithm.
    // 
    // Minimax Algorithm Pseudo-code:
    // 
    // function minimax(node, depth, maximizingPlayer) is
    //     if depth = 0 or node is a terminal node then
    //         return the heuristic value of node
    //     if maximizingPlayer then
    //         value := −∞
    //         for each child of node do
    //             value := max(value, minimax(child, depth − 1, FALSE))
    //         return value
    //     else (* minimizing player *)
    //         value := +∞
    //         for each child of node do
    //             value := min(value, minimax(child, depth − 1, TRUE))
    //         return value
    // 
    // (* Initial call *)
    // minimax(origin, depth, TRUE)
    // 
    // ------------------------------------------------------------------------------------------
    // During move generation, keep track of our best move and our opponents best move and
    // don't follow move paths that have less value than what we know our opponents's best
    // move already is.
    // This is known as the alpha-bea heuristic.
    // 
    // Minimax Algorithm with Alpha-Beta Heuristic Pseudo-code:
    // 
    // function alphabeta(node, depth, α, β, maximizingPlayer) is
    //     if depth = 0 or node is a terminal node then
    //         return the heuristic value of node
    //     if maximizingPlayer then
    //         value := −∞
    //         for each child of node do
    //             value := max(value, alphabeta(child, depth − 1, α, β, FALSE))
    //             if value > β then
    //                 break (* β cutoff *)
    //             α := max(α, value)
    //         return value
    //     else
    //         value := +∞
    //         for each child of node do
    //             value := min(value, alphabeta(child, depth − 1, α, β, TRUE))
    //             if value < α then
    //                 break (* α cutoff *)
    //             β := min(β, value)
    //         return value
    // 
    // (* Initial call *)
    // alphabeta(origin, depth, −∞, +∞, TRUE)
    // ------------------------------------------------------------------------------------------

    // Before we continue we check the evaluating flag to see if it is False, meaning that we are making this move for real.
    // There's no need to explore future plies if we've already made our mind up! We only recurse when we are evaluating 
    // (gen.evaluating == True)
    if (gen.evaluating) {
        // flag indicating whether we are traversing into quiescent moves
        quiescent = ((-1 != captured) && (game.ply < (game.options.max_quiescent_ply)) && (game.ply < game.options.max_max_ply));

        if ((/*(game.ply < game.options.maxply) ||*/ quiescent)) {
            timeout();

            if (!game.last_was_timeout1 || (game.ply < 1)) {
                // Indicate whether we are on a quiescent search or not
                if (quiescent) {
                    show_quiescent_search();
                }
                else {
                    digitalWrite(DEBUG2_PIN, LOW);
                }



/*

                if depth = 0 or node is a terminal node then


                    return the heuristic value of node
                if maximizingPlayer then
                    value := −∞
                    for each child of node do
                        value := max(value, alphabeta(child, depth − 1, α, β, FALSE))
                        if value > β then
                            break (* β cutoff *)
                        α := max(α, value)
                    return value
                else
                    value := +∞
                    for each child of node do
                        value := min(value, alphabeta(child, depth − 1, α, β, TRUE))
                        if value < α then
                            break (* α cutoff *)
                        β := min(β, value)
                    return value


*/



                // Explore The Future! (plies)
                game.ply++;
                game.turn = !game.turn;

                // Keep track of the deepest ply level we go to
                if (game.ply > game.stats.move_stats.maxply) {
                    game.stats.move_stats.maxply = game.ply;
                }

                reset_turn_flags();

                // Create variables to hold the best response moves
                wbest = { -1, -1, MIN_VALUE };
                bbest = { -1, -1, MAX_VALUE };

                choose_best_moves(wbest, bbest, consider_move);

                // Get our opponent's best response
                if (0) {
                    if (gen.whites_turn) {
                        // The awesome, amazing, culling magic of alpha-beta pruning
                        if (game.options.alpha_beta_pruning) {
                            gen.move.value = max(gen.move.value, wbest.value);
                            if (gen.move.value > game.beta) {
                                gen.cutoff = True;
                            }
                            else {
                                game.alpha = max(game.alpha, gen.move.value);
                            }
                        }
                        else {
                            if (-1 != wbest.from && -1 != wbest.to) {
                                gen.move.value = wbest.value;
                            }

                            if (game.white_king_in_check && !white_king_in_check) {
                                gen.move.value = MIN_VALUE;
                            }
                        }
                    }
                    else {
                        // The awesome, amazing, culling magic of alpha-beta pruning
                        if (game.options.alpha_beta_pruning) {
                            gen.move.value = min(gen.move.value, bbest.value);
                            if (gen.move.value < game.alpha) {
                                gen.cutoff = True;
                            }
                            else {
                                game.beta = min(game.beta, gen.move.value);
                            }
                        }
                        else {
                            if (-1 != bbest.from && -1 != bbest.to) {
                                gen.move.value = bbest.value;
                            }

                            if (game.black_king_in_check && !black_king_in_check) {
                                gen.move.value = MAX_VALUE;
                            }
                        }
                    }
                }

                game.ply--;
                game.turn = !game.turn;
            }
        }
    }

    // Periodically update the LED strip display and progress indicator if enabled
    if (game.options.live_update) {
        if ((game.ply > 0) && (game.ply < game.options.max_max_ply)) {
            if (last_led_update.from != gen.move.from || last_led_update.to != gen.move.to) {
                last_led_update = gen.move;
                set_led_strip(gen.move.from);
            }
        }
    }


    // Step 5: If we are just considering the move then put everything back

    if (gen.evaluating) {
        if (-1 == captured) {
            board.set(gen.move.to, vars.op);
        } else {
            captured_col = captured % 8;
            captured_row = captured / 8;

            // restore the captured board changes and
            // set it's "in-check" flag
            captured_piece = setCheck(captured_piece, True);
            board.set(captured, captured_piece);

            // restore the captured piece list changes
            game.pieces[taken_index] = { captured_col, captured_row };
        }

        // restore the taken pieces list changes
        game.white_taken_count = white_taken_count;
        game.black_taken_count = black_taken_count;

        // restore the changes made to the moves history
        memmove(game.history, history, sizeof(history));
        game.hist_count = hist_count;

        // restore the moved piece board changes
        board.set(gen.move.from, gen.piece);

        // restore the moved piece pieces list changes
        game.pieces[gen.piece_index] = { index_t(gen.col), index_t(gen.row) };

        // restore the last move made
        game.last_move = last_move;

        // restore the en passant
        game.last_was_en_passant = last_was_en_passant;
        game.last_was_castle = last_was_castle;
        game.last_was_pawn_promotion = last_was_pawn_promotion;

        game.white_king_in_check = white_king_in_check;
        game.black_king_in_check = black_king_in_check;

        // restore the king's locations
        game.wking = wking;
        game.bking = bking;

        // restore any rook moved during a castle move
        game.last_was_castle = last_was_castle;
        if (-1 != castly_rook) {
            if (game.pieces[castly_rook].x == 3) {
                game.pieces[castly_rook].x = 0;
            }
            else {
                game.pieces[castly_rook].x = 7;
            }

            rook = game.pieces[castly_rook].x + game.pieces[castly_rook].y * 8;
            board.set(rook, setMoved(board.get(rook), False));
        }
    }

    return gen.move.value;

}   // make_move(piece_gen_t & gen)


////////////////////////////////////////////////////////////////////////////////////////
// Evaluate the identity (score) of the board state.
// Positive scores indicate an advantage for white and
// Negative scores indicate an advantage for black.
// Uses pre-computed material bonus tables for speed.
// 
// returns the score/value of the current board
long evaluate() 
{
    // calculate the value of the board
    long materialTotal = 0L;
    long mobilityTotal = 0L;
    long centerTotal = 0L;
    long kingTotal = 0L;
    long score = 0L;

    for (index_t piece_index = 0; piece_index < game.piece_count; piece_index++) {
        index_t const col = game.pieces[piece_index].x;
        index_t const row = game.pieces[piece_index].y;
        if (-1 == col || -1 == row) continue;

        Piece   const p = board.get(col + row * 8);
        Piece   const ptype = getType(p);
        Color   const pside = getSide(p);

        if (Empty == ptype) continue;

        // material bonus
        if ((true)) {
            materialTotal += pgm_read_dword(&game.material_bonus[ptype][pside]) * game.options.materialBonus;
        }

        // Let's not encourage the King to wander to
        // the center of the board mmkay?
        if (King == ptype) {
            continue;
        }

        // center bonus
        centerTotal +=
            pgm_read_dword(&game.center_bonus[col][ptype][pside]) +
            pgm_read_dword(&game.center_bonus[row][ptype][pside]);

        // proximity to opponent's King
        index_t const kloc = (White == pside) ? game.bking : game.wking;
        index_t const col_dist = (col > (kloc % 8)) ? (col - (kloc % 8)) : ((kloc % 8) - col);
        index_t const row_dist = (row > (kloc / 8)) ? (row - (kloc / 8)) : ((kloc / 8) - row);
        index_t const proxomity = 14 - (col_dist + row_dist);
        if (White == pside) {
            kingTotal += proxomity;
        }
        else {
            kingTotal -= proxomity;
        }
    }

    // The score or 'identity property' of the board can include extra points for
    // how many total moves (mobility) the remaining pieces can make
    // if (filter & mobility) {
    //     long sideFactor = (Black == side) ? -1 : 1;
    //     mobilityTotal += static_cast<long>(game.move_count1 * mobilityBonus * sideFactor);
    //     mobilityTotal -= static_cast<long>(game.move_count2 * mobilityBonus * sideFactor);
    // }

    kingTotal *= game.options.kingBonus;

    score = kingTotal + materialTotal + centerTotal + mobilityTotal;

    // printf(Debug4, 
    //     "evaluation: %ld = centerTotal: %ld  materialTotal: %ld  mobilityTotal: %ld\n", 
    //     score, centerTotal, materialTotal, mobilityTotal);

    return score;

}   // evaluate()


////////////////////////////////////////////////////////////////////////////////////////
// Sort the game.pieces[] array by player side
void sort_and_shuffle(Color const side, index_t const shuffle_count = 16)
{
    if (White == side) {
        // lambda comparator to sort game.pieces[] by White and then Black
        auto compare = [](const void *a, const void *b) -> int {
            point_t const piece_a = *((point_t*) a);
            point_t const piece_b = *((point_t*) b);
            Color   const side_a = getSide(board.get(piece_a.x + piece_a.y * 8));
            Color   const side_b = getSide(board.get(piece_b.x + piece_b.y * 8));
            return (side_a == side_b) ? 0 : ((side_a < side_b) ? +1 : -1);
        };

        qsort(game.pieces, game.piece_count, sizeof(point_t), compare);
    }
    else {
        // lambda comparator to sort game.pieces[] by Black and then White
        auto compare = [](const void *a, const void *b) -> int {
            point_t const piece_a = *((point_t*) a);
            point_t const piece_b = *((point_t*) b);
            Color   const side_a = getSide(board.get(piece_a.x + piece_a.y * 8));
            Color   const side_b = getSide(board.get(piece_b.x + piece_b.y * 8));
            return (side_a == side_b) ? 0 : ((side_a > side_b) ? +1 : -1);
        };

        qsort(game.pieces, game.piece_count, sizeof(point_t), compare);
    }

    // Now count the number of pieces at the top on the same side and shuffle them
    index_t count = 0;
    for (count = 0; (count + 1) < game.piece_count; count++) {
        index_t const index1 = game.pieces[count].x + game.pieces[count].y * 8;
        index_t const index2 = game.pieces[count + 1].x + game.pieces[count + 1].y * 8;
        if (getSide(board.get(index1)) != getSide(board.get(index2))) {
            break;
        }
    }

    // Shuffle the pieces
    if (count > 1) {
        for (index_t i = 0; i < shuffle_count; i++) {
            index_t r1 = random(count);
            index_t r2 = random(count);
            if (r1 == r2) { continue; }
            point_t const tmp = game.pieces[r1];
            game.pieces[r1] = game.pieces[r2];
            game.pieces[r2] = tmp;
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////
// Evaluate all of the available moves for the specified side.
// The best move is stored in best.
// The callback is called for each move.
// 
// This function is the top of the recursive call chain:
// 
//  choose_best_move(...)
//      add_xxxx_moves(...)
//          consider_move(...)
//              make_move(...)
//                  choose_best_move(...)
// 
void choose_best_moves(move_t &wbest, move_t &bbest, generator_t const callback)
{
    #ifdef ENA_MEM_STATS
    game.freemem[0][game.ply].mem = freeMemory();
    #endif

    //  Check for low stack space
    if (check_mem()) { return; }

    move_t dummy = { -1, -1, 0 };
    piece_gen_t gen(dummy, wbest, bbest, callback, True);
    gen.num_wmoves = 0;
    gen.num_bmoves = 0;

    // Walk through the pieces list and evaluate the moves for each piece
    for (gen.piece_index = 0; gen.piece_index < game.piece_count; gen.piece_index++) {
        gen.col = game.pieces[gen.piece_index].x;
        if (-1 == gen.col) { continue; }

        // Construct a move_t object with the starting location
        gen.row = game.pieces[gen.piece_index].y;
        gen.move.from = gen.col + gen.row * 8;
        gen.move.to = -1;
        gen.piece = board.get(gen.move.from);
        gen.type = getType(gen.piece);
        gen.side = getSide(gen.piece);
        gen.whites_turn = White == gen.side;
        gen.move.value = gen.whites_turn ? MIN_VALUE : MAX_VALUE;

        if (Empty == gen.type) {
            continue;
        }

        // Keep track of the location of the Kings
        if (King == gen.type) {
            if (gen.whites_turn) {
                game.wking = gen.move.from;
            }
            else {
                game.bking = gen.move.from;
            }
        }

        // Check for move timeout (only if we're at ply level 2 or above, 
        // this happens internally in the timeout() function)
        if (timeout()) {
            break;
        }

        index_t &num_moves = (gen.whites_turn ? gen.num_wmoves : gen.num_bmoves);

        // Evaluate the moves for this Piece Type and get the highest value move
        switch (gen.type) {
            case   Pawn:    num_moves += add_pawn_moves(gen);       break;
            case Knight:    num_moves += add_knight_moves(gen);     break;
            case Bishop:    num_moves += add_bishop_moves(gen);     break;
            case   Rook:    num_moves += add_rook_moves(gen);       break;
            case  Queen:    num_moves += add_queen_moves(gen);      break;
            case   King:    num_moves += add_king_moves(gen);       break;

            default:
                printf(Debug1, "error: invalid type = %d at line %d\n", gen.type, __LINE__);
                game.options.print_level = Debug1;
                show();
                game.stats.stop_game_stats();
                show_stats();
                while ((1)) {}
                break;
        }

        // Check for alpha or beta cuttoff
        if (gen.cutoff) {
            break;
        }

        // Check for move timeout if we've finished ply level 1
        if (game.last_was_timeout1 && (game.ply > 1)) {
            break;
        }

    } // for each piece on both sides

    // See if the game is over
    if (0 == game.ply) {
        if ((0 == gen.num_wmoves) && (0 == gen.num_bmoves)) {
            game.state = STALEMATE;
        }
        else if (0 == gen.num_wmoves) {
            game.state = BLACK_CHECKMATE;
        }
        else if (0 == gen.num_bmoves) {
            game.state = WHITE_CHECKMATE;
        }
    }

}   // choose_best_moves(...)


////////////////////////////////////////////////////////////////////////////////////////
// reset the various move tracking flags
void reset_turn_flags() 
{
    for (point_t const &spot : game.pieces) {
        if (-1 == spot.x) continue;
        board.set(spot.x + spot.y * 8, setCheck(board.get(spot.x + spot.y * 8), False));
    }

    // Reset the 'ply completed' flags
    game.complete = 0;

    // reset the king-in-check flags
    game.white_king_in_check = False;
    game.black_king_in_check = False;

    game.last_was_en_passant = False;
    game.last_was_castle = False;
    game.last_was_timeout1 = False;
    game.last_was_timeout2 = False;
    game.last_was_pawn_promotion = False;

    // Set the alpha and beta edges to the worst case (brute force)
    // O(N) based on whose turn it is. Math is so freakin cool..
    // Also make any changes to the options that we want the two sides to have.
    if (White == game.turn) {
        game.alpha = MIN_VALUE;
        game.beta  = MAX_VALUE;
    }
    else {
        game.alpha = MAX_VALUE;
        game.beta  = MIN_VALUE;
    }

}   // reset_move_flags()


////////////////////////////////////////////////////////////////////////////////////////
// Make the next move in the game
// 
void take_turn()
{
    // Turn off the LED move indicators
    digitalWrite(DEBUG1_PIN, LOW);
    digitalWrite(DEBUG2_PIN, LOW);
    digitalWrite(DEBUG3_PIN, LOW);

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
    game.stats.move_stats.maxply = 0;

    reset_turn_flags();

    // Choose the best moves for both sides
    if (game.options.sort_pieces) {
        sort_and_shuffle(game.turn);
    }
    choose_best_moves(wmove, bmove, consider_move);

    // Gather the move statistics for this turn
    game.stats.stop_move_stats();
    game.last_moves_evaluated = game.stats.move_stats.counter();

    // Display the move that we chose * Before Modifying the Board *
    printf(Debug1, "\nMove #%d: ", game.move_num + 1);

    Bool const whites_turn = (White == game.turn) ? True : False;

    if (whites_turn) {
        show_move(wmove);
    }
    else {
        show_move(bmove);
    }

    // Save the number of pieces in the game before we make the move
    // in order to see if any pieces were taken
    index_t const piece_count = game.piece_count;

    // Make the move:
    move_t dummy;
    if (whites_turn) {
        piece_gen_t gen(wmove, dummy, dummy, consider_move, False);

        gen.piece_index = game.find_piece(wmove.from);

        gen.col = wmove.from % 8;
        gen.row = wmove.from / 8;

        // Construct a move_t object with the starting location
        gen.piece = board.get(gen.col + gen.row * 8);
        gen.type = getType(gen.piece);
        gen.side = getSide(gen.piece);
        gen.whites_turn = True;
        make_move(gen);

        // Check for move repetition
        if ((PLAYING == game.state) && add_to_history(wmove)) {
            game.state = WHITE_3_MOVE_REP;
        }
    }
    else {
        piece_gen_t gen(bmove, dummy, dummy, consider_move, False);

        gen.piece_index = game.find_piece(bmove.from);

        gen.col = bmove.from % 8;
        gen.row = bmove.from / 8;

        // Construct a move_t object with the starting location
        gen.piece = board.get(gen.col + gen.row * 8);
        gen.type = getType(gen.piece);
        gen.side = getSide(gen.piece);
        gen.whites_turn = False;
        make_move(gen);

        // Check for move repetition
        if ((PLAYING == game.state) && add_to_history(bmove)) {
            game.state = BLACK_3_MOVE_REP;
        }
    }

    // Set a flag if we took a piece
    Bool const piece_taken = piece_count != game.piece_count;

    if (game.last_was_en_passant) {
        printf(Debug1, " - en passant capture ")
    }

    if (game.last_was_pawn_promotion) {
        printf(Debug1, " - pawn promoted ")
    }

    if (game.last_was_timeout2) {
        printf(Debug1, " - timeout ")
    }

    if (game.last_was_castle) {
        printf(Debug1, " - castling ")
    }

    printf(Debug1, "\n");

    // Announce if either King is in check
    if (game.white_king_in_check) {
        printf(Debug1, "White King is in check!\n");
        if (whites_turn) {
            printf(Always, "illegal move\n");
        }
    }

    if (game.black_king_in_check) {
        printf(Debug1, "Black King is in check!\n");
        if (!whites_turn) {
            printf(Always, "illegal move\n");
        }
    }

    printf(Debug1, "\n");

    // Toggle whose turn it is
    game.turn = !game.turn;

    // Increase the game move counter
    game.move_num++;

    // Delete any soft-deleted pieces for real
    if (piece_taken) {
        for (index_t i = 0; i < game.piece_count; i++) {
            if (-1 == game.pieces[i].x) {
                game.pieces[i] = game.pieces[--game.piece_count];
                break;
            }
        }
    }

}   // take_turn()


////////////////////////////////////////////////////////////////////////////////////////
// Set all of the options for the game
// 
void set_game_options()
{
    // Set game.options.profiling to True (1) to disable output and profile the engine
    game.options.profiling = False;
    // game.options.profiling = True;

    // Set the ultimate maximum ply level
    game.options.max_max_ply = 2;

    // Set the max ply level (the number of turns we look ahead) for normal moves
    game.options.maxply = 1;

    // Set the percentage of moves that might be a mistake
    game.options.mistakes = 0;

    // Set game.options.random to True (1) to use randomness in the game decisions
    game.options.random = False;
    // game.options.random = True;

    // Set whether we play continuously or not
    // game.options.continuous = game.options.random;
    // game.options.continuous = False;
    game.options.continuous = True;
 
    // Set the time limit per turn in milliseconds
    // game.options.time_limit = 0;
    game.options.time_limit = 30000;

    // Enable or disable alpha-beta pruning
    game.options.alpha_beta_pruning = False;
    // game.options.alpha_beta_pruning = True;

    // When sort_pieces is True we sort the pieces[] array before each turn
    // so that we process the current side's pieces first.
    // game.options.sort_pieces = False;
    game.options.sort_pieces = True;

    // Set the maximum ply level to continue if a move takes a piece
    // The quiescent search depth is based off of the max ply level
    game.options.max_quiescent_ply = min(game.options.maxply + 1, game.options.max_max_ply);

    // set the 'live update' flag
    // game.options.live_update = False;
    game.options.live_update = True;

    // game seed hash for PRN generator - default to 4 hex prime numbers
    game.options.seed = 0x232F89A3;
    uint16_t upper = game.options.seed >> 16;
    uint16_t lower = word(game.options.seed);

    // Salt the psuedo-random number generator seed if enabled:
    if (game.options.random) {
        // Add salt to the psuedo random number generator seed
        // from the physical environment
        uint8_t const pins[] = { 2, 7, 8, 9, 10, 11, 12 };
        uint8_t const total_passes = random(23, 87);
        uint32_t some_bits = 1234567890;

        randomSeed(random());

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

        upper = game.options.seed >> 16;
        lower = word(game.options.seed);
    }

    char str[16] = "";
    ftostr(MAX_VALUE, 0, str);
    printf(Always, "MAX_VALUE: %14s\n", str);

    ftostr(MIN_VALUE, 0, str);
    printf(Always, "MIN_VALUE: %14s\n", str);

    printf(Always, "PRNG seed hash: 0x%04X%04X\n", upper, lower);

    printf(Always, "Ply limits: normal: %d, quiescent: %d, max: %d\n", 
        game.options.maxply,
        game.options.max_quiescent_ply,
        game.options.max_max_ply);

    printf(Always, "Percentage of mistakes: %d%%\n", game.options.mistakes);

    printf(Always, "Max number of moves: %d\n", game.options.move_limit);

    printf(Always, "Alpha-Beta pruning: ");
    if (game.options.alpha_beta_pruning) {
        printf(Always, "yes\n");
    }
    else {
        printf(Always, "no\n");
    }

    printf(Always, "Time limit: ");
    if (0 == game.options.time_limit) {
        printf(Always, " unlimited\n");
    }
    else {
        show_time(game.options.time_limit);
        printf(Always, "\n");
    }

    #ifdef ENA_MEM_STATS
    printf(Always, "RAM usage tracking: yes\n");
    #else
    printf(Always, "RAM usage tracking: no\n");
    #endif

    printf(Always, "Move Shuffling: ");
    if (game.options.sort_pieces) {
        printf(Always, "yes\n");
    }
    else {
        printf(Always, "no\n");
    }

    printf(Always, "Random: ");
    if (game.options.random) {
        printf(Always, "yes\n");
    }
    else {
        printf(Always, "no\n");
    }

    // Enable random seed when program is debugged.
    // Disable random seed to reproduce issues or to profile.
    if (game.options.profiling) {
        printf(Always, "Profiling:\n");

        // Turn off output if we are profiling
        game.options.print_level = None;
    } 
    printf(Always, "\n");

    randomSeed(game.options.seed);

}   // set_game_options()


////////////////////////////////////////////////////////////////////////////////////////
// Continually call play_game() until we reach the end of the game.
// Display the statistics for the game and start another game.
// 
void setup()
{
    // Initialize the Serial output
    Serial.begin(230400); while (!Serial); Serial.write('\n');

    // Initialize the LED strip
    init_led_strip();

    // Initialize the LED indicators
    static uint8_t const pins[3] = { DEBUG1_PIN, DEBUG2_PIN, DEBUG3_PIN };
    for (uint8_t pin : pins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    // Initialize the continuous game statistics
    uint32_t state_totals[6] = { 0, 0, 0, 0, 0, 0 };
    uint32_t white_wins = 0;
    uint32_t black_wins = 0;

    // Play a game until it is over
    do {
        set_game_options();

        // initialize the board and the game:
        board.init();
        game.init();

        // Shuffle our pieces really well so we evaluate them in a random order
        sort_and_shuffle(game.turn, 255);

        show();

        game.stats.start_game_stats();

        do {
            take_turn();
            show();

            // if (!game.compare_pieces_to_board(board)) {
            //     printf(Debug1, "Error: game.pieces[] contents are different from the board contents\n");
            //     game.set_pieces_from_board(board);
            // }

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
            case MOVE_LIMIT:       printf(Debug1, "%d-move limit reached!\n\n", game.options.move_limit);  break;
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

        printf(Debug1, "\n");

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
    
    } while (game.options.continuous);

}   // setup()


void loop() {}


////////////////////////////////////////////////////////////////////////////////////////
// display the game board
// 
void show()
{
    static char const icons[] PROGMEM = "pnbrqkPNBRQK";

    static const bool dev = true;

    long value = 0;

    index_t const offset = 0;

    for (unsigned char y = 0; y < 8; ++y) {
        printf(Debug1, "%c ", dev ? ('0' + y) : ('8' - y));
        for (unsigned char x = 0; x < 8; ++x) {
            Piece const piece = board.get(y * 8 + x);
            printf(Debug1, " %c ", 
                isEmpty(piece) ? ((y ^ x) & 1 ? '*' : '.') :
                pgm_read_byte(&icons[((getSide(piece) * 6) + getType(piece) - 1)]));
        }

        // display the extra status info on certain lines:
        switch (y) {
            // display the last move made if available
            case offset + 0:
                if (game.last_move.from != -1 && game.last_move.to != -1) {
                    printf(Debug1, "      Last Move: %c%c to %c%c", 
                        (game.last_move.from % 8) + 'A', 
                        '8' - (game.last_move.from / 8), 
                        (game.last_move.to   % 8) + 'A', 
                        '8' - (game.last_move.to   / 8) );
                }
                break;

            // display the time spent on the last move
            case offset + 1:
                if (0 == game.stats.move_stats.duration()) break;
                if (0 != game.last_moves_evaluated) {
                    char str_time[16] = "";
                    ftostr(game.stats.move_stats.duration(), 0, str_time);

                    char str_moves[16] = "";
                    ftostr(game.stats.move_stats.counter(), 0, str_moves);
                    printf(Debug1, "      %s moves in ", str_moves);

                    show_time(game.stats.move_stats.duration());

                    char str_moves_per_sec[16] = "";
                    ftostr(game.stats.move_stats.moveps(), 2, str_moves_per_sec);
                    printf(Debug1, " (%s moves/sec)", str_moves_per_sec);
                }
                break;

            // display the total game time so far
            case offset + 2:
                if (game.move_num > 0) {
                    printf(Debug1, "      Game time elapsed : ");
                    show_time(game.stats.game_stats.duration());
                }
                break;

            // display the max ply depth we were able to reach
            case offset + 3:
                if (game.move_num > 0) {
                    printf(Debug1, "      Max ply depth reached: %d", 
                        game.stats.move_stats.maxply);
                }
                break;

            // display the pieces taken by White
            case offset + 5:
                printf(Debug1, "      Taken 1: ");
                for (int i = 0; i < game.white_taken_count; i++) {
                    Piece const piece = game.taken_by_white[i];
                    Piece const ptype = getType(piece);
                    Color const pside = getSide(piece);
                    printf(Debug1, "%c ", pgm_read_byte(&icons[(pside * 6) + ptype - 1]));
                }
                break;

            // display the pieces taken by Black
            case offset + 6:
                printf(Debug1, "      Taken 2: ");
                for (int i = 0; i < game.black_taken_count; i++) {
                    Piece const piece = game.taken_by_black[i];
                    Piece const ptype = getType(piece);
                    Color const pside = getSide(piece);
                    printf(Debug1, "%c ", pgm_read_byte(&icons[(pside * 6) + ptype - 1]));
                }
                break;

            // display the current score
            // case offset + 7:
            //     break;
        }
        printf(Debug1, "%c", '\n');
    }
    printf(Debug1, "%s", 
        dev ? "   0  1  2  3  4  5  6  7 " : "   A  B  C  D  E  F  G  H ");

    {
        value = game.last_value;

        char str_score[16] = "";
        ftostr(value, 0, str_score);
        printf(Debug1, "      Board value: %8s %s", str_score, (value == 0) ? "" : 
            (value  < 0) ? "Black's favor" : "White's favor");
    }

    printf(Debug1, "\n\n");

    set_led_strip();

}   // show()