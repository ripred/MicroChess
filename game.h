/**
 * game.h
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * header file for MicroChess
 * 
 */
#ifndef GAME_INCL
#define GAME_INCL

#include "Arduino.h"
#include <stdint.h>
#include "board.h"
#include "stats.h"
#include "options.h"

extern board_t board;

////////////////////////////////////////////////////////////////////////////////////////
// board spot by column and row
struct point_t 
{
public:
    index_t  x : NUM_BITS_PT, 
             y : NUM_BITS_PT;

    point_t() {}
    point_t(index_t X, index_t Y) : x(X), y(Y) {}

};  // point_t

// The faze of the game
enum faze_t { OPENING = 2, MIDDLE = 1, CLOSING = 0 };

////////////////////////////////////////////////////////////////////////////////////////
// the state of a game
class game_t 
{
public:
#ifdef ENA_MEM_STATS
    // The amount of free memory at critical junctions and ply levels
    struct {
        uint16_t mem : 12;
    } freemem[4][5];

    uint16_t   lowest_mem;
    index_t    lowest_mem_ply;
#endif

    // The game options
    static options_t options;

    // static, pre-conputed tables for bonus values
    static long const center_bonus[8][7][2] PROGMEM;
    static long const  material_bonus[7][2] PROGMEM;

    // The last 'MAX_REPS * 2 - 1' moves are kept to recognize 'MAX_REPS' move repetition
    struct history_t {
    index_t 
        from : 6, 
          to : 6;

    } history[MAX_REPS * 2 - 1];

    index_t     hist_count;

    // The locations of the pieces on the board
    point_t     pieces[MAX_PIECES];
    uint8_t     piece_count;

    // The pieces that have been taken
    struct {
        uint8_t piece : 6;
    } taken_by_white[16];

    struct {
        uint8_t piece : 6;
    } taken_by_black[16];

    uint8_t     white_taken_count;
    uint8_t     black_taken_count;

    // The statistics of the game
    stat_t      stats;

    // the last move made
    move_t      last_move;                      // The last move made

    uint8_t
                last_was_pawn_promotion : 1,    // True when last move promoted a Pawn to a Queen
                    white_king_in_check : 1,    // The check state of the white king
                    black_king_in_check : 1,    // The check state of the black king
                    last_was_en_passant : 1,    // True when last move was an en-passaant capture
                        last_was_castle : 1,    // True when last move was a castling of a king and a rook
                          book_supplied : 1,    // True when the current move was supplied by an opening book move
                          user_supplied : 1,    // True when the current move was supplied by the user via serial
                           supply_valid : 1,    // True when the supplied move has been validated
                               timeout1 : 1,    // True when last move timed out - this version used only duing ply 0 and 1
                               timeout2 : 1,    // True when last move timed out - this version used during all plies
                                  state : 3,    // the current state of the game
                                  wking : 6,    // the location of the white king
                                  bking : 6,    // the location of the black king
                                   turn : 1,    // Whose turn it is: 0 := Black, 1 := White
                                    ply : 3;    // the current ply level

    // increasing move number
    uint8_t     move_num;

    // The alpha and beta boundaries of our search envelope
    long        alpha;
    long        beta;

    // The supplied move if book_supplied := True or user_supplied != True
    move_t      supplied;

public:

    // Constructor
    game_t();

    // Inititialize the contents based off of the move value
    void init();

    void sort_pieces(Color const side);
    void shuffle_pieces(index_t const shuffle_count);

    // Set the pieces[] array based off of the board contents
    void set_pieces_from_board(board_t &board);

    // Check the integrity of the game.pieces[] array compared to the board contents
    Bool compare_pieces_to_board(board_t &board);

    // Find the game.pieces[] array index for the given board location
    index_t find_piece(index_t const index);

};  // game_t

#endif // GAME_INCL