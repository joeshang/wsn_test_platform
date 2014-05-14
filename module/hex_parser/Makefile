CC = gcc
CFLAGS = -g -Wall -c
LDFLAGS =

TARGETS = hex_parser
OBJECTS = hex_parser.o\
		  main.o

$(TARGETS): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf *~ *.swp $(OBJECTS) $(TARGETS)
