# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lSDL2 -lSDL2_image -lm

# Source and object files
SRC = GridCutter.c BlobDetection.c Pixels.c Verbose.c FloodFill.c Morphology.c
OBJ = $(SRC:.c=.o)

# Executable name
EXE = grid_cutter

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXE) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXE)

.PHONY: all clean
