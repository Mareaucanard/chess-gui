GREEN 			= 	\033[1;32m
LIGHT_GREEN 	= 	\033[1;32m
LIGHT_CYAN 		= 	\033[1;36m
RED 			= 	\033[1;31m
MAGENTA 		= 	\033[1;35m
BLUE 			= 	\033[1;34m
RESET 			= 	\033[0m

TARGET 			= 	chess_gui.x86-64

SRC 			=	src/main.cpp \
					src/ressourceManager.cpp \
					src/chess/piece.cpp \
					src/chess/board.cpp \
					src/ArrowsManager.cpp \
					src/chess/MoveLog.cpp	\

INCLUDES 		=	-Iinclude -Isrc

CXXFLAGS 		= 	$(INCLUDES) -Wall -Wextra -Wpedantic -std=c++23 -g3 -Wno-narrowing

LDFLAGS 		= 	-lsfml-graphics -lsfml-window -lsfml-system

DEP 			=	$(OBJ:%.o=%.d)

OBJ 			=	$(SRC:%.cpp=%.o)

TEST_FEN_FILES 	=  	tests/epd_files/4000_openings_legacy.epd \
					tests/epd_files/new2500.epd \
 					tests/epd_files/train-00000-of-00001.epd \
					tests/edp_files/positions.epd \

$(TARGET): $(OBJ)
	g++ $(OBJ) -o $(TARGET) $(LDFLAGS)
	@echo "$(BLUE)Finished compiling with\
	\nCompilations flags $(CXXFLAGS)\
	\nLinking flags $(LDFLAGS)$(RESET)"

-include $(DEP)

%.o: %.cpp
	@g++ $(CXXFLAGS) -MMD -c $< -o $@\
	&& echo "[$(MAGENTA)compiled$(RESET)] $^ => $@"\
	|| echo "[$(RED)error$(RESET)]" $^

.PHONY : all clean fclean run_tests

run_tests: $(TARGET)
	python3 tests/main.py ./$(TARGET) $(TEST_FEN_FILES)

clean:
	@rm -f $(OBJ)
	@rm -f $(DEP)
	@echo "$(LIGHT_GREEN)Removed .o and .d files$(RESET)"

fclean: clean
	rm -f $(TARGET)
	@echo "$(LIGHT_GREEN)Removed target file$(RESET)"

re: fclean $(TARGET)