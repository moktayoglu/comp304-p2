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
Task* createTask(int giftType, char* taskType);
void printLog(Task* t);

time_t startTime;
int task_count = 0; 
int gift_count = 0;
int type4_package_cond = 0;
int type5_package_cond = 0;

int nextGiftType();
double passedTime();

//Task funcs
void* PackagingTask(void *arg);
void* PaintingTask(void *arg);
void* AssemblyTask(void *arg);
void* QATask(void *arg);
void* DeliveryTask(void *arg);
pthread_t package_thread, delivery_thread, paint_thread, assembly_thread, QA_thread;
pthread_cond_t time_cond;

//Task Queues
struct Queue *packaging_queue, *paint_queue, *assembly_queue, *QA_queue, *delivery_queue;
struct arg_struct {
    struct Queue *arg1;
    pthread_mutex_t arg2;
};

//mutexes
pthread_mutex_t packaging_mut, paint_mut, assembly_mut, QA_mut, delivery_mut; 
pthread_mutex_t task_count_mut, gift_count_mut;
pthread_mutex_t log_mut, taskCount_mut;
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
    

    //opening events log and adding headers.
    FILE *f = fopen("events.log", "w");
    fprintf(f, "TaskID\tGiftID\tGiftType\tTaskType\tRequestTime\tResponsible\n");
    fprintf(f, "______________________________________________________________\n");
    fclose(f);

    //Simulates and synchornizes adding of tasks to queues
    while(passedTime()<simulationTime){
	    	sleep(1);
	        printf("%f\n", passedTime());

	    	int gift_type = nextGiftType();
	    	int* giftT = malloc(sizeof(int));
	    	*giftT = gift_type;
	    	
	    	pthread_mutex_lock(&gift_count_mut);
	    	gift_count++;
	    	pthread_mutex_unlock(&gift_count_mut);
	    	
	    	//needs only packaging followed by delivery
	    	if(gift_type == 1){
	    		int giftT1 = 1;
	    		pthread_t package_thread, delivery_thread, paint_thread, assembly_thread, QA_thread;
	    		printf("package + deliver only\n");
	    		//addTaskQueue(packaging_queue, packaging_mut);
	    		
	    		pthread_create(&package_thread, NULL, addPackageQueue, giftT);
	    		pthread_join(package_thread, NULL);
	    		pthread_create(&delivery_thread, NULL, addDeliveryQueue, giftT);
			    pthread_join(delivery_thread, NULL);
		
	    	}
	    	
            // painting followed by packaging followed by delivery
	    	if(gift_type == 2){
	    		pthread_t package_thread, delivery_thread, paint_thread, assembly_thread, QA_thread;
	    		int giftT2 = 2;
	    		printf("package + paint\n");

                	pthread_create(&paint_thread, NULL, addPaintQueue, giftT);
                	pthread_join(paint_thread, NULL);
                	pthread_create(&package_thread, NULL, addPackageQueue,giftT);
                	pthread_join(package_thread, NULL);
	    		    pthread_create(&delivery_thread, NULL, addDeliveryQueue, giftT);
			       pthread_join(delivery_thread, NULL);
	
	    	}
            
            //assembly followed by packaging followed by delivery
	    	if(gift_type == 3){
	    		pthread_t package_thread, delivery_thread, paint_thread, assembly_thread, QA_thread;
	    		int giftT3 = 3;
	    		printf("package + assembly\n");
                	pthread_create(&assembly_thread, NULL, addAssemblyQueue, giftT);
                	pthread_join(assembly_thread, NULL);
                	pthread_create(&package_thread, NULL, addPackageQueue, giftT);
                	pthread_join(package_thread, NULL);
	    		pthread_create(&delivery_thread, NULL, addDeliveryQueue, giftT);
	    		pthread_join(delivery_thread, 0);

	    	}

            //require painting and QA followed by packaging followed by delivery
	    	if(gift_type == 4){
	    		pthread_t package_thread, delivery_thread, paint_thread, assembly_thread, QA_thread;
	    		int giftT4 = 4;
	    		printf("paint + pack + QA\n");
	    		pthread_create(&paint_thread, NULL, addPaintQueue,giftT);
	    		pthread_create(&QA_thread, NULL, addQAQueue, giftT);
                	pthread_create(&package_thread, NULL, addPackageQueue, giftT);
                	pthread_join(package_thread, NULL);
	    		pthread_create(&delivery_thread, NULL, addDeliveryQueue, giftT);
	    		pthread_join(delivery_thread, NULL);	

	    	}


            //require assembly and QA followed by packaging followed by delivery.
	    	if(gift_type == 5){
	    		pthread_t package_thread, delivery_thread, paint_thread, assembly_thread, QA_thread;
	    		printf("assembly + pack + QA\n");
                	pthread_create(&assembly_thread, NULL, addAssemblyQueue,  giftT);
	    		pthread_create(&QA_thread, NULL, addQAQueue, giftT);
                	pthread_create(&package_thread, NULL, addPackageQueue,giftT);
                	pthread_join(package_thread, NULL);
	    		pthread_create(&delivery_thread, NULL, addDeliveryQueue,  giftT);
	    		pthread_join(delivery_thread, NULL);
	    		
	    	}
		
	    
    }

    //pthread_cond_signal(&time_cond);
    printf("Ending Simulation...\n");
    
    //pthrread_cond_destroy(&time_cond);
 
    
    return 0;
}

// manages ElfA's tasks.
void* ElfA(void *arg){ 
	while(passedTime()<simulationTime){
		//pthread_sleep(1); //TODO
		PackagingTask(NULL);
        PaintingTask(NULL); //only does paint
		
	}
	pthread_exit(0);
}

// manages ElfB's tasks.
void* ElfB(void *arg){ 
	while(passedTime()<simulationTime){
		//pthread_sleep(1); //TODO
		PackagingTask(NULL);
        AssemblyTask(NULL); //only does assembly
        
		
	}
	pthread_exit(0);
}

// manages Santa's tasks
void* Santa(void *arg){
	while(passedTime()<simulationTime){
		//pthread_sleep(1); //TODO
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


/* This is for probability demonstration
// The 2 zeros corresp0nding to 2/20 which is %10 probibility that no tasks is obtained.
// 8 ones -> 8/20 which is %40 probability of the task needs only packaging followed by delivery.
// so on..
*/
int nextGiftType(){
     	int types[20] = {0,0,1,1,1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,5}; 
     	int index = rand() % 20;
     	//printf("index: %d %d",index, types[index]);
     	return types[index];

}

/*
//Methods for packaging, delivery, painting, assembly and QA tasks.
*/

void* PackagingTask(void *arg){
	if (!isEmpty(packaging_queue)){
			printf("elf in if\n");
			pthread_mutex_lock(&packaging_mut);
			Task ret = Dequeue(packaging_queue);
            ret.responsible = "A"; //TODO
			pthread_sleep(1); // packaging time 1 sec
			printf("dequeued: %d\n", ret.ID);
			pthread_mutex_unlock(&packaging_mut);
			printLog(&ret);
			
	}
}

void* DeliveryTask(void *arg){
	if (!isEmpty(delivery_queue)){
			printf("Santa starts work...\n");
			pthread_mutex_lock(&delivery_mut);
			Task ret = Dequeue(delivery_queue);
            ret.responsible = "S";
			pthread_sleep(1); // deliver time 1 sec
			printf("delivered: %d\n", ret.ID);
			pthread_mutex_unlock(&delivery_mut);
			printLog(&ret);
	}
}

void* PaintingTask(void *arg){
	if (!isEmpty(paint_queue)){
			printf("Painting...\n");
			pthread_mutex_lock(&paint_mut);

			Task ret = Dequeue(paint_queue);
            ret.responsible = "A";
            
			pthread_sleep(3); // painting time 3 sec
			printf("painted: %d\n", ret.ID);
		
			
			incrementType4CondCounter(ret); //increment flag for type4 packaging
				
			printf("incremented  in paint\n");

			pthread_mutex_unlock(&paint_mut);
			printLog(&ret);
	}
}

void* AssemblyTask(void *arg){
	if (!isEmpty(paint_queue)){
			printf("Assembling...\n");
			pthread_mutex_lock(&assembly_mut);

			Task ret = Dequeue(assembly_queue);
            ret.responsible = "B";
			pthread_sleep(2); // assembly time 2 sec
			printf("assembled: %d\n", ret.ID);
			
            		incrementType5CondCounter(ret);  //increment flag for type5 packaging
            		
            		printf("incremented  in assembly\n");

			pthread_mutex_unlock(&assembly_mut);
			printLog(&ret);
	}
}

void* QATask(void *arg){
	if (!isEmpty(QA_queue)){
		printf("QA...\n");
		pthread_mutex_lock(&QA_mut);
		Task ret = Dequeue(QA_queue);
        ret.responsible = "S";
		pthread_sleep(2); // assembly time 2 sec
		printf("QA done: %d\n", ret.ID);
		
		incrementType4CondCounter(ret);
		
        incrementType5CondCounter(ret);
        	
		printf("incremented  in QA\n");

		pthread_mutex_unlock(&QA_mut);	
		printLog(&ret);	
	}

}

/*
// Adding tasks to corresponding queues.
*/


void* addPackageQueue(void *arg){ 
	int giftType = *((int *) arg);
	pthread_join(assembly_thread, NULL);

	while (giftType == 4){
		pthread_mutex_lock(&type4_package_cond_mut);
		int now_type4_cond = type4_package_cond;
		pthread_mutex_unlock(&type4_package_cond_mut);

        //for both painting and QA threads to finish then proceed
		if ((now_type4_cond % 2) == 0 && now_type4_cond != 0) break;  
		
	}

    	printf("type4_package_cond %d\n", type4_package_cond);

    	while (giftType == 5){
		pthread_mutex_lock(&type5_package_cond_mut);
		int now_type5_cond = type5_package_cond; 
		pthread_mutex_unlock(&type5_package_cond_mut);

        //for both assembly and QA threads to finish then proceed
		if ((now_type5_cond % 2) == 0 && now_type5_cond != 0) break; 
		
	}

    printf("type5_package_cond %d\n", type5_package_cond);
	
	pthread_mutex_lock(&packaging_mut);
	Task t;
	pthread_mutex_lock(&taskCount_mut);
	task_count++;
	pthread_mutex_unlock(&taskCount_mut);
	t.ID = task_count;
	t.type = "C"; //packaging
	Enqueue(packaging_queue, t);
	pthread_mutex_unlock(&packaging_mut);
	printf("Added packaging %d\n",t.ID);
        
    pthread_exit(0);
	
}


void* addDeliveryQueue(void *arg){
	int giftType = *((int *) arg);
    Task* t = createTask(giftType, "D"); //delivery
    pthread_mutex_lock(&delivery_mut);
	Enqueue(delivery_queue, *t);
	pthread_mutex_unlock(&delivery_mut); 
	pthread_exit(0);
}

void* addPaintQueue(void *arg){  
	int giftType = *((int *) arg);
	//free(arg);
        Task* t = createTask(giftType, "P");
        pthread_mutex_lock(&paint_mut);
	Enqueue(paint_queue, *t);
	pthread_mutex_unlock(&paint_mut); 
	pthread_exit(0);
        //addQueue(t, paint_queue, paint_mut);	
	
}

void* addAssemblyQueue(void *arg){
	int giftType = *((int *) arg);
    Task* t = createTask(giftType, "A");
    pthread_mutex_lock(&assembly_mut);
	Enqueue(assembly_queue, *t);
	//free(t);
	pthread_mutex_unlock(&assembly_mut); 
	pthread_exit(0);
}

void* addQAQueue(void *arg){
	int giftType = *((int *) arg);
    Task* t = createTask(giftType, "Q");
    pthread_mutex_lock(&QA_mut);
	Enqueue(QA_queue, *t);
	pthread_mutex_unlock(&QA_mut); 
	pthread_exit(0);
      
}

/*
/ Incrementing the type 4 counter to make sure that
/ Packaging is started once Painting and QA are both finished.
*/

void incrementType4CondCounter(Task t){
	if (t.giftType == 4){
		pthread_mutex_lock(&type4_package_cond_mut);
		type4_package_cond++;
		pthread_mutex_unlock(&type4_package_cond_mut);
	}
}

/*
// Incrementing the type 5 counter to make sure that
// Packaging is started once Assembly and QA are both finished.
*/

void incrementType5CondCounter(Task t){
	if (t.giftType == 5){
		pthread_mutex_lock(&type5_package_cond_mut);
		type5_package_cond++;
		pthread_mutex_unlock(&type5_package_cond_mut);
	}
}

/*
// This method is constructed for creating a task and assigning
// ID, type, giftType, giftID and requestTime for that task.
*/

Task* createTask(int giftType, char* taskType){
	Task *t = (Task *) malloc(sizeof(Task));
	pthread_mutex_lock(&taskCount_mut);
        task_count++;
        printf("task count:  %d\n", task_count);
        pthread_mutex_unlock(&taskCount_mut);
        t->ID = task_count;
        t->type = taskType; //TODO
        t->giftType = giftType;
        t->requestTime = (int) passedTime();
        pthread_mutex_lock(&gift_count_mut);
        t->giftID = gift_count;
        pthread_mutex_unlock(&gift_count_mut);
        return t;

}

/*
// TaskID, GiftID, GiftType, TaskType, RequestTime, TaskArrival, TT, and Responsible
// values are printed into events.log file.
*/

void printLog(Task* t){
	printf("print CALLED\n");
	pthread_mutex_lock(&log_mut);
	FILE *f = fopen("events.log", "a");
	fprintf(f, "%d\t%d\t%d\t\t%s\t\t%d\t\t\t%s\n", t->ID, t->giftID, t->giftType, t->type, t->requestTime, t->responsible);
	pthread_mutex_unlock(&log_mut);
	fclose(f);
}

/*
// This function is used for ensuring that simulation time is satisfied and 
// requestTime is retrieved.
*/

double passedTime(){
     return time(NULL) % startTime;
}

