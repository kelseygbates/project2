#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <queue>
#include <signal.h>
#include <time.h>
#include <cstring>
#include <string>
#include <fstream>
#include <cassert>
using namespace std;

// Global file output objects
const string carOutput = "car.log";
const string workerOutput = "flagperson.log";
ofstream carLog("car.log", ofstream::out);
ofstream flagPerson("flagperson.log", ofstream::out);

// Car structure to ease output
struct car {
		int carId;
		char direction;
		string arrive;
		string start;
		string end;
};

//Current direction of the flagPerson
string fDirection = "north";

// create global semaphore objects
sem_t moreCars; // how worker knows that there are cars in queues

//Initialize mutex
pthread_mutex_t countMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;

int producedCars = 0; // # of cars created so far
int limitCars = 0; // # cars input from command line

// Ready queues for north and south cars
queue<car*> northC;
queue<car*> southC;

// Helper function for formatting current time correctly
string stringTime();

// True if car following, false if no car comes
bool probabilityModel() {
	return ((rand() % 10 + 1) < 8);
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


//Car consumer
void *consume(void *args){
	car* currCar = (car*)args;

	carLog << currCar->carId << "   " << currCar->direction << "    " << currCar->arrive << "   " << currCar->start
				 << "     " << currCar->end << "\n";
	carLog.flush();
	pthread_exit(NULL);
}

void *worker(void *arg) {
	car* currCar;
	while(1) {
        flagPerson << stringTime() << "     " << "sleep\n";
		sem_wait(&moreCars);
		flagPerson << stringTime() << "     " << "woken-up\n";
		flagPerson.flush();


		// log time + status flag person wakes up

		pthread_mutex_lock(&queueMutex); // acquire lock for job queue
		pthread_mutex_lock(&countMutex);

		// Remove the next car from the appropriate queue
		if (northC.size() >= 1 && southC.size() < 10) { // immediately set direction to north if
			fDirection = "north";                      // north car gets there first (need to reset direction)
			currCar = northC.front();
			northC.pop();
		} else if (southC.size() >= 1 && northC.size() < 10) {
			fDirection = "south";
			currCar = southC.front();
			southC.pop();
		}
		currCar->start = stringTime();
		pthread_sleep(2);
		currCar->end = stringTime();

		pthread_t carThread;
		assert(currCar);
		if(pthread_create(&carThread, NULL, &consume, (void*)currCar)) {
			perror("Worker couldn't create car thread");
			exit(-1);
		} else {
			cout << "Worker: successfully created car thread" << endl;
		}
		pthread_detach(carThread);
		if (currCar->carId >= limitCars) { // Means we are done
			cout << "Done" << endl;
			pthread_exit(NULL);
		}
		pthread_mutex_unlock(&countMutex);
		pthread_mutex_unlock(&queueMutex);

		// log time + status flag person going to sleep
        flagPerson.flush();

	}
}


//Southbound Car producer
//  - produces and enqueues cars
void *produceSouth(void *args)
{
	//struct car newCar;
	while(1)
	{
		pthread_mutex_lock(&queueMutex);
		pthread_mutex_lock(&countMutex);
		producedCars++;
		cout << "Car count(South): " << producedCars << endl;
		if (producedCars > limitCars) {
			cout << "Done producing cars South!" << endl;
			pthread_mutex_unlock(&queueMutex);
			pthread_mutex_unlock(&countMutex);
			pthread_exit(NULL);
		}
		car* newCar = new car();
		newCar->carId = producedCars; // set ID to car count

		string s = stringTime();
		newCar->arrive = s;
		newCar->direction = 'S';
		assert(newCar);

		southC.push(newCar);

		pthread_mutex_unlock(&countMutex);
		pthread_mutex_unlock(&queueMutex); // unlock after sleep?

		sem_post(&moreCars);
		if (!probabilityModel()){
			cout << "Southbound waiting 20 sec" << endl;
			pthread_sleep(20);
		}

	}
	return 0;
}
//Northbound Car producer
void *produceNorth(void *args)
{
	while(1)
	{
		pthread_mutex_lock(&queueMutex);
		pthread_mutex_lock(&countMutex);
		producedCars++;
		cout << "Car count(North): " << producedCars << endl;
		if (producedCars > limitCars) {
			cout << "Done producing North! " << endl;
			pthread_mutex_unlock(&queueMutex);
			pthread_mutex_unlock(&countMutex);
			pthread_exit(NULL);
		}
		car* newCar = new car();
		newCar->carId = producedCars;

		string s = stringTime();
		newCar->arrive = s;

		newCar->direction = 'N';
		assert(newCar);
		northC.push(newCar);

		pthread_mutex_unlock(&countMutex);
		pthread_mutex_unlock(&queueMutex);

		sem_post(&moreCars);
		//no car comes, 20 second delay
		if(!probabilityModel()){
			cout << "North sleeping 20" << endl;
			pthread_sleep(20);
		}
	}
}

string stringTime() {
	time_t currTime;
	tm * tmInfo;
	currTime = time(0);
	char buffer[800];
	tmInfo = localtime(&currTime);
	strftime(buffer,800,"%T",tmInfo);
	string s(buffer);
	return s;
}

int main(int argc, char* argv[]) {
	// get number of cars from command line
	if(argc < 2) {
		cout << "Please enter a number of cars to simulate" << endl;
		exit(-1);
	}

	string time = stringTime();
	cout << time << endl;
	int cars = atoi(argv[1]);
	limitCars = cars; // set global limit on number of cars to command line number
	cout << "Beginning simulation with " << cars << " cars" << endl;
	carLog << "carID    direction   arrival-time    start-time  end-time\n";
	//carLog.close();
	carLog.flush();
	flagPerson << "Time     State\n";
	flagPerson.flush();

	sem_init(&moreCars, 0,0);


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
	if(pthread_join(workerTh, NULL)) {
		perror("could not join north thread");
		exit(-1);
	}
	return 0;
}