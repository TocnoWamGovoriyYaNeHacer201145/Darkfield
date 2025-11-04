CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = darkfield
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
