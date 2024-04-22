# Makefile

CC = gcc
CPPFLAGS =
CFLAGS = -Wall -Wextra -O3 `pkg-config --cflags sdl2 SDL2_image` -lm
LDFLAGS =
LDLIBS = `pkg-config --libs sdl2 SDL2_image` -lm -lSDL2_ttf

all: main

SRC = main.c
OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=}

main: main.o

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${EXE}
