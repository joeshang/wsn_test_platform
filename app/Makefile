CC = gcc
CFLAGS += -g -Wall -c -D_DEBUG
LDFLAGS = 

COMMON_DIR = ../common
CFLAGS += -I$(COMMON_DIR)\
		  -I../drivers/gpio

TARGET = oint_board_agent
OBJECTS = main.o\
		  gather_board.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $< $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -rf *~ *.swp $(OBJECTS) $(TARGET)
