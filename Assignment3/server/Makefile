# Build the UDP Log Server
CXX := g++
CXXFLAGS := -Wall -Wextra -O2 -std=c++11
LDFLAGS := -lpthread

SERVER_SOURCES := LogServer.cpp
SERVER_TARGET  := logserver

all: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $(SERVER_SOURCES) $(LDFLAGS)

clean:
	rm -f $(SERVER_TARGET) *.o server.log
