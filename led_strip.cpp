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

static uint8_t constexpr piece_colors[12*3] PROGMEM = {
//      King         Pawn          Knight        Bishop        Rook          Queen
//   R,  G,  B,    R,  G,  B,    R,  G,  B,    R,  G,  B,    R,  G,  B,    R,  G,  B
     0, 16,  0,    8,  0,  0,    8,  8,  0,    0,  0,  8,    8,  0,  8,    0,  8,  8,     // Black
     0, 16,  0,   16,  0,  0,   16, 16,  0,    0,  0, 16,   16,  0, 16,    0, 16, 16      // White
    //  0,  0,  0,    0,  0,  0,    0,  0,  0,    0,  0,  0,    0,  0,  0,    0, 99,  0,     // Black
    //  0,  0,  0,    0,  0,  0,    0,  0,  0,    0,  0,  0,    0,  0,  0,    0, 99,  0      // White
};

void set_led_strip(index_t const flash /* = -1 */)
{
    for (index_t y = 0; y < 8; y++) {
        for (index_t x = 0; x < 8; x++) {
            index_t const board_index = x + y * 8;
            Piece   const piece = board.get(board_index);
            Piece   const type = getType(piece);
            Color   const side = getSide(piece);
            index_t const led_index = BOARD_SIZE - (((y & 1) ? (7 - x) : x) + y * 8) - 1;

            index_t const ex = 7 - x;

            static index_t constexpr values_per_led  = index_t(3);
            static index_t constexpr values_per_side = index_t(sizeof(piece_colors) / 2);

            index_t const clr = (type * values_per_led) + (side * values_per_side);

            leds[led_index] = (Empty == type) ? 
                // empty spots
                (((ex ^ y) & 1) ? CRGB(0,0,0) : CRGB(2,3,3)) : 

                // spot with piece
                (flash == led_index) ? CRGB(0,96,96) : 
                CRGB(pgm_read_byte(&piece_colors[clr + 0]), 
                     pgm_read_byte(&piece_colors[clr + 1]), 
                     pgm_read_byte(&piece_colors[clr + 2]));
        }
    }
    FastLED.show();

} // set_led_strip()