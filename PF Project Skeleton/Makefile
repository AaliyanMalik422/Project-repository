CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic
LDFLAGS ?= -lsfml-graphics -lsfml-window -lsfml-system

CORE_SRCS = core/simulation_state.cpp core/grid.cpp core/simulation.cpp core/io.cpp core/trains.cpp core/switches.cpp
SFML_SRCS = sfml/app.cpp sfml/main.cpp
OBJS = $(CORE_SRCS:.cpp=.o) $(SFML_SRCS:.cpp=.o)

TARGET = switchback_rails
LEVEL ?= data/levels/easy_level.lvl

.PHONY: all clean run run-gui

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) $(LEVEL)

run-gui: $(TARGET)
	./$(TARGET) $(LEVEL) --view

clean:
	rm -f $(OBJS) $(TARGET)
