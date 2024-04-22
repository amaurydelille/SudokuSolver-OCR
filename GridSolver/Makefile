# Définition des compilateurs
CC=gcc
CFLAGS=-I../DigitRecognition -g
LDFLAGS=-lSDL2 -lSDL2_image -lm

# Fichiers du dossier GridSolver
GRIDSOLVER_SRC= main.c solver.c ../DigitRecognition/neuralnetwork.c ../DigitRecognition/files.c
GRIDSOLVER_OBJ=$(GRIDSOLVER_SRC:.c=.o)

# Nom de l'exécutable
EXEC=gridsolver

# Règle par défaut
all: $(EXEC)

# Règle pour créer l'exécutable
$(EXEC): $(GRIDSOLVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Règle générique pour convertir un .c en .o
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Nettoyage
clean:
	rm -f $(GRIDSOLVER_OBJ) $(EXEC)
