/**
 * board.h
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 */
#ifndef BOARD_INCL
#define BOARD_INCL

#include <math.h>

struct board_t {
private:
    Piece  board[BOARD_SIZE];

public:
    board_t();

    void init();

    Piece get(index_t const index) const;

    void set(index_t const index, Piece const piece);
    
    Piece getType(index_t const index) const;
};

#endif // BOARD_INCL
