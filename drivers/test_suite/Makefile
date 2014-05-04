CC = arm-none-linux-gnueabi-gcc
CFLAGS = -g -Wall -I../gpio
LDFLAGS =
TARGETS = oint_gpio_test

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -rf *~ $(TARGETS)
