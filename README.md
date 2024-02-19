# seredina-chess
A chess engine written entirely in C++.

## How to build
Build with make (command may have to be adapted on Windows)

## Features
Search features include:
- Alpha-Beta pruning with PVS and move ordering
- Very basic aspiration windows (full window after fail)
- Killer, countermove, history and continuation history heuristics
- Transposition table with resized windows, IIR
- Adaptive null move pruning
- Late move reduction
- Reverse futility pruning
- Late move pruning
- Singular extensions
Evaluation is achieved through an incrementally updated (768->256)x2->1 SCReLU NNUE (not quantized)
