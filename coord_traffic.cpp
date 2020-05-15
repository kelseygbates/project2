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
using namespace std;

// Global file output objects
const string carOutput = "car.log";
const string workerOutput = "flagperson.log";
ofstream carLog("car.log", ofstream::out);
ofstream flagPerson("flagperson.log", ofstream::out);

// Car structure to hold specific data about each car
struct car {
		int carId;
		char direction;
		string arrive;
		string start;
		string end;
};

// create global semaphore objects
sem_t moreCars; // how worker knows that there are cars in queues

//Initialize mutex
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


// Car consumer thread function
// Logs to output file
void *consume(void *args){
	car* currCar = (car*)args;
	carLog << currCar->carId << "        " << currCar->direction << "           " << currCar->arrive << "        "
	       << currCar->start << "       " << currCar->end << "\n";
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

		pthread_mutex_lock(&queueMutex); // Enter critical section

		// Remove the next car from the appropriate queue
		if (northC.size() >= 1 && southC.size() < 10) {
			currCar = northC.front();
			northC.pop();
		} else if (southC.size() >= 1 && northC.size() < 10) {
			currCar = southC.front();
			southC.pop();
		}

		// Assign start time, allow 2 seconds to cross, then assign end time
		currCar->start = stringTime();
		pthread_sleep(2);
		currCar->end = stringTime();

		// Create and detach car thread, passing in current car
		pthread_t carThread;
		if(pthread_create(&carThread, NULL, &consume, (void*)currCar)) {
			perror("Worker couldn't create car thread");
			exit(-1);
		}
		pthread_detach(carThread);
		if (currCar->carId >= limitCars) { // Check if we have reached the user's desired number of cars
			pthread_exit(NULL);
		}
		pthread_mutex_unlock(&queueMutex); // Exit critical section

		flagPerson.flush();
	}
}


//Southbound Car producer
//  - produces and enqueues cars
void *produceSouth(void *args)
{
	while(1)
	{
		pthread_mutex_lock(&queueMutex); // Enter critical section

		producedCars++; // Increase number of cars produced
		if (producedCars > limitCars) { // Check to see if we have produced user's desired number of cars
			pthread_mutex_unlock(&queueMutex);
			pthread_exit(NULL);
		}

		// Create new car object and assign data
		car* newCar = new car();
		newCar->carId = producedCars; // set ID to car count
		string s = stringTime();
		newCar->arrive = s;
		newCar->direction = 'S';

		// Add to southbound queue
		southC.push(newCar);

		pthread_mutex_unlock(&queueMutex); // Exit critical section

		sem_post(&moreCars); // Wake up worker thread
		if (!probabilityModel()){ // Check for another car coming
			pthread_sleep(20);
		}
	}
}

//Northbound Car producer
// - produces and enqueues cars
void *produceNorth(void *args)
{
	while(1)
	{
		pthread_mutex_lock(&queueMutex); // Enter critical section

		producedCars++; // Increase number of cars to produce
		if (producedCars > limitCars) { // Check to see if we have produced user's desired number of cars
			pthread_mutex_unlock(&queueMutex);
			pthread_exit(NULL);
		}

		// Create new car and assign data
		car* newCar = new car();
		newCar->carId = producedCars;
		string s = stringTime();
		newCar->arrive = s;
		newCar->direction = 'N';

		// Add to northbound queue
		northC.push(newCar);

		pthread_mutex_unlock(&queueMutex); // Exit critical section

		sem_post(&moreCars); // Wake up worker
		if(!probabilityModel()){ // Check for another car coming
			pthread_sleep(20);
		}
	}
}

// Formats current time object into a string HH:MM:SS
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

	int cars = atoi(argv[1]);
	limitCars = cars; // set global limit on number of cars to command line number
	cout << endl <<  "Beginning simulation with " << cars << " cars" << endl << endl << ". . ." << endl;

	// Prep output files
	carLog << "carID    direction   arrival-time    start-time     end-time\n";
	carLog.flush();
	flagPerson << "Time         State\n";
	flagPerson.flush();

	// Initialize global semaphore object
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

	// Wait on worker thread to terminate
	if(pthread_join(workerTh, NULL)) {
		perror("could not join north thread");
		exit(-1);
	}

	carLog.close();
	flagPerson.close();

	// Prevent memory leaks
	sem_destroy(&moreCars);
	pthread_mutex_destroy(&queueMutex);

	cout << endl << "Simulation complete. Please see the files 'car.log' and 'flagperson.log'" << endl << endl;
	return 0;
}