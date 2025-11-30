CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

CORE_SRCS = core/simulation_state.cpp core/simulation.cpp core/io.cpp core/trains.cpp core/switches.cpp
SFML_SRCS = sfml/app.cpp sfml/main.cpp

OBJS = $(CORE_SRCS:.cpp=.o) $(SFML_SRCS:.cpp=.o)

TARGET = switchback_rails

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

