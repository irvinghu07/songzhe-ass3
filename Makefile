CC=gcc
CFLAGS=-Wall -g
LDFLAGS=-L./unity
LDLIBS=-lunity

SRCS=test.c mem.c
OBJS=$(SRCS:.c=.o)
TARGET=test

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

