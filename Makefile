CC = gcc
CFLAGS = -g -Wall

all: mysh

mysh: mysh.c
	$(CC) $(CFLAGS) -o mysh mysh.c

clean:
	rm -f mysh
