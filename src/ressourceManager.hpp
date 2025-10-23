#include <filesystem>
#include <map>
#include <SFML/Graphics/Texture.hpp>
#pragma once

class RessourceManager
{
public:
    std::filesystem::path pieces_folder;
    sf::Texture texture_piece_array[12];
    bool change_pieces_folder(std::filesystem::path);
    RessourceManager(std::string path);

private:
};