#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
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

// create global semaphore objects

int main(int argc, char* argv[]) {
	// get number of cars from command line
	if(argc < 2) {
		cout << "Please enter a number of cars to simulate" << endl;
		exit(-1);
	}
	int cars = atoi(argv[1]);
	cout << "Beginning simulation with " << cars << " cars" << endl;

	// Create threads for Northbound, Southbound, and worker
	pthread_id worker;
	pthread_id north;
	pthread_id south;
	if(pthread_create(&worker, NULL, worker, NULL)) {
		perror("could not create worker thread");
		exit(-1;)
	}
	if(pthread_create(&north, NULL, carfn, NULL)) {
		perror("could not create northbound thread");
		exit(-1);
	}
	if(pthread_create(&south, NULL, carfn, NULL)) {
		perror("could not create southbound thread");
		exit(-1);
	}
}

// Producer
void *carfn(void *arg) {
		// generate new car using probability model
		// set all necessary fields of the car struct

		// acquire lock for north/south bound car queue
		// add the car to the queue

		// wake up the worker thread

		// release lock for north/south bound car queue
}

// Consumer
void *worker(void *arg) {
	while(1) {
		// check to see if there are any cars to be served with
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

// Log information to carOutput file
void processCar() {

}

bool probabilityModel() {

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
