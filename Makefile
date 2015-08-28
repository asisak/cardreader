CC=gcc
CFLAGS=-c -Wall -std=c99
LDFLAGS=-lldap
SOURCES=main.c deciderLDAP.c UDPSocketClient.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=server

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm -f *~ *.o

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
.c.o:
	$(CC) $(CFLAGS) $< -o $@



