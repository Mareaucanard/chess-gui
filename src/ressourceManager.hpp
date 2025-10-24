#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <filesystem>
#include <map>
#pragma once

class RessourceManager
{
public:
    std::filesystem::path pieces_folder;
    sf::Texture texture_piece_array[12];
    sf::Sprite sprite_piece_array[12];
    bool change_pieces_folder(std::filesystem::path);
    RessourceManager(std::string path);

private:
};