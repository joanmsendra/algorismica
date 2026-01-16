CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
OBJ = main.o file_io.o problem1.o problem2.o
TARGET = algorismica

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

main.o: main.c file_io.h problem1.h problem2.h
	$(CC) $(CFLAGS) -c main.c

file_io.o: file_io.c file_io.h structs.h
	$(CC) $(CFLAGS) -c file_io.c

problem1.o: problem1.c problem1.h structs.h
	$(CC) $(CFLAGS) -c problem1.c

problem2.o: problem2.c problem2.h structs.h
	$(CC) $(CFLAGS) -c problem2.c

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET) $(TARGET).exe
