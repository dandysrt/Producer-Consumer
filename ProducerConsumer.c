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
int producer();
int consumer();


int main(int argc, char *argv[]){
    srand(time(NULL));
    int p_count = 1, c_count = DEFAULT;
    int arg;
    if(argc > 1){
        while((arg = getopt(argc, argv, "p:c:")) != -1){
            switch(arg){
                case 'p':
                    p_count = atoi(optarg);
                break;
                case 'c':
                    c_count = atoi(optarg);
                break;
                case '?':
                    abort();
                default:
                break;
            }
        }
    }
    producers = p_count;
    consumers = c_count;
    pthread_mutex_lock(&c_mutex);
    printf("Producer(s): %d\n", p_count);
    printf("Consumer(s): %d\n", c_count);
    pthread_t *ptID = (pthread_t *) malloc(sizeof(pthread_t) * p_count);
    pthread_t *ctID = (pthread_t *) malloc(sizeof(pthread_t) * c_count);
   for(int p = 0; p < p_count; p++){
        if(pthread_create(ptID+p, NULL, (void *) &producer, NULL) != 0){
            perror("producer thread");
        }
    }

    for(int c = 0; c < c_count; c++){
        if(pthread_create(ctID+c, NULL, (void *) &consumer, NULL) != 0){
            perror("consumer thread");
        }
    }

    for(int i = 0; i < p_count; i++){
        pthread_join(ptID[i], NULL);
    }
    for(int j = 0; j < c_count; j++){
        pthread_join(ctID[j], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_2);
    pthread_mutex_destroy(&c_mutex);
    return 0;
}

int producer(){
    int produce = 1 + ((double)rand() / (double) RAND_MAX) * 20;
    pthread_mutex_lock(&mutex);
        producers--;
        product += produce;
        printf("Producer has stocked %d product.\n", produce);
        if(need > 0 && product > need){
            pthread_mutex_unlock(&c_mutex);
        }else{
            pthread_mutex_unlock(&mutex);
        }
    return 0;
}

int consumer(){
    int take = ((double)rand() / (double) RAND_MAX) * 20;
    pthread_mutex_lock(&mutex_2);
        printf("DEBUG CONSUMER COUNT: %d\n", consumers);
        printf("DEBUG PRODUCER COUNT: %d\n", producers);
        printf("TOTAL PRODUCT: %d\n", product);
            if(producers == 0){// attempt to eliminate deadlocking issue
                // wait and adjust consumption until it falls within tolerances
                producers--;
                while((take = ((double)rand() / (double) RAND_MAX) * 20) > product);
                pthread_mutex_unlock(&mutex);
            }
        pthread_mutex_lock(&mutex);
            if(product >= take){
                product -= take;
                printf("Consumer has removed %d product.\n", take);
                consumers--;
                pthread_mutex_unlock(&mutex);
                pthread_mutex_unlock(&mutex_2);
            }else{
                need = take;
                pthread_mutex_unlock(&mutex);
                pthread_mutex_lock(&c_mutex);
                    product -= take;
                    need = 0;
                    printf("Consumer has removed %d product.\n", take);
                    consumers--;
                pthread_mutex_unlock(&mutex);
                pthread_mutex_unlock(&mutex_2);
            }
    return 0;
}

