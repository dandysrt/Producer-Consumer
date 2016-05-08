#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#define DEFAULT 5
#define ITR_COUNT 2

// global variables
int product = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t c_mutex = PTHREAD_MUTEX_INITIALIZER;
int need = 0;
int producers, consumers;

// prototypes:
void producer();
void consumer();


int main(int argc, char *argv[]){
    srand(time(NULL));
    int p_count = 1, c_count = DEFAULT;
    int arg;
    if(argc > 1){
        while((arg = getopt(argc, argv, "p:c:")) != -1){
            switch(arg){
                case 'p':
                    p_count = atoi(optarg); // assign number of producers
                break;
                case 'c':
                    c_count = atoi(optarg); // assign number of consumers
                break;
                case '?':   // incorrect arguments
                    abort();
                default:
                break;
            }
        }
    }

    // assign values to counters
    producers = p_count;
    consumers = c_count;
    pthread_mutex_lock(&c_mutex);
    printf("Producer(s): %d\n", p_count);
    printf("Consumer(s): %d\n", c_count);
    pthread_t *ptID = (pthread_t *) malloc(sizeof(pthread_t) * p_count); // producer thread ID memory allocation
    pthread_t *ctID = (pthread_t *) malloc(sizeof(pthread_t) * c_count); // consumer thread ID memory allocation

    // initialize producer threads
    for(int p = 0; p < p_count; p++){
        if(pthread_create(ptID+p, NULL, (void *) &producer, NULL) != 0){
            perror("producer thread");
        }
    }

    // initialize consumer threads
    for(int c = 0; c < c_count; c++){
        if(pthread_create(ctID+c, NULL, (void *) &consumer, NULL) != 0){
            perror("consumer thread");
        }
    }

    // clean up clean up
    for(int i = 0; i < p_count; i++){
        pthread_join(ptID[i], NULL);
    }
    for(int j = 0; j < c_count; j++){
        pthread_join(ctID[j], NULL);
    }

    // everybody everywhere
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_2);
    pthread_mutex_destroy(&c_mutex);

    // clean up clean up everybody do your share
    free(ptID);
    free(ctID);
    return 0;
}

void producer(){
    int produce = 1 + ((double)rand() / (double) RAND_MAX) * 20;
    printf("producer: mutex lock\n");
    pthread_mutex_lock(&mutex);
        producers--;
        product += produce;
        printf("Producer has stocked %d product.\n", produce);
        if(need > 0 && product > need){
            pthread_mutex_unlock(&c_mutex);
            printf("producer: c_mutex unlock\n");
        }else{
            pthread_mutex_unlock(&mutex);
            printf("producer: mutex unlock\n");
        }
}

void consumer(){
    int take = ((double)rand() / (double) RAND_MAX) * 20;
    printf("consumer: mutex_2 lock\n");
    pthread_mutex_lock(&mutex_2);
        printf("DEBUG CONSUMER COUNT: %d\n", consumers);
        printf("DEBUG PRODUCER COUNT: %d\n", producers);
        printf("TOTAL PRODUCT: %d\n", product);

            // deadlock protection
            if(producers <= 0){

                // wait and adjust consumption until it falls within tolerances
                while((take = ((double)rand() / (double) RAND_MAX) * 20) > product);

                if(producers == 0){// first incidence of producer absence
                    printf("deadlock protection: mutex unlock\n");
                    pthread_mutex_unlock(&mutex);
                    producers--;
                }
            }
        printf("consumer: mutex lock\n");
        pthread_mutex_lock(&mutex);
            if(product >= take){
                product -= take;
                printf("Consumer has removed %d product.\n", take);
                consumers--;
                pthread_mutex_unlock(&mutex);
                pthread_mutex_unlock(&mutex_2);
                printf("consumer: mutex unlock\n");
            }else{
                need = take;
                pthread_mutex_unlock(&mutex);
                printf("need: c_mutex lock\n");
                pthread_mutex_lock(&c_mutex);
                    product -= take;
                    need = 0;
                    printf("Consumer has removed %d product.\n", take);
                    consumers--;
                pthread_mutex_unlock(&mutex);
                pthread_mutex_unlock(&mutex_2);
                printf("need: mutex unlock\n");
            }
}

