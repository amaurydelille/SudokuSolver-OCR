# Makefile
CC = gcc
CPPFLAGS = $(shell pkg-config --cflags sdl2 SDL2_image)
CFLAGS = -Wall -Wextra -O3
LDFLAGS =
LDLIBS = $(shell pkg-config --libs sdl2 SDL2_image) -lm

all: neuralnetwork 

SRC = neuralnetwork.c main.c files.c
OBJ = ${SRC:.c=.o}

neuralnetwork: ${OBJ}
	${CC} ${CFLAGS} ${CPPFLAGS} -o $@ $^ ${LDLIBS}

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} neuralnetwork