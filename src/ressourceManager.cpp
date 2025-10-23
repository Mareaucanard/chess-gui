#include "ressourceManager.hpp"
#include <SFML/Graphics/Texture.hpp>
#include <string>
#include <vector>
#include <stdexcept>

using std::string;

bool RessourceManager::change_pieces_folder(std::filesystem::path path)
{
    const std::vector<std::string> colors = {"w", "b"};
    const std::vector<std::string> piece_types = {"P", "N", "B", "R", "Q", "K"};
    int index = 0;
    for (auto color : colors)
    {
        for (auto piece_type : piece_types)
        {
            sf::Texture texture;
            const string filename = color + piece_type + string(".png");
            if (texture.loadFromFile(path / filename))
            {
                this->texture_piece_array[index++] = texture;
            }
            else
            {
                throw std::invalid_argument("Folder " + path.string() + " was invalid");
                return false;
            }
        }
    }
    this->pieces_folder = path;
    return true;
}

RessourceManager::RessourceManager(std::string path)
{
    change_pieces_folder(std::filesystem::path(path));
}
