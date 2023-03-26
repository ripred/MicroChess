/**
 * ArduinoChess.ino
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * written March 2023 - Trent M. Wyatt
 * 
 * version 1.0.0
 *   First base
 * 
 * version 1.1.0
 * Modified board_t to save 16 bytes
 * 
 * version 1.2.0
 * Added evaluation
 * 
 * version 1.3.0
 * Added storage for pieces and moves, printf adjustable print debugging
 * 
 * version 1.4.0
 * Added board traversal, pawn movements
 * 
 * version 1.5.0
 * Added knight movements
 * 
 * version 1.6.0
 * Added move execution
 * 
 * version 1.7.0
 * Major bug fixes in piece tracking and move generation
 * 
 * version 1.8.0
 * Added Rook, Bishop, Queen, and King move generation
 * Added 'in check' attributes
 * Added 'has moved' attributes
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * TODO: for version 1.9.0
 * 
 *  [+] create new macro for printf that automatically declares the
 *      format string as a PROGMEM array behind the scenes without
 *      requiring a PROGMEM to be decared at each use of printf(...)!
 *  [+] replace all checks for Empty == getType(p) with isEmpty(p)
 *  [+] add the ability turn off all output in order to profile 
 *      the engine without waiting on serial i/o.
 *  [+] move the offsets into PROGMEM.
 *  [+] add pawn promotion to queen when reaching the last row.
 *  [+] move the move generation for each Piece type into it's own function.
 *  [+] create a separate file for each piece.
 *  [ ] add castling.
 *  [ ] update the show_move(...) function to properly display when a pawn executes 
 *      an en-passant capture.
 *  [ ] 
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * TODO: for version 1.nn.0
 * 
 *  [ ] add reading and writing of FEN notation.
 *  [ ] add ply level awareness and minimax algorithm.
 *  [ ] add alpha-beta pruning.
 *  [ ] 
 * 
 * BUGBUGS: to fix!
 * 
 *  [+] fix and test the new Rook, Bishop, Queen, and King pieces
 *  [ ] 
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 */
#include <Arduino.h>
#include <stdlib.h>

#include "MicroChess.h"

////////////////////////////////////////////////////////////////////////////////////////
// the game board
board_t board;

////////////////////////////////////////////////////////////////////////////////////////
// the currently running game
game_t game;


void show_stats(Bool profiling = False) {
    game.stats.stop_game_stats();
    // print out the game move counts and time statistics
    char fstr[16]= "";
    double fmoves = game.stats.moves_gen_game;
    double ftime = game.stats.game_time;
    dtostrf(fmoves / (ftime / 1000), 8, 4, fstr);

    printf(Debug1, "total game time: %ld ms\n", game.stats.game_time);
    printf(Debug1, "max move count: %d\n", game.stats.max_moves);
    printf(Debug1, "total game moves evaluated: %d\n", game.stats.moves_gen_game);
    printf(Debug1, "moves per second: %s %s\n", 
        fstr, profiling ? "" : "(this includes waiting on the serial output)\n");
}


////////////////////////////////////////////////////////////////////////////////////////
// add a move to the move list: game.moves1 (White) or game.moves2 (Black), 
// depending on the color of the piece.
void add_move(Color side, index_t from, index_t to, long value) 
{
    printf(Debug3, "call to add_move(from: %d,%d, to: %d,%d)\n", from%8,from/8, to%8,to/8);

    if (White == side) {
        if (game.move_count1 < MAX_MOVES) {
            game.moves1[game.move_count1++] = { from, to, value };
        }
        else {
            printf(Debug1, "attempt to add too many move1\n");
        }
    }
    else {
        if (game.move_count2 < MAX_MOVES) {
            game.moves2[game.move_count2++] = { from, to, value };
        }
        else {
            printf(Debug1, "attempt to add too many move2\n");
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////
// remove a move to the move list: game.moves1 (White) or game.moves2 (Black), 
// depending on the color of the piece.
index_t remove_move(Color const side, index_t const from, index_t const to = -1)
{
    printf(Debug2, "call to remove_move(from: %d,%d, to: %d,%d)\n", from%8,from/8, to%8,to/8);

    index_t result = 0;

    if (White == side) {
        for (index_t i = 0; i < game.move_count1; i++) {
            move_t &move = game.moves1[i];
            if (move.from == from) {
                if (-1 == to || to == move.to) {
                    --game.move_count1;
                    if (game.move_count1 > 0 && i != game.move_count1) {
                        move = game.moves1[game.move_count1];
                    }
                    --i;
                    result++;
                }
            }
        }

        return result;
    }

    for (index_t i = 0; i < game.move_count2; i++) {
        move_t &move = game.moves2[i];
        if (move.from == from) {
            if (-1 == to || to == move.to) {
                --game.move_count2;
                if (game.move_count2 > 0 && i != game.move_count2) {
                    move = game.moves2[game.move_count2];
                }
                --i;
                result++;
            }
        }
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////////////
/// Evaluate the identity (score) of the board state.
/// Positive scores indicate an advantage for white and
/// Negative scores indicate an advantage for black.
long evaluate(Color side) 
{
    // flags choices for which attributes are included in the board score
    static uint8_t const material = 0x01u;
    static uint8_t const   center = 0x02u;
    static uint8_t const mobility = 0x04u;

    // Adjust as desired
    // Note: Do not include mobility unless you are prepared to evaluate all moves
    // for both sides for future plies. On the first pass when neither side has any moves
    // this gives favor to White when the Black response moves have not been generated yet
    static uint8_t const   filter = material | center;

    // adjustable multipiers to alter importance of mobility or center proximity.
    // season to taste
    static long const mobilityBonus = 3L;
    static long const   centerBonus = 5L;

    // Material bonus lambda function
    // Gives more points for moves leave more or higher value pieces on the board
    auto materialEvaluator = [](Piece p) -> long { return getValue(p) / 100; };

    // Center location bonus lambda function
    // Gives more points for moves that are closer the center of the board
    auto centerEvaluator = [](index_t location, Piece piece) -> long {
        Piece type = getType(piece);

        // let's not encourage the king to wander to the board center mmkay?
        if (King == type) return 0;

        index_t dx = location % 8;
        if (dx > 3) dx = 7 - dx;
        index_t dy = location / 8;
        if (dy > 3) dy = 7 - dy;

        return static_cast<long>((dx + dy) * type);
    };

    // calculate the value of the board
    long materialTotal = 0L;
    long mobilityTotal = 0L;
    long centerTotal = 0L;
    long sideFactor = 1L;
    long score = 0L;

    // enumerate over the pieces on the board if necessary
    if ((filter & material) || (filter & center)) {
        for (index_t piece_index = 0; piece_index < game.piece_count; piece_index++) {
            index_t const board_index = game.pieces[piece_index].x + game.pieces[piece_index].y * 8;
            Piece const p = board.get(board_index);
            sideFactor = (Black == getSide(p)) ? -1 : 1;

            // The score or 'identity property' of the board can include extra points for
            // the material value of pieces
            if (filter & material) {
                materialTotal += materialEvaluator(p) * sideFactor;
            }

            // The score or 'identity property' of the board can include extra points for
            // the "proximity to the center" value of pieces
            if (filter & center) {
                centerTotal += centerEvaluator(board_index, p) * centerBonus * sideFactor;
            }
        }
    }

    // The score or 'identity property' of the board can include extra points for
    // how many total moves (mobility) the remaining pieces can make
    if (filter & mobility) {
        sideFactor = (Black == side) ? -1 : 1;
        mobilityTotal += static_cast<long>(game.move_count1 * mobilityBonus * sideFactor);
        mobilityTotal -= static_cast<long>(game.move_count2 * mobilityBonus * sideFactor);
    }

    score = materialTotal + centerTotal + mobilityTotal;

    printf(Debug4, 
        "evaluation: %ld = centerTotal: %ld  materialTotal: %ld  mobilityTotal: %ld\n", 
        score, centerTotal, materialTotal, mobilityTotal);

    return score;
}


////////////////////////////////////////////////////////////////////////////////////////
// find the piece index for a given board index
index_t find_piece(int const index) {
    // print_t dbg = game.move_num == 7 ? Debug1 : Debug3;
    print_t dbg = Debug3;

    printf(dbg, "find_piece(index: %2d) called, %2d total pieces\n", index, game.piece_count);

    for (index_t piece_index = 0; piece_index < game.piece_count; piece_index++) {
        point_t const &loc = game.pieces[piece_index];
        index_t const board_index = loc.x + (loc.y * 8);

        printf(print_t(dbg + 1), "game.pieces[%2d] = point_t(x:%d, y: %d) (%2d)\n", 
            piece_index, loc.x, loc.y, board_index);
        if (board_index == index) {
            printf(print_t(dbg + 1), " returning %d\n", piece_index);

            return piece_index;
        }
    }

    return -1;
};


////////////////////////////////////////////////////////////////////////////////////////
// move a piece on the board, taking a piece if necessary.
// If 'restore' != 0 then we put the piece(s) bac after evaluating the move value
// 
long make_move(move_t const &move, Bool const restore) 
{
    game.stats.inc_moves_count();

    index_t const col = move.from % 8;
    index_t const row = move.from / 8;
    index_t const from = col + row * 8;
    Piece   const p = board.get(from);
    Color   const side = getSide(p);
    Bool    const moved = hasMoved(p);

    index_t const to_col = move.to % 8;
    index_t const to_row = move.to / 8;
    index_t const to = to_col + to_row * 8;
    Piece         op = board.get(to);
    Piece   const otype = getType(op);
    Color   const oside = getSide(op);

    // for debugging; remove when finished validating capture restores
    index_t const orig_piece_count = game.piece_count;

    // find piece in the list of pieces:
    index_t const piece_index = find_piece(from);
    if (piece_index < 0) {
        printf(Error, 
            "error in make_move(...): could not find piece from move_t in pieces list: "
            "col = %d, row = %d\n\n",
            col, row);

        print_level = Debug1;
        show();
        show_stats();
        while ((1)) {}
    }

    // TODO: implement en-passant captures the way the other moves are implemented.
    // checks for en-passant:

    // if (type == Pawn && isEmpty(otype) && col != to_col) {  // en-passant capture
    //     if (White == side) { game.taken1[game.taken_count1++] = p; }
    //     else { game.taken2[game.taken_count2++] = p; }
    //     board.set(to_col + row * 8, Empty);
    // } else {
    //     if (!isEmpty(otype)) {
    //         // This move captures a piece
    //         if (White == side) { game.taken1[game.taken_count1++] = p; }
    //         else { game.taken2[game.taken_count2++] = op; }
    //     }
    // }

    // See if the destination is not empty and not a piece on our side.
    // i.e. an opponent's piece.
    index_t taken_index = -1;
    if (!isEmpty(otype) && side != oside) {
        // remember the piece index of the piece being taken
        taken_index = find_piece(to);

        // Decrement the piece count to point to the last entry
        game.piece_count--;

        // replace the taken piece in the piece list with the last piece
        // in the piece list (remove it).
        // ONLY if it is NOT the last piece in the list (that we just moved in the list)
        if (taken_index != piece_index) {
            game.pieces[taken_index] = game.pieces[game.piece_count];
        }

        // add the piece to the list of taken pieces
        if (White == side) {
            game.taken1[game.taken_count1++] = op;
        }
        else {
            game.taken2[game.taken_count2++] = op;
        }
    }

    // move our piece on the board
    board.set(from, Empty);

    // promote the pawn to a queen if it reached the back row
    if (Pawn == getType(p) && (to_row == ((White == side) ? index_t(0) : index_t(7)))) {
        board.set(to, setMoved(setType(p, Queen), True));
    }
    else {
        board.set(to, setMoved(p, True));
    }

    // move our piece in the piece list
    game.pieces[piece_index] = { to_col, to_row };

    // get the value of the board after the move
    long const value = evaluate(side);

    // if this move is just being evaluated then put the pieces back
    if (restore) {
        // if we took a piece then restore it
        if (taken_index != -1) {
            // restore the last piece in the piece list
            game.pieces[game.piece_count++] = game.pieces[taken_index];
            game.pieces[taken_index] = { to_col, to_row };

            // remove the piece from the taken pieces list
            if (White == side) {
                game.taken_count1--;
            }
            else {
                game.taken_count2--;
            }
        }

        // put the pieces back where they were on the board
        board.set(from, setMoved(p, moved));

        // put any piece back that we took and set it to being in check
        if (isEmpty(otype)) {
            board.set(to, Empty);
        }
        else if (side != oside) {
            op = setCheck(op, True);
            board.set(to, op);

            if (King == otype) {
                if (White == oside) {
                    game.white_king_in_check = True;
                }
                else {
                    game.black_king_in_check = True;
                }
            }
        }

        // restore the position of the piece we moved in the piece list
        game.pieces[piece_index] = { col, row };

        if (orig_piece_count != game.piece_count) {
            printf(Error, 
                "error: orig_piece_count: %d != game.piece_count: %d\n\n", 
                orig_piece_count, game.piece_count);

            print_level = Debug1;
            show();
            show_stats();
            while ((1)) {}
        }
    }

    return value;
}


////////////////////////////////////////////////////////////////////////////////////////
// Go through both list of moves (game.moves1 and game.moves2) and get a valuation for
// each move. 
// 
// Then sort the lists in descending and ascending order respectively - see Note
// 
// Note: We do this because the best move for White is the highest value score whereas
// the best move for Black is the one with the 'highest' negative value (the lowest value)
// 
void evaluate_moves() 
{
    // lambda compatator to fill in and sort game.moves1 in decending order
    auto compare = [](const void *a, const void *b) -> int {
        move_t const move_a = *((move_t*) a);
        move_t const move_b = *((move_t*) b);
        return (move_a.value == move_b.value) ?  
            0 : (move_a.value  < move_b.value) ? +1 : -1;
    };

    // lambda compatator to fill in and sort game.moves2 in decending order
    auto reverse_compare = [](const void *a, const void *b) -> int {
        move_t const move_a = *((move_t*) a);
        move_t const move_b = *((move_t*) b);
        return (move_a.value == move_b.value) ?  0 :
            (move_a.value  < move_b.value) ? -1 : +1;
    };

    // reset the king-in-check flags
    game.white_king_in_check = False;
    game.black_king_in_check = False;

    // fill in and sort game.moves1 in decending order
    for (uint8_t ndx=0; ndx < game.move_count1; ++ndx) {
        move_t &move = game.moves1[ndx];
        move.value = make_move(move, True);
    }
    qsort(game.moves1, game.move_count1, sizeof(long), compare);

    // fill in and sort game.moves2 in decending order
    for (uint8_t ndx=0; ndx < game.move_count2; ++ndx) {
        move_t &move = game.moves2[ndx];
        move.value = make_move(move, True);
    }
    qsort(game.moves2, game.move_count2, sizeof(long), reverse_compare);
}


////////////////////////////////////////////////////////////////////////////////////////
// clear the "in-check" flag for all pieces
void clear_checks() {
    for (game.eval_ndx = 0; game.eval_ndx < game.piece_count; game.eval_ndx++) {
        index_t const col = game.pieces[game.eval_ndx].x;
        index_t const row = game.pieces[game.eval_ndx].y;
        index_t const from = col + row * 8;
        Piece   const p = board.get(from);
        board.set(from, setCheck(p, False));
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// Add all of the available moves for all pieces to the game.moves1 (White) 
// and game.moves2 (Black) lists
void add_all_moves() {
    clear_checks();

    // walk through the pieces list and generate all moves for each piece
    for (game.eval_ndx = 0; game.eval_ndx < game.piece_count; game.eval_ndx++) {
        index_t const col = game.pieces[game.eval_ndx].x;
        index_t const row = game.pieces[game.eval_ndx].y;
        index_t const from = col + row * 8;
        Piece   const p = board.get(from);
        Color   const side = getSide(p);
        Piece   const type = getType(p);
        index_t const fwd = (White == side) ? -1 : 1;      // which indexing direction 'forward' is for the current side

        printf(Debug3, "game.eval_ndx = %2d of %2d, point = %d,%d, %5s %s\n", 
            game.eval_ndx, game.piece_count, col, row, getColor(p), getName(p));

        if (isEmpty(type)) {
            printf(Error, "error: Empty piece in piece list: game.eval_ndx = %d, board index = %d\n", 
                game.eval_ndx, from);

            print_level = Debug1;
            show_pieces();
            show();
            show_stats();
            while ((1)) {}
        }

        static Bool const   enable_pawns = True;
        static Bool const enable_knights = False;
        static Bool const enable_bishops = False;
        static Bool const   enable_rooks = False;
        static Bool const  enable_queens = True;
        static Bool const   enable_kings = True;

        switch (type) {
                default:
                printf(Error, "error: invalid type = %d\n", type);
                print_level = Debug1;
                show();
                show_stats();
                while ((1)) {}
                break;

            case   Pawn: if ((enable_pawns))   { add_pawn_moves(p, from, fwd, side); }  break;
            case Knight: if ((enable_knights)) { add_knight_moves(from, fwd, side); }   break;
            case Bishop: if ((enable_bishops)) { add_bishop_moves(from, fwd, side); }   break;
            case   Rook: if ((enable_rooks))   { add_rook_moves(from, fwd, side); }     break;
            case  Queen: if ((enable_queens))  { add_queen_moves(from, fwd, side); }    break;
            case   King: if ((enable_kings))   { add_king_moves(from, fwd, side); }     break;
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////
// play a game until reach a stalemate or checkmate
void play_game() 
{
    show();

    // clear the list of moves
    game.move_count1 = 0;
    game.move_count2 = 0;

    // add all moves for all pieces
    printf(Debug3, "\nadding all available moves..\n\n");
    add_all_moves();

    // evaluate all moves for all pieces
    printf(Debug3, "\nevaluating all available moves..\n\n");
    evaluate_moves();

    // track the max number of move entries we need for debugging
    if (game.move_count1 > game.stats.max_moves) {
        game.stats.max_moves = game.move_count1;
    }

    if (game.move_count2 > game.stats.max_moves) {
        game.stats.max_moves = game.move_count2;
    }

    move_t move(-1, -1, 0);

    // pick our next move
    do {
        if (White == game.turn) {
            if (game.move_count1 > 0) {
                index_t top = 0;
                for (top = 0; (top + 1 < game.move_count1) && game.moves1[top].value == game.moves1[top + 1].value; top++) {};
                index_t random_move = random(0, top);
                move = game.moves1[random_move];
            }
        }
        else {
            if (game.move_count2 > 0) {
                index_t top = 0;
                for (top = 0; (top + 1 < game.move_count2) && game.moves2[top].value == game.moves2[top + 1].value; top++) {};
                index_t random_move = random(0, top);
                move = game.moves2[random_move];
            }
        }
    } while ((move.from == -1 || move.to == -1) && game.move_count1 > 0 && game.move_count2 > 0);

    // see if we've hit the move limit (used for testing scenarios that never run out of moves)
    static int const move_limit = 500;
    if (game.move_num > move_limit) {
        printf(Debug1, "\nmove limit of %d exceeded\n", move_limit);
        game.done = True;
        return;
    }

    // see if we have a stalemate
    if (0 == game.move_count1 && 0 == game.move_count2) {
        printf(Debug1, "\nStalemate!\n");
        game.done = True;
        return;
    }
    
    // see if the game has been won:
    if (0 == game.move_count1 || 0 == game.move_count2) {
        printf(Debug1, "\nCheckmate!\n");
        game.done = True;

        if (0 == game.move_count1) {
            printf(Debug1, "\nWhite as no moves.\nBlack wins!\n");
        }
        else if (0 == game.move_count2) {
            printf(Debug1, "\nBlack as no moves.\nWhite wins!\n");
        } 

        return;
    }

    // Display the move that we chose:
    index_t const    to = move.to;
    Piece   const    op = board.get(to);
    Piece   const otype = getType(op);

    printf(Debug1, "\nMove #%d: ", game.move_num + 1);
    show_move(move);

    if (!isEmpty(otype)) {
        printf(Debug1, " taking a ");
        show_piece(op);
    }

    printf(Debug1, "\n\n");

    // Make the move:
    move.value = make_move(move, False);

    // remove the move from the move list
    index_t removed = remove_move(game.turn, move.from);
    if (0 == removed) {
        print_level = Debug1;
        printf(Error, "failed to remove move from %d to %d from move list\n", move.from, move.to);
        show_stats();
        while ((1)) {}
    }
    else {
        printf(Debug2, "removed %d moves from move list\n", removed);
    }

    // force re-examination of check states
    evaluate_moves();

    if (game.white_king_in_check) {
        printf(Debug1, "White King is in check!\n");
        if (White == game.turn) {
            printf(Debug1, "illegal move\n");
        }
    }

    if (game.black_king_in_check) {
        printf(Debug1, "Black King is in check!\n");
        if (Black == game.turn) {
            printf(Debug1, "illegal move\n");
        }
    }

    // remember the last move made
    game.last_move = move;

    // toggle whose turn it is
    ++game.turn %= 2;

    // increase the game moves counter
    game.move_num++;
}


////////////////////////////////////////////////////////////////////////////////////////
void setup() 
{
    Serial.begin(115200); while (!Serial); Serial.write('\n');

    Serial.println("starting..\n");

    // set to 1 to disable output and profile the program
    static Bool profiling = True;
    static Bool useRandom = False;

    // game hash
    uint32_t seed = 0x232F89A3;

    // Enable random seed when program is debugged.
    // Disable random seed to reproduce issues or to profile.

    if (profiling) {
        useRandom = False;
        printf(Debug1, "game hash: %08X, profiling...\n", seed);
        print_level = None;
    } 
    else {
        if (useRandom) {
            seed = analogRead(A0) + analogRead(A1) + micros();
        }
        printf(Debug1, "game hash: %08X\n", seed);
        print_level = Debug1;
    }

    randomSeed(seed);

    game.stats.start_game_stats();

    // initialize the board and the game:
    board.init();
    game.init();

    do {
        play_game();
    } while (!game.done);

    game.stats.stop_game_stats();

    Serial.println("finished.\n");

    print_level = Debug1;

    // show the final board    
    show();

    // print out the game move counts and time statistics
    char fstr[16]= "";
    double fmoves = game.stats.moves_gen_game;
    double ftime = game.stats.game_time;
    dtostrf(fmoves / (ftime / 1000), 8, 4, fstr);

    printf(Debug1, "total game time: %ld ms\n", game.stats.game_time);
    printf(Debug1, "max move count: %d\n", game.stats.max_moves);
    printf(Debug1, "total game moves evaluated: %d\n", game.stats.moves_gen_game);
    printf(Debug1, "moves per second: %s %s\n", 
        fstr, profiling ? "" : "(this includes waiting on the serial output)\n");
}


////////////////////////////////////////////////////////////////////////////////////////
void loop() { }


////////////////////////////////////////////////////////////////////////////////////////
// display the game board
void show()
{
    static char const icons[] = "pnbrqkPNBRQK";

    static const bool dev = true;

    long value = 0;

    index_t const offset = 1;

    for (unsigned char y = 0; y < 8; ++y) {
        printf(Debug1, "%c ", dev ? ('0' + y) : ('8' - y));
        for (unsigned char x = 0; x < 8; ++x) {
            Piece const piece = board.get(y * 8 + x);
            printf(Debug1, " %c ", 
                isEmpty(piece) ? ((y ^ x) & 1 ? '.' : '*') :
                icons[((getSide(piece) * 6) + getType(piece) - 1)]);
        }

        // display the extra status info on certain lines:
        switch (y) {
            // display the last move made if available
            case offset + 0:
                if (game.last_move.from != -1 && game.last_move.to != -1) {
                    printf(Debug1, "    Last Move: %c%c to %c%c", 
                        (game.last_move.from % 8) + 'A', 
                        '8' - (game.last_move.from / 8), 
                        (game.last_move.to   % 8) + 'A', 
                        '8' - (game.last_move.to   / 8) );
                }
                break;

            // display the pieces taken by White
            case offset + 2:
                printf(Debug1, "    Taken %d: ", 1);
                for (int i = 0; i < game.taken_count1; i++) {
                    char c = icons[(getSide(game.taken1[i]) * 6) + getType(game.taken1[i]) - 1];
                    printf(Debug1, "%c ", c);
                }
                break;

            // display the pieces taken by Black
            case offset + 3:
                printf(Debug1, "    Taken %d: ", 2);
                for (int i = 0; i < game.taken_count2; i++) {
                    char c = icons[(getSide(game.taken2[i]) * 6) + getType(game.taken2[i]) - 1];
                    printf(Debug1, "%c ", c);
                }
                break;

            // display the current score
            case offset + 5:
                value = evaluate(game.turn);
                printf(Debug1, "    Board value: %8ld %s", 
                value, 
                (value == 0) ? "" : 
                (value  < 0) ? "Black's favor" : 
                                "White's favor");
                break;
        }
        printf(Debug1, "%c", '\n');
    }
    printf(Debug1, "%s", 
        dev ? "   0  1  2  3  4  5  6  7\n\n" : "   A  B  C  D  E  F  G  H\n");
}