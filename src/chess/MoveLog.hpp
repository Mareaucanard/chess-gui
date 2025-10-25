#include <filesystem>
#include <string.h>
#include <vector>

#include "piece.hpp"

using std::filesystem::path;

namespace Chess
{

const char default_FEN[57] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
struct Move {
    int start_pos;
    int end_pos;
    Piece::piece_type promotion = Piece::NONE;

    static std::string get_clean_coordinate(char pos);
    static std::string to_string(int pos);
    inline Move(int spos, int epos) : start_pos(spos), end_pos(epos) {}
    inline Move() : start_pos(-1), end_pos(-1) {}
    bool operator==(const Move &other) const;
};

struct LogInstance {
    LogInstance(){};

    std::string fen;
    Move move;
    Piece piece;
    bool isCapture;
    bool isCheck;
    bool isCastle;
    bool isMate;
    bool isDraw;
    bool showRank;
    bool showFile;

    std::string print_move();
};

class MoveLog
{
public:
    MoveLog();
    void reset(std::string FEN);
    void push_move();

    void init_new_move();

    bool undo();
    bool redoo();
    std::string get_active_fen();

    void write_PGN(path file_path);

    LogInstance *operator->();

    std::string starting_FEN = default_FEN;
    std::vector<LogInstance> move_history;
    LogInstance cur_move;
    int ply_index = -1;
};

std::ostream &operator<<(std::ostream &os, Move move);

} // namespace Chess