CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra
LDFLAGS = -lm
SRC = src/exercicio.c
OBJ = $(SRC:.c=.o)
BINDIR = build/bin
TARGET = $(BINDIR)/exercicio

.PHONY: all clean run example

all: $(TARGET)

$(BINDIR):
	mkdir -p $(BINDIR)

$(TARGET): $(SRC) | $(BINDIR)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -rf build

run: all
	$(TARGET) seq 10000

example: all
	$(TARGET) par 10000 4 pipe
