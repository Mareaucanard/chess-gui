#include "piece.hpp"

#include <iostream>

bool Chess::Piece::operator==(const Piece &other) const
{
    return other.indexed_position == this->indexed_position;
}

char Chess::Piece::print_piece(Piece::piece_type type, bool is_white)
{
    char r = 0;
    switch (type) {
    case piece_type::Pawn:
        return 0;
    case piece_type::Knight:
        r = 'n';
        break;
    case piece_type::Bishop:
        r = 'b';
        break;
    case piece_type::Rook:
        r = 'r';
        break;
    case piece_type::Queen:
        r = 'q';
        break;
    case piece_type::King:
        r = 'k';
        break;
    default:
        return 0;
    }
    if (is_white) {
        r = toupper(r);
    }
    return r;
}

char Chess::Piece::print_piece()
{
    return print_piece(this->type, this->is_white);
}

Chess::Piece::Piece()
{
    this->is_white = true;
    this->type = piece_type::NONE;
    this->indexed_position = -1;
}

Chess::Piece::Piece(bool is_white, piece_type type, int indexed_position)
{
    this->is_white = is_white;
    this->type = type;
    this->indexed_position = indexed_position;
}
