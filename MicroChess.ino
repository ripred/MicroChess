/**
 * ArduinoChess.ino
 * 
 * March 2023 - Trent M. Wyatt
 * version 1.0.0
 *   First base
 * 
 * version 1.1.0
 * Modified board_t to save 8 bytes
 * 
 */

#include <Arduino.h>

#include "MicroChess.h"
#include "move.h"
#include "board.h"

void info();
void show();

board_t board;

void setup() {
    Serial.begin(115200); while (!Serial); Serial.write('\n');

    info();

    board.init();    

    show();
}

void loop() {

}

void show()
{
    static char const icons[16] = "PNBRQKpnbrqk";

    Serial.write('\n');

    for (unsigned char y=0; y < 8; ++y) {
        for (unsigned char x=0; x < 8; ++x) {
            Piece piece = board.get(y * 8 + x);
            Serial.write(' ');
            Serial.write((Empty == getType(piece)) ? 
                ((y ^ x) & 1 ? '*' : '.') : 
                icons[(getSide(piece) * 6) + getType(piece) - 1]);
            Serial.write(' ');
        }
        Serial.write('\n');
    }
    Serial.write('\n');
}

void info() 
{
    board_t1 b1;
    Serial.print(F("sizeof(board_t1) = "));
    Serial.println(sizeof(b1), DEC);

    board_t2 b2;
    Serial.print(F("sizeof(board_t2) = "));
    Serial.println(sizeof(b2), DEC);

    Serial.print(F("sizeof(board   ) = "));
    Serial.println(sizeof(board), DEC);
}
