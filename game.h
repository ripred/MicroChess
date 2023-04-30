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

public:
    point_t() /* : x(0), y(0) */ {}
    point_t(index_t X, index_t Y) : x(X), y(Y) {}

};  // point_t


////////////////////////////////////////////////////////////////////////////////////////
// the state of a game
struct game_t 
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

    // Flags indicating when all pieces for this ply level have been evaluated
    uint8_t     complete;

    // The game options
    static options_t options;

    // static, pre-conputed tables for bonus values
    static long const center_bonus[8][7][2] PROGMEM;
    static long const material_bonus[7][2] PROGMEM;                                

    // The last 'MAX_REPS * 2 - 1' moves are kept to recognize 'MAX_REPS' move repetition
    move_t      history[MAX_REPS * 2 - 1];
    index_t     hist_count;

    // The locations of the pieces on the board
    point_t     pieces[MAX_PIECES];
    uint8_t     piece_count;

    // The pieces that have been taken
    Piece       taken_by_white[16];
    Piece       taken_by_black[16];
    uint8_t     white_taken_count;
    uint8_t     black_taken_count;

    // The statistics of the game
    stat_t      stats;

    // the last move made
    long        last_value;                     // The value of the last move made
    move_t      last_move;                      // The last move made
    Bool        last_was_en_passant     : 1,    // True when last move was an en-passaant capture
                last_was_castle         : 1,    // True when last move was a castling of a king and a rook
                last_was_timeout1       : 1,    // True when last move timed out - this version used only duing ply 0 and 1
                last_was_timeout2       : 1,    // True when last move timed out - this version used during all plies
                last_was_pawn_promotion : 1,    // True when last move promoted a Pawn to a Queen
                    white_king_in_check : 1,    // The check state of the two kings
                    black_king_in_check : 1,
                                   turn : 1,    // Whose turn it is: 0 := Black, 1 := White
                          user_supplied : 1;    // True when the user has supplied a value via serial

    // last move statistics
    // uint32_t    last_move_time;
    uint32_t    last_moves_evaluated;

    // the current state of the game
    state_t     state;

    // increasing move number
    uint16_t    move_num;

    // the current ply level
    uint8_t     ply;

    // the location of both kings
    index_t     wking;
    index_t     bking;

    // the alpha and beta edges of our search envelope
    long        alpha;
    long        beta;

    move_t      user;
public:
    // constructor
    game_t();

    // init function
    void init();

    // set the pieces[] array based off of the board contents
    void set_pieces_from_board(board_t &board);
    Bool compare_pieces_to_board(board_t &board);
    index_t find_piece(index_t const index);

};  // game_t

#endif // GAME_INCL