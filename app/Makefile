CC = gcc
CFLAGS += -g -Wall -c
LDFLAGS = 

COMMON_DIR = ../common
CFLAGS += -I$(COMMON_DIR)

TARGET = oint_board_agent
OBJECTS = main.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $< $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -rf *~ *.swp $(OBJECTS) $(TARGET)
