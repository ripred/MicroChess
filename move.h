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
class move_t 
{
    public:
    int8_t    from : NUM_BITS_SPOT,     // the index into the board the move starts at
                to : NUM_BITS_SPOT;     // the index into the board the move finishes at
    int32_t  value;                     // the value of the move

    move_t();
    move_t(index_t f, index_t t, long v);

};  // move_t

#endif // MOVE_INCL