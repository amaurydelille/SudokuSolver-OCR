# Makefile

CC = gcc
CPPFLAGS =
CFLAGS = -Wall -Wextra -O3 `pkg-config --cflags sdl2 SDL2_image`
LDFLAGS =
LDLIBS = -lm `pkg-config --libs sdl2 SDL2_image`

all: main treatment.c

SRC = main.c
OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=}

main: main.o treatment.o

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${EXE}

# END