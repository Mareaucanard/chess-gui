#include <SFML/Graphics.hpp>
#include <vector>

#include "chess/board.hpp"

class Arrow
{
public:
    Arrow(int start, int end, Chess::Board &origin);
    sf::RectangleShape shape;
    sf::CircleShape triangle;
    int start_pos;
    int end_pos;
    static sf::RectangleShape make_line_from_coordinates(sf::Vector2i start, sf::Vector2i end, float width);
    static sf::CircleShape make_triangle_from_coordinates(sf::Vector2i start, sf::Vector2i end, float width);
    static float get_effective_angle(const sf::Vector2i start_pos, const sf::Vector2i end_pos);
};

class ArrowsManager
{
public:
    std::vector<Arrow> arrows_list;

    sf::RectangleShape mouse_arrow;
    bool mouse_arrow_enabled = false;

    void update_mouse_arrow(sf::Vector2i mouse_pos);

    void resize_arrows(Chess::Board &board);

    void add_arrow(int start, int end, Chess::Board &board);
    void draw(sf::RenderWindow &window);
};
