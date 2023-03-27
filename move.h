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
private:

    static constexpr uint8_t NUM_BITS_VALUE = ((sizeof(int32_t) * 8) - (NUM_BITS_SPOT * 2) - 1);
public:
    int32_t  from : NUM_BITS_SPOT,  // the index into the board the move starts at
               to : NUM_BITS_SPOT,  // the index into the board the move finishes at
            value : NUM_BITS_VALUE, // the value of the move

            // flag used for soft-delete so that moves can be deleted and ignored without actually having to move them.
            // TODO: add the same idiom to the deletion of pieces during make_move(...) to avoid moving memory.
          deleted : 1;

public:
    move_t() : from(0), to(0), value(0) { }
    move_t(index_t f, index_t t, long v) : from(f), to(t), value(v) {
    }

};

#endif // MOVE_INCL