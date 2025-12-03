# Makefile for Fireboy & Watergirl Game (Cross Platform)

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
SRCDIR = src
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/console.c $(SRCDIR)/input.c $(SRCDIR)/map.c $(SRCDIR)/renderer.c $(SRCDIR)/player.c
OBJECTS = $(SOURCES:.c=.o)

# 플랫폼별 설정
ifeq ($(OS),Windows_NT)
    TARGET = fireboy_watergirl.exe
    LDFLAGS = -luser32
else
    # Unix/macOS/Linux
    TARGET = fireboy_watergirl
    LDFLAGS =
endif

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS) -lm

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) $(TARGET).exe

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET)