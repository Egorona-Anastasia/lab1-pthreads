CC      := gcc
CFLAGS  := -O2 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
LDFLAGS := -pthread -lm

.PHONY: all task1 task2 task3 clean

all: task1 task2 task3

task1: task1/mandelbrot_set.c task1/my_rand.c
	$(CC) $(CFLAGS) $^ -o task1/mandelbrot_set $(LDFLAGS)

task2: task2/n_points.c
	$(CC) $(CFLAGS) $^ -o task2/n_points $(LDFLAGS)

task3: task3/bench_lib.c task3/bench_custom.c task3/my_rwlock.c task3/my_rand.c
	$(CC) $(CFLAGS) $(LDFLAGS) task3/bench_lib.c task3/my_rand.c -o task3/bench_lib
	$(CC) $(CFLAGS) $(LDFLAGS) task3/bench_custom.c task3/my_rwlock.c task3/my_rand.c -o task3/bench_custom
clean:
	rm -f task1/mandelbrot_set task2/n_points task3/bench_lib task3/bench_custom task1/*.o task2/*.o task3/*.o