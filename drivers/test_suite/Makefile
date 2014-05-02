CC = arm-none-linux-gnueabi-gcc
CFLAGS = -g -Wall -I../led
LDFLAGS =
TARGETS = oint_leds_test

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -rf *~ $(TARGETS)
