#ifndef BOARD_INCL
#define BOARD_INCL

#include <math.h>

struct Board {
    Piece   board[BOARD_SIZE];

    Piece getType(unsigned char from) const 
    {
        return getType(board[from]);
    }
};

#endif // BOARD_INCL
