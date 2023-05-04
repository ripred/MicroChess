/**
 * move.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * MicroChess move_t implementation
 * 
 */

#include <Arduino.h>
#include "MicroChess.h"
#include "move.h"


move_t::move_t() {

}


move_t::move_t(index_t f, index_t t, long v) : from(f), to(t), value(v) {

}