# Build
requires:

c++ compilter with c++23 \
SFML dev lib \
[Argparse](https://github.com/p-ranav/argparse) \
A chess engine like [StockFish](https://github.com/official-stockfish/Stockfish/releases/latest) \

# Tests
### Requirements

requires:
python3
[chess python package](https://python-chess.readthedocs.io/en/latest/)

The chess python package is just for checking if the GUI legal moves match
I tried to use the `go perft 1` from stockfish but it was quite a bit slower

### Test database sources
[4000_openings_legacy.epd and new2500.epd](https://github.com/MichaelB7/Crafty/)
[train-00000-of-00001.epd](https://huggingface.co/datasets/Lichess/chess-openings/tree/main)
[positions.epd](https://www.chessprogramming.org/Perft_Results)

# Usage
Usage: Chess GUI [--help] [--version] [--FEN VAR] [--log_FEN] [--pieces VAR] [--board VAR] [--get_moves]