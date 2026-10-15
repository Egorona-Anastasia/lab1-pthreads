CC      := gcc
CFLAGS  := -O2 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
LDFLAGS := -pthread -lm

.PHONY: all task1 clean

all: task1

task1: task1/mandelbrot_set.c task1/my_rand.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o task1/mandelbrot_set

clean:
	rm -f task1/mandelbrot_set task1/*.o