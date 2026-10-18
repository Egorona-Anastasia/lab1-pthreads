CC      := gcc
CFLAGS  := -O2 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
LDFLAGS := -pthread -lm

.PHONY: all task1 task2 clean

all: task1 task2

task1: task1/mandelbrot_set.c task1/my_rand.c
	$(CC) $(CFLAGS) $^ -o task1/mandelbrot_set $(LDFLAGS)

task2: task2/n_points.c
	$(CC) $(CFLAGS) $^ -o task2/n_points $(LDFLAGS)

clean:
	rm -f task1/mandelbrot_set task2/n_points task1/*.o task2/*.o
