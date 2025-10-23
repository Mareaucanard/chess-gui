#include "ArrowsManager.hpp"

#include <math.h>

sf::RectangleShape Arrow::make_line_from_coordinates(sf::Vector2i start, sf::Vector2i end, float width)
{
    float length = sqrt(pow(start.x - end.x, 2) + pow(start.y - end.y, 2));
    sf::RectangleShape line({length, width});
    line.setOrigin(0, width / 2);
    line.setPosition(start.x, start.y);
    line.setFillColor(sf::Color(50, 255, 50));

    float effective_angle = get_effective_angle(start, end);
    line.setRotation(effective_angle * 180 / M_PI);
    return line;
}

sf::CircleShape Arrow::make_triangle_from_coordinates(sf::Vector2i start, sf::Vector2i end, float width)
{
    sf::CircleShape circle(width * 3, 3);
    circle.setPosition(end.x, end.y);
    circle.setFillColor(sf::Color(0, 255, 0));
    circle.setOrigin(width * 3, width * 3);
    circle.setRotation(-30 + (get_effective_angle(start, end) * 180 / M_PI));
    return circle;
}

void ArrowsManager::update_mouse_arrow(sf::Vector2i mouse_pos)
{
    mouse_arrow = Arrow::make_line_from_coordinates({mouse_arrow.getPosition().x, mouse_arrow.getPosition().y}, mouse_pos, mouse_arrow.getSize().y);
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
        line.shape = Arrow::make_line_from_coordinates(start_pos, end_pos, width);
        line.triangle = Arrow::make_triangle_from_coordinates(start_pos, end_pos, width);
    }
}

void ArrowsManager::add_arrow(int start, int end, Chess::Board &board)
{
    for (auto it = arrows_list.begin(); it != arrows_list.end(); it++) {
        if (it->start_pos == start && it->end_pos == end) {
            arrows_list.erase(it);
            return;
        }
    }

    Arrow arrow(start, end, board);
    arrows_list.push_back(arrow);
}

void ArrowsManager::draw(sf::RenderWindow &window)
{
    for (auto &line : arrows_list) {
        window.draw(line.shape);
        window.draw(line.triangle);
    }
    if (mouse_arrow_enabled) {
        update_mouse_arrow(sf::Mouse::getPosition(window));
        window.draw(mouse_arrow);
    }
}

float Arrow::get_effective_angle(const sf::Vector2i start_pos, const sf::Vector2i end_pos)
{
    return (atan2(end_pos.x - start_pos.x, end_pos.y - start_pos.y) - M_PI_2) * -1;
}

Arrow::Arrow(int start, int end, Chess::Board &board)
{
    this->start_pos = start;
    this->end_pos = end;

    auto origin = board.get_origin();
    int start_x = start % 8;
    int start_y = start / 8;
    sf::Vector2i start_pos = {origin.x + board.square_size * (start_x + 0.5), origin.y - (board.square_size * (start_y + 0.5))};

    int end_x = end % 8;
    int end_y = end / 8;
    sf::Vector2i end_pos = {origin.x + board.square_size * (end_x + 0.5), origin.y - (board.square_size * (end_y + 0.5))};

    float width = board.square_size / 10.f;
    this->triangle = make_triangle_from_coordinates(start_pos, end_pos, width);
    this->shape = make_line_from_coordinates(start_pos, end_pos, width);
    this->triangle = triangle;
}
