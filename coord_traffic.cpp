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
#include <fstream>
using namespace std;

const string carOutput = "car.log";
const string workerOutput = "flagperson.log";

struct car {
    int carId;
    char direction;
    string arrive;
    string start;
    string end;
};

//Initialize mutex
//pthread_mutex_t flagMutex;

//Current direction of the flagPerson
string fDirection = "north";

// create global semaphore objects
sem_t moreCars; // how worker knows that there are cars in queues

pthread_mutex_t countMutex;
int producedCars = 0; //# of cars created
int limitCars = 0; // # cars input from command line

// need semaphores for N and S queues???
//      - southProducer pushes values while consumer pops...
// Ready queues for north and south cars

pthread_mutex_t queueMutex;
queue<car> northC;
queue<car> southC;

ofstream carLog("car.log", ofstream::out);

//worker thread -> should flag person be awake?
//worker pops cars off threads, then sends to consume() thread?

string stringTime();

//True if car following, false if no car comes
bool probabilityModel() {
    return (random() * 10 < 8);
}

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


//Car consumer
// - how does car consumer access
void *consume(void *args){
        pthread_mutex_lock(&queueMutex); // unlock flag person as car goes by
        pthread_sleep(2);
        car * currCar = (car*) args;
        currCar->end = stringTime();
        cout << "carId: " << currCar->carId << endl;
        cout << "direction: " << currCar->direction << endl;
        cout << "arrive: " << currCar->arrive << endl;
        cout << "start: " << currCar->start << endl;
        cout << "direction: " << currCar->end << endl;
        carLog << currCar->carId << "   " << currCar->direction << "    " << currCar->arrive << "   " << currCar->start
               << "     " << currCar->end << "\n";
        carLog.flush();

        pthread_mutex_unlock(&queueMutex);
        pthread_exit(NULL);
}

void *worker(void *arg) {
    cout << "about to start worker" << endl;
    queue<car> currQueue;
    car currCar;
    while(1) {

        sem_wait(&moreCars);

        // log time + status flag person wakes up

        pthread_mutex_lock(&queueMutex);
        if (northC.size() == 1 && southC.size() == 0) { // immediately set direction to north if
            fDirection = "north";                      // north car gets there first (need to reset direction)
            currQueue = northC;
        } else if (southC.size() == 1 && northC.size() == 0){
            fDirection = "south";
            currQueue = southC;
        } else if (northC.size() >= 10 && southC.size() < 10) {
            fDirection = "north";
            currQueue = northC;
        } else if (southC.size() >= 10 && northC.size() < 10) {
            fDirection = "south";
            currQueue = southC;
        }


        currCar = currQueue.front();
        currQueue.pop();
        // log car start-time
        currCar.start = stringTime();


        pthread_t carThread;
        cout << "about to create car thread from worker" << endl;
        pthread_create(&carThread, NULL, &consume, (void*)&currCar);
        pthread_detach(carThread);
        if (producedCars == limitCars) {
            pthread_mutex_unlock(&queueMutex);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&queueMutex);

        // log time + status flag person going to sleep

    }
}


//Southbound Car producer
//  - enqueues cars
void *produceSouth(void *args)
{
    //time_t arrival; // need to set arrival time
    struct car newCar;
    cout << "about to start produceSouth" << endl;

    while(1)
    {
        pthread_mutex_lock(&queueMutex);
        pthread_mutex_lock(&countMutex);
        producedCars++;
        if (producedCars > limitCars) {
            pthread_exit(NULL);
        }
        newCar.carId = producedCars; // set ID to car count
        pthread_mutex_unlock(&countMutex);

        newCar.arrive = stringTime();

        newCar.direction = 'S';
        southC.push(newCar);

        pthread_mutex_unlock(&queueMutex); // unlock after sleep?
        cout << "about to send signal to worker from produceSouth " << endl;
        sem_post(&moreCars);
        if (!probabilityModel()){
            pthread_sleep(20);
        }


    }
    return 0;
}
//Northbound Car producer
void *produceNorth(void *args)
{
     // need to set arrival time

    struct car newCar;

    cout << "about to start produceNorth" << endl;

    while(1)
    {

        pthread_mutex_lock(&queueMutex);
        pthread_mutex_lock(&countMutex);
        producedCars++;
        if (producedCars > limitCars) {
            pthread_exit(NULL);
        }
        newCar.carId = producedCars;
        pthread_mutex_unlock(&countMutex);

        // log car arrival-time
        //newCar.arrive = arrival;
        newCar.arrive = stringTime();

        //newCar

        newCar.direction = 'N';
        northC.push(newCar);
        pthread_mutex_unlock(&queueMutex);
        cout << "about to send signal to worker from produceNorth " << endl;
        sem_post(&moreCars);

        //no car comes, 20 second delay
        if(!probabilityModel()){
            pthread_sleep(20);
        }

    }
    //return 0;
}

string stringTime() {
    time_t currTime = time(NULL);
    struct tm* timeInfo;
    char buffer[80];
    timeInfo = localtime(&currTime);
    strftime (buffer,80,"now it's %T",timeInfo);
    string s(buffer);
    return s;
    //puts(buffer);
}

int main(int argc, char* argv[]) {
    // get number of cars from command line
    if(argc < 2) {
        cout << "Please enter a number of cars to simulate" << endl;
        exit(-1);
    }
    int cars = atoi(argv[1]);
    limitCars = cars; // set global limit on number of cars to command line number
    cout << "Beginning simulation with " << cars << " cars" << endl;
    //ofstream carLog ("car.log");
    carLog << "carID    direction   arrival-time    start-time  end-time\n";
    //carLog.close();


    // Create threads for Northbound, Southbound, and worker
    pthread_t workerTh;
    pthread_t north;
    pthread_t south;
    if(pthread_create(&north, NULL, &produceNorth, NULL)) {
        perror("could not create northbound thread");
        exit(-1);
    }
    if(pthread_create(&south, NULL, &produceSouth, NULL)) {
        perror("could not create southbound thread");
        exit(-1);
    }
    if(pthread_create(&workerTh, NULL, &worker, NULL)) {
        perror("could not create worker thread");
        exit(-1);
    }
    return 0;
}

