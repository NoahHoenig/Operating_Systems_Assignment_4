/*
 *  Program # 4
 *  Implementation of producer threads
 *  CS480-1001
 *  4-22-2023
 *  @author  Noah Hoenig
 *  @redID   824412349
 */

#include "producer.h"
#include "shared.h"

/**
 * @brief Accepts (produces) Bitcoin or Ethereum trade requests and publishes them to
 *        the broker.
 * 
 * @param voidPtr           Points to some data that is passed as an argument to the thread method. 
 *                          In this case, it is passed to the producer thread method as a parameter.
 *                          When the thread is created, the producerData pointer is cast from 
 *                          void pointer to PRODUCER_DATA pointer type, 
 *                          allowing the producerData pointer to be used to access the data passed 
 *                          as an argument to the thread. 
 */
void * producer(void * voidPtr)
{
    PRODUCER_DATA *producerData = (PRODUCER_DATA*)voidPtr;
    SHARED_DATA *shared = producerData->shared;
    
    /* Set up delay */
    int DelayMS;
    struct timespec	SleepTime;
    if(producerData->crypto_type == Bitcoin){
        DelayMS = shared->delay_bitcoin;
    }else{
        DelayMS = shared->delay_ethereum;
    }
	SleepTime.tv_sec = DelayMS / MSPERSEC;	 /* # secs */
	SleepTime.tv_nsec = (DelayMS % MSPERSEC) * NSPERMS; /* # nanosecs */
    /* Set up delay */

    /* track how many requests were produced by the thread and what was inserted to the broker */
    unsigned int produced[RequestTypeN] = {Bitcoin}, inRequestQueue[RequestTypeN] = {Bitcoin};
    /* condition to exit */
    bool flag = true;
    
    /* Begin production of trade requests */
    while(flag) {
        
        /* First simulate production of the item */
        nanosleep(&SleepTime, NULL);
        
        /* make sure to wait for bitcoin to be removed from broker if limit is reached */
        if(producerData->crypto_type == Bitcoin){
            sem_wait(&shared->bitcoinConstraint);
        } 
        
        /* make sure there is room in the broker */
        sem_wait(&shared->availableSlots); 
        
        /* Critical Region */
        /* Access broker exclusively */
        sem_wait(&shared->mutex); 
        
        /* Check if production limit has been reached for both producer threads */
        /* If so don't produce a request, release the lock, signal consumer, exit thread */
        if(shared->bitProduced + shared->ethProduced >= shared->total_num_requests){
            flag = false;
            sem_post(&shared->mutex); 
            sem_post(&shared->unconsumed);
            break;
        }
        
        /* Insert the request type into the broker */
        shared->broker.push(producerData->crypto_type);
        
        /* Increment the number of request type produced and in broker for data shared between producers and consumers */
        if(producerData->crypto_type==Bitcoin){
            shared->bitProduced++, shared->bitInBroker++;
        }else{
            shared->ethProduced++, shared->ethInBroker++;
        }

        /* Show that a request has been added to the request queue 
        and print the current status of the broker request queue. */
        produced[Bitcoin] = shared->bitProduced, produced[Ethereum] = shared->ethProduced;
        inRequestQueue[Bitcoin] = shared->bitInBroker, inRequestQueue[Ethereum] = shared->ethInBroker;
        log_request_added(producerData->crypto_type,produced,inRequestQueue);
        
        sem_post(&shared->mutex);
        /* Release the lock */ 
        /* Critical Region */

        /* Inform consumer there is a request ready to be consumed */
        sem_post(&shared->unconsumed); 

    }

    pthread_exit(NULL);
}