/**
 * board.h
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 */
#ifndef BOARD_INCL
#define BOARD_INCL

#include <math.h>

// equivelent to/shadows the Piece date type layout
struct spot_t {
    uint8_t  type : 3,     // one of the Piece values
             side : 1,     // one of the Color values
            moved : 1,     // 1 if moved
            check : 1;     // 1 if in check

    spot_t();
    spot_t(Piece p);
    spot_t(uint8_t t, uint8_t s, uint8_t m, uint8_t c);
};

/***
 * board_t version 1
 * 
 * simple. 64 bytes
 */
struct board_t1 {
private:
    struct row_t {
        spot_t  cols[8];
    } rows[8];

public:
    Piece get(index_t index) const;

    void set(index_t index, Piece const piece);
};


/***
 * board_t version 2
 *
 * 48 bytes
 */
struct board_t2 {
private:
    struct row_t {
        uint8_t  type1 : 3,     // one of the Piece types
                 side1 : 1,     // one of the Color types
                moved1 : 1,     // 1 if moved
                check1 : 1,     // 1 if in check

                 type2 : 3,
                 side2 : 1,
                moved2 : 1,
                check2 : 1,

                 type3 : 3,
                 side3 : 1,
                moved3 : 1,
                check3 : 1,

                 type4 : 3,
                 side4 : 1,
                moved4 : 1,
                check4 : 1,

                 type5 : 3,
                 side5 : 1,
                moved5 : 1,
                check5 : 1,

                 type6 : 3,
                 side6 : 1,
                moved6 : 1,
                check6 : 1,

                 type7 : 3,
                 side7 : 1,
                moved7 : 1,
                check7 : 1,

                 type8 : 3,
                 side8 : 1,
                moved8 : 1,
                check8 : 1;

    } rows[8];

public:
    Piece get(index_t index) const;

    void set(index_t index, Piece const piece);
};


struct board_t {
private:
    board_t2  board;

public:
    Piece get(index_t index) const;

    void set(index_t index, Piece const piece);
    
    void init();

    Piece getType(index_t from) const;
};

#endif // BOARD_INCL
