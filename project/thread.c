#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

struct mutex {
    pthread_mutex_t mutex;
};

int number_of_thread = 3;
struct condvar {
    struct mutex *mutex;
    int number_cond;
};

void mutex_init(struct mutex *m) {
    pthread_mutex_init(&m->mutex, NULL);
}

void condvar_init(struct condvar *c, struct mutex *m) {
    c->mutex = m;
    c->number_cond = 0;
}

void condvar_wait(struct condvar *c) {
    pthread_mutex_unlock(&c->mutex->mutex);

    while (c->number_cond == 0) {
        pthread_mutex_unlock(&c->mutex->mutex);
        pthread_mutex_lock(&c->mutex->mutex);
    }

    c->number_cond--;
}

void condvar_signal(struct condvar *c) {
    c->number_cond = 1;

    pthread_mutex_unlock(&c->mutex->mutex);  
}

void condvar_broadcast(struct condvar *c) {
    c->number_cond = number_of_thread;
    pthread_mutex_unlock(&c->mutex->mutex);

    for (int i = 0; i < c->number_cond; ++i) {
        // Sinaliza todas as threads uma por uma
        pthread_mutex_lock(&c->mutex->mutex);
        pthread_mutex_unlock(&c->mutex->mutex);
    }

    //c->number_cond = 0;
}

struct mutex mutex;
struct condvar cv;

void *worker_thread(void *arg) {
    struct mutex *mutex_work = (struct mutex *)arg;

    printf("Worker thread is starting\n");

    mutex_init(mutex_work);

    pthread_mutex_lock(&mutex_work->mutex);

    printf("Worker thread is waiting\n");

    // Espera pelo sinal
    condvar_wait(&cv);

    printf("Worker thread received signal and is continuing\n");
    pthread_mutex_unlock(&mutex_work->mutex);

    pthread_exit(NULL);
}

int main() {
    pthread_t threads[3];

    // Inicializa a condição
    condvar_init(&cv, &mutex);

    // Cria 3 threads
    for (int i = 0; i < number_of_thread; ++i) {
        pthread_create(&threads[i], NULL, worker_thread, &mutex);
    }

    // Aguarda um pouco antes de sinalizar as threads
    sleep(2);

    // Sinaliza todas as threads para continuar
    condvar_broadcast(&cv);

    // Aguarda que todas as threads terminem
    for (int i = 0; i < 3; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
