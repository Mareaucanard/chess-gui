#include "piece.hpp"

#include <iostream>

bool Chess::Piece::operator==(const Piece &other) const
{
    return other.indexed_position == this->indexed_position;
}

// void Chess::Piece::update_ressources(RessourceManager &manager)
// {
//     this->sprite.setTexture(manager.texture_piece_array[this->get_texture_index()]);
// }

// void Chess::Piece::update_sprite_position(sf::Vector2<float> board_origin, int square_size)
// {
//     if (type == Piece::NONE || this->sprite.getTexture() == nullptr) {
//         return;
//     }
//     int x = indexed_position % 8;
//     int y = indexed_position / 8;

//     float true_x = board_origin.x + square_size * x;
//     float true_y = board_origin.y - square_size * (1 + y);

//     this->sprite.setPosition(true_x, true_y);
// }

// void Chess::Piece::scale_piece(int square_size, RessourceManager &manager)
// {
//     if (this->sprite.getTexture() == nullptr) {
//         update_ressources(manager);
//     }
//     sf::Vector2 test_size = this->sprite.getTexture()->getSize();

//     this->sprite.setScale((float)square_size / test_size.x, (float)square_size / test_size.y);
// }

char Chess::Piece::print_piece()
{
    char r = 0;
    switch (this->type) {
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
