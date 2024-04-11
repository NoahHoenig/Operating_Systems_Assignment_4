/*
 *  Program # 4
 *  Shared constants, semaphores, and data structures among threads 
 *  CS480-1001
 *  4-22-2023
 *  @author  Noah Hoenig
 *  @redID   824412349
 */
 
#ifndef SHARED_H
#define SHARED_H

#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include "log.h"

#define NORMALEXIT 0
#define BADFLAG 1
#define THREADERROR 2

#define DEFAULT_PROUDCTION_LIMIT 100
#define DEFAULT_DELAY 0
#define BROKER_SIZE 16
#define BITCOIN_LIMIT 5
/* One million nanoseconds per millisecond */
#define	NSPERMS		1000000	
/* One thousand milliseconds per second */
#define MSPERSEC	1000

/* Common data shared between threads */ 
typedef struct { 
    
    /* optional command line args */
    unsigned int total_num_requests, delay_blockchainX, delay_blockchainY, delay_bitcoin, delay_ethereum;
    
    /* track number of bitcoin produced and consumed and how much of each type is in the broker */
    unsigned int bitProduced, ethProduced, bitConsumed, ethConsumed, bitInBroker, ethInBroker;

    /* mutex semaphore - a producer or consumer must acquire a mutex to gain access to broker */
    /* availableSlots semaphore - number of empty slots in the broker */
    /* unconsumed semaphore - number of items in the broker */
    /* bitcoinConstraint semaphore - there is a limit to the number of bitcoin allowed in the broker */
    /* mainLock semaphore - main thread should wait until last request has been consumed */
    sem_t mutex, availableSlots, unconsumed, bitcoinConstraint, mainLock;
    
    /* FIFO broker containing RequestType items- Bitcoin or Ethereum */
    std::queue <RequestType>broker; 

} SHARED_DATA; 

/* Contains data that the producer threads needs to perform its work */
typedef struct { 
    
    SHARED_DATA *shared;
   
    /* indicator for indicating which type of the crypto it would produce */
    RequestType crypto_type;

} PRODUCER_DATA;

/* Contains data that the consumer threads needs to perform its work */
typedef struct { 
    
    SHARED_DATA *shared;
   
    /* indicator for indicating which type of blockchain is consuming */
    ConsumerType consumer_type;

    /* track how many of request type were consumed by consumer threads */
    unsigned int consumed[ConsumerTypeN];

} CONSUMER_DATA;

#endif