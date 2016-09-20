# Makefile to build sunsetter for linux.

EXE = sunsetter

CXXFLAGS = -O3 -DNDEBUG
LDFLAGS = -O3

OBJECTS = aimoves.o bitboard.o board.o bughouse.o evaluate.o moves.o search.o capture_moves.o check_moves.o interface.o notation.o order_moves.o partner.o quiescense.o tests.o transposition.o validate.o

# sunsetter is the default target, so either "make" or "make sunsetter" will do
$(EXE): $(OBJECTS) .depend
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(EXE)

# so "make clean" will wipe out the files created by a make.
.PHONY:
clean:
	rm -f $(OBJECTS) $(EXE) .depend

.depend:
	$(CXX) $(DEPENDFLAGS) -MM $(OBJECTS:.o=.cpp) > $@

-include .depend
