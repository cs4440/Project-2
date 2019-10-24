/*
 * Simulate an mother and father program, where the mother will perform tasks
 * for each children and the father will perofrm the last two tasks. THe mother
 * will sleep once her tasks are done and the father will sleep and also wake
 * up the mother to continue the next day.
 *
 * DETAILS
 * -------
 * The mother thread will cycle through a number of days and within each day
 * cycle the number of children to perform x number of tasks. Once the mother
 * reaches the task to clean the child in a bath, the mother thread will signal
 * FATHER_WAKE to wakeup the father thread. When the mother thread has
 * completed the task to clean the child, she will signal CHILD_READ each time.
 * Once all children tasks are complete, the mother thread will sleep by
 * waiting for MOTHER_WAKE semaphore.
 * The father thread will wait for FATHER_WAKE to prepare for reading a book to
 * the child. For each child, the father wiil wait for CHILD_READ to read a
 * book to the child. Once every children has completed the tasks, the father
 * thread will signal MOTHER_WAKE to cotinue the next day and sleep by waiting
 * for FATHER_WAKE.
 */
#include <pthread.h>    // posix threads functions
#include <semaphore.h>  // sempahore functions
#include <unistd.h>     // usleep()
#include <cstdlib>      // atoi()
#include <iostream>     // cout
#include <string>       // string class
#include <vector>       // vector class

// GLOGBAL
int MAX_CHILD = 12;
sem_t FATHER_WAKE;
sem_t MOTHER_WAKE;
sem_t CHILD_READ;

void* father(void* args) {
    int cycle = *(int*)args;
    std::vector<std::string> tasks{"is being read a book", "is tucked in bed"};

    for(int day = 0; day < cycle; ++day) {
        sem_wait(&FATHER_WAKE);

        std::cout << "Father is home to help mother" << std::endl;

        for(std::size_t i = 0; i < tasks.size(); ++i) {
            for(int j = 0; j < MAX_CHILD; ++j) {
                // @task reading a book, wait for child to be ready
                if(i == 0) sem_wait(&CHILD_READ);

                std::cout << "\tFather: child #" << j + 1 << " " << tasks[i]
                          << std::endl;
            }
        }

        std::cout << "Father is going to sleep\n\n" << std::endl;

        sem_post(&MOTHER_WAKE);
    }

    pthread_exit(0);
}

void* mother(void* args) {
    int cycle = *(int*)args;
    std::vector<std::string> tasks{"is waking up", "is fed breakfast",
                                   "is sent to school", "has eaten dinner",
                                   "has taken a bath"};

    for(int day = 0; day < cycle; ++day) {
        std::string day_msg = "This is day #" + std::to_string(day + 1) +
                              " in the life of Mother Hubbard";

        sem_wait(&MOTHER_WAKE);

        std::cout << std::string(day_msg.size(), '-') << '\n';
        std::cout << day_msg << '\n';
        std::cout << std::string(day_msg.size(), '-') << std::endl;
        std::cout << "Mother is waking up to take care of the children"
                  << std::endl;

        for(std::size_t i = 0; i < tasks.size(); ++i) {
            for(int j = 0; j < MAX_CHILD; ++j) {
                // @start of 'giving bath', father comes home
                if(i == (tasks.size() - 1) && j == 0) sem_post(&FATHER_WAKE);

                usleep(100);
                std::cout << "\tMother: child #" << j + 1 << " " << tasks[i]
                          << std::endl;

                // @end of 'giving bath', signal father to read book
                if(i == (tasks.size() - 1)) sem_post(&CHILD_READ);
            }
        }

        std::cout << "Mother is going to take a nap break" << std::endl;
    }

    pthread_exit(0);
}

int main(int argc, char* argv[]) {
    int cycle = 5;

    if(argc > 1) cycle = atoi(argv[1]);

    pthread_t tid[2];

    sem_init(&FATHER_WAKE, 0, 0);
    sem_init(&MOTHER_WAKE, 0, 1);
    sem_init(&CHILD_READ, 0, 0);

    pthread_create(&tid[0], NULL, father, &cycle);
    pthread_create(&tid[1], NULL, mother, &cycle);

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    return 0;
}
