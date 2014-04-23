CC = gcc
CFLAGS += -g -Wall -c -D_DEBUG
LDFLAGS += -lm
RM = rm -rf
LIB_DIR = lib

CFLAGS += -I$(LIB_DIR)/cJSON/ \
		  -I$(LIB_DIR)/libtelnet

TARGET = deployer

OBJECTS += main.o \
		   logger.o \
		   ftp_uploader.o \
		   telnet_commander.o \
		   deployer.o

OBJECTS += $(LIB_DIR)/cJSON/cJSON.o\
		   $(LIB_DIR)/libtelnet/libtelnet.o

$(TARGET): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(TARGET) *~ *.swp log
