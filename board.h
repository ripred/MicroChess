#ifndef BOARD_INCL
#define BOARD_INCL

#include <math.h>

/***
 * board_t version 1
 *
 */
struct board_t1 {
    struct row_t {
        spot_t  cols[8];
    } rows[8];

    Piece get(unsigned char index) const 
    {
        return *((Piece*) &rows[index / 8].cols[index % 8]);
    }

    void set(unsigned char index, Piece const piece) 
    {
        rows[index / 8].cols[index % 8] = piece;
    }    
};


/***
 * board_t version 2
 *
 */
struct board_t2 {
    struct row_t {
        uint8_t  type1 : 3,     // one of the Piece types
                 side1 : 1,     // one of the Color types
                moved1 : 1,     // 1 if moved
                check1 : 1,     // 1 if in check
             promoted1 : 1,     // 1 if promoted pawn

                 type2 : 3,
                 side2 : 1,
                moved2 : 1,
                check2 : 1,
             promoted2 : 1,

                 type3 : 3,
                 side3 : 1,
                moved3 : 1,
                check3 : 1,
             promoted3 : 1,

                 type4 : 3,
                 side4 : 1,
                moved4 : 1,
                check4 : 1,
             promoted4 : 1,

                 type5 : 3,
                 side5 : 1,
                moved5 : 1,
                check5 : 1,
             promoted5 : 1,

                 type6 : 3,
                 side6 : 1,
                moved6 : 1,
                check6 : 1,
             promoted6 : 1,

                 type7 : 3,
                 side7 : 1,
                moved7 : 1,
                check7 : 1,
             promoted7 : 1,

                 type8 : 3,
                 side8 : 1,
                moved8 : 1,
                check8 : 1,
             promoted8 : 1;

    } rows[8];

    Piece get(unsigned char index) const 
    {
        int const row = index / 8;
        int const col = index % 8;

        board_t2::row_t const &r = rows[row];
        spot_t const &spot = 
            0 == col ? spot_t(r.type1, r.side1, r.moved1, r.check1, r.promoted1) :
            1 == col ? spot_t(r.type2, r.side2, r.moved2, r.check2, r.promoted2) :
            2 == col ? spot_t(r.type3, r.side3, r.moved3, r.check3, r.promoted3) :
            3 == col ? spot_t(r.type4, r.side4, r.moved4, r.check4, r.promoted4) :
            4 == col ? spot_t(r.type5, r.side5, r.moved5, r.check5, r.promoted5) :
            5 == col ? spot_t(r.type6, r.side6, r.moved6, r.check6, r.promoted6) :
            6 == col ? spot_t(r.type7, r.side7, r.moved7, r.check7, r.promoted7) :
                       spot_t(r.type8, r.side8, r.moved8, r.check8, r.promoted8);

        return *((Piece*) &spot);
    }

    void set(unsigned char index, Piece const piece) 
    {
        int const row = index / 8;
        int const col = index % 8;

        switch (col) 
        {
            case 0:
                rows[row].type1 = getType(piece);
                rows[row].side1 = getSide(piece);
                rows[row].moved1 = hasMoved(piece);
                rows[row].check1 = inCheck(piece);
                rows[row].promoted1 = isPromoted(piece);
                return;
            
            case 1:
                rows[row].type2 = getType(piece);
                rows[row].side2 = getSide(piece);
                rows[row].moved2 = hasMoved(piece);
                rows[row].check2 = inCheck(piece);
                rows[row].promoted2 = isPromoted(piece);
                return;
            
            case 2:
                rows[row].type3 = getType(piece);
                rows[row].side3 = getSide(piece);
                rows[row].moved3 = hasMoved(piece);
                rows[row].check3 = inCheck(piece);
                rows[row].promoted3 = isPromoted(piece);
                return;
            
            case 3:
                rows[row].type4 = getType(piece);
                rows[row].side4 = getSide(piece);
                rows[row].moved4 = hasMoved(piece);
                rows[row].check4 = inCheck(piece);
                rows[row].promoted4 = isPromoted(piece);
                return;
            
            case 4:
                rows[row].type5 = getType(piece);
                rows[row].side5 = getSide(piece);
                rows[row].moved5 = hasMoved(piece);
                rows[row].check5 = inCheck(piece);
                rows[row].promoted5 = isPromoted(piece);
                return;
            
            case 5:
                rows[row].type6 = getType(piece);
                rows[row].side6 = getSide(piece);
                rows[row].moved6 = hasMoved(piece);
                rows[row].check6 = inCheck(piece);
                rows[row].promoted6 = isPromoted(piece);
                return;
            
            case 6:
                rows[row].type7 = getType(piece);
                rows[row].side7 = getSide(piece);
                rows[row].moved7 = hasMoved(piece);
                rows[row].check7 = inCheck(piece);
                rows[row].promoted7 = isPromoted(piece);
                return;
            
            case 7:
                rows[row].type8 = getType(piece);
                rows[row].side8 = getSide(piece);
                rows[row].moved8 = hasMoved(piece);
                rows[row].check8 = inCheck(piece);
                rows[row].promoted8 = isPromoted(piece);
                return;
        }
    }    

};


struct board_t {
    board_t2  board;

    Piece get(unsigned char index) const 
    {
        return board.get(index);
    }

    void set(unsigned char index, Piece const piece) 
    {
        board.set(index, piece);
    }    
    
    void init() {
        set( 0, makeSpot(  Rook, Black, 0, 0));
        set( 1, makeSpot(Knight, Black, 0, 0));
        set( 2, makeSpot(Bishop, Black, 0, 0));
        set( 3, makeSpot(  King, Black, 0, 0));
        set( 4, makeSpot( Queen, Black, 0, 0));
        set( 5, makeSpot(Bishop, Black, 0, 0));
        set( 6, makeSpot(Knight, Black, 0, 0));
        set( 7, makeSpot(  Rook, Black, 0, 0));
        set( 8, makeSpot(  Pawn, Black, 0, 0));
        set( 9, makeSpot(  Pawn, Black, 0, 0));
        set(10, makeSpot(  Pawn, Black, 0, 0));
        set(11, makeSpot(  Pawn, Black, 0, 0));
        set(12, makeSpot(  Pawn, Black, 0, 0));
        set(13, makeSpot(  Pawn, Black, 0, 0));
        set(14, makeSpot(  Pawn, Black, 0, 0));
        set(15, makeSpot(  Pawn, Black, 0, 0));

        set(48, makeSpot(  Pawn, White, 0, 0));
        set(49, makeSpot(  Pawn, White, 0, 0));
        set(50, makeSpot(  Pawn, White, 0, 0));
        set(51, makeSpot(  Pawn, White, 0, 0));
        set(52, makeSpot(  Pawn, White, 0, 0));
        set(53, makeSpot(  Pawn, White, 0, 0));
        set(54, makeSpot(  Pawn, White, 0, 0));
        set(55, makeSpot(  Pawn, White, 0, 0));

        set(56, makeSpot(  Rook, White, 0, 0));
        set(57, makeSpot(Knight, White, 0, 0));
        set(58, makeSpot(Bishop, White, 0, 0));
        set(59, makeSpot(  King, White, 0, 0));
        set(60, makeSpot( Queen, White, 0, 0));
        set(61, makeSpot(Bishop, White, 0, 0));
        set(62, makeSpot(Knight, White, 0, 0));
        set(63, makeSpot(  Rook, White, 0, 0));
    }

    Piece getType(unsigned char from) const 
    {
        return getType(get(from));
    }
};

#endif // BOARD_INCL
