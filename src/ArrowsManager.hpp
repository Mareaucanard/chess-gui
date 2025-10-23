#include <SFML/Graphics.hpp>
#include <vector>

#include "chess/board.hpp"

class Arrow
{
public:
    Arrow(int start, int end, sf::RectangleShape line);
    sf::RectangleShape shape;
    int start_pos;
    int end_pos;
};

class ArrowsManager
{
public:
    std::vector<Arrow> arrows_list;

    sf::RectangleShape make_line_from_coordinates(sf::Vector2i start, sf::Vector2i end, float width);
    sf::RectangleShape mouse_arrow;
    bool mouse_arrow_enabled = false;

    void update_mouse_arrow(sf::Vector2i mouse_pos);

    void resize_arrows(Chess::Board &board);

    void add_arrow(int start, int end, Chess::Board &board);
    void draw(sf::RenderWindow &window);
};
