#CC=clang++
#CC=g++
CFLAGS=-c -std=c++0x -g -Wall -Ilib/libcec/include
LDFLAGS=-Llib/libcec/src/lib/.libs/ -lcec -ldl
EXECUTABLE=libcec-daemon
SOURCES=src/main.cpp src/uinput.cpp src/libcec.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
