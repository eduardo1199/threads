#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

struct mutex {
    pthread_mutex_t mutex;
};

struct condvar {
    pthread_cond_t condition;
};

void mutex_init(struct mutex *m) {
    pthread_mutex_init(&m->mutex, NULL);
}

void condvar_init(struct condvar *c) { // iniciar a variavel de condição
    pthread_cond_init(&c->condition, NULL);
}

void condvar_wait(struct condvar *c, struct mutex *m) { // destrava um mutex e poe a thread corrente para dormir
    pthread_cond_wait(&c->condition, &m->mutex);
}

void condvar_signal(struct condvar *c) { // acorde ao menos um thread
    pthread_cond_signal(&c->condition);
}

void condvar_broadcast(struct condvar *c) { // acordar todas as threads
    pthread_cond_broadcast(&c->condition);
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
    condvar_wait(&cv, mutex_work);

    printf("Worker thread received signal and is continuing\n");
    pthread_mutex_unlock(&mutex_work->mutex);

    pthread_exit(NULL);
}

int main() {
    pthread_t threads[3];

    // Inicializa a condição
    condvar_init(&cv);

    // crie 3 threads
    for (int i = 0; i < 3; ++i) {
        pthread_create(&threads[i], NULL, worker_thread, &mutex);
    }

    // Aguarda um pouco antes de sinalizar as threads
    sleep(2);

    printf("Main thread is signaling unique worker thread to continue\n");

    // Sinaliza todas as threads para continuar
    //condvar_broadcast(&cv);

    // Sinaliza para ao menos 1 thread continuar
    condvar_signal(&cv);

    // Aguarda que todas as threads terminem
    for (int i = 0; i < 3; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}