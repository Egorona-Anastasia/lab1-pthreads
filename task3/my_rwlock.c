#include "my_rwlock.h"

int my_rwlock_init(my_rwlock_t* lock) {
    int err = 0;

    //в отличае от аналогичного куска семинар 2 ставим null - нам не нужны данные атрибуты.
    if ((err = pthread_mutex_init(&lock->mutex, NULL)) != 0) return err;
    if ((err = pthread_cond_init(&lock->read_cond, NULL)) != 0)  return err;
    if ((err = pthread_cond_init(&lock->write_cond, NULL)) != 0) return err;

    //счетчики
    lock->active_readers  = 0;
    lock->waiting_readers = 0;
    lock->waiting_writers = 0;
    
    lock->writer_active   = 0; //флаг

    return 0;
}

int my_rdlock(my_rwlock_t* lock) {
    pthread_mutex_lock(&lock->mutex);

    if (lock->writer_active || lock->waiting_writers > 0) {
        // надо подождать, тк либо писатель внутри, либо писатель ждёт и не пускаем читателя вперед него
        lock->waiting_readers++;
        while (lock->writer_active || lock->waiting_writers > 0) {
            pthread_cond_wait(&lock->read_cond, &lock->mutex);
        }
        lock->waiting_readers--;
    }

    lock->active_readers++;
    pthread_mutex_unlock(&lock->mutex);
    return 0;
}

int my_wrlock(my_rwlock_t* lock) {
    pthread_mutex_lock(&lock->mutex);

    // пусть писатель ждёт, пока др писатель работает/пока читают
    if (lock->writer_active || lock->active_readers > 0) {
        lock->waiting_writers++;
        while (lock->writer_active || lock->active_readers > 0) {
            pthread_cond_wait(&lock->write_cond, &lock->mutex);
        }
        lock->waiting_writers--;
    }

    lock->writer_active = 1;
    pthread_mutex_unlock(&lock->mutex);
    return 0;
}

int my_unlock(my_rwlock_t* lock) {
    pthread_mutex_lock(&lock->mutex);

    if (lock->writer_active) {
        // был писатель —> отпускаем
        lock->writer_active = 0;
        // если в  ожидании писатель будим только 1 из них (signal)
        if (lock->waiting_writers > 0) {
            pthread_cond_signal(&lock->write_cond);
        } else if (lock->waiting_readers > 0) { //будим всех (broadcast), тк читатели др др не мешают
            pthread_cond_broadcast(&lock->read_cond);
        }
    } else {
        // был читатель —> уменьшаем счётчик
        lock->active_readers--;
        // если в  ожидании есть писатель и нет читающих будим только 1 из первых
        if (lock->active_readers == 0 && lock->waiting_writers > 0) {
            pthread_cond_signal(&lock->write_cond);
        }
    }

    pthread_mutex_unlock(&lock->mutex);
    return 0;
}
/*
                                                        !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
нельзя вызывать, пока блокировка используется хоть кем то
-> вызываем после завершения всех потоков
*/

int my_rwlock_destroy(my_rwlock_t* lock) {
    int err = 0;

    if ((err = pthread_mutex_destroy(&lock->mutex)) != 0) return err;
    if ((err = pthread_cond_destroy(&lock->read_cond)) != 0)  return err;
    if ((err = pthread_cond_destroy(&lock->write_cond)) != 0) return err;

    return 0;
}