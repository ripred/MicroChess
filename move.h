/**
 * move.h
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * header file for MicroChess
 * 
 */
#ifndef MOVE_INCL
#define MOVE_INCL

////////////////////////////////////////////////////////////////////////////////////////
// an entry in a move list
struct move_t 
{
public:
    int32_t  from : NUM_BITS_SPOT, 
               to : NUM_BITS_SPOT,
            value : ((sizeof(int32_t) * 8) - (NUM_BITS_SPOT * 2));

public:
    move_t() : from(0), to(0), value(0) { }
    move_t(index_t f, index_t t, long v) : from(f), to(t), value(v) {
    }

};

#endif // MOVE_INCL