/*
 *  Program # 4
 *  This files runs the application and main thread 
 *  Compile using the Makefile command 'make' in command prompt
 *  Run program using './cryptoexc' with optional arg
 *  -r, -x, -y, -b, -e, in command prompt
 * 1) Your program should create two producers and two consumers of trade requests as pthreads.
 * 2) Producer threads will accept and publish crypto trade requests to the broker until reaching the limit of the production, then exit.
 * 3) Consumer threads will consume all requests from the broker before exiting.
 * 4) Main thread should wait for the consumer threads complete consuming the last request before exiting
 *  CS480-1001
 *  4-22-2023
 *  @author  Noah Hoenig
 *  @redID   824412349
 */

#include <iostream>
#include <unistd.h>
#include "shared.h"
#include "producer.h"
#include "consumer.h"

using namespace std; 

/**
 * @brief program starts from the main thread (where the main method is), which reads and 
 *        processes command line arguments, creates and initializes a shared data structure 
 *        to be shared between the threads, then spawns 4 worker threads- 2 producer threads 
 *        and 2 consumer threads.  Wait for all requests to be processed and consumed and 
 *        then ouput some results.
 * 
 * @param argc
 * @param argv
 */
int main(int argc, char **argv)
{
    
    int option;   /* command line switch */


    /* Initalize SHARED_DATA and optional arguments to default values */
    SHARED_DATA sharedData;
    sharedData.total_num_requests = DEFAULT_PROUDCTION_LIMIT;
    sharedData.delay_blockchainX = sharedData.delay_blockchainY = sharedData.delay_bitcoin = sharedData.delay_ethereum = DEFAULT_DELAY;
    sharedData.bitProduced = sharedData.ethProduced = sharedData.bitInBroker = sharedData.ethInBroker = sharedData.bitConsumed = sharedData.ethConsumed = 0;    
    sem_init(&sharedData.mutex, 1, 1);
    sem_init(&sharedData.unconsumed, 1, 0);
    sem_init(&sharedData.availableSlots, 1, BROKER_SIZE);
    sem_init(&sharedData.bitcoinConstraint, 1, BITCOIN_LIMIT);
    sem_init(&sharedData.mainLock, 1, 0);

    /* Read in optional command line args */
    while ( (option = getopt(argc, argv, "r:x:y:b:e:")) != -1) {
        switch (option) {
            case 'r': /* Assume this takes a number */
                /* optarg will contain the string following -r
                 * -r is expected to be an integer in this case, so convert the
                 * string to an integer.
                 */
                sharedData.total_num_requests = atoi(optarg);  
                break;
            case 'x': /* optarg points to whatever follows -x */
                sharedData.delay_blockchainX = atoi(optarg);
                break;
            case 'y':  /* optarg points to whatever follows -y */
                sharedData.delay_blockchainY = atoi(optarg);
                break;
            case 'b':  /* optarg points to whatever follows -b */
                sharedData.delay_bitcoin = atoi(optarg);
                break;
            case 'e':  /* optarg points to whatever follows -e */
                sharedData.delay_ethereum = atoi(optarg);
                break;
            default:
                exit(BADFLAG); 
        }
    }    
    
    /* Decalre threads */
    pthread_t bitcoinThread, ethereumThread, blockhainXThread, blockhainYThread;

    /* Each thread will have it's own data upon its creation */
    PRODUCER_DATA bitcoin_data, ethereum_data;
    CONSUMER_DATA blockchainX_data, blockchainY_data;

    /* They will all have a pointer to the SHARED_DATA */
    SHARED_DATA *shared_ptr = &sharedData;

    /* Give them all access to the SHARED_DATA struct */
    bitcoin_data.shared = ethereum_data.shared = blockchainX_data.shared = blockchainY_data.shared = shared_ptr; 

    /* Initialize request types for PRODUCER_DATA */
    bitcoin_data.crypto_type = Bitcoin; // give them unique identifiers
    ethereum_data.crypto_type = Ethereum; 
    
    /* Initialize consumer types for CONSUMER_DATA */
    blockchainX_data.consumer_type = BlockchainX;
    blockchainY_data.consumer_type = BlockchainY;
    /* Initialize arrays containing how much of each request type did a consumer type consume */
    blockchainX_data.consumed[ConsumerTypeN] = blockchainY_data.consumed[ConsumerTypeN] = {Bitcoin};

    /* Create and start threads */ 
    if (pthread_create(&bitcoinThread, NULL, &producer, &bitcoin_data)) {
        cerr << "Error"<< endl;
        exit(THREADERROR);
    }
    if (pthread_create(&ethereumThread, NULL, &producer, &ethereum_data)) {
        cerr << "Error"<< endl;
        exit(THREADERROR);
    }
    if (pthread_create(&blockhainXThread, NULL, &consumer,&blockchainX_data)) {
        cerr << "Error"<< endl;
        exit(THREADERROR);
    } 
    if (pthread_create(&blockhainYThread, NULL, &consumer,&blockchainY_data)) {
        cerr << "Error"<< endl;
        exit(THREADERROR);
    }

    /* Main thread should wait for signal from whichever consumer thread consumes the last request */
    sem_wait(&sharedData.mainLock);
    
    /* Show how many requests of each type produced.  
       and how many requests consumed by each consumer */
    unsigned int produced[RequestTypeN]={sharedData.bitProduced,sharedData.ethProduced};
    unsigned int *consumed[ConsumerTypeN]={blockchainX_data.consumed,blockchainY_data.consumed};
    log_production_history(produced,consumed);
    
    exit(NORMALEXIT);
}