/**
 * ArduinoChess.ino
 * 
 */

#include "MicroChess.h"
#include "move.h"
#include "board.h"

Board board;

void setup() {
//    Serial.begin(9600);
    memset(board.board, Empty, BOARD_SIZE);
}

void loop() {

}
