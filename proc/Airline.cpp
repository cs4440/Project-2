/*
 * Simulate an airline with passenger threads, baggage handler threads,
 * security screen threadds, and flight attendant threads. Each passenger
 * threads will need to pass through baggage handler, security screener,
 * and flight attendant before it can be seaed in he airplane.
 *
 * DETAILS
 * -------
 * baggage_handler, security_screener, and flight_attendant threads will
 * wait for BAGGAGE, SEC_SCREENER and FLIGHT_ATTENDNT semaphores respectively.
 * When wait passes, each of these threads will signal BAGGAGE_DONE,
 * SEC_SCREENER_DONE, and FLIGHT_ATTENDANT_DONE semeaphores to the corresponding
 * passenger thread.
 * Each passenger threads will signal SEC_SCREENER and FLIGHT_ATTENDNT in order
 * and wait for response for BAGGAGE_DONE, SEC_SCREENER_DONE, and
 * FLIGHT_ATTENDANT_DONE. Once each of the semaphores pass, the passenger has
 * been seated and signal SEATED to a controller thread.
 * The controlle thread will count the number of SEATED signals and once every
 * passengers has seated, the controller thread will resignal all the workers
 * to stop and exit.
 */
#include <pthread.h>    // posix thread
#include <semaphore.h>  // semaphore functions
#include <unistd.h>     // usleep()
#include <cstdlib>      // atoi()
#include <iostream>     // cout
#include <vector>       // vector class

// GLOBAL
bool STOP = false;
int PASS_NUM = 100, BH_NUM = 3, SS_NUM = 5, FA_NUM = 2;
pthread_mutex_t PRINT_LOCK;
sem_t SEATED, BAGGAGE, BAGGAGE_DONE, SEC_SCREENER, SEC_SCREENER_DONE,
    FLIGHT_ATTENDANT, FLIGHT_ATTENDANT_DONE;

void *controller(void *args) {
    // wait for all passgers to be seated
    for(int i = 0; i < PASS_NUM; ++i) sem_wait(&SEATED);

    // signal all proccesses with STOP = true
    STOP = true;
    for(int i = 0; i < BH_NUM; ++i) sem_post(&BAGGAGE);
    for(int i = 0; i < SS_NUM; ++i) sem_post(&SEC_SCREENER);
    for(int i = 0; i < FA_NUM; ++i) sem_post(&FLIGHT_ATTENDANT);

    pthread_exit(0);
}

void *passenger(void *args) {
    int pass_id = int(*(int *)args);

    // passanger arrives
    pthread_mutex_lock(&PRINT_LOCK);
    std::cout << std::string("Passenger #" + std::to_string(pass_id) +
                             " has arrived at terminal")
              << std::endl;
    pthread_mutex_unlock(&PRINT_LOCK);

    // signal to baggage handler
    sem_post(&BAGGAGE);

    pthread_mutex_lock(&PRINT_LOCK);
    std::cout << std::string("Passenger #" + std::to_string(pass_id) +
                             " is waiting at baggage processsing for a handler")
              << std::endl;
    pthread_mutex_unlock(&PRINT_LOCK);

    // wait for baggage handler to finish
    sem_wait(&BAGGAGE_DONE);

    // signal to security screener
    sem_post(&SEC_SCREENER);

    pthread_mutex_lock(&PRINT_LOCK);
    std::cout << std::string("Passenger #" + std::to_string(pass_id) +
                             " is waiting for a security screener")
              << std::endl;
    pthread_mutex_unlock(&PRINT_LOCK);

    // wait for baggage handler to finish
    sem_wait(&SEC_SCREENER_DONE);

    // signal to flight attendant
    sem_post(&FLIGHT_ATTENDANT);

    pthread_mutex_lock(&PRINT_LOCK);
    std::cout << std::string("Passenger #" + std::to_string(pass_id) +
                             " is waiting to board the plane by an attendant")
              << std::endl;
    pthread_mutex_unlock(&PRINT_LOCK);

    // wait to be seated
    sem_wait(&FLIGHT_ATTENDANT_DONE);

    // sginal that passenger has been seated
    sem_post(&SEATED);

    pthread_mutex_lock(&PRINT_LOCK);
    std::cout << std::string("Passenger #" + std::to_string(pass_id) +
                             " has been seated and relaxes")
              << std::endl;
    pthread_mutex_unlock(&PRINT_LOCK);

    pthread_exit(0);
}

void *baggage_handler(void *args) {
    while(true) {
        // wait for passenger baggage
        sem_wait(&BAGGAGE);

        if(STOP) break;

        usleep(100);

        // signal baggage is done
        sem_post(&BAGGAGE_DONE);
    }
    pthread_exit(0);
}

void *security_screener(void *args) {
    while(true) {
        // waiting for passenger to be screened
        sem_wait(&SEC_SCREENER);

        if(STOP) break;

        usleep(100);

        // signal screening is done
        sem_post(&SEC_SCREENER_DONE);
    }
    pthread_exit(0);
}

void *flight_attendant(void *args) {
    while(true) {
        // waiting for passenger to be attended
        sem_wait(&FLIGHT_ATTENDANT);

        if(STOP) break;

        usleep(100);

        // signal flight attendant is done
        sem_post(&FLIGHT_ATTENDANT_DONE);
    }
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    std::vector<int *> args{&PASS_NUM, &BH_NUM, &SS_NUM, &FA_NUM};
    std::vector<int> pass_ids;
    pthread_t control, *passengers, *bhandlers, *sec_screeners,
        *flight_attendants;

    // update global count from arguments
    for(int i = 1; i < argc && i <= (int)args.size(); ++i)
        *args[i - 1] = atoi(argv[i]);

    if(PASS_NUM == 0 || BH_NUM == 0 || SS_NUM == 0 || FA_NUM == 0) {
        std::cerr << "Invalid passenger, baggage handler, security screen or "
                     "flight attendants count"
                  << std::endl;
        return 1;
    }

    // allocate threads
    bhandlers = new pthread_t[BH_NUM];
    sec_screeners = new pthread_t[SS_NUM];
    flight_attendants = new pthread_t[FA_NUM];
    passengers = new pthread_t[PASS_NUM];

    // init mutex and semaphore
    pthread_mutex_init(&PRINT_LOCK, NULL);
    sem_init(&SEATED, 0, 0);
    sem_init(&BAGGAGE, 0, 0);
    sem_init(&BAGGAGE_DONE, 0, 0);
    sem_init(&SEC_SCREENER, 0, 0);
    sem_init(&SEC_SCREENER_DONE, 0, 0);
    sem_init(&FLIGHT_ATTENDANT, 0, 0);
    sem_init(&FLIGHT_ATTENDANT_DONE, 0, 0);

    // init passenger ids
    for(int i = 0; i < PASS_NUM; ++i) pass_ids.emplace_back(i);

    // spawn threads
    pthread_create(&control, NULL, controller, NULL);
    for(int i = 0; i < BH_NUM; ++i)
        pthread_create(&bhandlers[i], NULL, baggage_handler, NULL);
    for(int i = 0; i < SS_NUM; ++i)
        pthread_create(&sec_screeners[i], NULL, security_screener, NULL);
    for(int i = 0; i < FA_NUM; ++i)
        pthread_create(&flight_attendants[i], NULL, flight_attendant, NULL);
    for(int i = 0; i < PASS_NUM; ++i)
        pthread_create(&passengers[i], NULL, passenger, &pass_ids[i]);

    // join threads
    pthread_join(control, NULL);
    for(int i = 0; i < BH_NUM; ++i) pthread_join(bhandlers[i], NULL);
    for(int i = 0; i < SS_NUM; ++i) pthread_join(sec_screeners[i], NULL);
    for(int i = 0; i < FA_NUM; ++i) pthread_join(flight_attendants[i], NULL);
    for(int i = 0; i < PASS_NUM; ++i) pthread_join(passengers[i], NULL);

    // delete allocations
    delete[] bhandlers;
    delete[] sec_screeners;
    delete[] flight_attendants;
    delete[] passengers;

    return 0;
}
