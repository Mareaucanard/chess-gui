#pragma once

#include <SFML/Graphics.hpp>

#include "ressourceManager.hpp"

namespace Chess
{
class Piece
{
public:
    enum piece_type { NONE = -1, Pawn = 0, Knight = 1, Bishop = 2, Rook = 3, Queen = 4, King = 5 };

    bool operator==(const Piece &other) const;
    operator bool() const { return this->type != piece_type::NONE; }
    bool is_white = true;
    piece_type type = Pawn;
    int indexed_position = 0;
    char print_piece();

    Piece();
    Piece(bool is_white, piece_type type, int indexed_position);
    inline int get_texture_index() { return is_white ? type : type + 6; };
};
} // namespace Chess