# Makefile

CC = gcc
CPPFLAGS =
CFLAGS = -Wall -Wextra
LDFLAGS =
LDLIBS = 

all: main

SRC = main.c
OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=}

main: main.o

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${EXE}
