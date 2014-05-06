CC = gcc
CFLAGS += -g -Wall -c -D_DEBUG
LDFLAGS += -lm
RM = rm -rf
LIB_DIR = lib
COMMON_DIR = ../common

CFLAGS += -I$(LIB_DIR)/cJSON/ \
		  -I$(LIB_DIR)/libtelnet \
		  -I$(COMMON_DIR)

TARGET = deployer

OBJECTS += main.o \
		   dlist.o \
		   ftp_uploader.o \
		   telnet_commander.o \
		   deployer.o

OBJECTS += $(LIB_DIR)/cJSON/cJSON.o\
		   $(LIB_DIR)/libtelnet/libtelnet.o\
		   $(COMMON_DIR)/logger.o

$(TARGET): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(TARGET) *~ *.swp *.log
