FEN=$1
GUI=$2
ENGINE=$3

F_STOCK=/tmp/fen_check_diff_stockfish.txt
F_MY_GUI=/tmp/fen_check_diff_my_gui.txt

$GUI -F "$FEN" --get_moves | sort > $F_MY_GUI
echo -e "position fen $FEN \n go perft 1" |  $ENGINE | grep ": 1" | cut -c-4 | sort > $F_STOCK

diff -y --color "$F_STOCK" "$F_MY_GUI" > /tmp/fen_check_res.txt

if [ $? != "0" ]; then
    echo "Invalid diff"
    echo "Stockfish - My_GUI"
    cat /tmp/fen_check_res.txt
    exit 1
else
    exit 0
fi
