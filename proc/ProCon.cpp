#include <pthread.h>
#include <semaphore.h>
#include <iostream>

// GLOBAL
const int BUF_SZ = 5;
bool STOP = false;           // flag to stop threads
int IN = 0,                  // index for consumer
    OUT = 0;                 // index for producer
pthread_mutex_t PRINT_LOCK;  // lock for printing
sem_t PRODUCE,               // signal need to produce
    CONSUME,                 // signal need to consume
    CONTROLLER;              // signal controller to count

void *consumer(void *args) {
    char chr, *buf = (char *)args;

    while(true) {
        sem_wait(&CONSUME);  // waiting to consume

        if(STOP) break;

        chr = buf[OUT++];

        pthread_mutex_lock(&PRINT_LOCK);
        std::cout << "Consuming [" << OUT - 1 << "]: " << chr << std::endl;
        pthread_mutex_unlock(&PRINT_LOCK);

        OUT %= BUF_SZ;

        sem_post(&CONTROLLER);

        sem_post(&PRODUCE);  // signal to produce
    }
    pthread_exit(0);
}

void *producer(void *args) {
    char chr = 33, *buf = (char *)args;

    while(true) {
        sem_wait(&PRODUCE);  // waiting to produce

        if(STOP) break;
        if(chr > 126) chr = 33;  // reset ascii character

        buf[IN++] = chr++;

        pthread_mutex_lock(&PRINT_LOCK);
        std::cout << "Producing [" << IN - 1 << "]: " << char(chr - 1)
                  << std::endl;
        pthread_mutex_unlock(&PRINT_LOCK);

        IN %= BUF_SZ;

        sem_post(&CONSUME);  // signal to consume
    }
    pthread_exit(0);
}

// The controller will stop consumer/producer
void *controller(void *args) {
    int limit = *(int *)args;

    for(int i = 0; i < limit; ++i) sem_wait(&CONTROLLER);

    // signal consumer/producer with flag STOP = true
    STOP = true;
    sem_post(&CONSUME);
    sem_post(&PRODUCE);

    pthread_exit(0);
}

int main() {
    int count_limit = 10;
    char buf[BUF_SZ];
    pthread_t tid[3];

    pthread_mutex_init(&PRINT_LOCK, NULL);

    std::cout << "Simulating buffer empty" << std::endl;

    sem_init(&CONTROLLER, 0, 0);
    sem_init(&CONSUME, 0, 0);       //  init empty buffer
    sem_init(&PRODUCE, 0, BUF_SZ);  // producer need to produce by BUF_SZ times
    STOP = false;
    OUT = 0;
    IN = 0;

    pthread_create(&tid[0], NULL, controller, &count_limit);
    pthread_create(&tid[1], NULL, consumer, buf);
    pthread_create(&tid[2], NULL, producer, buf);
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);

    std::cout << "\n\nSimulating buffer full" << std::endl;

    // init full buffer data
    for(int i = 0; i < BUF_SZ; ++i) buf[i] = i + 'A';

    sem_init(&CONTROLLER, 0, 0);
    sem_init(&CONSUME, 0, BUF_SZ);  // init full buffer of BUF_SZ
    sem_init(&PRODUCE, 0, 0);       // producer can't produce
    STOP = false;
    OUT = 0;
    IN = 0;

    pthread_create(&tid[0], NULL, controller, &count_limit);
    pthread_create(&tid[1], NULL, consumer, buf);
    pthread_create(&tid[2], NULL, producer, buf);
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);

    std::cout << "\n\nSimulating buffer not empty" << std::endl;

    // init full buffer data
    buf[0] = 'a';
    buf[1] = 'b';

    sem_init(&CONTROLLER, 0, 0);
    sem_init(&CONSUME, 0, 2);  // init buffer has 2 characters
    sem_init(&PRODUCE, 0, 3);  // producer need to produce 3 characters
    STOP = false;
    OUT = 0;  // consumer index at 0
    IN = 2;   // producer index at 2

    pthread_create(&tid[0], NULL, controller, &count_limit);
    pthread_create(&tid[1], NULL, consumer, buf);
    pthread_create(&tid[2], NULL, producer, buf);
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);

    return 0;
}
