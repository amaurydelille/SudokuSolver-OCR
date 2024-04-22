# Makefile

CC = gcc
CPPFLAGS =
CFLAGS = -Wall -Wextra -O3 `pkg-config --cflags sdl2 SDL2_image` -lm -lSDL2_ttf
LDFLAGS =
LDLIBS = `pkg-config --libs sdl2 SDL2_image` -lm -lSDL2_ttf

all: main

SRC = main.c interface.c ../ImageTreatment/treatment.c
OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=}

main: main.o interface.o ../ImageTreatment/treatment.o

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${EXE}

# END