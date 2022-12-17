#include "queue.c"
#include <string.h>
#include <time.h>
#include <pthread.h>

int simulationTime = 120;    // simulation time
int seed = 10;               // seed for randomness
int emergencyFrequency = 30; // frequency of emergency gift requests from New Zealand


void* ElfA(void *arg); // the one that can paint
void* ElfB(void *arg); // the one that can assemble
void* Santa(void *arg); 

void* ControlThread(void *arg); // handles printing and queues (up to you)

//void* addPackageQueue(void *arg);
void* addPackageQueue(int giftType, int giftID);
void* addDeliveryQueue(int giftType,  int giftID);
void* addPaintQueue(void *arg);
void* addAssemblyQueue(void *arg);
void* addPackageQueueType1(void *arg);
void* addQAQueue(void *arg);
void incrementType4CondCounter();
void incrementType5CondCounter();
Task* createTask(int giftType, char* taskType);
Task* createDownTask(int giftType, int giftID, char* taskType);
void addQueue(Task t, Queue *q, pthread_mutex_t que_mut);
void printLog(Task* t);
void* doType1(void *arg);
void* doType2(void *arg);
void* doType3(void *arg);
void* doType4(void *arg);
void* doType5(void *arg);
void incrementCurrentTask();

time_t startTime;
int task_count = 0; 
int gift_count = 0;
int type4_package_cond = 0;
int type5_package_cond = 0;
int current_task_ID = 1;
//new funcs
int nextGiftType();
double passedTime();

//Task funcs
void* PackagingTask(void *arg);
void* PaintingTask(void *arg);
void* AssemblyTask(void *arg);
void* QATask(void *arg);
void* DeliveryTask(void *arg);
//pthread_t package_thread, delivery_thread, paint_thread, assembly_thread, QA_thread;
pthread_cond_t time_cond;

//Task Queues
struct Queue *packaging_queue, *paint_queue, *assembly_queue, *QA_queue, *delivery_queue;
struct arg_struct {
    struct Queue *arg1;
    pthread_mutex_t arg2;
};
//mutexes
pthread_mutex_t packaging_mut, paint_mut, assembly_mut, QA_mut, delivery_mut; 
pthread_mutex_t task_count_mut, gift_count_mut, current_task_ID_mut;
pthread_mutex_t log_mut, taskCount_mut;
pthread_mutex_t type4_package_cond_mut, type5_package_cond_mut;

pthread_t packaging_thread;
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
    
    //pthread_t package_thread, delivery_thread, paint_thread, assembly_thread, QA_thread;
    //printf("start: %d\n", startTime);
    FILE *f = fopen("events.log", "w");
    fprintf(f, "TaskID\tGiftID\tGiftType\tTaskType\tRequestTime\tResponsible\n");
    fprintf(f, "_____________________________________________________________________________________\n");
    fclose(f);
    //Simulates and synchornizes adding of tasks to queues
    while(passedTime()<simulationTime){
	    	sleep(1);
	        printf("%f\n", passedTime());
	        
	    	//type 1 gift
	    	int gift_type = nextGiftType();
	    	//int gift_type = 5;
	    	int* giftT = (int*) malloc(sizeof(int));
	    	*giftT = gift_type;
	    	
	    	pthread_mutex_lock(&gift_count_mut);
	    	gift_count++;
	    	pthread_mutex_unlock(&gift_count_mut);
	    	pthread_t type3_thread, type4_thread,  type5_thread;
	    	//pthread_t package_thread, delivery_thread, paint_thread, assembly_thread, QA_thread;
	    	if(gift_type == 1){
	    		pthread_t type1_thread;
	    		pthread_create(&type1_thread, NULL, doType1, giftT);
	    

	    		
	    		
	    	}
	    	
	    	if(gift_type == 2){
	    		pthread_t type2_thread;
	    		pthread_create(&type2_thread, NULL, doType2, giftT);
	    		

               		
	   
	    	}
            
	    	if(gift_type == 3){
	    		pthread_create(&type3_thread, NULL, doType3, giftT);
	    	}

            
	    	if(gift_type == 4){
	    		pthread_create(&type4_thread, NULL, doType4, giftT);
	    	
	    	}


	    	if(gift_type == 5){
	    		pthread_create(&type5_thread, NULL, doType5, giftT);
	    		
	    	}
		
	    
    }
    //pthread_cond_signal(&time_cond);
    printf("Ending Simulation...\n");
    
    //pthrread_cond_destroy(&time_cond);
 
    
    return 0;
}

void* doType1(void *arg){
	void *status;
	//int giftT = *((int *) arg);
	pthread_t package_thread1, delivery_thread1;
	printf("package + deliver only\n");
	    		
	pthread_create(&package_thread1, NULL, addPackageQueueType1, arg);
	pthread_join(packaging_thread, NULL);

	
}

void* doType2(void *arg){
	//int giftT = *((int *) arg);
	pthread_t package_thread2, delivery_thread2, paint_thread2;
	    
	printf("package + paint\n");

        pthread_create(&paint_thread2, NULL, addPaintQueue, arg);
        pthread_join(paint_thread2, NULL);
  

}

void* doType3(void *arg){

	pthread_t package_thread3, delivery_thread3, assembly_thread3;
	    		
	printf("package + assembly\n");
        pthread_create(&assembly_thread3, NULL, addAssemblyQueue, arg);
        pthread_join(assembly_thread3, NULL);
     


}

void* doType4(void *arg){
	
	pthread_t package_thread4, delivery_thread4, paint_thread4, QA_thread4;

	printf("paint + pack + QA\n");
	pthread_create(&paint_thread4, NULL, addPaintQueue,arg);
	pthread_create(&QA_thread4, NULL, addQAQueue, arg);


}

void* doType5(void *arg){
	pthread_t package_thread5, delivery_thread5, assembly_thread5, QA_thread5;
	printf("assembly + pack + QA\n");
        pthread_create(&assembly_thread5, NULL, addAssemblyQueue,  arg);
	pthread_create(&QA_thread5, NULL, addQAQueue, arg);

	

}
void* ElfA(void *arg){ 
	while(passedTime()<simulationTime){
		pthread_sleep(1); //TODO
		//printf("elfA\n");
		PackagingTask(NULL);
        	PaintingTask(NULL); //only does paint
		
	}
	pthread_exit(0);
}

void* ElfB(void *arg){ 
	while(passedTime()<simulationTime){
		//pthread_sleep(1); //TODO
		//printf("elfB\n");
		PackagingTask(NULL);
        	AssemblyTask(NULL); //only does assembly
		
	}
	pthread_exit(0);
}

// manages Santa's tasks
void* Santa(void *arg){
	while(passedTime()<simulationTime){
		//pthread_sleep(1); //TODO
		//printf("Santa\n");
		DeliveryTask(NULL); //prioritizes delivery
		if (isEmpty(delivery_queue) != QA_queue->size>=3){  //XOR
			QATask(NULL);
		}
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
     	int types[20] = {1,1,1,1,1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,5}; 
     	int index = rand() % 20;
     	//printf("index: %d %d",index, types[index]);
     	return types[index];

}

void* PackagingTask(void *arg){
	if (!isEmpty(packaging_queue)&& current_task_ID == packaging_queue->head->data.ID){
			printf("elf in if\n");
			pthread_mutex_lock(&packaging_mut);
			Task ret = Dequeue(packaging_queue);
            		ret.responsible = "A";  //TODO
			pthread_sleep(1); //packaging time 1 sec
			printf("dequeued: %d\n", ret.ID);
			pthread_mutex_unlock(&packaging_mut);
			printLog(&ret);
			addDeliveryQueue(ret.giftType, ret.giftID);
			incrementCurrentTask();
			
			
	}
}

void* DeliveryTask(void *arg){
	if (!isEmpty(delivery_queue)&& current_task_ID == delivery_queue->head->data.ID){
			printf("Santa starts work...\n");
			pthread_mutex_lock(&delivery_mut);
			Task ret = Dequeue(delivery_queue);
            		ret.responsible = "S";
			pthread_sleep(1); //deliver time 1 sec
			printf("delivered: %d\n", ret.ID);
			pthread_mutex_unlock(&delivery_mut);
			printLog(&ret);
			incrementCurrentTask();
	}
}

void* PaintingTask(void *arg){
	if (!isEmpty(paint_queue) && current_task_ID == paint_queue->head->data.ID){
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
			incrementCurrentTask();
			while (ret.giftType == 4){
				pthread_mutex_lock(&type4_package_cond_mut);
				int now_type4_cond = type4_package_cond;
				//printf("waits for type4 finish.. %d\n", type4_package_cond);
		
				//pthread_sleep(1);
				pthread_mutex_unlock(&type4_package_cond_mut);
				if ((now_type4_cond % 2) == 0 && now_type4_cond != 0) break; 
		
			}
			
			addPackageQueue(ret.giftType, ret.giftID);
			
	}
}

void* AssemblyTask(void *arg){
	if (!isEmpty(assembly_queue) && current_task_ID == assembly_queue->head->data.ID){
			printf("Assembling...\n");
			pthread_mutex_lock(&assembly_mut);

			Task ret = Dequeue(assembly_queue);
            		ret.responsible= "B";
			pthread_sleep(2); // assembly time 2 sec
			printf("assembled: %d\n", ret.ID);
			
            		incrementType5CondCounter(ret);  //increment flag for type5 packaging
            		
            		printf("incremented  in assembly\n");

			pthread_mutex_unlock(&assembly_mut);
			printLog(&ret);
			incrementCurrentTask();
			
			while (ret.giftType == 5){
				pthread_mutex_lock(&type5_package_cond_mut);
				//printf("waits for type5 finish.. %d\n", type5_package_cond);
				int now_type5_cond = type5_package_cond;
				//pthread_sleep(1); 
				pthread_mutex_unlock(&type5_package_cond_mut);
				if ((now_type5_cond % 2) == 0 && now_type5_cond != 0) break; //for threads to finish then proceed
		
			}

   			 addPackageQueue(ret.giftType, ret.giftID);
			
	}
	
}

void* QATask(void *arg){
	if (!isEmpty(QA_queue) && current_task_ID == QA_queue->head->data.ID){
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
		incrementCurrentTask();
	}

}

void* addPackageQueue(int giftType, int giftID){ //hangi typedan geliyo? 4-> (Veya 5se) cond variableını oku->uygunsa ekle

    	printf("type4_package_cond %d\n", type4_package_cond);

	
	Task* t = createDownTask(giftType, giftID, "C");
        pthread_mutex_lock(&packaging_mut);
	Enqueue(packaging_queue, *t);
	pthread_mutex_unlock(&packaging_mut); 

  	
}

void* addPackageQueueType1(void *arg){ 
	int giftType = *((int *) arg);
	Task* t = createTask(giftType, "C");
	pthread_mutex_lock(&packaging_mut);
	Enqueue(packaging_queue, *t);
	pthread_mutex_unlock(&packaging_mut); 
	pthread_exit(0); 
	}


void* addDeliveryQueue(int giftType, int giftID){
	//int giftType = *((int *) arg);
	//free(arg);
        Task* t = createDownTask(giftType, giftID, "D");
        pthread_mutex_lock(&delivery_mut);
	Enqueue(delivery_queue, *t);
	pthread_mutex_unlock(&delivery_mut); 
	//pthread_exit(0);
	
        //addQueue(t, delivery_queue, delivery_mut);	
	
}

void* addPaintQueue(void *arg){  //Task no 3
	int giftType = *((int *) arg);
	//free(arg);
        Task* t = createTask(giftType, "P");
        pthread_mutex_lock(&paint_mut);
	Enqueue(paint_queue, *t);
	pthread_mutex_unlock(&paint_mut); 
	pthread_exit(0);
        //addQueue(t, paint_queue, paint_mut);	
	
}

void* addAssemblyQueue(void *arg){ //Task no 4
	int giftType = *((int *) arg);
	//free(arg);
	
        Task* t = createTask(giftType, "A");
        pthread_mutex_lock(&assembly_mut);
	Enqueue(assembly_queue, *t);
	//free(t);
	pthread_mutex_unlock(&assembly_mut); 
	pthread_exit(0);
        //addQueue(t, assembly_queue, assembly_mut);	
}

void* addQAQueue(void *arg){ 
	int giftType = *((int *) arg);

        Task* t = createTask(giftType, "Q");
        pthread_mutex_lock(&QA_mut);
	Enqueue(QA_queue, *t);
	pthread_mutex_unlock(&QA_mut); 
	pthread_exit(0);
        
      
}

void incrementType4CondCounter(Task t){
	if (t.giftType == 4){
		pthread_mutex_lock(&type4_package_cond_mut);
		type4_package_cond++;
		pthread_mutex_unlock(&type4_package_cond_mut);
	}
}

void incrementType5CondCounter(Task t){
	if (t.giftType == 5){
		pthread_mutex_lock(&type5_package_cond_mut);
		type5_package_cond++;
		pthread_mutex_unlock(&type5_package_cond_mut);
	}
}

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

Task* createDownTask(int giftType, int giftID, char* taskType){
	Task *t = (Task *) malloc(sizeof(Task));
	pthread_mutex_lock(&taskCount_mut);
        task_count++;
        printf("task count:  %d\n", task_count);
        pthread_mutex_unlock(&taskCount_mut);
        t->ID = task_count;
        t->type = taskType; //TODO
        t->giftType = giftType;
        t->requestTime = (int) passedTime();
       
        t->giftID = giftID;
        
        return t;
        }


void printLog(Task* t){
	printf("print CALLED\n");
	pthread_mutex_lock(&log_mut);
	FILE *f = fopen("events.log", "a");
	fprintf(f, "%d\t%d\t%d\t\t%s\t\t\t%d\t\t%s\n", t->ID, t->giftID, t->giftType, t->type, t->requestTime, t->responsible);
	pthread_mutex_unlock(&log_mut);
	fclose(f);
}

void incrementCurrentTask(){
	pthread_mutex_lock(&current_task_ID_mut);
	current_task_ID++;
	pthread_mutex_unlock(&current_task_ID_mut);
}
double passedTime(){
     return time(NULL) % startTime;
}

