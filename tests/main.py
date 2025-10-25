#!/bin/python3
from pathlib import Path
import subprocess
from threading import Lock
from concurrent.futures import ThreadPoolExecutor
from colorama import Fore, Style, Back
from multiprocessing import cpu_count
import chess
from sys import argv

all_purpose_lock = Lock()
error_count = 0
tasks_done = 0


def check_for_fen(FEN: str, chess_gui: Path):
    global error_count
    global tasks_done
    chess_gui_proc = subprocess.run([chess_gui, "-F", FEN, "--get_moves"], capture_output=True)
    if chess_gui_proc.returncode != 0:
        with all_purpose_lock:
            print(f"{Fore.RED} GUI didn't manage to read FEN: {FEN}")
            print()
            print(f"stdder: {chess_gui_proc.stderr.decode()}")
            print(Style.RESET_ALL)
            error_count += 1
            tasks_done += 1
        return

    chess_gui_res = set(chess_gui_proc.stdout.splitlines())

    board = chess.Board(FEN)
    engine_res = set(board.uci(x).encode() for x in board.legal_moves)

    if chess_gui_res != engine_res:
        with all_purpose_lock:
            print(f"{Fore.RED}Error found for FEN {FEN}")
            print(f"Missing in GUI: {[x.decode() for x in engine_res.difference(chess_gui_res)]}")
            print(f"Moves that don't exists: {[x.decode() for x in chess_gui_res.difference(engine_res)]}")
            print(Style.RESET_ALL)
            error_count += 1

    with all_purpose_lock:
        tasks_done += 1
        if tasks_done % 100 == 0:
            print(f"{Fore.GREEN}{tasks_done} tasks completed.{Style.RESET_ALL}")


def test_for_file(FENs_path: Path, chess_gui: Path):

    FENs_file = open(FENs_path)
    check_for_fen_single_arg = lambda FEN: check_for_fen(FEN, chess_gui)
    with ThreadPoolExecutor(max_workers=cpu_count()) as executor:
        executor.map(check_for_fen_single_arg, (l.strip() for l in FENs_file))


def main():
    global error_count
    global tasks_done

    if "-h" in argv or "--help" in argv:
        print(f"Usage: {argv[0]} [chess_gui_exec] [FILE]...")
        print(f"")
    chess_gui = argv[1]

    for file in argv[2:]:
        print(f"Doing file: {file}")
        test_for_file(file, chess_gui)

    if error_count == 0:
        print(f"{Fore.GREEN}All {tasks_done} tests ok!{Style.RESET_ALL}")
        return True
    else:
        print(f"{Fore.RED}{error_count = } test(s) failed")
        print(f"{(1 - error_count / tasks_done)*100:.2f}% success rate{Style.RESET_ALL}")
        return False


if __name__ == "__main__":
    main()
