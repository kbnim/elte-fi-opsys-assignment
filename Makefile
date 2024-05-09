
CC = gcc
CFLAGS = -W -Wall -Wextra -pedantic

all: bunny

bunny: src/main.c src/String.c src/Vector.c src/Application.c src/PosixUtils.c
	$(CC) $(CFLAGS) $^ -o $@ 

clean:
	rm bunny