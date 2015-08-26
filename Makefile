CC=gcc
CFLAGS=-c -Wall -std=c99
LDFLAGS=
SOURCES=main.c deciderHardwire.c UDPSocketClient.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=server

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
.c.o:
	$(CC) $(CFLAGS) $< -o $@



