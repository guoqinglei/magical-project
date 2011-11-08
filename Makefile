CC       = g++
CFLAGS   = -Wall -Wextra -fopenmp

FILES_H  = types.h paths.h
FILES_CC = types.cpp paths.cpp johnson_tsplib_test.cpp

BINARY   = paths

all: clean compile

clean:
	find . -name '*.o' -exec rm -f '{}' ';'
	rm -f $(BINARY);

compile:
	$(CC) $(CFLAGS) $(FILES_CC) -o $(BINARY)

run:
	./$(BINARY)
