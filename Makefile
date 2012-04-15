CC       = g++
CFLAGS   = -Wall -Wextra -fopenmp -O3
# -lefence -Dsamer_debug

FILES_H  = types.h paths.h mst.h euler_tour.h
FILES_CC = types.cpp paths.cpp mst.cpp euler_tour.cpp magical_config.cpp mst_test.cpp
FILES_TINYXML = tinyxml_src/tinyxml.cpp tinyxml_src/tinyxmlparser.cpp tinyxml_src/tinyxmlerror.cpp tinyxml_src/tinystr.cpp

BINARY   = magical_test

all: clean compile

clean:
	find . -name '*.o' -exec rm -f '{}' ';'
	rm -f $(BINARY);

compile:
	$(CC) $(CFLAGS) $(FILES_TINYXML) $(FILES_CC) -o $(BINARY)

run:
	./$(BINARY)
