CC=mpicc
CFLAGS=-Wall -Werror -g

EXECUTABLES=main

all: $(EXECUTABLES)

main: main.c
	$(CC) $< $(CFLAGS) -o $@

clean:
	rm -f $(EXECUTABLES)
