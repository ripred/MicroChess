/**
 * led_strip.cpp
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * MicroChess LED Strp Functions
 * 
 */
#include <Arduino.h>
#include <stdint.h>
#include "MicroChess.h"

#if not ARDUINO_ARCH_RENESAS && not ESP32
#include <FastLED.h>

FASTLED_USING_NAMESPACE

CRGB leds[BOARD_SIZE];

void init_led_strip() {
    // tell FastLED about the LED strip configuration
    FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, BOARD_SIZE).setCorrection(TypicalLEDStrip);
}

static uint8_t constexpr piece_colors[12*3] PROGMEM = {
  //    King         Pawn          Knight        Bishop        Rook          Queen
  // R,  G,  B,      R,  G,  B,      R,  G,  B,      R,  G,  B,      R,  G,  B,      R,  G,  B
     0, 16,  0,      8,  0,  0,      8,  8,  0,      0,  0,  8,      8,  0,  8,      0,  8,  8,     // Black
     0, 16,  0,     16,  0,  0,     16, 16,  0,      0,  0, 16,     16,  0, 16,      0, 16, 16      // White
  // 0,  0,  0,      0,  0,  0,      0,  0,  0,      0,  0,  0,      0,  0,  0,      0, 99,  0,     // Black
  // 0,  0,  0,      0,  0,  0,      0,  0,  0,      0,  0,  0,      0,  0,  0,      0, 99,  0      // White
};

void set_led_strip(index_t const flash /* = -1 */)
{
    // Stack Management
    // DECLARE ALL LOCAL VARIABLES USED IN THIS CONTEXT HERE AND
    // DO NOT MODIFY ANYTHING BEFORE CHECKING THE AVAILABLE STACK
    struct local_t {
        uint8_t x : 4,
                y : 4,
      board_index : 6,
            piece : 6,
             type : 3,
             side : 1,
        led_index : 6,
               ex : 6;
    } vars;

    index_t clr;

    //  Check for low stack space
    if (check_mem(MAKE)) { return; }

    // Now we can alter local variables! 😎 

    for (vars.y = 0; vars.y < 8; vars.y++) {
        for (vars.x = 0; vars.x < 8; vars.x++) {
            vars.board_index = vars.x + vars.y * 8u;
            vars.piece = board.get(vars.board_index);
            vars.type = getType(vars.piece);
            vars.side = getSide(vars.piece);
            vars.ex = 7 - vars.x;
            vars.led_index = BOARD_SIZE - (((vars.y & 1) ? vars.ex : vars.x) + vars.y * 8u) - 1;

            static index_t constexpr values_per_led  = index_t(3);
            static index_t constexpr values_per_side = index_t(sizeof(piece_colors) / 2);

            // There are only six color definitions per side in piece_colors.
            // Piece type values range from 0 (Empty) to 6 (King) so using the
            // raw type value would index beyond the table for Kings.
            // Mod the type by six to keep the index within bounds.
            clr = ((vars.type % 6) * values_per_led) +
                  (vars.side * values_per_side);

            leds[vars.led_index] = (Empty == vars.type) ? 
                // empty spots
                (((vars.ex ^ vars.y) & 1) ? CRGB(0,0,0) : CRGB(2,3,3)) : 

                // spot with piece
                (flash == vars.led_index) ? CRGB(0,96,96) : 
                CRGB(pgm_read_byte(&piece_colors[clr + 0]), 
                     pgm_read_byte(&piece_colors[clr + 1]), 
                     pgm_read_byte(&piece_colors[clr + 2]));
        }
    }
    FastLED.show();

} // set_led_strip()

#else

// dummy stubs

void init_led_strip() { }
void set_led_strip(index_t const /* flash = -1 */) { }

#endif // #if not ARDUINO_ARCH_RENESAS 

