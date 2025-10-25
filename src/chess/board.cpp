#include "board.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <stdexcept>

using namespace Chess;

Board::Board()
{
    precompute_distance_to_borders();
}

Chess::Board::~Board()
{
    free(this->promotion_popup);
}

void Chess::Board::move_piece(Move move)
{
    full_board[move.end_pos] = full_board[move.start_pos];
    full_board[move.end_pos].indexed_position = move.end_pos;
    full_board[move.start_pos] = Piece();
    if (move.promotion != Piece::piece_type::NONE) {
        full_board[move.end_pos].type = move.promotion;
    }
}

void Board::play_move(Move move)
{
    bool is_capture = full_board[move.end_pos].type != Piece::NONE;
    auto &moving_piece = full_board[move.start_pos];

    move_history->move = move;
    move_history->piece = moving_piece;
    move_history->isCapture = is_capture;

    if (is_capture || moving_piece.type == Piece::Pawn) {
        halfmove_clock = 0;
    } else {
        halfmove_clock += 1;
    }

    if (is_white_turn == false) {
        fullmove_number += 1;
    }

    // Check for en passant to capture pawn
    if (moving_piece.type == Piece::Pawn && move.end_pos == en_passant_square) {
        move_history->isCapture = true;
        if (moving_piece.is_white) {
            full_board[move.end_pos - 8] = Piece();
        } else {
            full_board[move.end_pos + 8] = Piece();
        }
    }

    // Check for castling
    if (moving_piece.type == Piece::King) {
        if (moving_piece.is_white) {
            if (move.start_pos == 4 && move.end_pos == 6) {
                move_piece(Move(7, 5));
                move_history->isCastle = true;
            }
            if (move.start_pos == 4 && move.end_pos == 2) {
                move_piece(Move(0, 3));
                move_history->isCastle = true;
            }
        } else {
            if (move.start_pos == 60 && move.end_pos == 62) {
                move_piece(Move(63, 61));
                move_history->isCastle = true;
            }
            if (move.start_pos == 60 && move.end_pos == 58) {
                move_piece(Move(56, 59));
                move_history->isCastle = true;
            }
        }
    }

    en_passant_square = -1;
    if (moving_piece.type == Piece::Pawn && abs(move.start_pos - move.end_pos) == 16) {
        if (is_white_turn) {
            en_passant_square = move.start_pos + 8;
        } else {
            en_passant_square = move.start_pos - 8;
        }
    }

    if (!is_mini_board) {
        check_if_move_voids_castle(move, moving_piece);
    }

    for (auto &potential_move : legal_moves) {
        if (potential_move.start_pos == move.start_pos) {
            continue;
        }
        auto potential_piece = full_board[potential_move.start_pos];
        if (potential_move.end_pos == move.end_pos && potential_piece.type == moving_piece.type) {
            if (move.start_pos % 8 == potential_move.start_pos % 8) { // If there is a matching file, show the rank
                move_history->showRank = true;
            } else { // If there is not matching file, show the file (if nothing matches priority for the file)
                move_history->showFile = true;
            }
        }
    }

    move_piece(move);

    bool is_check = !is_king_safe(!is_white_turn);

    is_white_turn = !is_white_turn;

    if (log_FEN) {
        std::cout << get_FEN() << std::endl;
    }
    if (!is_mini_board) {
        this->legal_moves = get_all_legal_moves(is_white_turn);
        move_history->isCheck = is_check;
        move_history->isMate = is_check && legal_moves.size() == 0;
        move_history->fen = get_FEN();
        move_history->isDraw = !is_check && legal_moves.size() == 0;
        move_history.push_move();
        show_last_move();
    }
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

void Board::setup_textures(std::filesystem::path path, RessourceManager *manager)
{
    this->manager = manager;
    this->promotion_popup = new PromotionPopup();
    setup_board_textures(path);
}

void Board::setup_board_textures(std::filesystem::path path)
{
    if (!this->texture.loadFromFile(path)) {
        throw std::runtime_error("SFML board error");
    }
    this->sprite.setTexture(this->texture);
    scale_board();
}

void Board::setup_pieces_textures()
{
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

bool Board::load_from_FEN_board(std::string fen_board)
{
    int file = 0;
    int rank = 7;
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
    return true;
}

bool Board::load_from_FEN_color(std::string fen_color)
{
    if (fen_color.size() == 1) {
        if (fen_color[0] == 'w') {
            is_white_turn = true;
        } else if (fen_color[0] == 'b') {
            is_white_turn = false;
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

bool Board::load_from_FEN_castle(std::string fen_castle)
{
    king_black_castle = false;
    king_white_castle = false;
    queen_black_castle = false;
    queen_white_castle = false;

    for (auto c : fen_castle) {
        switch (c) {
        case 'K':
            king_white_castle = true;
            break;
        case 'Q':
            queen_white_castle = true;
            break;
        case 'k':
            king_black_castle = true;
            break;
        case 'q':
            queen_black_castle = true;
            break;
        }
    }

    return true;
}

bool Board::load_from_FEN_en_passant(std::string fen_en_passant)
{
    if (fen_en_passant == "-") {
        en_passant_square = -1;
        return true;
    }
    if (fen_en_passant.size() != 2) {
        return false;
    }
    int file = fen_en_passant[0] - 'a';
    int rank = fen_en_passant[1] - '1';

    if (file > 7 || file < 0 || rank < 0 || rank > 7) {
        return false;
    }
    en_passant_square = rank * 8 + file;
    return true;
}

bool Board::load_from_FEN_half_move(std::string fen_half_move)
{
    try {
        halfmove_clock = std::stoi(fen_half_move);
        return true;
    } catch (std::invalid_argument const &e) {
        halfmove_clock = 0;
        return false;
    }
}
bool Board::load_from_FEN_full_move(std::string fen_full_move)
{
    try {
        fullmove_number = std::stoi(fen_full_move);
        return true;
    } catch (std::invalid_argument const &e) {
        fullmove_number = 0;
        return false;
    }
}

bool Board::load_from_FEN(std::string FEN)
{
    for (int i = 0; i < 64; i++) {
        full_board[i] = Piece();
    }

    int nb_spaces = std::count(FEN.begin(), FEN.end(), ' ');

    if (nb_spaces > 5 || nb_spaces < 3) {
        std::cerr << "Invalid fen: \"" << FEN << "\"" << std::endl;
        return false;
    }

    std::stringstream ss(FEN);

    std::string fen_board;
    std::string fen_color;
    std::string fen_castling;
    std::string fen_en_passant;
    std::string fen_half_move;
    std::string fen_full_move;

    ss >> fen_board;
    ss >> fen_color;
    ss >> fen_castling;
    ss >> fen_en_passant;
    ss >> fen_half_move;
    ss >> fen_full_move;

    if (load_from_FEN_board(fen_board) && load_from_FEN_color(fen_color) && load_from_FEN_castle(fen_castling)
        && load_from_FEN_en_passant(fen_en_passant)) {
    } else {
        std::cerr << "Invalid fen: \"" << FEN << "\"" << std::endl;
        return false;
    }

    halfmove_clock = 0;
    fullmove_number = 0;
    if (nb_spaces >= 4 && !load_from_FEN_half_move(fen_half_move)) {
        std::cerr << "Half_move_error" << std::endl;
        std::cerr << "Invalid fen: \"" << FEN << "\"" << std::endl;
        return false;
    }
    if (nb_spaces >= 5 && !load_from_FEN_full_move(fen_full_move)) {
        std::cerr << "Full_move_error" << std::endl;
        std::cerr << "Invalid fen: \"" << FEN << "\"" << std::endl;
        return false;
    }

    if (log_FEN) {
        std::cout << get_FEN() << std::endl;
    }
    if (!is_mini_board) {
        legal_moves = get_all_legal_moves(is_white_turn);
    }

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
        FEN += Move::get_clean_coordinate(en_passant_square);
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
    this->promotion_popup->scale(*this);
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
    for (int i = 0; i < 12; i++) {
        sf::Vector2u size = manager->sprite_piece_array[i].getTexture()->getSize();
        manager->sprite_piece_array[i].setScale((float)square_size / size.x, (float)square_size / size.y);
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

bool Chess::Board::is_square_safe(int square, bool cur_is_white)
{
    std::vector theorical_moves = get_all_legal_moves(!cur_is_white);
    for (auto &move : theorical_moves) {
        if (move.end_pos == square) {
            return false;
        }
    }
    return true;
}

bool Chess::Board::is_king_safe(bool cur_is_white)
{
    int king_square = -1;

    for (int i = 0; i < 64; i++) {
        auto &cur_piece = full_board[i];
        if (cur_piece.type == Piece::King && cur_piece.is_white == cur_is_white) {
            king_square = i;
            break;
        }
    }
    if (king_square == -1) {
        return true;
    }
    return is_square_safe(king_square, cur_is_white);
}

void Chess::Board::show_last_move()
{
    auto &last_move = move_history.move_history.back();
    std::cout << last_move.print_move() << std::endl;
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
    if (is_mini_board) {
        return result;
    }

    // Filter for checks
    Board mini_board;
    mini_board.log_FEN = false;
    mini_board.is_mini_board = true;
    std::string base_fen = get_FEN();

    auto it = result.begin();
    while (it != result.end()) {
        mini_board.load_from_FEN(base_fen);
        mini_board.play_move(*it);
        if (!mini_board.is_king_safe(is_mover_white)) {
            it = result.erase(it);
        } else {
            it++;
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
    auto push_pawn_move = [&](Move move) {
        if (move.end_pos / 8 == (is_white_turn ? 7 : 0)) {
            move.promotion = Piece::piece_type::Queen;
            moves.push_back(move);
            move.promotion = Piece::piece_type::Rook;
            moves.push_back(move);
            move.promotion = Piece::piece_type::Bishop;
            moves.push_back(move);
            move.promotion = Piece::piece_type::Knight;
            moves.push_back(move);
        } else {
            moves.push_back(move);
        }
    };

    int y = piece.indexed_position / 8;
    int x = piece.indexed_position % 8;
    int forward = piece.is_white ? +8 : -8;
    int start_rank = piece.is_white ? 1 : 6;
    bool is_first_move = y == start_rank;

    int target_square = piece.indexed_position + forward;
    if (is_inside_board(target_square) && !full_board[target_square]) {
        push_pawn_move(Move(piece.indexed_position, target_square));
        target_square = piece.indexed_position + forward * 2;
        if (is_first_move && is_inside_board(target_square) && !full_board[target_square]) {
            moves.push_back(Move(piece.indexed_position, target_square));
        }
    }

    auto is_square_capturable = [&](int square) {
        if (is_inside_board(square)) {
            return (full_board[square].type != Piece::NONE && piece.is_white != full_board[square].is_white) || square == en_passant_square;
        }
        return false;
    };

    // Check for captures
    target_square = piece.indexed_position + forward + 1;
    if (x != 7 && (is_mini_board || is_square_capturable(target_square))) {
        push_pawn_move(Move(piece.indexed_position, target_square));
    }

    target_square = piece.indexed_position + forward - 1;
    if (x != 0 && (is_mini_board || is_square_capturable(target_square))) {
        push_pawn_move(Move(piece.indexed_position, target_square));
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

    if (is_mini_board) {
        return;
    }

    Board mini_board;
    mini_board.log_FEN = false;
    mini_board.is_mini_board = true;
    mini_board.load_from_FEN(get_FEN());

    if (!mini_board.is_king_safe(is_white_turn)) {
        return;
    }

    auto check_square = [&](int s) { return !full_board[s] && mini_board.is_square_safe(s, is_white_turn); };
    // Castle
    if (is_white_turn) {
        if (king_white_castle && !full_board[6] && check_square(5)) {
            moves.push_back(Move(piece.indexed_position, 6));
        }
        if (queen_white_castle && !full_board[1] && !full_board[2] && check_square(3)) {
            moves.push_back(Move(piece.indexed_position, 2));
        }
    } else {
        if (king_black_castle && !full_board[62] && check_square(61)) {
            moves.push_back(Move(piece.indexed_position, 62));
        }
        if (queen_black_castle && !full_board[57] && check_square(59) && !full_board[58]) {
            moves.push_back(Move(piece.indexed_position, 58));
        }
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
        std::set<int> tmp_set;
        for (auto s : moves_for_selected_piece) {
            tmp_set.insert(s);
        }
        for (auto square : tmp_set) {
            update_sprite_position(highlighted_square_shape, origin, square);
            window.draw(highlighted_square_shape);
        }
    }
    for (int i = 0; i < 64; i++) {
        auto &piece = full_board[i];
        if (piece.type == Piece::NONE) {
            continue;
        }
        auto &sprite = manager->sprite_piece_array[piece.get_texture_index()];
        int x = piece.indexed_position % 8;
        int y = piece.indexed_position / 8;
        float true_x = origin.x + square_size * x;
        float true_y = origin.y - square_size * (1 + y);
        sprite.setPosition(true_x, true_y);
        if (piece.indexed_position == selected_piece && is_piece_picked_up) {
            auto mouse_pos = sf::Mouse::getPosition(window);
            auto sprite_size = sprite.getGlobalBounds();
            sprite.setPosition(mouse_pos.x - sprite_size.width / 2, mouse_pos.y - sprite_size.height / 2);
        }
        window.draw(sprite);
    }
    promotion_popup->draw(window, *this);
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

std::string Move::get_clean_coordinate(char pos)
{
    char x = pos % 8;
    char y = pos / 8;

    return std::string(1, 'a' + x) + std::string(1, '1' + y);
}

std::string Chess::Move::to_string(int pos)
{
    int rank = pos / 8;
    int file = pos % 8;

    std::string res = "";
    res += 'a' + file;
    res += '1' + rank;
    return res;
}

bool Chess::Move::operator==(const Move &other) const
{
    return start_pos == other.start_pos && end_pos == other.end_pos;
}

Chess::PromotionPopup::PromotionPopup()
{
    this->square.setFillColor(sf::Color(127, 127, 127, 255));
}

void Chess::PromotionPopup::show(Move move)
{
    visible = true;
    this->move = move;
    is_direction_up = this->move.end_pos / 8 == 0;
}

void Chess::PromotionPopup::draw(sf::RenderWindow &window, Board &board)
{
    if (!visible) {
        return;
    }

    auto origin = board.get_origin();
    for (int i = 0; i < 4; i++) {
        int index = move.end_pos + 8 * i * (is_direction_up ? 1 : -1);
        int x = index % 8;
        int y = index / 8;

        float true_x = origin.x + board.square_size * x;
        float true_y = origin.y - board.square_size * (1 + y);

        square.setRadius(board.square_size / 2);
        square.setPosition({true_x, true_y});

        window.draw(square);
        int texture_index = Piece::get_texture_index(promotion_order[i], !is_direction_up);
        auto &sprite = board.manager->sprite_piece_array[texture_index];
        sprite.setPosition(square.getPosition());
        window.draw(sprite);
    }
}

void Chess::PromotionPopup::scale(Board &board)
{
}

Piece::piece_type Chess::PromotionPopup::select(int square)
{
    visible = false;
    for (int i = 0; i < 4; i++) {
        int target_square = move.end_pos + 8 * i * (is_direction_up ? 1 : -1);
        if (square == target_square) {
            return promotion_order[i];
        }
    }
    return Piece::piece_type::NONE;
}
