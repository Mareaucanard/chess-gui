#include "piece.hpp"

#include <iostream>

bool Chess::Piece::operator==(const Piece &other) const
{
    return other.indexed_position == this->indexed_position;
}

void Chess::Piece::update_ressources(RessourceManager &manager)
{
    this->sprite.setTexture(manager.texture_piece_array[this->get_texture_index()]);
}

void Chess::Piece::update_sprite_position(sf::Vector2<float> board_origin, int square_size)
{
    if (type == Piece::NONE || this->sprite.getTexture() == nullptr) {
        return;
    }
    int x = indexed_position % 8;
    int y = indexed_position / 8;

    float true_x = board_origin.x + square_size * x;
    float true_y = board_origin.y - square_size * (1 + y);

    this->sprite.setPosition(true_x, true_y);
}

void Chess::Piece::scale_piece(int square_size)
{
    sf::Vector2 test_size = this->sprite.getTexture()->getSize();

    this->sprite.setScale((float)square_size / test_size.x, (float)square_size / test_size.y);
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
