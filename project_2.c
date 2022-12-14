#include "queue.c"
#include <string.h>
#include <time.h>
#include <pthread.h>

int simulationTime = 30;    // simulation time
int seed = 10;               // seed for randomness
int emergencyFrequency = 30; // frequency of emergency gift requests from New Zealand
time_t startTime;

void* ElfA(void *arg); // the one that can paint
void* ElfB(void *arg); // the one that can assemble
void* Santa(void *arg); 
void* ControlThread(void *arg); // handles printing and queues (up to you)

int task_count = 0; 
//new funcs
int nextGiftType();
double passedTime();

//Task funcs
void* PackagingTask(void *arg);
void* PaintingTask(void *arg);
void* AssemblyTask(void *arg);
void* QATask(void *arg);
void* DeliveryTask(void *arg);


//Task Queues
struct Queue *packaging_queue, *paint_queue, *assembly_queue, *QA_queue, *delivery_queue;

//mutexes
pthread_mutex_t packaging_mut, paint_mut, assembly_mut, QA_mut, delivery_mut, task_count_mut;

// pthread sleeper function
int pthread_sleep (int seconds)
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
    struct timeval tp;
    //When to expire is an absolute time, so get the current time and add it to our delay time
    gettimeofday(&tp, NULL);
    timetoexpire.tv_sec = tp.tv_sec + seconds; timetoexpire.tv_nsec = tp.tv_usec * 1000;
    
    pthread_mutex_lock(&mutex);
    int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);
    
    //Upon successful completion, a value of zero shall be returned
    return res;
}


int main(int argc,char **argv){
    // -t (int) => simulation time in seconds
    // -s (int) => change the random seed
    for(int i=1; i<argc; i++){
        if(!strcmp(argv[i], "-t")) {simulationTime = atoi(argv[++i]);}
        else if(!strcmp(argv[i], "-s"))  {seed = atoi(argv[++i]);}
    }
    
    srand(seed); // feed the seed
    
    /* Queue usage example
        Queue *myQ = ConstructQueue(1000);
        Task t;
        t.ID = myID;
        t.type = 2;
        Enqueue(myQ, t);
        Task ret = Dequeue(myQ);
        DestructQueue(myQ);
    */

    // your code goes here
    // you can simulate gift request creation in here, 
    // but make sure to launch the threads first
    
    //get start time to control simulation time
    startTime = time(NULL);
    
    pthread_t santaThread, elfBThread, elfAThread;
    
    //Santa and Elf initializations
    /*
    pthread_create(&santaThread, NULL, &Santa, NULL);
    pthread_create(&elfAThread, NULL, &ElfA, NULL);
    pthread_create(&elfBThread, NULL, &ElfB, NULL);*/
    
    //task queue creations
    packaging_queue = ConstructQueue(1000);
    paint_queue = ConstructQueue(1000);
    assembly_queue = ConstructQueue(1000);
    QA_queue = ConstructQueue(1000);
    delivery_queue = ConstructQueue(1000);
    
    //mutex initializations
    pthread_mutex_init(&packaging_mut, NULL);
    pthread_mutex_init(&paint_mut, NULL);
    pthread_mutex_init(&assembly_mut, NULL);
    pthread_mutex_init(&QA_mut, NULL);
    pthread_mutex_init(&delivery_mut, NULL);
    //printf("start: %d\n", startTime);
    while(passedTime()<simulationTime){
	    	sleep(1);
	        printf("%f\n", passedTime());
	    	//type 1 gift
	    	int gift_type = nextGiftType();
	    	if(gift_type == 1){
	    		printf("package only\n");
	    		pthread_t package_thread;
	    		pthread_create(&package_thread, NULL, PackagingTask, NULL);
	    		pthread_detach(package_thread);
	    	}
	    	/*
	    	if(gift_type == 2){
	    		printf("package + paint\n");
	    		
	    	}
	    	if(gift_type == 3){
	    		printf("package + assembly\n");
	    	}
	    	if(gift_type == 4){
	    		printf("paint + pack + QA\n");
	    	}
	    	if(gift_type == 5){
	    		printf("assembly + pack + QA\n");
	    	}*/
		
	    
    }
    printf("Ending Simulation...\n");
 
    
    return 0;
}

void* ElfA(void *arg){
	
}

void* ElfB(void *arg){

}

// manages Santa's tasks
void* Santa(void *arg){
	
}

// the function that controls queues and output
void* ControlThread(void *arg){

}
/*
int nextProb(double p){
	return (rand() % 100) < p;
}*/

int nextGiftType(){
     	int types[20] = {0,0,1,1,1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,5};
     	int index = rand() % 20;
     	return types[index];

}
void* PackagingTask(void *arg){
	Task t;
        task_count++;
        t.ID = task_count;
        t.type = 'P';
        Enqueue(packaging_queue, t);
        pthread_exit(0);
}
double passedTime(){
     return time(NULL) % startTime;
}
/*int giftRequestArrives(){
	

}*/
