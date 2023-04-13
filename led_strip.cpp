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

// set the LED strip to match the board state
// 
void set_led_strip() 
{
    // tell FastLED about the LED strip configuration
    CRGB leds[BOARD_SIZE];
    FastLED.addLeds<WS2811,DATA_PIN,GRB>(leds, BOARD_SIZE).setCorrection(TypicalLEDStrip);

    CRGB clrs[2][7];

    index_t wclr = 16;
    index_t bclr = 4;

    clrs[Black][  Pawn] = CRGB(bclr,    0,    0);
    clrs[Black][Knight] = CRGB(   0, bclr,    0);
    clrs[Black][Bishop] = CRGB(bclr, bclr,    0);
    clrs[Black][  Rook] = CRGB(   0,    0, bclr);
    clrs[Black][ Queen] = CRGB(bclr,    0, bclr);
    clrs[Black][  King] = CRGB(   0, bclr, bclr);

    clrs[White][  Pawn] = CRGB(wclr,    0,    0);
    clrs[White][Knight] = CRGB(   0, wclr,    0);
    clrs[White][Bishop] = CRGB(wclr, wclr,    0);
    clrs[White][  Rook] = CRGB(   0,    0, wclr);
    clrs[White][ Queen] = CRGB(wclr,    0, wclr);
    clrs[White][  King] = CRGB(   0, wclr, wclr);

    for (index_t y = 0; y < 8; y++) {
        for (index_t x = 0; x < 8; x++) {
            index_t const board_index = ((y & 1) ? (7-x) : x) + y * 8;
            Piece   const piece = board.get(board_index);
            Piece   const type = getType(piece);
            Color   const side = getSide(piece);
            index_t const led_index = BOARD_SIZE - (x + y * 8) - 1;

            index_t ex = (y & 1) ? 7 - x : x;

            leds[led_index] = (Empty == type) ? (((ex^y)&1) ? CRGB(0,0,0) : CRGB(2,3,3)) : clrs[side][type];
        }
    }

    FastLED.show();
}


