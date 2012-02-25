CC=clang
#CFLAGS=-c -Wall -I/usr/include/libcec
CFLAGS=-c -Wall -Ilib/libcec/include
LDFLAGS=-lcec -ldl
EXECUTABLE=libcec-daemon
SOURCES=src/main.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
