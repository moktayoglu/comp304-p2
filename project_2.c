#include "queue.c"
#include <string.h>
#include <time.h>
#include <pthread.h>

int simulationTime = 20;    // simulation time
int seed = 10;               // seed for randomness
int emergencyFrequency = 30; // frequency of emergency gift requests from New Zealand


void* ElfA(void *arg); // the one that can paint
void* ElfB(void *arg); // the one that can assemble
void* Santa(void *arg); 

void* ControlThread(void *arg); // handles printing and queues (up to you)

void* addPackageQueue(void *arg);
void* addDeliveryQueue(void *arg);
void* addPaintQueue(void *arg);
void* addAssemblyQueue(void *arg);
void* addQAQueue(void *arg);
void incrementType4CondCounter();
void incrementType5CondCounter();

time_t startTime;
int task_count = 0; 
int type4_package_cond = 0;
int type5_package_cond = 0;
//new funcs
int nextGiftType();
double passedTime();

//Task funcs
void* PackagingTask(void *arg);
void* PaintingTask(void *arg);
void* AssemblyTask(void *arg);
void* QATask(void *arg);
void* DeliveryTask(void *arg);

pthread_cond_t time_cond;

//Task Queues
struct Queue *packaging_queue, *paint_queue, *assembly_queue, *QA_queue, *delivery_queue;
struct arg_struct {
    struct Queue *arg1;
    pthread_mutex_t arg2;
};
//mutexes
pthread_mutex_t packaging_mut, paint_mut, assembly_mut, QA_mut, delivery_mut, task_count_mut;

pthread_mutex_t type4_package_cond_mut, type5_package_cond_mut;

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
    //pthread_cond_init(time_cond,  NULL);
    pthread_t santaThread, elfBThread, elfAThread;
    
    
    //Santa and Elf initializations
    pthread_create(&elfAThread, NULL, ControlThread, NULL);
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
    
    pthread_t package_thread, delivery_thread, paint_thread, assembly_thread, QA_thread;
    //printf("start: %d\n", startTime);
    
    //Simulates and synchornizes adding of tasks to queues
    while(passedTime()<simulationTime){
	    	sleep(1);
	        printf("%f\n", passedTime());
	    	//type 1 gift
	    	int gift_type = nextGiftType();
	    	if(gift_type == 1){
	    		printf("package + deliver only\n");
	    		//addTaskQueue(packaging_queue, packaging_mut);
	    		
	    		pthread_create(&package_thread, NULL, addPackageQueue, (void *) &gift_type);
	    		pthread_create(&delivery_thread, NULL, addDeliveryQueue, NULL);

	    		//pthread_detach(package_thread);
                	//pthread_detach(delivery_thread);
	
	    		pthread_join(package_thread, NULL);
	    		pthread_join(delivery_thread, NULL);
	    		
	    	}
	    	
	    	if(gift_type == 2){
	    		printf("package + paint\n");

                	pthread_create(&paint_thread, NULL, addPaintQueue, NULL);
                	pthread_create(&package_thread, NULL, addPackageQueue, (void *) &gift_type);
	    		pthread_create(&delivery_thread, NULL, addDeliveryQueue, NULL);

	    		/*pthread_detach(paint_thread);
                	pthread_detach(package_thread);
                	pthread_detach(delivery_thread);*/

               		pthread_join(paint_thread, NULL);
	    		pthread_join(package_thread, NULL);
	    		pthread_join(delivery_thread, NULL);
	    		
	    	}
            
	    	if(gift_type == 3){
	    		printf("package + assembly\n");
                pthread_create(&assembly_thread, NULL, addAssemblyQueue, NULL);
                pthread_create(&package_thread, NULL, addPackageQueue, (void *) &gift_type);
	    		pthread_create(&delivery_thread, NULL, addDeliveryQueue, NULL);

                pthread_join(assembly_thread, NULL);
	    		pthread_join(package_thread, NULL);
	    		pthread_join(delivery_thread, NULL);

	    	}

            
	    	if(gift_type == 4){
	    		printf("paint + pack + QA\n");
	    		pthread_create(&paint_thread, NULL, addPaintQueue, NULL);
	    		pthread_create(&QA_thread, NULL, addQAQueue, NULL);
                pthread_create(&package_thread, NULL, addPackageQueue, (void *) &gift_type);
	    		pthread_create(&delivery_thread, NULL, addDeliveryQueue, NULL);
	    		
	    		pthread_join(package_thread, NULL);
	    		pthread_join(delivery_thread, NULL);

	    	}


	    	if(gift_type == 5){
	    		printf("assembly + pack + QA\n");
                pthread_create(&assembly_thread, NULL, addAssemblyQueue, NULL);
	    		pthread_create(&QA_thread, NULL, addQAQueue, NULL);
                pthread_create(&package_thread, NULL, addPackageQueue, (void *) &gift_type);
	    		pthread_create(&delivery_thread, NULL, addDeliveryQueue, NULL);
	    		
	    		pthread_join(package_thread, NULL);
	    		pthread_join(delivery_thread, NULL);

	    	}
		
	    
    }
    //pthread_cond_signal(&time_cond);
    printf("Ending Simulation...\n");
    //pthrread_cond_destroy(&time_cond);
 
    
    return 0;
}

void* ElfA(void *arg){ 
	while(passedTime()<simulationTime){
		pthread_sleep(1);
		printf("elfA\n");
		PackagingTask(NULL);
        PaintingTask(NULL); //only does paint
		
	}
	pthread_exit(0);
}

void* ElfB(void *arg){ 
	while(passedTime()<simulationTime){
		pthread_sleep(1);
		printf("elfB\n");
		PackagingTask(NULL);
        AssemblyTask(NULL); //only does assembly
		
	}
	pthread_exit(0);
}

// manages Santa's tasks
void* Santa(void *arg){
	while(passedTime()<simulationTime){
		pthread_sleep(1);
		printf("Santa\n");
		DeliveryTask(NULL); //prioritizes delivery
		QATask(NULL);
		
	}
	pthread_exit(0);
	
}

// the function that controls queues and output
void* ControlThread(void *arg){
	pthread_t elfAThread, elfBThread, santaThread;

	pthread_create(&elfAThread, NULL, ElfA, NULL);
	pthread_create(&elfBThread, NULL, ElfB, NULL);
	pthread_create(&santaThread, NULL, Santa, NULL);

	pthread_detach(elfBThread);
	pthread_detach(elfAThread);
	pthread_detach(santaThread);
	pthread_exit(0);
}


/* This is for probability demonstration*/
int nextGiftType(){
     	int types[20] = {0,0,1,1,1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,5}; 
     	int index = rand() % 20;
     	return types[index];

}

void* PackagingTask(void *arg){
	if (!isEmpty(packaging_queue)){
			printf("elf in if\n");
			pthread_mutex_lock(&packaging_mut);
			Task ret = Dequeue(packaging_queue);
			pthread_sleep(1); //packaging time 1 sec
			printf("dequeued: %d\n", ret.ID);
			pthread_mutex_unlock(&packaging_mut);
	}
}

void* DeliveryTask(void *arg){
	if (!isEmpty(delivery_queue)){
			printf("Santa starts work...\n");
			pthread_mutex_lock(&delivery_mut);
			Task ret = Dequeue(delivery_queue);
			pthread_sleep(1); //deliver time 1 sec
			printf("delivered: %d\n", ret.ID);
			pthread_mutex_unlock(&delivery_mut);
	}
}

void* PaintingTask(void *arg){
	if (!isEmpty(paint_queue)){
			printf("Painting...\n");
			pthread_mutex_lock(&paint_mut);

			Task ret = Dequeue(paint_queue);
			pthread_sleep(3); // painting time 3 sec
			printf("painted: %d\n", ret.ID);
		
			incrementType4CondCounter(); //increment flag for type4 packaging
			printf("incremented  in paint\n");

			pthread_mutex_unlock(&paint_mut);
	}
}

void* AssemblyTask(void *arg){
	if (!isEmpty(paint_queue)){
			printf("Assembling...\n");
			pthread_mutex_lock(&assembly_mut);

			Task ret = Dequeue(assembly_queue);
			pthread_sleep(2); // assembly time 2 sec
			printf("assembled: %d\n", ret.ID);

            incrementType5CondCounter();  //increment flag for type5 packaging
            printf("incremented  in assembly\n");

			pthread_mutex_unlock(&assembly_mut);
	}
}

void* QATask(void *arg){
	if (!isEmpty(QA_queue)){
		printf("QA...\n");
		pthread_mutex_lock(&QA_mut);
		Task ret = Dequeue(QA_queue);
		pthread_sleep(2); // assembly time 2 sec
		printf("QA done: %d\n", ret.ID);

		incrementType4CondCounter();
        incrementType5CondCounter();
		printf("incremented  in QA\n");

		pthread_mutex_unlock(&QA_mut);		
	}

}

void* addPackageQueue(void *arg){ //hangi typedan geliyo? 4-> (Veya 5se) cond variableını oku->uygunsa ekle
	int giftType = *((int *) arg);

	while (giftType == 4){
		pthread_mutex_lock(&type4_package_cond_mut);
		printf("waits for type4 finish.. %d\n", type4_package_cond);
		
		pthread_sleep(1);

		if ((type4_package_cond % 2) == 0 && type4_package_cond != 0) break; 
		pthread_mutex_unlock(&type4_package_cond_mut);
	}

    printf("type4_package_cond %d\n", type4_package_cond);

    while (giftType == 5){
		pthread_mutex_lock(&type5_package_cond_mut);
		printf("waits for type5 finish.. %d\n", type5_package_cond);
		
		pthread_sleep(2); 

		if ((type5_package_cond % 2) == 0 && type5_package_cond != 0) break; //for threads to finish then proceed
		pthread_mutex_unlock(&type5_package_cond_mut);
	}

    printf("type5_package_cond %d\n", type5_package_cond);
	
	pthread_mutex_lock(&packaging_mut);
	Task t;
	task_count++;
	t.ID = task_count;
	t.type = 1; //TODO
	Enqueue(packaging_queue, t);
	pthread_mutex_unlock(&packaging_mut);
	printf("Added packaging %d\n",t.ID);
        
    pthread_exit(0);
	
}

void* addDeliveryQueue(void *arg){
	Task t;
        task_count++;
        pthread_mutex_lock(&delivery_mut);
        t.ID = task_count;
        t.type = 2; //TODO
        Enqueue(delivery_queue, t);
        pthread_mutex_unlock(&delivery_mut);
        printf("Added delivery %d\n",t.ID);
        pthread_exit(0);
	
}

void* addPaintQueue(void *arg){ 
	Task t;
        task_count++;
        pthread_mutex_lock(&paint_mut);  //acquire the lock
        t.ID = task_count;
        t.type = 3; //TODO
        Enqueue(paint_queue, t);
        pthread_mutex_unlock(&paint_mut);  //release the lock
        printf("Added painting %d\n",t.ID);
        pthread_exit(0);
	
}

void* addAssemblyQueue(void *arg){
	Task t;
        task_count++;
        pthread_mutex_lock(&assembly_mut);  //acquire the lock
        t.ID = task_count;
        t.type = 4; //TODO
        Enqueue(assembly_queue, t);
        pthread_mutex_unlock(&assembly_mut);  //release the lock
        printf("Added assembly %d\n",t.ID);
        pthread_exit(0);
	
}

void* addQAQueue(void *arg){
	Task t;
        task_count++;
        pthread_mutex_lock(&QA_mut);  //acquire the lock
        t.ID = task_count;
        t.type = 5; //TODO
        Enqueue(QA_queue, t);
        pthread_mutex_unlock(&QA_mut);  //release the lock
        printf("Added QA %d\n",t.ID);
        pthread_exit(0);

}

void incrementType4CondCounter(){
	pthread_mutex_lock(&type4_package_cond_mut);
	type4_package_cond++;
	pthread_mutex_unlock(&type4_package_cond_mut);
}

void incrementType5CondCounter(){
	pthread_mutex_lock(&type5_package_cond_mut);
	type5_package_cond++;
	pthread_mutex_unlock(&type5_package_cond_mut);
}


double passedTime(){
     return time(NULL) % startTime;
}

