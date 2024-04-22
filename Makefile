# Makefile

CC = gcc
CPPFLAGS =
CFLAGS = -Wall -Wextra -O3 `pkg-config --cflags sdl2 SDL2_image` -lm -lSDL2_ttf
LDFLAGS =
LDLIBS = `pkg-config --libs sdl2 SDL2_image` -lm -lSDL2_ttf

all: main Interface/interface.c ImageTreatment/treatment.c

SRC = main.c
OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=}

main: main.o Interface/interface.o ImageTreatment/treatment.o
	$(CC) -o main $^ $(LDFLAGS) $(LDLIBS)

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${EXE}

# END