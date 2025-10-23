#include "ArrowsManager.hpp"

#include <math.h>

sf::RectangleShape ArrowsManager::make_line_from_coordinates(sf::Vector2i start, sf::Vector2i end, float width)
{
    float length = sqrt(pow(start.x - end.x, 2) + pow(start.y - end.y, 2));
    sf::RectangleShape line({length, width});
    line.setPosition(start.x, start.y);
    line.setFillColor(sf::Color(0, 255, 0));

    float effective_angle = (atan2(end.x - start.x, end.y - start.y) - M_PI_2) * -1;
    line.setRotation(effective_angle * 180 / M_PI);
    return line;
}

void ArrowsManager::update_mouse_arrow(sf::Vector2i mouse_pos)
{
    mouse_arrow = make_line_from_coordinates({mouse_arrow.getPosition().x, mouse_arrow.getPosition().y}, mouse_pos, mouse_arrow.getSize().y);
}

void ArrowsManager::resize_arrows(Chess::Board &board)
{
    float width = board.square_size / 10.f;
    for (auto &line : arrows_list) {
        auto origin = board.get_origin();
        int start_x = line.start_pos % 8;
        int start_y = line.start_pos / 8;
        sf::Vector2i start_pos = {origin.x + board.square_size * (start_x + 0.5), origin.y - (board.square_size * (start_y + 0.5))};

        int end_x = line.end_pos % 8;
        int end_y = line.end_pos / 8;
        sf::Vector2i end_pos = {origin.x + board.square_size * (end_x + 0.5), origin.y - (board.square_size * (end_y + 0.5))};
        line.shape = make_line_from_coordinates(start_pos, end_pos, width);
    }
}

void ArrowsManager::add_arrow(int start, int end, Chess::Board &board)
{
    auto origin = board.get_origin();
    int start_x = start % 8;
    int start_y = start / 8;
    sf::Vector2i start_pos = {origin.x + board.square_size * (start_x + 0.5), origin.y - (board.square_size * (start_y + 0.5))};

    int end_x = end % 8;
    int end_y = end / 8;
    sf::Vector2i end_pos = {origin.x + board.square_size * (end_x + 0.5), origin.y - (board.square_size * (end_y + 0.5))};

    float width = board.square_size / 10.f;
    Arrow arrow(start, end, make_line_from_coordinates(start_pos, end_pos, width));
    arrows_list.push_back(arrow);
}

void ArrowsManager::draw(sf::RenderWindow &window)
{
    for (auto &line : arrows_list) {
        window.draw(line.shape);
    }
    if (mouse_arrow_enabled) {
        update_mouse_arrow(sf::Mouse::getPosition(window));
        window.draw(mouse_arrow);
    }
}

Arrow::Arrow(int start, int end, sf::RectangleShape line)
{
    this->start_pos = start;
    this->end_pos = end;
    this->shape = line;
}
