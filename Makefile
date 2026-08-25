CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -Iinclude
SRCS     := $(wildcard src/*.cpp)
BIN      := ot_mini

.PHONY: all clean run

all: $(BIN)

$(BIN): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(SRCS)

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)
