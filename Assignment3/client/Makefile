# Build the client (TravelSimulator + Automobile + Logger)
CXX := g++
CXXFLAGS := -Wall -Wextra -O2 -std=c++11
LDFLAGS := -lpthread

# Adjust these filenames if your provided files have different names
CLIENT_SOURCES := TravelSimulator.cpp Automobile.cpp Logger.cpp
CLIENT_TARGET  := travel

all: $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $(CLIENT_SOURCES) $(LDFLAGS)

clean:
	rm -f $(CLIENT_TARGET) *.o
