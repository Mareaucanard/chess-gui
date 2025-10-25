#include "MoveLog.hpp"

#include <fstream>
#include <iostream>
namespace Chess
{
MoveLog::MoveLog()
{
    init_new_move();
}

void MoveLog::reset(std::string FEN)
{
    move_history.clear();
    ply_index = -1;
    starting_FEN = FEN;
    init_new_move();
}

void Chess::MoveLog::push_move()
{
    move_history.erase(move_history.begin() + ply_index + 1, move_history.end());
    move_history.push_back(cur_move);
    ply_index += 1;
    init_new_move();
}

void Chess::MoveLog::init_new_move()
{
    cur_move.isCapture = false;
    cur_move.isCastle = false;
    cur_move.isCheck = false;
    cur_move.isMate = false;
    cur_move.showFile = false;
    cur_move.showRank = false;
}

bool MoveLog::undo()
{
    if (ply_index > -1) {
        ply_index -= 1;
        return true;
    }
    return false;
}

bool MoveLog::redoo()
{
    if ((unsigned)(ply_index + 1) < move_history.size()) {
        ply_index += 1;
        return true;
    }
    return false;
}

std::string MoveLog::get_active_fen()
{
    if (ply_index == -1) {
        return starting_FEN;
    }
    return move_history[ply_index].fen;
}

void MoveLog::write_PGN(path file_path)
{
    std::ofstream PGN_file;
    PGN_file.open(file_path, std::ios::out);
    if (!PGN_file) {
        throw std::runtime_error("Couldn't open file: " + file_path.string());
    }

    if (this->starting_FEN != Chess::default_FEN) { // Starting FEN header
        PGN_file << "[Variant \"From position\"]" << std::endl;
        PGN_file << "[FEN \"" << starting_FEN << "\"]" << std::endl;
        PGN_file << std::endl;
    }

    int move_counter = 1;
    if (move_history.front().piece.is_white == false) {
        PGN_file << "1... ";
        move_counter += 1;
    }
    for (auto &move : move_history) {
        if (move.piece.is_white) {
            PGN_file << move_counter << ". ";
            move_counter++;
        }
        PGN_file << move.print_move() << " ";
    }
    PGN_file.flush();
    PGN_file.close();
}

LogInstance *MoveLog::operator->()
{
    return &cur_move;
}

std::string LogInstance::print_move()
{
    if (isCastle) {
        if (move.start_pos < move.end_pos) {
            return "O-O";
        } else {
            return "O-O-O";
        }
    }
    std::string res = "";

    char c = this->piece.print_piece();
    if (c != 0) {
        res += c;
    }
    if (showFile || (this->piece.type == Piece::Pawn && isCapture)) {
        res += 'a' + move.start_pos % 8;
    }
    if (showRank) {
        res += '1' + move.start_pos / 8;
    }
    if (isCapture) {
        res += "x";
    }
    res += Move::to_string(move.end_pos);
    if (move.promotion != Piece::piece_type::NONE) {
        res += '=';
        res += Piece::print_piece(move.promotion, true);
    }

    if (isMate) {
        res += "#";
    } else if (isCheck) {
        res += "+";
    }

    return res;
}

std::ostream &operator<<(std::ostream &os, Move move)
{
    os << Move::get_clean_coordinate(move.start_pos) << Move::get_clean_coordinate(move.end_pos);
    if (move.promotion != Piece::piece_type::NONE) {
        os << Piece::print_piece(move.promotion, false);
    }
    return os;
}

} // namespace Chess