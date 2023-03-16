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
 */
#include <Arduino.h>
#include "MicroChess.h"
#include "board.h"
#include "move.h"

void info();
void show();

static 
PrintType const level = Debug2;

typedef   int8_t   index_t;

board_t board;

////////////////////////////////////////////////////////////////////////////////////////
// macro to validate a board spot
#define  isValidPos(col, row) (col >= 0 && col < 8 && row >= 0 && row < 8)

Bool isValidTest()
{
    for (index_t y=0; y < 8; y++) {
        for (index_t x=0; x < 8; x++) {
            if (!isValidPos(x, y)) {
                printf(Debug1, level, "\nERROR - failed isValidPos(x,y) test 1!\n\n");
                return 0;
            }
        }
    }

    if (isValidPos(-1, 0) || isValidPos(0, -1) || isValidPos(8, 0) || isValidPos(0, 8)) {
        printf(Debug1, level, "\nERROR - failed isValidPos(x,y) test 2!\n\n");
        return 0;
    }

    printf(Debug2, level, "passed isValidPos(x,y) tests\n\n");
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////////
// limits
enum 
{
    MAX_PIECES = 32,
    MAX_MOVES  = 64,
};



////////////////////////////////////////////////////////////////////////////////////////
// board spot by column and row
struct point_t 
{
public:
    index_t  x, y;

public:
    point_t() : x(0), y(0) { }
    point_t(index_t X, index_t Y) : x(X), y(Y) {
        if (!isValidPos(X, Y)) {
            printf(Debug1, level, "error: adding invalid board spot %u %u\n", X, Y);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////////////
// an entry in a move list
struct move2_t 
{
public:
    index_t spot, 
              to;
    long   value;

public:
    move2_t() : spot(0), to(0), value(0) { }
    move2_t(index_t f, index_t t, long v) : spot(f), to(t), value(v) {
        if (!isValidPos(f%8,f/8) || !isValidPos(t%8,t/8)) {
            printf(Debug1, level, "error: adding invalid move %u, %u to %u, %u\n", f%8,f/8, t%8,t/8);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////////////
// the state of a game
struct game_t 
{
public:
    point_t     pieces[MAX_PIECES];
    move2_t     moves1[MAX_MOVES];
    move2_t     moves2[MAX_MOVES];
    move2_t     last_move;
    uint8_t     piece_count;
    uint8_t     move_count1;
    uint8_t     move_count2;
    uint8_t     eval_ndx,   // board index being currently evaluated
                    turn,   // 0 = Black, 1 = White
                    done;   // 1 if game is over
public:
    // constructor
    game_t() : 
        piece_count(0),
        move_count1(0),
        move_count2(0),
        turn(White), 
        done(0)
    {
        init();
    }


    void init()
    {
        // initialize list of pieces in the game
        piece_count = 0;
        for (uint8_t ndx=0; ndx < BOARD_SIZE; ++ndx) {
            if (Empty == getType(board.get(ndx))) continue;
            if (piece_count < MAX_PIECES) {
                pieces[piece_count++] = point_t( ndx % 8, ndx / 8);
            }
        }
    }

};


////////////////////////////////////////////////////////////////////////////////////////
// the currently running game
game_t game;


////////////////////////////////////////////////////////////////////////////////////////
/// Center location bonus
/// gives more points for moves that are closer the center of the board
long centerEvaluator(unsigned int location, Piece piece) 
{
    Piece type = getType(piece);
    if (type == King)
        return 0;  // let's not encourage the king to wander to the board center mmkay?

    unsigned int dx = location % 8;
    if (dx > 3) dx = 7 - dx;
    unsigned int dy = location / 8;
    if (dy > 3) dy = 7 - dy;

    return static_cast<long>((dx + dy) * type);
}


////////////////////////////////////////////////////////////////////////////////////////
long materialEvaluator(Piece p) 
{ 
    return getValue(p) / 100; 
}


////////////////////////////////////////////////////////////////////////////////////////
/// Evaluate the identity (score) of the board state.
/// Positive scores indicate an advantage for white and
/// Negative scores indicate an advantage for black.
long evaluate(unsigned filter /* = material | center | mobility */) 
{
    static unsigned int const material = 0x01u;
    static unsigned int const   center = 0x02u;
    static unsigned int const mobility = 0x04u;

    long score = 0;

    long mobilityBonus = 3;
    long centerBonus = 5;

    for (uint8_t ndx = 0; ndx < BOARD_SIZE; ndx++) {
        Piece p = board.get(ndx);
        long sideFactor = (getSide(p) == Black) ? -1 : 1;

        /// The score or 'identity property' of the board includes points for
        /// all pieces the player has remaining.
        score += (filter & material) ? sideFactor * materialEvaluator(p) : 0;

        /// The score or 'identity property' of the board includes points for
        /// how close the remaining pieces are to the center of the board.
        score += (filter & center) ? sideFactor * centerEvaluator(ndx, p) * centerBonus : 0;
    }

    long sideFactor = (game.turn == Black) ? -1 : 1;

    /// The score or 'identity property' of the board includes extra points for
    /// how many totals moves (mobility) the remaining pieces can make.
    if (filter & mobility) {
        score += static_cast<int>(game.move_count1 * mobilityBonus * sideFactor);
        score -= static_cast<int>(game.move_count2 * mobilityBonus * sideFactor);
    }

    return score;
}


////////////////////////////////////////////////////////////////////////////////////////
// the offsets a knight can move to
static struct knight_offset_t 
{
    int8_t x, y;
} const knight_offsets[8] = {
    { -2, +1 }, { -2, -1 }, // left
    { +2, +1 }, { +2, -1 }, // right
    { +1, +2 }, { -1, +2 }, // forward
    { +1, -2 }, { -1, -2 }  // backward
};

////////////////////////////////////////////////////////////////////////////////////////
void add_move(Color side, index_t spot, index_t to, long value) 
{
    printf(Debug2, level, "call to add move\n");

    if (White == side) {
        if (game.move_count1 < MAX_MOVES) {
            game.moves1[game.move_count1++] = { spot, to, value };
        }
        else {
            printf(Debug1, level, "attempt to add too many move1\n");
        }
    }
    else {
        if (game.move_count2 < MAX_MOVES) {
            game.moves2[game.move_count2++] = { spot, to, value };
        }
        else {
            printf(Debug1, level, "attempt to add too many move2\n");
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////
void play_game() 
{
    show();

    // find the best move for the current side
    game.move_count1 = 0;
    game.move_count2 = 0;

    // point_t best_move = { -1, -1 };
    // long best_value = 0L;

    for (game.eval_ndx=0; game.eval_ndx < game.piece_count; ++game.eval_ndx)
    {
    int8_t col = game.pieces[game.eval_ndx].x;
    int8_t row = game.pieces[game.eval_ndx].y;
    int8_t spot = col + row * 8;
    Piece p = board.get(spot);
    Color side = getSide(p);
    Piece type = getType(p);
    int8_t fwd = (White == side) ? -1 : 1;      // which indexing direction 'forward' is for the current side

    printf(Debug2, level, "game.eval_ndx = %2d, point = %d,%d, %5s %6s\n", 
        game.eval_ndx, 
        col, row, 
        getColor(p), getName(p)
        );

    if (Empty == type) {
        printf(Debug1, level, 
        "error: Empty piece in piece list: game.eval_ndx = %d, board index = %d\n", 
        game.eval_ndx, spot);
    }


    int8_t epx = col;

    Piece op = Empty;
    int8_t to_col = 0;
    int8_t to_row = 0;
    int8_t to = 0;

    switch (type) 
    {
    default:
        printf(Debug1, level, "error: invalid type = %d\n", type);
        break;

    case Pawn:
        // see if we can move 1 spot in front of this pawn
        to_col = col;
        to_row = row + fwd;
        to = to_col + (to_row * 8);

        printf(Debug2, level, "valid pawn checkpoint 1\n");

        if (isValidPos(to_col, to_row)) {
            printf(Debug2, level, "valid pawn checkpoint 2\n");
            op = board.get(to);    // get piece at location 1 spot in front of pawn
            if (Empty == op) {
                add_move(
                    side, 
                    spot,   // spot
                    to,     // to
                    0       // value
                );
            }
        }

        // see if we can move 2 spots in front of this pawn
        to_col = col;
        to_row = row + fwd + fwd;
        to = to_col + (to_row * 8);
        if (isValidPos(to_col, to_row)) {
            op = board.get(to);    // get piece at location 2 spots in front of pawn
            if (Empty == op) {
                add_move(
                    side, 
                    spot,   // spot
                    to,     // to
                    0       // value
                );
            }
        }

        // see if we can capture a piece diagonally to the left
        to_col = col - 1;
        to_row = row + fwd;
        to = to_col + (to_row * 8);
        if (isValidPos(to_col, to_row)) {
            op = board.get(to);    // get piece diagonally to the right
            if (Empty != op && getSide(op) != side) {
                add_move(
                    side, 
                    spot,   // spot
                    to,     // to
                    0       // value
                );
            }
        }

        // see if we can capture a piece diagonally to the right
        to_col = col + 1;
        to_row = row + fwd;
        to = to_col + (to_row * 8);
        if (isValidPos(to_col, to_row)) {
            op = board.get(to);    // get piece diagonally to the right
            if (Empty != op && getSide(op) != side) {
                add_move(
                    side, 
                    uint8_t(spot),  // spot
                    uint8_t(to),    // to
                    0               // value
                );
            }
        }

        // en-passant on the left
        to_col = col - 1;
        to_row = row + fwd;
        to = to_col + (to_row * 8);
        if (isValidPos(to_col, to_row)) {
            op = board.get(to);    // get piece diagonally to the right
        }
        epx = to_col;
        if (isValidPos(to_col, to_row)) {
            op = board.get(to);    // get piece diagonally to the right
            if (Empty != op && getSide(op) != side) {
                uint8_t last_move_spot_row = uint8_t(game.last_move.spot / 8);
                uint8_t last_move_to_col = uint8_t(game.last_move.to % 8);
                uint8_t last_move_to_row = uint8_t(game.last_move.to / 8);
                if (last_move_to_col == epx && last_move_to_row == row) {
                    if (abs(int(last_move_spot_row) - int(last_move_to_row)) > 1) {
                        if (getType(op) == Pawn) {
                            add_move(
                                side, 
                                uint8_t(spot), // spot
                                uint8_t(to),   // to
                                0              // value
                            );
                        }
                    }
                }
            }
        }

        // en-passant on the right
        to_col = col - 1;
        to_row = row + fwd;
        to = to_col + (to_row * 8);
        if (isValidPos(to_col, to_row)) {
            op = board.get(to);    // get piece diagonally to the right
        }
        epx = to_col;
        if (isValidPos(to_col, to_row)) {
            op = board.get(to);    // get piece diagonally to the right
            if (Empty != op && getSide(op) != side) {
                uint8_t last_move_spot_row = uint8_t(game.last_move.spot / 8);
                uint8_t last_move_to_col = uint8_t(game.last_move.to % 8);
                uint8_t last_move_to_row = uint8_t(game.last_move.to / 8);
                if (last_move_to_col == epx && last_move_to_row == row) {
                    if (abs(int(last_move_spot_row) - int(last_move_to_row)) > 1) {
                        if (getType(op) == Pawn) {
                            add_move(
                                side, 
                                uint8_t(spot), // spot
                                uint8_t(to),   // to
                                0              // value
                            );
                        }
                    }
                }
            }
        }
        break;


    case Knight:
        for (unsigned i=0; i < ARRAYSZ(knight_offsets); ++i) {
            to_col = col + knight_offsets[i].x;
            to_row = row + knight_offsets[i].y;
            to = to_col + (to_row * 8);
            if (isValidPos(to_col, to_row)) {
                Piece op = board.get(to);
                if (Empty == op || getSide(op) != side) {
                    add_move(
                        side, 
                        uint8_t(spot),  // spot
                        uint8_t(to),    // to
                        0               // value
                    );
                }
            }
        }
        break;

    case Bishop:
        break;

    case Rook:
        break;

    case Queen:
        break;

    case King:
        break;
    }
    }

    printf(Debug3, level, "\nsetting game.done = 1\n");
    game.done = 1;
}


////////////////////////////////////////////////////////////////////////////////////////
void setup() 
{
    Serial.begin(115200); while (!Serial); Serial.write('\n');

    board.init();
    game.init();

    isValidTest();    
}


////////////////////////////////////////////////////////////////////////////////////////
void loop() 
{ 
    if (0 == game.done) {
        play_game();
        info();
    }

}


////////////////////////////////////////////////////////////////////////////////////////
// [[nodiscard]]
void show()
{
    static const char icons[] = "pnbrqkPNBRQK";
    for (unsigned char y = 0; y < 8; ++y) {
        for (unsigned char x = 0; x < 8; ++x) {
            Piece piece = board.get(y * 8 + x);
            Serial.write(' ');
            Serial.write((Empty == getType(piece)) ? ((y ^ x) & 1 ? '*' : '.') :
                         icons[((getSide(piece) * 6) + getType(piece) - 1)]);
            Serial.write(' ');
        }
        Serial.write('\n');
    }
    Serial.write('\n');
}


////////////////////////////////////////////////////////////////////////////////////////
// display various stats and deug info
void info() {
    auto print_move = [](Color side, move2_t *moves, uint8_t count) -> void {
        char buf[8] = "";
        printf(Debug1, level, "moves%c[%u] = {\n", '1' + side, count);
        for (uint8_t i = 0; i < count; ++i) {
            uint8_t spot = moves[i].spot;
            uint8_t to = moves[i].to;

            Piece p = board.get(spot);
            Piece o = board.get(to);
            strcpy(buf, getName(p));

            printf(Debug1, level, "    moves1[%2d] = %s %6s spot: (%d, %d) to: (%d, %d)", i,
                  getColor(p), buf, spot % 8, spot / 8, to % 8, to / 8);

            if (Empty != o) {
                strcpy(buf, getName(o));
                printf(Debug1, level, " (captures %s %6s)", getColor(o), buf);
            }

            printf(Debug1, level, " value: %ld\n", moves[i].value);
        }
        printf(Debug1, level, "};\n\n");
    };

    print_move(Black, game.moves2, game.move_count2);
    print_move(White, game.moves1, game.move_count1);
}
