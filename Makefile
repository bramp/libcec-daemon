CC=clang
CFLAGS=-c -std=c++0x -Wall -Ilib/libcec/include
LDFLAGS=-lcec -ldl
EXECUTABLE=libcec-daemon
SOURCES=src/main.cpp src/uinput.cpp src/libcec.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
