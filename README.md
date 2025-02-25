<!-- [![Arduino CI](https://github.com/ripred/MicroChess/workflows/Arduino%20CI/badge.svg)](https://github.com/marketplace/actions/arduino_ci) -->
[![Arduino-lint](https://github.com/ripred/MicroChess/actions/workflows/arduino-lint.yml/badge.svg)](https://github.com/ripred/MicroChess/actions/workflows/arduino-lint.yml)
![code size:](https://img.shields.io/github/languages/code-size/ripred/MicroChess)
[![GitHub release](https://img.shields.io/github/release/ripred/MicroChess.svg?maxAge=3600)](https://github.com/ripred/MicroChess/releases)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/ripred/MicroChess/blob/master/LICENSE)

# MicroChess
MicroChess is an embedded chess engine designed to run with only 2K of RAM and 32K of program flash, complete with en passant capture, castling, and quiescent searches. 

The design includes use of the minimax/maximin algorithm complete with alpha-beta pruning and culling. The project is designed to teach the theory and practical implementation of a complete turn-based game engine suitable for a range of games including chess and checkers. 

Keeping under 2K of memory, the game is capable of looking up to 6 ply levels, or turns, ahead. The source code is fully available and can be used as the base for many creative projects. 

All of the heavy lifting has been done and the code can be used as-is or modified to support additional inputs or outputs. 

Of special note is the implementation of "stack surfing" (for lack of a better term). This enables the code to check the stack depth at runtime and descend to deeper ply depths if there is stack space available for the recursive call! This is disabled by default but can be enabled in the code by changing the runtime options.

Currently it displays the game board using an LED strip arranged in an 8x8 grid. It also outputs the game board in ASCII to the serial debug console. The game plays itself for both sides and accepts moves via the serial port at any time so it can be easily played against or controlled by another serial device. 

Note that it's a work in progress so it changes often.

Update: The en-passant capture bug has been fixed!

&nbsp;
[Reddit Discusson](https://www.reddit.com/r/arduino/comments/11q4916/so_you_want_to_build_a_chess_engine/?utm_source=share&utm_medium=web2x&context=3)

&nbsp;
Example LED Strip Board (while profiling ~ 900 moves/sec):

![LED Strip Game Board](MicroChessSmall.gif)

&nbsp;

Example Serial Console Output during 4-move Opening Book Checkmate:

![Console Output](MicroChessConsole2.gif)

&nbsp;
