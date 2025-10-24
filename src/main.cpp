#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <argparse/argparse.hpp>
#include <cmath>
#include <iostream>

#include "ArrowsManager.hpp"
#include "chess/board.hpp"
#include "chess/piece.hpp"
#include "ressourceManager.hpp"

void on_left_mouse_clicked(Chess::Board &board, sf::Vector2i position, ArrowsManager &arrows)
{
    int pos = board.get_square_from_mouse(position);
    if (pos != -1) {
        arrows.arrows_list.clear();
        bool was_move_played = false;
        if (board.selected_piece != -1) {
            auto &vec = board.moves_for_selected_piece;
            if (std::find(vec.begin(), vec.end(), pos) != vec.end()) {
                board.play_move(Chess::Move(board.selected_piece, pos));
                board.display_square_moves(-1);
                was_move_played = true;
            }
        }

        if (!was_move_played) {
            if (pos != -1) {
                board.display_square_moves(pos);
                board.is_piece_picked_up = true;
            }
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
                    on_left_mouse_clicked(board, sf::Mouse::getPosition(window), arrowsManager);
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
                    int pos = board.get_square_from_mouse(sf::Mouse::getPosition(window));
                    if (board.selected_piece != -1) {
                        auto &vec = board.moves_for_selected_piece;

                        if (std::find(vec.begin(), vec.end(), pos) != vec.end()) {
                            board.play_move(Chess::Move(board.selected_piece, pos));
                            board.display_square_moves(-1);
                        } else if (pos != board.selected_piece) {
                            board.display_square_moves(-1);
                        }
                        board.is_piece_picked_up = false;
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
                }
                if (event.key.code == sf::Keyboard::S) {
                    std::cout << board.get_square_from_mouse(sf::Mouse::getPosition(window)) << std::endl;
                }
                if (event.key.code == sf::Keyboard::P) {
                    board.move_history.write_PGN("./game.PGN");
                }
                if (event.key.code == sf::Keyboard::Left) {
                    if (board.move_history.undo()) {
                        board.load_from_FEN(board.move_history.get_active_fen());
                    }
                }
                if (event.key.code == sf::Keyboard::Right) {
                    if (board.move_history.redoo()) {
                        board.load_from_FEN(board.move_history.get_active_fen());
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
    board.load_from_FEN(FEN);
    return board;
}

// TEST FEN r1bnk2r/pppq1ppp/3b1n2/4p1N1/2B5/1QN1P3/PP1P1PPP/R1B2RK1 w kq - 9
// 10"
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