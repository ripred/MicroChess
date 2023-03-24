/**
 * ArduinoChess.ino
 * 
 * March 2023 - Trent M. Wyatt
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
 * TODO:
 * 
 *  [+] move the offsets into PROGMEM. add accessor functions.
 *  [ ] add pawn promotion when they reach the last row.
 *  [ ] add reading and writing of FEN notation.
 *  [ ] move the move generation for each Piece type into it's own function.
 *  [ ] add ply level awareness and minimax algorithm.
 *  [ ] add alpha-beta pruning.
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


////////////////////////////////////////////////////////////////////////////////////////
// add a move to the move list: game.moves1 (White) or game.moves2 (Black), 
// depending on the color of the piece.
void add_move(Color side, index_t from, index_t to, long value) 
{
    static char const fmt[] PROGMEM = "call to add move from %d,%d to %d,%d\n";
    printf(Debug3, fmt, from%8,from/8, to%8,to/8);

    if (White == side) {
        if (game.move_count1 < MAX_MOVES) {
            game.moves1[game.move_count1++] = { from, to, value };
        }
        else {
            static char const fmt[] PROGMEM = "attempt to add too many move1\n";
            printf(Debug1, fmt);
        }
    }
    else {
        if (game.move_count2 < MAX_MOVES) {
            game.moves2[game.move_count2++] = { from, to, value };
        }
        else {
            static char const fmt[] PROGMEM = "attempt to add too many move2\n";
            printf(Debug1, fmt);
        }
    }
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
    // 
    // Note: Do not include mobility unless you are prepared to evaluate all moves
    // for both sides for future plies. On the first pass when neither side has any moved
    // this gives favor to White when the Black response moves have not been counted yet
//  static uint8_t const   filter = material | center | mobility;
    static uint8_t const   filter = material | center;

    static long const mobilityBonus = 3L;
    static long const   centerBonus = 5L;

    // Material bonus lambda function
    // Gives more points for moves leave more or higher value pieces on the board
    auto materialEvaluator = [](Piece p) -> long { return getValue(p) / 100; };

    // Center location bonus lambda function
    // Gives more points for moves that are closer the center of the board
    auto centerEvaluator = [](index_t location, Piece piece) -> long {
        Piece type = getType(piece);
        if (type == King)
            return 0;  // let's not encourage the king to wander to the board center mmkay?

        uint8_t dx = location % 8;
        if (dx > 3) dx = 7 - dx;
        uint8_t dy = location / 8;
        if (dy > 3) dy = 7 - dy;

        return static_cast<long>((dx + dy) * type);
    };

    long materialTotal = 0L;
    long mobilityTotal = 0L;
    long centerTotal = 0L;
    long sideFactor = 1L;
    long score = 0L;

    // enumerate over the pieces on the board if necessary
    if ((filter & material) || (filter & center)) {
        for (uint8_t piece_index = 0; piece_index < game.piece_count; piece_index++) {
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

    static char const fmt[] PROGMEM = 
        "evaluation: %ld = centerTotal: %ld  materialTotal: %ld  mobilityTotal: %ld\n";
    printf(Debug4, fmt, score, centerTotal, materialTotal, mobilityTotal);

    return score;
}


////////////////////////////////////////////////////////////////////////////////////////
// find the piece index for a given board index
index_t find_piece(int const index) {
    // print_t dbg = game.move_num == 7 ? Debug1 : Debug3;
    print_t dbg = Debug3;

    static char const fmt[] PROGMEM = "find_piece(index: %2d) called, %2d total pieces\n";
    printf(dbg, fmt, index, game.piece_count);

    for (index_t piece_index = 0; piece_index < game.piece_count; piece_index++) {
        point_t const &loc = game.pieces[piece_index];
        index_t const board_index = loc.x + (loc.y * 8);

        static char const fmt[] PROGMEM = "game.pieces[%2d] = point_t(x:%d, y: %d) (%2d)\n";
        printf(dbg + 1, fmt, piece_index, loc.x, loc.y, board_index);
        if (board_index == index) {
            static char const fmt[] PROGMEM = " returning %d\n";
            printf(dbg + 1, fmt, piece_index);
            return piece_index;
        }
    }

    // return an invalid position -> the index one past the available number
    return -1;
};


////////////////////////////////////////////////////////////////////////////////////////
// move a piece on the board, taking a piece if necessary.
// If 'restore' != 0 then we put the piece(s) bac after evaluating the move value
// 
long make_move(move_t const &move, Bool const restore) 
{
    index_t const col = move.from % 8;
    index_t const row = move.from / 8;
    index_t const from = col + row * 8;
    Piece   const p = board.get(from);
    Color   const side = getSide(p);
    Piece   const type = getType(p);
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
        static char const fmt[] PROGMEM = 
            "error: could not find piece from move_t in pieces list: "
            "col = %d, row = %d\n\n";
        printf(Error, fmt, col, row);

        show();
        while ((1));
    }

    // TODO: implement the way the other moves are implemented.
    // checks for en-passant:

    // if (type == Pawn && otype == Empty && col != to_col) {  // en-passant capture
    //     if (White == side) { game.taken1[game.taken_count1++] = p; }
    //     else { game.taken2[game.taken_count2++] = p; }
    //     board.set(to_col + row * 8, Empty);
    // } else {
    //     if (otype != Empty) {
    //         // This move captures a piece
    //         if (White == side) { game.taken1[game.taken_count1++] = p; }
    //         else { game.taken2[game.taken_count2++] = op; }
    //     }
    // }

    // See if the destination is not empty and not a piece on our side.
    // i.e. an opponent's piece.
    index_t taken_index = -1;
    if (Empty != otype && side != oside) {
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
    board.set(to, setMoved(p, 1));

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
        if (Empty == otype) {
            board.set(to, Empty);
        }
        else if (side != oside) {
            op = setCheck(op, 1);
            board.set(to, op);

            if (King == otype) {
                if (White == oside) {
                    game.white_king_in_check = 1;
                }
                else {
                    game.black_king_in_check = 1;
                }
            }
        }

        // restore the position of the piece we moved in the piece list
        game.pieces[piece_index] = { col, row };

        if (orig_piece_count != game.piece_count) {
            static char const fmt[] PROGMEM = 
                "error: orig_piece_count: %d != game.piece_count: %d\n\n";
            printf(Error, fmt, orig_piece_count, game.piece_count);

            show();
            while ((1));
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

    // fill in and sort game.moves1 in decending order
    for (uint8_t ndx=0; ndx < game.move_count1; ++ndx) {
        move_t &move = game.moves1[ndx];
        move.value = make_move(move, 1);
    }
    qsort(game.moves1, game.move_count1, sizeof(long), compare);

    // fill in and sort game.moves2 in decending order
    for (uint8_t ndx=0; ndx < game.move_count2; ++ndx) {
        move_t &move = game.moves2[ndx];
        move.value = make_move(move, 1);
    }
    qsort(game.moves2, game.move_count2, sizeof(long), reverse_compare);
}


////////////////////////////////////////////////////////////////////////////////////////
// Add all of the available moves for all pieces to the game.moves1 (White) 
// and game.moves2 (Black) lists
void add_all_moves() {
    // walk through the pieces list and generate all moves for each piece
    for (game.eval_ndx = 0; game.eval_ndx < game.piece_count; game.eval_ndx++) {
        index_t const col = game.pieces[game.eval_ndx].x;
        index_t const row = game.pieces[game.eval_ndx].y;
        index_t const from = col + row * 8;
        Piece   const p = board.get(from);
        Color   const side = getSide(p);
        Piece   const type = getType(p);
        index_t const fwd = (White == side) ? -1 : 1;      // which indexing direction 'forward' is for the current side

        static char const fmt[] PROGMEM = "game.eval_ndx = %2d of %2d, point = %d,%d, %5s %s\n";
        printf(Debug3, fmt, 
            game.eval_ndx, 
            game.piece_count, 
            col, row, 
            getColor(p), getName(p));

        if (Empty == type) {
            static char const fmt[] PROGMEM = 
                "error: Empty piece in piece list: game.eval_ndx = %d, board index = %d\n";
            printf(Error, fmt, game.eval_ndx, from);

            show_pieces();
            show();
            static const char fmt1[] PROGMEM = "max move count = %d\n";
            printf(Debug1, fmt1, game.max_moves);

            while ((1)) {}
        }

        static Bool const   enable_pawns = 1;
        static Bool const enable_knights = 0;
        static Bool const enable_bishops = 0;
        static Bool const   enable_rooks = 0;
        static Bool const  enable_queens = 0;
        static Bool const   enable_kings = 0;

        switch (type) {
            default:
                {
                    static char const fmt[] PROGMEM = "error: invalid type = %d\n";
                    printf(Error, fmt, type);
                }
                show();
                while ((1)) {}
                break;

            case Pawn:
                if ((enable_pawns)) {
                    add_pawn_moves(p, from, col, row, fwd, side);
                }
                break;

            case Knight:
                if ((enable_knights)) {
                    add_pawn_moves(p, from, col, row, fwd, side);
                }
                break;

            case Bishop:
                if ((enable_bishops)) {
                    add_bishop_moves(from, col, row, fwd, side);
                }
                break;

            case Rook:
                if ((enable_rooks)) {
                    add_rook_moves(from, col, row, fwd, side);
                }                
                break;

            case Queen:
                if ((enable_queens)) {
                    add_queen_moves(from, col, row, fwd, side);
                }                
                break;

            case King:
                if ((enable_kings)) {
                    add_king_moves(from, col, row, fwd, side);
                }
                break;
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
    static char const fmt1[] PROGMEM = "\nadding all available moves..\n\n";
    printf(Debug3, fmt1);
    add_all_moves();

    // evaluate all moves for all pieces
    static char const fmt2[] PROGMEM = "\nevaluating all available moves..\n\n";
    printf(Debug3, fmt2);
    evaluate_moves();

    // track the max number of move entries we need for debugging
    if (game.move_count1 > game.max_moves) {
        game.max_moves = game.move_count1;
    }

    if (game.move_count2 > game.max_moves) {
        game.max_moves = game.move_count2;
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

    // see if the game has been won:
    if (0 == game.move_count1 && 0 == game.move_count2) {
        static char const fmt[] PROGMEM = "\nStalemate!\n";
        printf(Debug1, fmt);
        game.done = 1;
        return;
    }
    else if (0 == game.move_count1 || 0 == game.move_count2) {
        static char const fmt[] PROGMEM = "\nCheckmate!\n";
        printf(Debug1, fmt);
        game.done = 1;

        if (0 == game.move_count1) {
            static char const fmt[] PROGMEM = "\nWhite as no moves.\nBlack wins!\n";
            printf(Debug1, fmt);
        }
        else if (0 == game.move_count2) {
            static char const fmt[] PROGMEM = "\nBlack as no moves.\nWhite wins!\n";
            printf(Debug1, fmt);
        } 

        return;
    }

    // Display the move that we chose:
    index_t const    to = move.to;
    Piece   const    op = board.get(to);
    Piece   const otype = getType(op);

    static char const fmt3[] PROGMEM = "\nMove #%d: ";
    printf(Debug1, fmt3, game.move_num + 1);
    show_move(move);

    if (Empty != otype) {
        static char const fmt[] PROGMEM = " taking a ";
        printf(Debug1, fmt);
        show_piece(op);
    }

    static char const fmt4[] PROGMEM = "\n\n";
    printf(Debug1, fmt4);

    // Make the move:
    move.value = make_move(move, 0);

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

    // Enable random seed when program is debugged.
    // Disable random seed to reproduce issues.
    randomSeed(analogRead(A0) + analogRead(A1));

    Serial.println("starting..\n");

    // initialize the board and the game:
    board.init();
    game.init();

    // if (!isValidTest()) {
    //     while ((1)) {}
    // }

    do {
        play_game();
    } while (!game.done);

    show();

    static const char fmt[] PROGMEM = "max move count = %d\n";
    printf(Debug1, fmt, game.max_moves);
}


////////////////////////////////////////////////////////////////////////////////////////
void loop() 
{ 

}


////////////////////////////////////////////////////////////////////////////////////////
// display the game board
void show()
{
    static const char icons[] = "pnbrqkPNBRQK";
    static const char fmt1[] PROGMEM = "%c";
    static const char fmt2[] PROGMEM = "%c ";
    static const char fmt3[] PROGMEM = " %c ";
    static const char fmt4[] PROGMEM = "%s";
    static const char fmt5[] PROGMEM = "    Taken %d: ";
    static const char fmt6[] PROGMEM = "    Board value: %8ld %s";

    static const bool dev = true;

    long value = 0;

    index_t const offset = 1;

    for (unsigned char y = 0; y < 8; ++y) {
        printf(Debug1, fmt2, dev ? ('0' + y) : ('8' - y));
        for (unsigned char x = 0; x < 8; ++x) {
            Piece const piece = board.get(y * 8 + x);
            printf(Debug1, fmt3, 
                (Empty == getType(piece)) ? ((y ^ x) & 1 ? '.' : '*') :
                icons[((getSide(piece) * 6) + getType(piece) - 1)]);
        }

        // display the extra status info on certain lines:
        switch (y) {
            // display the last move made if available
            case offset + 0:
                if (game.last_move.from != -1 && game.last_move.to != -1) {
                    static char const fmt[] PROGMEM = "    Last Move: %c%c to %c%c";
                    printf(Debug1, fmt, 
                        (game.last_move.from % 8) + 'A', 
                        '8' - (game.last_move.from / 8), 
                        (game.last_move.to   % 8) + 'A', 
                        '8' - (game.last_move.to   / 8) );
                }
                break;

            // display the pieces taken by White
            case offset + 2:
                printf(Debug1, fmt5, 1);
                for (int i = 0; i < game.taken_count1; i++) {
                    char c = icons[(getSide(game.taken1[i]) * 6) + getType(game.taken1[i]) - 1];
                    printf(Debug1, fmt2, c);
                }
                break;

            // display the pieces taken by Black
            case offset + 3:
                printf(Debug1, fmt5, 2);
                for (int i = 0; i < game.taken_count2; i++) {
                    char c = icons[(getSide(game.taken2[i]) * 6) + getType(game.taken2[i]) - 1];
                    printf(Debug1, fmt2, c);
                }
                break;

            // display the current score
            case offset + 5:
                value = evaluate(game.turn);
                printf(Debug1, fmt6, 
                value, 
                (value == 0) ? "" : 
                (value  < 0) ? "Black's favor" : 
                                "White's favor");
                break;
        }
        printf(Debug1, fmt1, '\n');
    }
    printf(Debug1, fmt4, 
        dev ? "   0  1  2  3  4  5  6  7\n\n" : "   A  B  C  D  E  F  G  H\n");
}