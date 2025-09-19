CC      = g++
CFLAGS  = -std=c++17 -Wall -Wextra
LIB     = libPidUtil.a
TARGET  = Lab2-ChetanArora
SOURCES = Lab2-ChetanArora.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS) $(LIB)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LIB)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
