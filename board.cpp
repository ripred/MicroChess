/**
 * board.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * implementation file for the board class
 * 
 */
#include <Arduino.h>
#include "MicroChess.h"
#include "board.h"

spot_t::spot_t() : type(Empty), side(Black), moved(0), check(0) {

}

spot_t::spot_t(Piece p) {
    *this = *((spot_t *) &p); 
}

spot_t::spot_t(uint8_t t, uint8_t s, uint8_t m, uint8_t c) : type(t), side(s), moved(m), check(c) {

}

Piece board_t1::get(index_t index) const 
{
    return *((Piece*) &rows[index / 8].cols[index % 8]);
}

void board_t1::set(index_t index, Piece const piece) 
{
    rows[index / 8].cols[index % 8] = piece;
}    


Piece board_t2::get(index_t index) const 
{
    index_t const row = index / 8;
    index_t const col = index % 8;

    board_t2::row_t const &r = rows[row];
    spot_t const &spot = 
        0 == col ? spot_t(r.type1, r.side1, r.moved1, r.check1) :
        1 == col ? spot_t(r.type2, r.side2, r.moved2, r.check2) :
        2 == col ? spot_t(r.type3, r.side3, r.moved3, r.check3) :
        3 == col ? spot_t(r.type4, r.side4, r.moved4, r.check4) :
        4 == col ? spot_t(r.type5, r.side5, r.moved5, r.check5) :
        5 == col ? spot_t(r.type6, r.side6, r.moved6, r.check6) :
        6 == col ? spot_t(r.type7, r.side7, r.moved7, r.check7) :
                   spot_t(r.type8, r.side8, r.moved8, r.check8);

    return *((Piece*) &spot);
}

void board_t2::set(index_t index, Piece const piece) 
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
            return;
        
        case 1:
            rows[row].type2 = getType(piece);
            rows[row].side2 = getSide(piece);
            rows[row].moved2 = hasMoved(piece);
            rows[row].check2 = inCheck(piece);
            return;
        
        case 2:
            rows[row].type3 = getType(piece);
            rows[row].side3 = getSide(piece);
            rows[row].moved3 = hasMoved(piece);
            rows[row].check3 = inCheck(piece);
            return;
        
        case 3:
            rows[row].type4 = getType(piece);
            rows[row].side4 = getSide(piece);
            rows[row].moved4 = hasMoved(piece);
            rows[row].check4 = inCheck(piece);
            return;
        
        case 4:
            rows[row].type5 = getType(piece);
            rows[row].side5 = getSide(piece);
            rows[row].moved5 = hasMoved(piece);
            rows[row].check5 = inCheck(piece);
            return;
        
        case 5:
            rows[row].type6 = getType(piece);
            rows[row].side6 = getSide(piece);
            rows[row].moved6 = hasMoved(piece);
            rows[row].check6 = inCheck(piece);
            return;
        
        case 6:
            rows[row].type7 = getType(piece);
            rows[row].side7 = getSide(piece);
            rows[row].moved7 = hasMoved(piece);
            rows[row].check7 = inCheck(piece);
            return;
        
        case 7:
            rows[row].type8 = getType(piece);
            rows[row].side8 = getSide(piece);
            rows[row].moved8 = hasMoved(piece);
            rows[row].check8 = inCheck(piece);
            return;
    }
}

Piece board_t::get(index_t index) const 
{
    return board.get(index);
}

void board_t::set(index_t index, Piece const piece) 
{
    board.set(index, piece);
}    

void board_t::init() {
    set( 0, makeSpot(  Rook, Black, 0, 0));
    set( 1, makeSpot(Knight, Black, 0, 0));
    set( 2, makeSpot(Bishop, Black, 0, 0));
    set( 3, makeSpot( Queen, Black, 0, 0));
    set( 4, makeSpot(  King, Black, 0, 0));
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
    set(59, makeSpot( Queen, White, 0, 0));
    set(60, makeSpot(  King, White, 0, 0));
    set(61, makeSpot(Bishop, White, 0, 0));
    set(62, makeSpot(Knight, White, 0, 0));
    set(63, makeSpot(  Rook, White, 0, 0));
}

Piece board_t::getType(index_t from) const 
{
    return getType(get(from));
}
