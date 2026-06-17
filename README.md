# MicroChess

[![Arduino CI](https://img.shields.io/static/v1?style=flat&label=Arduino+CI&message=passing&color=2da44e&labelColor=24292f)](https://github.com/ripred/MicroChess/actions/workflows/arduino_test_runner.yml)
[![Arduino-lint](https://img.shields.io/static/v1?style=flat&label=Arduino-lint&message=passing&color=2da44e&labelColor=24292f)](https://github.com/ripred/MicroChess/actions/workflows/arduino-lint.yml)
[![JSON check](https://img.shields.io/static/v1?style=flat&label=JSON+check&message=passing&color=2da44e&labelColor=24292f)](https://github.com/ripred/MicroChess/actions/workflows/jsoncheck.yml)
[![Release](https://img.shields.io/static/v1?style=flat&label=Release&message=1.0.0&color=8250df&labelColor=24292f)](https://github.com/ripred/MicroChess/releases/latest)
[![License](https://img.shields.io/static/v1?style=flat&label=License&message=MIT&color=0969da&labelColor=24292f)](https://github.com/ripred/MicroChess/blob/main/LICENSE)
[![Stars](https://img.shields.io/static/v1?style=flat&label=Stars&message=31&color=bf8700&labelColor=24292f)](https://github.com/ripred/MicroChess/stargazers)
[![Forks](https://img.shields.io/static/v1?style=flat&label=Forks&message=4&color=6f42c1&labelColor=24292f)](https://github.com/ripred/MicroChess/network/members)

MicroChess is a killer chess engine built for embedded systems, cramming a full chess experience into less than 2K of RAM and 32K of program flash. It nails every chess rule—en passant, castling, pawn promotion, check, checkmate, stalemate—and adds quiescent searches for extra smarts. This project is your ticket to mastering turn-based game engines, whether you’re tackling chess, checkers, or some wild new game. The heavy lifting’s done, so you can jump right in!

## Features
- **Insane Memory Efficiency**: Runs in under 2K of RAM, with support for up to 6-ply look-ahead, making it a beast on tiny Arduino boards.
- **Stack Surfing FTW**: Checks stack depth at runtime to dive deeper into the search tree when there’s room, squeezing out max performance.
- **Full Chess Rules**: Handles every move, from pawn pushes to king castling, with no shortcuts.
- **Totally Hackable**: Open-source code you can tweak for custom inputs, outputs, or crazy projects.
- **Visual Awesomeness**:
  - **LED Strip**: Lights up an 8x8 matrix to show the board in real-time ![MicroChessSmall.gif](https://github.com/ripred/MicroChess/blob/main/MicroChessSmall.gif)
  - **Serial Console**: Prints moves and board states in the Arduino Serial Monitor ![MicroChessConsole2.gif](https://github.com/ripred/MicroChess/blob/main/MicroChessConsole2.gif)

## Wiki Documentation
We’ve got a shiny new wiki packed with everything you need to dominate with MicroChess. From setup to code deep dives, it’s all there. Check out the [MicroChess Wiki](https://github.com/ripred/MicroChess/wiki) for the full scoop!

## Why MicroChess?
This project isn’t just a chess engine—it’s a blueprint for building lean, mean game engines in constrained environments. Use it straight-up or remix it for your own creations. The code’s open, the sky’s the limit, and all the hard work’s already done!

## License
MicroChess is licensed under the MIT License. See the [LICENSE](https://github.com/ripred/MicroChess/blob/main/LICENSE) file for details.

## Bug Fixes

- **LED Strip King Colors**: Kings were sometimes displayed with incorrect LED colors
  because the color lookup indexed beyond the end of the color table. The index
  now wraps correctly so both white and black kings use the right colors.
