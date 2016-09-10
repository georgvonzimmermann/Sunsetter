# Makefile to build sunsetter for linux.

EXE = sunsetter
CXX = clang

# -Wall turns on full compiler warnings, only of interest to developers
# -O1 turns on optimization at a moderate level.
# -O3 turns on optimization at the highest level. (confuses the debugger though)
# -g turns on debugging data in the executable.
# -DNDEBUG turns off assert() debugging in the code
#
# uncomment this for a "release" build with no debugging and highly optimized.
CFLAGS = -O3 -DNDEBUG
#CFLAGS = -DDEBUG_STATS=1 -DDEBUG_HASH=1

# uncomment this line to build a full debug version(slow, more likely to crash).
# need to type "make clean;make" to get the full effect
#CFLAGS = -Wall -g -DDEBUG
#
# or this one for a "light debug" version, works with gdb
# but is otherwise like the release version.
# CFLAGS = -Wall -g -O1 -DNDEBUG
#
# uncomment following line if compiling with mingw under Windows
# LINKFLAGS = -static

ifeq ($(ARCH),js)
CXX = em++
EXE = sunsetter.dev.js
CFLAGS += -s TOTAL_MEMORY=33550000 -s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1
CFLAGS += -s EMTERPRETIFY_WHITELIST='["__Z10searchMove4moveii", "__Z10searchRootiP4movePi", "__Z12pollForInputv", "__Z12waitForInputv", "__Z13recursiveHashPiS_S_P4moveiiS0_", "__Z15recursiveSearchPiS_S_P4moveiiS0_i", "__Z15searchFirstMove4moveii", "__Z19recursiveFullSearchPiS_S_P4moveiiS0_", "__Z21recursiveCheckEvasionPiS_S_P4moveiiS0_", "__Z6ponderv", "__Z6searchiiiii", "__Z8findMoveP4move", "__Z8testbpgniPPc", "_main"]'
LINKFLAGS += --memory-init-file 0 -s NO_EXIT_RUNTIME=1 -s EXPORTED_FUNCTIONS="['_main', '_queue_command']" --pre-js pre.js --post-js post.js
endif

OBJECTS = aimoves.o bitboard.o board.o book.o bughouse.o evaluate.o moves.o search.o capture_moves.o check_moves.o interface.o notation.o order_moves.o partner.o quiescense.o tests.o transposition.o validate.o

# sunsetter is the default target, so either "make" or "make sunsetter" will do
$(EXE): $(OBJECTS) pre.js post.js
	$(CXX) $(CFLAGS) $(LINKFLAGS) $(OBJECTS) -o $(EXE)

# so "make clean" will wipe out the files created by a make.
.PHONY:
clean:
	rm $(OBJECTS)

sunsetter.js: sunsetter.dev.js
	cat preamble.js sunsetter.dev.js > sunsetter.js

aimoves.o: aimoves.cpp definitions.h variables.h board.h brain.h interface.h
	$(CXX) $(CFLAGS) -c aimoves.cpp -o $@

bitboard.o: bitboard.cpp board.h bughouse.h interface.h brain.h
	$(CXX) $(CFLAGS) -c bitboard.cpp -o $@

board.o: board.cpp board.h brain.h interface.h definitions.h bughouse.h \
	variables.h notation.h
	$(CXX) $(CFLAGS) -c board.cpp -o $@

book.o: book.cpp variables.h definitions.h board.h
	$(CXX) $(CFLAGS) -c book.cpp -o $@

bughouse.o: bughouse.cpp variables.h definitions.h board.h interface.h bughouse.h brain.h
	$(CXX) $(CFLAGS) -c bughouse.cpp -o $@

capture_moves.o: capture_moves.cpp board.h brain.h
	$(CXX) $(CFLAGS) -c capture_moves.cpp -o $@

check_moves.o: check_moves.cpp board.h
	$(CXX) $(CFLAGS) -c check_moves.cpp -o $@

evaluate.o: evaluate.cpp brain.h board.h evaluate.h
	$(CXX) $(CFLAGS) -c evaluate.cpp -o $@

interface.o: interface.cpp interface.h variables.h notation.h bughouse.h brain.h board.h
	$(CXX) $(CFLAGS) -c interface.cpp -o $@

moves.o: moves.cpp interface.h variables.h board.h
	$(CXX) $(CFLAGS) -c moves.cpp -o $@

notation.o: notation.cpp bughouse.h notation.h interface.h board.h brain.h variables.h
	$(CXX) $(CFLAGS) -c notation.cpp -o $@

order_moves.o: order_moves.cpp board.h brain.h interface.h notation.h
	$(CXX) $(CFLAGS) -c order_moves.cpp -o $@

partner.o: partner.cpp board.h brain.h interface.h notation.h
	$(CXX) $(CFLAGS) -c partner.cpp -o $@

quiescense.o: quiescense.cpp board.h brain.h interface.h variables.h notation.h
	$(CXX) $(CFLAGS) -c quiescense.cpp -o $@

search.o: search.cpp board.h brain.h bughouse.h notation.h interface.h
	$(CXX) $(CFLAGS) -c search.cpp -o $@

tests.o: tests.cpp board.h brain.h notation.h interface.h
	$(CXX) $(CFLAGS) -c tests.cpp -o $@

transposition.o: transposition.cpp interface.h definitions.h board.h notation.h brain.h
	$(CXX) $(CFLAGS) -c transposition.cpp -o $@

validate.o: validate.cpp interface.h board.h bughouse.h
	$(CXX) $(CFLAGS) -c validate.cpp -o $@
