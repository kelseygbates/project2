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
}

void *worker(void *arg) {
	while(1) {
		// check to see if there are any cars to be served with
		// call to sem_wait()?

		// check if there are 10 or more cars waiting on the other side

		// create car thread and detach
		// use pthread_sleep to simulate 2 seconds
		// log information to carOutput file

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
