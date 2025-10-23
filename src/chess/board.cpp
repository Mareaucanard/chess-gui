#include "board.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace Chess;

Board::Board()
{
    precompute_distance_to_borders();
}

void Board::play_move(Move move)
{
    bool is_capture = full_board[move.end_pos].type != Piece::NONE;
    auto &moving_piece = full_board[move.start_pos];

    if (is_capture || moving_piece.type == Piece::Pawn) {
        halfmove_clock = 0;
    } else {
        halfmove_clock += 1;
    }

    if (is_white_turn == false) {
        fullmove_number += 1;
    }

    en_passant_square = -1;
    if (moving_piece.type == Piece::Pawn && abs(move.start_pos - move.end_pos) == 16) {
        if (is_white_turn) {
            en_passant_square = move.start_pos + 8;
        } else {
            en_passant_square = move.start_pos - 8;
        }
    }

    check_if_move_voids_castle(move, moving_piece);

    full_board[move.end_pos] = full_board[move.start_pos];
    full_board[move.end_pos].indexed_position = move.end_pos;
    full_board[move.start_pos] = Piece();

    is_white_turn = !is_white_turn;
    move_list.push_back(move);
    if (log_FEN) {
        std::cout << get_FEN() << std::endl;
    }
    this->legal_moves = get_all_legal_moves(is_white_turn);
}

void Board::check_if_move_voids_castle(Move move, Piece &moving_piece)
{
    auto &captured_piece = full_board[move.end_pos];

    if (moving_piece.type == Piece::King) {
        if (is_white_turn) {
            king_white_castle = false;
            queen_white_castle = false;
        } else {
            king_black_castle = false;
            queen_black_castle = false;
        }
    }

    if (moving_piece.type == Piece::Rook) {
        int rank = moving_piece.indexed_position / 8;
        int file = moving_piece.indexed_position % 8;
        bool is_on_starting_rank = rank == (is_white_turn ? 0 : 7);
        if (file == 0 && is_on_starting_rank) {
            if (is_white_turn) {
                queen_white_castle = false;
            } else {
                queen_black_castle = false;
            }
        }
        if (file == 7 && is_on_starting_rank) {
            if (is_white_turn) {
                king_white_castle = false;
            } else {
                king_black_castle = false;
            }
        }
    }

    if (captured_piece.type == Piece::Rook) {
        int rank = moving_piece.indexed_position / 8;
        int file = moving_piece.indexed_position % 8;
        bool is_on_starting_rank = rank == (is_white_turn ? 7 : 0);
        if (file == 0 && is_on_starting_rank) {
            if (is_white_turn) {
                queen_black_castle = false;
            } else {
                queen_white_castle = false;
            }
        }
        if (file == 7 && is_on_starting_rank) {
            if (is_white_turn) {
                king_black_castle = false;
            } else {
                king_white_castle = false;
            }
        }
    }
}

void Board::setup_textures(std::filesystem::path path, RessourceManager &manager)
{
    setup_board_textures(path);
    setup_pieces_textures(manager);
}

void Board::setup_board_textures(std::filesystem::path path)
{
    if (!this->texture.loadFromFile(path)) {
        throw std::runtime_error("SFML board error");
    }
    this->sprite.setTexture(this->texture);
    scale_board();
}

void Board::setup_pieces_textures(RessourceManager &manager)
{
    for (int i = 0; i < 64; i++) {
        auto &piece = full_board[i];
        if (piece.type != Piece::NONE) {
            piece.update_ressources(manager);
        }
    }
}

Piece::piece_type type_from_char(char c)
{
    switch (tolower(c)) {
    case 'k':
        return Piece::piece_type::King;
    case 'q':
        return Piece::piece_type::Queen;
    case 'r':
        return Piece::piece_type::Rook;
    case 'b':
        return Piece::piece_type::Bishop;
    case 'n':
        return Piece::piece_type::Knight;
    default:
    case 'p':
        return Piece::piece_type::Pawn;
    }
}

bool Board::load_from_FEN(std::string FEN)
{
    for (int i = 0; i < 64; i++) {
        full_board[i] = Piece();
    }

    int file = 0;
    int rank = 7;

    std::string fen_board = FEN.substr(0, FEN.find(' '));

    for (char symbol : fen_board) {
        if (symbol == '/') {
            file = 0;
            rank -= 1;
        } else {
            if (isdigit(symbol)) {
                file += symbol - '0';
            } else {
                bool is_white = isupper(symbol);
                auto type = type_from_char(symbol);
                int pos = rank * 8 + file;

                this->add_piece(is_white, type, pos);
                file += 1;
            }
        }
    }
    legal_moves = get_all_legal_moves(is_white_turn);
    return true;
}

std::string Board::get_FEN()
{
    std::string FEN;
    for (int rank = 7; rank >= 0; rank -= 1) {
        int numEmptyFiles = 0;
        for (int file = 0; file < 8; file += 1) {
            int i = rank * 8 + file;
            auto &piece = full_board[i];
            if (piece.type != Piece::NONE) {
                if (numEmptyFiles != 0) {
                    FEN += std::to_string(numEmptyFiles);
                    numEmptyFiles = 0;
                }
                int pieceType = piece.type;
                char pieceChar = ' ';
                switch (pieceType) {
                case Piece::Rook:
                    pieceChar = 'R';
                    break;
                case Piece::Knight:
                    pieceChar = 'N';
                    break;
                case Piece::Bishop:
                    pieceChar = 'B';
                    break;
                case Piece::Queen:
                    pieceChar = 'Q';
                    break;
                case Piece::King:
                    pieceChar = 'K';
                    break;
                case Piece::Pawn:
                    pieceChar = 'P';
                    break;
                }
                FEN += piece.is_white ? pieceChar : tolower(pieceChar);
            } else {
                numEmptyFiles += 1;
            }
        }
        if (numEmptyFiles != 0) {
            FEN += std::to_string(numEmptyFiles);
        }
        if (rank != 0) {
            FEN += "/";
        }
    }

    FEN = FEN + " " + (is_white_turn ? "w" : "b");

    FEN += " ";
    // If no castle is possible
    if ((king_white_castle || king_black_castle || queen_white_castle || queen_black_castle) == false) {
        FEN += "-";
    } else {
        if (king_white_castle) {
            FEN += "K";
        }
        if (queen_white_castle) {
            FEN += "Q";
        }
        if (king_black_castle) {
            FEN += "k";
        }
        if (queen_black_castle) {
            FEN += "q";
        }
    }

    FEN += " ";
    if (en_passant_square == -1) {
        FEN += "-";
    } else {
        FEN += get_clean_coordinate(en_passant_square);
    }
    FEN = FEN + " " + std::to_string(halfmove_clock) + " " + std::to_string(fullmove_number);

    return FEN;
}

void Board::print_all_legal_moves()
{
    for (auto &move : legal_moves) {
        std::cout << move << std::endl;
    }
}

int Board::get_square_from_mouse(sf::Vector2i local_position)
{
    sf::Vector2f board_location = {local_position.x - get_origin().x, local_position.y - get_origin().y};

    int true_x = std::floor(board_location.x / square_size);
    int true_y = std::floor(board_location.y / -square_size);
    if (true_x < 8 && true_x >= 0 && true_y < 8 && true_y >= 0) {
        return true_y * 8 + true_x;
    } else {
        return -1;
    }
}

void Chess::Board::resize_board(unsigned width, unsigned height)
{
    square_size = std::min(width, height) / 8;
    scale_board();
    scale_pieces();
}

void Chess::Board::scale_board()
{
    float scale = square_size * 8.f / this->texture.getSize().x;
    this->sprite.setScale(scale, scale);

    this->highlighted_square_shape.setSize({(float)square_size, (float)square_size});
    this->main_highlighted_square_shape.setSize({(float)square_size, (float)square_size});
    this->highlighted_square_shape.setFillColor(sf::Color(255, 0, 0, 100));
    this->main_highlighted_square_shape.setFillColor(sf::Color(255, 100, 0, 100));
}

void Board::scale_pieces()
{
    for (int i = 0; i < 64; i++) {
        auto &piece = full_board[i];
        if (piece.type != Piece::NONE) {
            piece.scale_piece(square_size);
        }
    }
}

sf::Vector2f Board::get_origin()
{
    return this->sprite.getPosition() + sf::Vector2f(0, square_size * 8);
}

void Board::add_piece(bool is_white, Piece::piece_type type, int indexed_pos)
{
    Piece piece(is_white, type, indexed_pos);
    full_board[indexed_pos] = piece;
}

std::vector<Move> Board::get_all_legal_moves(bool is_mover_white)
{
    std::vector<Move> result;
    for (int i = 0; i < 64; i++) {
        auto &piece = full_board[i];
        if (is_mover_white != piece.is_white) {
            continue;
        }
        if (piece.type == Piece::Rook || piece.type == Piece::Bishop || piece.type == Piece::Queen) {
            add_sliding_moves(piece, result);
        } else if (piece.type == Piece::Knight) {
            add_knight_moves(piece, result);
        } else if (piece.type == Piece::Pawn) {
            add_pawn_moves(piece, result);
        } else if (piece.type == Piece::King) {
            add_king_moves(piece, result);
        }
    }
    return result;
}

void Board::add_knight_moves(Piece &piece, std::vector<Move> &moves)
{
    int x = piece.indexed_position % 8;
    int y = piece.indexed_position / 8;

    const std::pair<int, int> knight_moves[8] = {
        std::pair(2, 1), std::pair(-2, -1), std::pair(2, -1), std::pair(-2, 1),
        std::pair(1, 2), std::pair(-1, -2), std::pair(1, -2), std::pair(-1, 2),
    };
    for (int i = 0; i < 8; i++) {
        int target_x = x + knight_moves[i].first;
        int target_y = y + knight_moves[i].second;
        if (target_x < 0 || target_x >= 8 || target_y < 0 || target_y >= 8) {
            continue;
        }

        int target_square = target_y * 8 + target_x;
        if (full_board[target_square] && full_board[target_square].is_white == piece.is_white) {
            continue;
        }

        moves.push_back(Move(piece.indexed_position, target_square));
    }
}

bool is_inside_board(int square)
{
    return square >= 0 && square <= 63;
}

void Board::add_pawn_moves(Piece &piece, std::vector<Move> &moves)
{
    int y = piece.indexed_position / 8;
    int x = piece.indexed_position % 8;
    int forward = piece.is_white ? +8 : -8;
    int start_rank = piece.is_white ? 1 : 6;
    bool is_first_move = y == start_rank;

    int target_square = piece.indexed_position + forward;
    if (is_inside_board(target_square) && !full_board[target_square]) {
        moves.push_back(Move(piece.indexed_position, target_square));
        target_square = piece.indexed_position + forward * 2;
        if (is_first_move && is_inside_board(target_square) && !full_board[target_square]) {
            moves.push_back(Move(piece.indexed_position, target_square));
        }
    }

    // Check for captures
    target_square = piece.indexed_position + forward + 1;
    if (x != 7 && is_inside_board(target_square) && full_board[target_square] && piece.is_white != full_board[target_square].is_white) {
        moves.push_back(Move(piece.indexed_position, target_square));
    }

    target_square = piece.indexed_position + forward - 1;
    if (x != 0 && is_inside_board(target_square) && full_board[target_square] && piece.is_white != full_board[target_square].is_white) {
        moves.push_back(Move(piece.indexed_position, target_square));
    }
}

void Chess::Board::add_king_moves(Piece &piece, std::vector<Move> &moves)
{
    int x = piece.indexed_position % 8;
    int y = piece.indexed_position / 8;

    const std::pair<int, int> knight_moves[8] = {
        std::pair(0, 1), std::pair(1, 1), std::pair(1, 0), std::pair(1, -1), std::pair(0, -1), std::pair(-1, -1), std::pair(-1, 0), std::pair(-1, 1),
    };
    for (int i = 0; i < 8; i++) {
        int target_x = x + knight_moves[i].first;
        int target_y = y + knight_moves[i].second;
        if (target_x < 0 || target_x >= 8 || target_y < 0 || target_y >= 8) {
            continue;
        }

        int target_square = target_y * 8 + target_x;
        if (full_board[target_square] && full_board[target_square].is_white == piece.is_white) {
            continue;
        }

        moves.push_back(Move(piece.indexed_position, target_square));
    }
}

std::vector<Move> Board::get_all_moves_for_square(int indexed_square)
{
    std::vector<Move> result;

    for (auto move : legal_moves) {
        if (move.start_pos == indexed_square) {
            result.push_back(move);
        }
    }
    return result;
}

void Board::display_square_moves(int index)
{
    moves_for_selected_piece.clear();

    for (auto move : get_all_moves_for_square(index)) {
        moves_for_selected_piece.push_back(move.end_pos);
    }
    if (moves_for_selected_piece.size() != 0) {
        selected_piece = index;
    } else {
        selected_piece = -1;
    }
}

void Board::add_sliding_moves(Piece &piece, std::vector<Move> &moves)
{
    auto square = piece.indexed_position;
    int start_dir = piece.type == Piece::Bishop ? 4 : 0;
    int end_dir = piece.type == Piece::Rook ? 4 : 8;

    for (int direction_index = start_dir; direction_index < end_dir; direction_index++) {
        for (int n = 0; n < distance_to_borders[square][direction_index]; n++) {
            int target_square = square + direction_offsets[direction_index] * (n + 1);
            Piece &piece_on_target_square = full_board[target_square];
            if (piece_on_target_square.type == Piece::NONE) {
                moves.push_back(Move(square, target_square));
                continue;
            }

            if (piece_on_target_square.is_white != piece.is_white) {
                moves.push_back(Move(square, target_square));
            } else {
            }
            break;
        }
    }
}

void Board::update_sprite_position(sf::RectangleShape &shape, sf::Vector2f &board_origin, int index)
{
    int x = index % 8;
    int y = index / 8;

    float true_x = board_origin.x + square_size * x;
    float true_y = board_origin.y - square_size * (1 + y);

    shape.setSize({(float)square_size, (float)square_size});
    shape.setPosition({true_x, true_y});
}

void Board::draw_board(sf::RenderWindow &window)
{
    window.draw(this->sprite);
    auto origin = this->get_origin();

    if (selected_piece != -1) {
        update_sprite_position(main_highlighted_square_shape, origin, selected_piece);
        window.draw(main_highlighted_square_shape);
        for (auto square : moves_for_selected_piece) {
            update_sprite_position(highlighted_square_shape, origin, square);
            window.draw(highlighted_square_shape);
        }
    }
    for (int i = 0; i < 64; i++) {
        auto &piece = full_board[i];
        if (piece.type == Piece::NONE) {
            continue;
        }
        piece.update_sprite_position(origin, square_size);
        if (piece.indexed_position == selected_piece && is_piece_picked_up) {
            auto mouse_pos = sf::Mouse::getPosition(window);
            auto sprite_size = piece.sprite.getGlobalBounds();
            piece.sprite.setPosition(mouse_pos.x - sprite_size.width / 2, mouse_pos.y - sprite_size.height / 2);
        }
        window.draw(piece.sprite);
    }
}

void Board::precompute_distance_to_borders()
{
    for (int file = 0; file < 8; file++) {
        for (int rank = 0; rank < 8; rank++) {
            int numNorth = 7 - rank;
            int numSouth = rank;
            int numEast = 7 - file;
            int numWest = file;

            int square_index = rank * 8 + file;

            distance_to_borders[square_index][0] = numNorth;
            distance_to_borders[square_index][1] = numSouth;
            distance_to_borders[square_index][2] = numWest;
            distance_to_borders[square_index][3] = numEast;
            distance_to_borders[square_index][4] = std::min(numNorth, numWest);
            distance_to_borders[square_index][5] = std::min(numSouth, numEast);
            distance_to_borders[square_index][6] = std::min(numNorth, numEast);
            distance_to_borders[square_index][7] = std::min(numSouth, numWest);
        }
    }
}

std::string Board::get_clean_coordinate(char pos)
{
    char x = pos % 8;
    char y = pos / 8;

    return std::string(1, 'a' + x) + std::string(1, '1' + y);
}

std::ostream &Chess::operator<<(std::ostream &os, Move move)
{
    os << Board::get_clean_coordinate(move.start_pos) << Board::get_clean_coordinate(move.end_pos);
    return os;
}

bool Chess::Move::operator==(const Move &other) const
{
    return start_pos == other.start_pos && end_pos == other.end_pos;
}
