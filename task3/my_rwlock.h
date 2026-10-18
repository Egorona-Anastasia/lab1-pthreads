#ifndef MY_RWLOCK_H
#define MY_RWLOCK_H

#include <pthread.h>
"""
Создать структуру, включающую в себя:

Мьютекс;

Две условные переменные (одну для читателей, другую для писателей);

Счётчик читателей (сколько потоков в данный момент читают);

Счётчик количества потоков, ожидающих получения блокировки на чтение;

Счётчик количества потоков, ожидающих получения блокировки на запись;

Флаг, показывающий, получил ли блокировку хотя бы один писатель в данный момент."""

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t  read_cond;
    pthread_cond_t  write_cond;
    int active_readers;
    int waiting_readers;
    int waiting_writers;
    int writer_active;
} my_rwlock_t;

"""
Инициализация и
уничтожение осуществляются по знакомой схеме:
в pth_ll_rwl.c используется от библиотеки:
[  ] pthread_rwlock_t rwlock;
[  ] pthread_rwlock_init(&rwlock, NULL);
[  ] pthread_rwlock_rdlock(&rwlock);
[  ] pthread_rwlock_wrlock(&rwlock);
[  ] pthread_rwlock_unlock(&rwlock);
[  ] pthread_rwlock_destroy(&rwlock);
"""
int my_rwlock_init(my_rwlock_t* lock);
int my_rwlock_destroy(my_rwlock_t* lock);
int my_rdlock(my_rwlock_t* lock);
int my_wrlock(my_rwlock_t* lock);
int my_unlock(my_rwlock_t* lock);

#endif