#ifndef MOVE_INCL
#define MOVE_INCL

#include "MicroChess.h"
#include "board.h"

struct move_t {
    unsigned long from : 6,
                    to : 6,
              captured : 7,
                 value;

    move_t()
        : from(0), to(0), captured(Empty), value(0) {}

    move_t(unsigned int fromCol, unsigned int fromRow, unsigned int toCol, unsigned int toRow,
               int value)
        : from(fromRow*8+toCol),
          to(toRow*8+toCol),
          captured(Empty),
          value(value) 
    {
        this->from = fromCol + fromRow * 8;
        this->to = toCol + toRow * 8;
    }

    unsigned int getFromCol() const { return from % 8; }
    unsigned int getFromRow() const { return from / 8; }
    unsigned int getToCol() const { return to % 8; }
    unsigned int getToRow() const { return to / 8; }
    unsigned int getFrom() const { return from; }
    unsigned int getTo() const { return to; }
    long getValue() const { return value; }
    Piece getCaptured() const { return captured; }
    bool isCapture() const { return captured != Empty; }

    void setValue(int val) { value = val; }
    void setCaptured(Piece p) { captured = p; }

    bool operator == (move_t const& move) const {
        if (this == &move) return true;
        return from == move.from && to == move.to;
    }

    char *to_string(unsigned int const flag, char *buff) const {
        *buff = 0;
        if (flag & 1u) 
        {
            strcat(buff, getCoords(from));
            strcat(buff, " to ");
            strcat(buff, getCoords(to));
            strcat(buff, " ");
        }
        
        if (flag & 2u) {
            strcat(buff, getNotate(from));
            strcat(buff, " to ");
            strcat(buff, getNotate(to));
        }

        if (flag & 4u) 
        {
            strcat(buff, " value:");
            strcat(buff, to_string(value, buff));
        }

        return buff;
    }

    bool isValid() const { return from != to; }

    bool isValid(board_t const& board) const {
        return (from != to) && (board.getType(from) != Empty);
    }
};

#endif // MOVE_INCL
