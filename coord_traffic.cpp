#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <queue>
#include <signal.h>
#include <time.h>
#include <string>
using namespace std;

const string carOutput = "car.log";
const string workerOutput = "flagperson.log";

struct car {
    int carId;
    char direction;
    time_t arrive;
    time_t start;
    time_t end;
};

//Initialize mutex
pthread_mutex_t flagMutex;

//Current direction of the flagPerson
string fDirection = "north";

// create global semaphore objects
sem_t carSem;

int count = 0; //# of cars created

// Ready queues for north and south cars
queue<car> northC;
queue<car> southC;

//worker thread -> should flag person be awake?
void *worker(void *arg) {
    while(1) {
        // check to see if there are any cars to be served with
        while (!northC.empty() && !southC.empty())
        {

        }
        // call to sem_wait()?

        // acquire lock for north/south bound car queue
        // pthread_mutex_lock(&north_queue_mutex)
        // pthread_mutex_lock(&south_queue_mutex)

        // remove the next car from the queue
        // create car thread and detach
        // use pthread_sleep to simulate 2 seconds
        // call to processCar()
        // check if there are 10 or more cars waiting on the other side

        // release the lock for north/south bound car queue
        // pthread_mutex_unlock(&north_queue_mutex)
        // pthread_mutex_unlock(&south_queue_mutex)

    }
}

void processCar() {
    // pop cars off of threads depending on direction of worker

    // log car info in car.log
}

//True if car following, false if no car comes
bool probabilityModel() {
    return (random() * 10 < 8);
}

//Car consumer
void *consume(void *args)
{
    while(1)
    {
        // lock flag person
        pthread_mutex_lock(&flagMutex);

        // check sizes of queues
        // switch direction empty or 10+ cars on opp. side
        //


    }
}

// Included from assignment
int pthread_sleep(int seconds)
{
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;
    if(pthread_mutex_init(&mutex,NULL))
    {
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL))
    {
        return -1;
    }
    //When to expire is an absolute time, so get the current time and add
    //it to our delay time
    timetoexpire.tv_sec = (unsigned int)time(NULL) + seconds;
    timetoexpire.tv_nsec = 0;
    return pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
}

//Southbound Car producer
void *produceSouth(void *args)
{
    time_t arrival; // need to set arrival time
    struct car newCar;

    while(1)
    {
        sem_wait(&carSem);
        pthread_mutex_lock(&flagMutex); // acquire lock for north/south bound car queue

        //generate a new car using probability model
        while(probabilityModel()) { //how do we make this change??
            count++; //increment car counter
            newCar.carId = count; // set ID to car count
            newCar.arrive = arrival;
            newCar.direction = 'S';
            southC.push(newCar); // add car to the queue
            pthread_sleep(2); // not sure if this is where the 2 second wait should go
        }
        // no car comes, 20 second delay
        pthread_sleep(20);
        pthread_mutex_unlock(&flagMutex); // wake up worker thread

        sem_post(&carSem);
    }
    return 0;
}
//Northbound Car producer
void *produceNorth(void *args)
{
    time_t arrival; // need to set arrival time
    struct car newCar;

    while(1)
    {
        sem_wait(&carSem);
        pthread_mutex_lock(&flagMutex);

        while(probabilityModel()) {
            count++;
            newCar.carId = count;
            newCar.arrive = arrival;
            newCar.direction = 'N';
            northC.push(newCar);
            pthread_sleep(2);
        }

        //no car comes, 20 second delay
        pthread_sleep(20);
        pthread_mutex_unlock(&flagMutex);

        sem_post(&carSem);
    }
    return 0;
}

int main(int argc, char* argv[]) {
    // get number of cars from command line
    if(argc < 2) {
        cout << "Please enter a number of cars to simulate" << endl;
        exit(-1);
    }
    int cars = atoi(argv[1]);
    cout << "Beginning simulation with " << cars << " cars" << endl;

    // Create threads for Northbound, Southbound, and worker
    pthread_t worker;
    pthread_t north;
    pthread_t south;
    if(pthread_create(&worker, NULL, consume, NULL)) {
        perror("could not create worker thread");
        exit(-1);
    }
    if(pthread_create(&north, NULL, produceNorth, NULL)) {
        perror("could not create northbound thread");
        exit(-1);
    }
    if(pthread_create(&south, NULL, produceSouth, NULL)) {
        perror("could not create southbound thread");
        exit(-1);
    }
}

