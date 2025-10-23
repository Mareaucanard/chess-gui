#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <chess/piece.hpp>
#include <filesystem>
#include <iostream>
#include <vector>

namespace Chess
{
const char default_FEN[57] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
struct Move {
    int start_pos;
    int end_pos;

    inline Move(int spos, int epos) : start_pos(spos), end_pos(epos) {}
    bool operator==(const Move &other) const;
};

std::ostream &operator<<(std::ostream &os, Move move);

class Board
{
public:
    int square_size = 64;

    sf::Sprite sprite;
    sf::Texture texture;
    RessourceManager *manager = nullptr;
    std::vector<Move> legal_moves;

    Board();

    void play_move(Move move);
    void check_if_move_voids_castle(Move move, Piece &moving_piece);

    void setup_textures(std::filesystem::path path, RessourceManager &manager);
    void setup_board_textures(std::filesystem::path path);
    void setup_pieces_textures(RessourceManager &manager);

    void update_sprite_position(sf::RectangleShape &shape, sf::Vector2f &board_origin, int index);
    static std::string get_clean_coordinate(char pos);

    bool load_from_FEN(std::string FEN);
    std::string get_FEN();
    void print_all_legal_moves();
    int get_square_from_mouse(sf::Vector2i local_position);

    void resize_board(unsigned width, unsigned height);
    void scale_board();

    void add_piece(bool is_white, Piece::piece_type type, int indexed_pos);

    std::vector<Move> get_all_legal_moves(bool is_mover_white);
    void add_sliding_moves(Piece &piece, std::vector<Move> &moves);
    void add_knight_moves(Piece &piece, std::vector<Move> &moves);
    void add_pawn_moves(Piece &piece, std::vector<Move> &moves);
    void add_king_moves(Piece &piece, std::vector<Move> &moves);
    std::vector<Move> get_all_moves_for_square(int indexed_square);
    void display_square_moves(int index);

    void scale_pieces();

    sf::Vector2f get_origin();
    void draw_board(sf::RenderWindow &window);
    bool is_piece_picked_up = false;

    Piece full_board[64];
    std::vector<int> moves_for_selected_piece;
    int selected_piece = -1;
    sf::RectangleShape highlighted_square_shape;
    sf::RectangleShape main_highlighted_square_shape;

    int direction_offsets[8] = {8, -8, -1, 1, 7, -7, 9, -9};
    int distance_to_borders[64][8];
    void precompute_distance_to_borders();

    bool is_white_turn = true;

    bool king_white_castle = true;
    bool queen_white_castle = true;
    bool king_black_castle = true;
    bool queen_black_castle = true;

    int en_passant_square = -1; // -1 means no en passant
    int halfmove_clock = 0;
    int fullmove_number = 1;

    bool log_FEN = false;

    std::vector<Move> move_list;
};
} // namespace Chess