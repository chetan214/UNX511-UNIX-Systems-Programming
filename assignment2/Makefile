CXX = g++
CXXFLAGS = -Wall -std=c++11

all: networkMonitor intfMonitor

networkMonitor: src/networkMonitor.cpp
	$(CXX) $(CXXFLAGS) -o networkMonitor src/networkMonitor.cpp

intfMonitor: src/intfMonitor.cpp
	$(CXX) $(CXXFLAGS) -o intfMonitor src/intfMonitor.cpp

clean:
	rm -f networkMonitor intfMonitor
	rm -f /tmp/netmon.sock
