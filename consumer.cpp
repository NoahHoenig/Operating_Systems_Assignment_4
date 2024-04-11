/*
 *  Program # 4
 *  Implementation of consumer threads
 *  CS480-1001
 *  4-22-2023
 *  @author  Noah Hoenig
 *  @redID   824412349
 */

#include "consumer.h"
#include "shared.h"

/**
 * @brief Uses Blockchain X or Blockchain Y to consume trade request and complete the
 * transaction for the request.
 * 
 * @param voidPtr           Points to some data that is passed as an argument to the thread method. 
 *                          In this case, it is passed to the consumer thread method as a parameter.
 *                          When the thread is created, the consumerData pointer is cast from 
 *                          void pointer to CONSUMER_DATA pointer type, 
 *                          allowing the consumerData pointer to be used to access the data passed 
 *                          as an argument to the thread. 
 */
void * consumer(void * voidPtr)
{
    CONSUMER_DATA *consumerData = (CONSUMER_DATA*)voidPtr;
    SHARED_DATA *shared = consumerData->shared;
    
    /* Set up delay */
    int DelayMS;
    struct timespec	SleepTime;
    if(consumerData->consumer_type == BlockchainX){
        DelayMS = shared->delay_blockchainX;
    }else{
        DelayMS = shared->delay_blockchainY;
    }
	SleepTime.tv_sec = DelayMS / MSPERSEC;	 /* # secs */
	SleepTime.tv_nsec = (DelayMS % MSPERSEC) * NSPERMS; /* # nanosecs */
    /* Set up delay */

    /* track how many requests were consumed by the thread and what was removed from the broker */
    unsigned int bitConsumedByThisThread = 0, ethConsumedByThisThread = 0, inRequestQueue[ConsumerTypeN] = {Bitcoin};
    /* condition to exit */
    bool flag = true;

    /* Begin consuming trade requests from broker */
    while (flag) {

        /* First block until something to consume */
        sem_wait(&shared->unconsumed);
        
        /* Critical Region */
        /* Access broker exclusively */
        sem_wait(&shared->mutex); 

        /* save the item at the front of the broker (item to be removed) */
        RequestType itemType = shared->broker.front();
        
        /* Check if all the requests have been consumed and the broker is empty */
        /* If so signal the main thread, OS automatically will close all the child threads */
        /* Just to be safe release the lock */
        if(shared->bitConsumed + shared->ethConsumed >= shared->total_num_requests && shared->broker.size() == 0){
            flag = false;
            sem_post(&shared->mutex);
            if(itemType==Bitcoin){
                sem_post(&shared->bitcoinConstraint); 
            }
            sem_post(&shared->availableSlots);
            
            /* Signal the main thread */ 
            sem_post(&shared->mainLock);
            
            break;
        }
        /* Remove from the front of the broker (FIFO) */
        shared->broker.pop();
        
        /* Increment the number of request type consumed by the threads and the number of request type consumed by both consumer threads */
        /* Decrement the number of request type in broker */
        if(itemType==Bitcoin){
            bitConsumedByThisThread++, shared->bitConsumed++, shared->bitInBroker--;
        } else {
            ethConsumedByThisThread++, shared->ethConsumed++, shared->ethInBroker--;
        }

        /* Show that an item has been removed from the request queue 
           and print the current status of the broker request queue. */
        consumerData->consumed[Bitcoin] = bitConsumedByThisThread, consumerData->consumed[Ethereum] = ethConsumedByThisThread;
        inRequestQueue[Bitcoin] = shared->bitInBroker, inRequestQueue[Ethereum] = shared->ethInBroker;
        log_request_removed(consumerData->consumer_type,itemType,consumerData->consumed,inRequestQueue);
        
        sem_post(&shared->mutex);
        /* Release the lock */ 
        /* Critical Region */
        
        /* Inform the producers */
        sem_post(&shared->availableSlots); 
        if(itemType==Bitcoin){
            sem_post(&shared->bitcoinConstraint); 
        }
        
        /* Simulate consuming the item */
        nanosleep(&SleepTime, NULL); 

    }
    
    pthread_exit(NULL);
}