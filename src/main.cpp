#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <argparse/argparse.hpp>
#include <cmath>
#include <iostream>

#include "ArrowsManager.hpp"
#include "chess/board.hpp"
#include "chess/piece.hpp"
#include "ressourceManager.hpp"

void on_left_mouse_clicked(Chess::Board &board, sf::Vector2i position, ArrowsManager &arrows, bool is_release)
{
    int pos = board.get_square_from_mouse(position);
    if (pos != -1) {
        arrows.arrows_list.clear();
        bool was_move_played = false;
        if (board.promotion_popup->visible) {
            was_move_played = true;
            Chess::Piece::piece_type selection_type = board.promotion_popup->select(pos);
            if (selection_type != Chess::Piece::NONE) {
                auto base_move = board.promotion_popup->move;
                base_move.promotion = selection_type;
                board.play_move(base_move);
            } else {
                board.selected_piece = -1;
            }
        } else if (board.selected_piece != -1) {
            auto &vec = board.moves_for_selected_piece;
            auto move_found = std::find(vec.begin(), vec.end(), pos);
            if (move_found != vec.end()) {
                if ((std::find(move_found + 1, vec.end(), pos) != vec.end())) { // If promotion
                    was_move_played = true;
                    board.promotion_popup->show(Chess::Move(board.selected_piece, pos));
                    board.selected_piece = -1;

                } else {
                    board.play_move(Chess::Move(board.selected_piece, pos));
                    board.display_square_moves(-1);
                    was_move_played = true;
                }
            }
        }

        if (!was_move_played && !is_release) {
            board.display_square_moves(pos);
            board.is_piece_picked_up = true;
        }
        if (pos == board.selected_piece && is_release) {
            board.is_piece_picked_up = false;
        }
    }
}

int graphics_loop(Chess::Board &board, argparse::ArgumentParser &program)
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Chess GUI");
    RessourceManager manager(program.get<std::string>("--pieces"));
    board.setup_textures(program.get<std::string>("--board"), &manager);
    board.resize_board(1280, 720);
    board.scale_pieces();
    board.scale_board();

    ArrowsManager arrowsManager;
    int right_click_pos = -1;

    window.setFramerateLimit(60);
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    on_left_mouse_clicked(board, sf::Mouse::getPosition(window), arrowsManager, false);
                } else if (event.mouseButton.button == sf::Mouse::Right) {
                    sf::Vector2i mouse_location = sf::Mouse::getPosition(window);
                    right_click_pos = board.get_square_from_mouse(mouse_location);
                    arrowsManager.mouse_arrow_enabled = true;
                    arrowsManager.mouse_arrow.setPosition({mouse_location.x, mouse_location.y});
                    arrowsManager.mouse_arrow.setSize({0, board.square_size / 10.f});
                }
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Right) {
                    int pos = board.get_square_from_mouse(sf::Mouse::getPosition(window));
                    if (right_click_pos != -1 && pos != -1 && right_click_pos != pos) {
                        arrowsManager.add_arrow(right_click_pos, pos, board);
                    }
                    arrowsManager.mouse_arrow_enabled = false;
                } else if (event.mouseButton.button == sf::Mouse::Left) {
                    if (board.selected_piece != -1) {
                        on_left_mouse_clicked(board, sf::Mouse::getPosition(window), arrowsManager, true);
                    }
                }
            }
            if (event.type == sf::Event::Resized) {
                board.resize_board(event.size.width, event.size.height);
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(visibleArea));
                arrowsManager.resize_arrows(board);
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::R) {
                    board.load_from_FEN(Chess::default_FEN);
                    board.move_history.reset(Chess::default_FEN);
                    board.selected_piece = -1;
                }
                if (event.key.code == sf::Keyboard::S) {
                    std::cout << board.get_square_from_mouse(sf::Mouse::getPosition(window)) << std::endl;
                }
                if (event.key.code == sf::Keyboard::P) {
                    board.move_history.write_PGN("./game.PGN");
                    board.selected_piece = -1;
                }
                if (event.key.code == sf::Keyboard::Left) {
                    if (board.move_history.undo()) {
                        board.load_from_FEN(board.move_history.get_active_fen());
                    }
                }
                if (event.key.code == sf::Keyboard::Right) {
                    if (board.move_history.redoo()) {
                        board.load_from_FEN(board.move_history.get_active_fen());
                        board.selected_piece = -1;
                    }
                }
            }
        }

        window.clear();
        board.draw_board(window);
        arrowsManager.draw(window);
        window.display();
    }
    return 0;
}

int generate_moves(Chess::Board &board)
{
    board.print_all_legal_moves();
    return 0;
}

Chess::Board setup_board(argparse::ArgumentParser &program)
{
    Chess::Board board;

    std::string FEN = program.get<std::string>("--FEN");
    board.log_FEN = program.get<bool>("--log_FEN");
    if (!board.load_from_FEN(FEN)) {
        exit(1);
    }
    board.move_history.reset(FEN);
    return board;
}

int main(int argc, char **argv)
{
    argparse::ArgumentParser program("Chess GUI");

    program.add_description("Standard chess GUI made in c++ with SFML");

    program.add_argument("--FEN", "-F").help("Starting fen position").default_value(std::string(Chess::default_FEN)).nargs(1);
    program.add_argument("--log_FEN").help("Logs FEN after every move").default_value(false).implicit_value(true);
    program.add_argument("--pieces").help("Folder containing the pieces sprites").default_value("./default_textures/pieces/regular").nargs(1);
    program.add_argument("--board").help("Folder containing the board sprite").default_value("./default_textures/board.jpg").nargs(1);
    program.add_argument("--get_moves").help("Doesn't start the gui; list all moves from position").default_value(false).implicit_value(true);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    bool help = program.get<bool>("help");
    bool print_moves = program.get<bool>("--get_moves");
    if (help) {
        std::cout << program << std::endl;
        return 0;
    }
    Chess::Board board = setup_board(program);
    if (print_moves) {
        generate_moves(board);
    } else {
        graphics_loop(board, program);
    }

    return 0;
}