/**
 * led_strip.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * MicroChess LED Strp Functions
 * 
 */

#include <Arduino.h>
#include <FastLED.h>
#include "MicroChess.h"

#define   DATA_PIN    6

FASTLED_USING_NAMESPACE

CRGB leds[BOARD_SIZE];

void init_led_strip() {
    // tell FastLED about the LED strip configuration
    FastLED.addLeds<WS2811,DATA_PIN,GRB>(leds, BOARD_SIZE).setCorrection(TypicalLEDStrip);
}


static uint8_t const blackColors[12*3] PROGMEM = {
    //  Pawn         Knight         Rook         Bishop        Queen          King
     0,  4,  0,    4,  0,  0,    4,  4,  0,    0,  0,  4,    0,  4,  4,    4,  0,  4,     // Black
     0, 16,  0,   16,  0,  0,   16, 16,  0,    0,  0, 16,    0, 16, 16,   16,  0, 16      // White
};

void set_led_strip() 
{
    for (index_t y = 0; y < 8; y++) {
        for (index_t x = 0; x < 8; x++) {
            index_t const board_index = ((y & 1) ? (7-x) : x) + y * 8;
            Piece   const piece = board.get(board_index);
            Piece   const type = getType(piece);
            Color   const side = getSide(piece);
            index_t const led_index = BOARD_SIZE - (x + y * 8) - 1;

            index_t ex = (y & 1) ? 7 - x : x;

            index_t foo = type * 3 + side * 18;

            leds[led_index] = (Empty == type) ? (((ex^y)&1) ? 
                CRGB(0,0,0) : 
                CRGB(2,3,3)) : 
                CRGB(pgm_read_byte(&blackColors[foo]), 
                     pgm_read_byte(&blackColors[foo+1]), 
                     pgm_read_byte(&blackColors[foo+2]));
        }
    }
    FastLED.show();

} // set_led_strip()