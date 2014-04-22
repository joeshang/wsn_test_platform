CC = gcc
CFLAGS += -g -Wall -c
LDFLAGS += -lm
LIB_DIR = ../lib
CFLAGS += -I$(LIB_DIR)/cJSON/ \
		  -I../

TARGET = config_json_generator

OBJECTS += config_json_generator.o \
		   $(LIB_DIR)/cJSON/cJSON.o

$(TARGET): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJECTS) *.swp *~
