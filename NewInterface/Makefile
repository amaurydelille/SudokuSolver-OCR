aCC = gcc
CPPFLAGS =
CFLAGS = -Wall -Wextra -O3 `pkg-config --cflags gtk+-3.0 sdl2 SDL2_image` -lm -lSDL2_ttf
LIBS = `pkg-config --libs gtk+-3.0`
LDFLAGS =
LDLIBS = `pkg-config --libs sdl2 SDL2_image gtk+-3.0` -lm -lSDL2_ttf

SRC = main.c ../ImageTreatment/treatment.c ../GridDetection/cutter.c ../GridDetection/crop-manager.c ../GridDetection/utils.c ../GridReconstruction/main.c ../GridSolver/solver.c ../DigitRecognition/neuralnetwork.c ../DigitRecognition/files.c
OBJ = ${SRC:.c=.o}

# Default rule to compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

main: $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o main $(LDLIBS)

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} main

