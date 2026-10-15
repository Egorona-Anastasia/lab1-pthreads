#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#include "my_rand.h"
#include "timer.h"

#define _POSIX_C_SOURCE 200809L 
#define MAX_ITER 1000
#define ESCAPE_RADIUS_SQ 4.0

int thread_count;
int npoints;

double* re_buf;
double* im_buf;
int*    found;

// как в листинг 3 («Передача и возвращение структур из потока»)
// Структура: номер потока, кол-во точек которые должен найти этот конкретный поток, свои значения для потоков (чтобы последовательности не совпадали и не происходила пустая работа)
struct thread_arg {
    long rank;
    int capacity;
    unsigned seed;
};

//проверка принадлежности, возвращает 1, если не убежала за MAX_ITER итераций
int is_mandelbrot(double c_re, double c_im) {
    double z_re = 0.0;
    double z_im = 0.0;
    for (int i = 0; i < MAX_ITER; ++i) {
        double z_re2 = z_re * z_re; // ^2
        double z_im2 = z_im * z_im; // ^2
        if (z_re2 + z_im2 > ESCAPE_RADIUS_SQ) return 0;
        double new_re = z_re2 - z_im2 + c_re;
        double new_im = 2.0 * z_re * z_im + c_im;
        z_re = new_re;
        z_im = new_im;
    }
    return 1;
}

void* thread_work(void* arg_p) {
    struct thread_arg* arg = (struct thread_arg*) arg_p;
    long my_rank    = arg->rank;
    int  my_capacity = arg->capacity;
    unsigned seed   = arg->seed;
    int  my_count   = 0; // счётчик найденных точек для ЭТОГО потока
    while (my_count < my_capacity) {
        double c_re = -2.0 + 3.0 * my_drand(&seed);
        double c_im = -1.5 + 3.0 * my_drand(&seed);
        if (is_mandelbrot(c_re, c_im)) {
            //начало блока + позиция внутри блока = >  гарант, что потоки никогда не пересекутся по индексам
            re_buf[my_rank * my_capacity + my_count] = c_re;
            im_buf[my_rank * my_capacity + my_count] = c_im;
            my_count++;
        }
    }
    found[my_rank] = my_count;
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s nthreads npoints\n", argv[0]);
        return 1;
    }
    thread_count = (int) strtol(argv[1], NULL, 10);
    npoints      = (int) strtol(argv[2], NULL, 10);

    pthread_t* thread_handles = malloc(thread_count * sizeof(pthread_t));
    struct thread_arg* args   = malloc(thread_count * sizeof(struct thread_arg));

    int base = npoints / thread_count;
    int rem  = npoints % thread_count;
    int cap_max = base + (rem > 0 ? 1 : 0);

    re_buf = malloc((size_t) thread_count * cap_max * sizeof(double));
    im_buf = malloc((size_t) thread_count * cap_max * sizeof(double));
    found  = malloc(thread_count * sizeof(int));

    double start, finish;
    GET_TIME(start); //макрос из timer.h

    //раскидывается поток
    for (long t = 0; t < thread_count; ++t) {
        args[t].rank     = t;
        args[t].capacity = base + (t < rem ? 1 : 0);
        args[t].seed     = 1u + 7919u * (unsigned) t;
        pthread_create(&thread_handles[t], NULL, thread_work, (void*) &args[t]);
    }
    for (long t = 0; t < thread_count; ++t) {
        pthread_join(thread_handles[t], NULL);
    }
    GET_TIME(finish);
    printf("nthreads=%d npoints=%d elapsed=%e s\n", thread_count, npoints, finish - start);
    // записываем (или пере___) в файл
    FILE* out = fopen("mandelbrot.csv", "w");
    if (out == NULL) { perror("fopen"); return 1; }
    fprintf(out, "re,im\n");
    for (int t = 0; t < thread_count; ++t) {
        for (int k = 0; k < found[t]; ++k) {
            fprintf(out, "%.10f,%.10f\n",
                    re_buf[t * cap_max + k],
                    im_buf[t * cap_max + k]);
        }
    }
    // освобождаем всё, что выделялось malloc
    fclose(out);
    free(thread_handles);
    free(args);
    free(re_buf);
    free(im_buf);
    free(found);
    return 0;
}