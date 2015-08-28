CC=gcc
CFLAGS=-c -Wall -std=c99
LDFLAGS=-lldap -llber
SOURCES=main.c oracleLDAP.c UDPSocketClient.c logger.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=rfcontrol-server

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm -f *~ *.o

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
.c.o:
	$(CC) $(CFLAGS) $< -o $@
