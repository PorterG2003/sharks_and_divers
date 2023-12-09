#define _DEFAULT_SOURCE
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>


// total number of sharks and divers
const int SHARK_COUNT = 6;
const int DIVER_COUNT = 2;

// capacity of the reef (this many sharks OR divers)
const int MAX_IN_REEF = 2;

// max time a shark waits before getting hungry (in microseconds)
const int SHARK_WAITING_TIME = 2000000;

// max time a shark spends feeding in the reef
const int SHARK_FISHING_TIME = 2000000;

// max time a diver waits before wanting to fish
const int DIVER_WAITING_TIME = 2000000;

// max time a diver spends fishing in the reef
const int DIVER_FISHING_TIME = 2000000;

// total time the simulation should run (in seconds)
const int TOTAL_SECONDS = 60;

// whether or not each shark/diver is currently in the reef
bool *divers_fishing;
bool *sharks_feeding;

//
// declare synchronization variables here
//

// create a semaphore
sem_t mutex;
int s = sem_init(&mutex, 0, 1);
assert(s == 0);

sem_t reef;
int s = sem_init(&reef, 0, 1);
assert(s == 0);

sem_t turnstile;
int s = sem_init(&turnstile, 0, 1);
assert(s == 0);

sem_t Q_s;
int s = sem_init(&Q_s, 0, 1);
assert(s == 0);

sem_t Q_d;
int s = sem_init(&Q_d, 0, 1);
assert(s == 0);

int divers = 0;
int sharks = 0;


//
// end synchronization variables
//

void report(void) {
    // shark report
    int total_sharks = 0;
    char shark_report[100];
    shark_report[0] = 0;

    for (int i = 0; i < SHARK_COUNT; i++) {
        if (sharks_feeding[i]) {
            strcat(shark_report, "+");
            total_sharks++;
        } else {
            strcat(shark_report, " ");
        }
    }

    // diver report
    int total_divers = 0;
    char diver_report[100];
    diver_report[0] = 0;

    for (int i = 0; i < DIVER_COUNT; i++) {
        if (divers_fishing[i]) {
            strcat(diver_report, "*");
            total_divers++;
        } else {
            strcat(diver_report, " ");
        }
    }

    // reef report
    char reef_report[100];
    reef_report[0] = 0;
    for (int i = 0; i < total_sharks; i++)
        strcat(reef_report, "+");
    for (int i = 0; i < total_divers; i++)
        strcat(reef_report, "*");
    for (int i = strlen(reef_report); i < MAX_IN_REEF; i++)
        strcat(reef_report, " ");

    printf("[%s] %s [%s]\n", shark_report, reef_report, diver_report);
    if (total_sharks > 0 && total_divers > 0)
        printf("!!! ERROR: diver getting eaten\n");

    fflush(stdout);
}


// function to simulate a single shark
void *shark(void *arg) {
    int k = *(int *) arg;
    printf("starting shark #%d\n", k);
    fflush(stdout);

    for (;;) {
        // sleep for some time
        usleep(random() % SHARK_WAITING_TIME);

        //
        // write code here to safely start the shark feeding
        // note: call report() after setting sharks_feeding[k] to true
        //

        s = sem_wait(&Q_s);
        assert(s == 0);
        s = sem_wait(&turnstile);
        assert(s == 0);
        s = sem_wait(&mutex);
        assert(s == 0);

        sharks++;
        s = sem_post(&Q_s);
        assert(s == 0);
        
        if (sharks == 1) {
            s = sem_wait(&reef);
            assert(s == 0);
        } else if (sharks > 1) {
            s = sem_wait(&Q_s);
            assert(s == 0);
        }

        s = sem_post(&mutex);
        assert(s == 0);
        s = sem_post(&turnstile);
        assert(s == 0);

        //
        // end code to start shark feeding
        //

        // feed for a while
        usleep(random() % SHARK_FISHING_TIME);

        //
        // write code here to stop the shark feeding
        // note: call report() after setting sharks_feeding[k] to false
        //

        s = sem_wait(&mutex);
        assert(s == 0);
        
        sharks--;

        if (sharks == 0) {
            s = sem_post(&reef);
            assert(s == 0);
        } else if (sharks > 0) {
            s = sem_post(&Q_s);
            assert(s == 0);
        }

        s = sem_post(&mutex);
        assert(s == 0);


        //
        // end code to stop shark feeding
        //
    }

    return NULL;
}

// function to simulate a single diver
void *diver(void *arg) {
    int k = *(int *) arg;
    printf("starting diver #%d\n", k);
    fflush(stdout);

    for (;;) {
        // sleep for some time
        usleep(random() % DIVER_WAITING_TIME);

        //
        // write code here to safely start the diver fishing
        // note: call report() after setting divers_fishing[k] to true
        //

        s = sem_wait(&Q_d);
        assert(s == 0);
        s = sem_wait(&turnstile);
        assert(s == 0);
        s = sem_wait(&mutex);
        assert(s == 0);

        divers++;
        s = sem_post(&Q_d);
        assert(s == 0);
        
        if (divers == 1) {
            s = sem_wait(&reef);
            assert(s == 0);
        } else if (divers == 2) {
            s = sem_wait(&Q_d);
            assert(s == 0);
        }

        s = sem_post(&mutex);
        assert(s == 0);
        s = sem_post(&turnstile);
        assert(s == 0);

        //
        // end code to start diver fishing
        //

        // fish for a while
        usleep(random() % DIVER_FISHING_TIME);

        //
        // write code here to stop the diver feeding
        // note: call report() after setting divers_fishing[k] to false
        //

        s = sem_wait(&mutex);
        assert(s == 0);
        
        divers--;

        if (divers == 0) {
            s = sem_post(&reef);
            assert(s == 0);
        } else if (divers == 1) {
            s = sem_post(&Q_d);
            assert(s == 0);
        }

        s = sem_post(&mutex);
        assert(s == 0);

        //
        // end code to stop diver fishing
        //
    }

    return NULL;
}

int main(void) {
    //
    // initialize synchronization variables here
    //


    //
    // end of synchronization variable initialization
    //

    // initialize shared state
    sharks_feeding = malloc(sizeof(bool) * SHARK_COUNT);
    assert(sharks_feeding != NULL);
    for (int i = 0; i < SHARK_COUNT; i++)
        sharks_feeding[i] = false;

    divers_fishing = malloc(sizeof(bool) * SHARK_COUNT);
    assert(divers_fishing != NULL);
    for (int i = 0; i < DIVER_COUNT; i++)
        divers_fishing[i] = false;

    pthread_t sharks[SHARK_COUNT];
    pthread_t divers[DIVER_COUNT];

    // spawn the sharks
    int shark_counts[SHARK_COUNT];
    for (int i = 0; i < SHARK_COUNT; i++) {
        // create a new thread for this shark
        shark_counts[i] = i;
        int s = pthread_create(&sharks[i], NULL, shark, &shark_counts[i]);
        assert(s == 0);
        s = pthread_detach(sharks[i]);
        assert(s == 0);
    }

    // spawn the divers
    int diver_counts[DIVER_COUNT];
    for (int i = 0; i < DIVER_COUNT; i++) {
        // create a new thread for this diver
        diver_counts[i] = i;
        int s = pthread_create(&divers[i], NULL, diver, &diver_counts[i]);
        assert(s == 0);
        s = pthread_detach(divers[i]);
        assert(s == 0);
    }

    // let the simulation run for a while
    sleep(TOTAL_SECONDS);
    fflush(stdout);

    return 0;
}
