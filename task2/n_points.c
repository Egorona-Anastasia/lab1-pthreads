#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "../task1/timer.h"

#define G 6.67430e-11
#define STEPS 1000
#define DEFAULT_THREAD_COUNT 4

struct Point {
    double mass;
    double x, y, z;
    double vx, vy, vz;
};

struct Force {
    double x, y, z;
};

struct ThreadData {
    int start, end;
    int n;
    struct Point* points;
    struct Force* forces;
};

void* calculate_forces(void* arg) {
    struct ThreadData* data = (struct ThreadData*)arg;

    for (int i = data->start; i < data->end; ++i) {
        for (int j = i + 1; j < data->n; ++j) {
            double dx = data->points[j].x - data->points[i].x;
            double dy = data->points[j].y - data->points[i].y;
            double dz = data->points[j].z - data->points[i].z;
            double r2 = dx * dx + dy * dy + dz * dz;

            if (r2 == 0.0) {
                continue;
            }

            double r = sqrt(r2);
            double k = G * data->points[i].mass * data->points[j].mass / (r2 * r);

            double fx = k * dx;
            double fy = k * dy;
            double fz = k * dz;

            data->forces[i].x += fx;
            data->forces[i].y += fy;
            data->forces[i].z += fz;

            data->forces[j].x -= fx;
            data->forces[j].y -= fy;
            data->forces[j].z -= fz;
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        printf("Неверное количество аргументов.\n");
        return 1;
    }

    char* endptr;
    double tend = strtod(argv[1], &endptr);

    if (endptr == argv[1] || *endptr != '\0' || tend <= 0.0) {
        printf("Конечное время должно быть положительным числом.\n");
        return 1;
    }

    long requested_threads = DEFAULT_THREAD_COUNT;

    if (argc == 4) {
        requested_threads = strtol(argv[3], &endptr, 10);

        if (endptr == argv[3] || *endptr != '\0'
            || requested_threads <= 0) {
            printf("Количество потоков должно быть положительным целым числом.\n");
            return 1;
        }
    }

    FILE* in = fopen(argv[2], "r");

    if (in == NULL) {
        perror("Ошибка открытия входного файла");
        return 1;
    }

    int n;

    if (fscanf(in, "%d", &n) != 1 || n <= 0) {
        printf("Неверное количество точек.\n");
        fclose(in);
        return 1;
    }

    struct Point* points = malloc((size_t)n * sizeof(struct Point));

    for (int i = 0; i < n; ++i) {
        struct Point* p = &points[i];

        if (fscanf(in, "%lf %lf %lf %lf %lf %lf %lf", &p->mass, &p->x, &p->y, &p->z, &p->vx, &p->vy, &p->vz) != 7 || p->mass <= 0.0) {
            printf("Неверные данные точки %d.\n", i + 1);
            fclose(in);
            free(points);
            return 1;
        }
    }

    fclose(in);

    int thread_count = (int)requested_threads;
    if (thread_count > n) {
        thread_count = n;
    }

    pthread_t* threads = malloc((size_t)thread_count * sizeof(pthread_t));

    struct ThreadData* thread_data = malloc((size_t)thread_count * sizeof(struct ThreadData));
    struct Force* local_forces = calloc((size_t)thread_count * n, sizeof(struct Force));
    struct Force* forces = calloc((size_t)n, sizeof(struct Force));

    double start, end;
    double total_time = 0.0;

    FILE* out = fopen("n_points.csv", "w");

    if (out == NULL) {
        perror("Ошибка открытия выходного файла");
        free(points);
        free(threads);
        free(thread_data);
        free(local_forces);
        free(forces);
        return 1;
    }

    fprintf(out, "t");
    for (int i = 0; i < n; ++i) {
        fprintf(out, ",x%d,y%d", i + 1, i + 1);
    }
    fprintf(out, "\n");
    fprintf(out, "%.10e", 0.0);
    for (int i = 0; i < n; ++i) {
        fprintf(out, ",%.10e,%.10e", points[i].x, points[i].y);
    }
    fprintf(out, "\n");

    double dt = tend / STEPS;
    for (int step = 1; step <= STEPS; ++step) {
        GET_TIME(start);
        for (int t = 0; t < thread_count; ++t) {
            struct Force* my_forces = local_forces + (size_t)t * n;

            for (int i = 0; i < n; ++i) {
                my_forces[i].x = 0.0;
                my_forces[i].y = 0.0;
                my_forces[i].z = 0.0;
            }

            thread_data[t].start = t * n / thread_count;
            thread_data[t].end = (t + 1) * n / thread_count;
            thread_data[t].n = n;
            thread_data[t].points = points;
            thread_data[t].forces = my_forces;

            int err = pthread_create(&threads[t], NULL, calculate_forces, &thread_data[t]);

            if (err != 0) {
                printf("Ошибка создания потока.\n");
                for (int j = 0; j < t; ++j) {
                    pthread_join(threads[j], NULL);
                }
                fclose(out);
                free(points);
                free(threads);
                free(thread_data);
                free(local_forces);
                free(forces);
                return 1;
            }
        }

        for (int t = 0; t < thread_count; ++t) {
            pthread_join(threads[t], NULL);
        }

        // Объединяем силы, рассчитанные потоками.
        for (int i = 0; i < n; ++i) {
            forces[i].x = 0.0;
            forces[i].y = 0.0;
            forces[i].z = 0.0;

            for (int t = 0; t < thread_count; ++t) {
                struct Force* my_forces = local_forces + (size_t)t * n;
                forces[i].x += my_forces[i].x;
                forces[i].y += my_forces[i].y;
                forces[i].z += my_forces[i].z;
            }
        }

        // Метод Эйлера первого порядка. 
        for (int i = 0; i < n; ++i) {
            points[i].x += points[i].vx * dt;
            points[i].y += points[i].vy * dt;
            points[i].z += points[i].vz * dt;

            points[i].vx += forces[i].x / points[i].mass * dt;
            points[i].vy += forces[i].y / points[i].mass * dt;
            points[i].vz += forces[i].z / points[i].mass * dt;
        }

        GET_TIME(end);
        total_time += end - start;

        fprintf(out, "%.10e", step * dt);
        for (int i = 0; i < n; ++i) {
            fprintf(out, ",%.10e,%.10e", points[i].x, points[i].y);
        }
        fprintf(out, "\n");
    }
    printf("Время вычислений: %.6f секунд\n", total_time);

    fclose(out);
    free(points);
    free(threads);
    free(thread_data);
    free(local_forces);
    free(forces);
    return 0;
}
