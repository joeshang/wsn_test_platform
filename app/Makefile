CC = gcc
#CC = arm-none-linux-gnueabi-gcc
CFLAGS += -g -Wall -c -D_DEBUG
LDFLAGS = 

COMMON_DIR = ../common
MODULE_DIR = ../module
CFLAGS += -I$(COMMON_DIR)\
		  -I../drivers/gpio\
		  -I$(MODULE_DIR)/hex_parser\

TARGET = oint_board_agent
OBJECTS = main.o\
		  gather_board.o\
		  packet_transfer.o\
		  reprogrammer.o\
		  util.o\
		  $(COMMON_DIR)/dlist.o\
		  $(MODULE_DIR)/hex_parser/hex_parser.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c, %.h
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf *~ *.swp $(OBJECTS) $(TARGET)
