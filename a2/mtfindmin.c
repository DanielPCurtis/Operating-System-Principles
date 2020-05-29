/*
 *  CSC 139
 *  Fall 2019
 *  Second Assignment
 *  Curtis, Daniel
 *  Section 02
 *  OS Tested on: Linux and ...
 *
 *  Architecture:          x86_64
 *  CPU op-mode(s):        32-bit, 64-bit
 *  Byte Order:            Little Endian
 *  CPU(s):                8
 *  On-line CPU(s) list:   0-7
 *  Thread(s) per core:    2
 *  Core(s) per socket:    4
 *  Socket(s):             1
 *  NUMA node(s):          1
 *  Vendor ID:             GenuineIntel
 *  CPU family:            6
 *  Model:                 94
 *  Model name:            Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
 *  Stepping:              3
 *  CPU MHz:               800.000
 *  BogoMIPS:              6816.00
 *  Virtualization:        VT-x
 *  L1d cache:             32K
 *  L1i cache:             32K
 *  L2 cache:              256K
 *  L3 cache:              8192K
 *  NUMA node0 CPU(s):     0-7
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/timeb.h>
#include <semaphore.h>

#define MAX_SIZE 100000000
#define MAX_THREADS 16
#define RANDOM_SEED 7665
#define MAX_RANDOM_NUMBER 5000

// Global variables
long gRefTime; //For timing
int gData[MAX_SIZE]; //The array that will hold the data

int gThreadCount; //Number of threads
int gDoneThreadCount; //Number of threads that are done at a certain point. Whenever a thread is done, it increments this. Used with the semaphore-based solution
int gThreadMin[MAX_THREADS]; //The minimum value found by each thread
bool gThreadDone[MAX_THREADS]; //Is this thread done? Used when the parent is continually checking on child threads

// Semaphores
sem_t completed; //To notify parent that all threads have completed or one of them found a zero
sem_t mutex; //Binary semaphore to protect the shared variable gDoneThreadCount

int SqFindMin(int size); //Sequential FindMin (no threads)
void *ThFindMin(void *param); //Thread FindMin but without semaphores 
void *ThFindMinWithSemaphore(void *param); //Thread FindMin with semaphores 
int SearchThreadMin(); // Search all thread minima to find the minimum value found in all threads 
void InitSharedVars();
void GenerateInput(int size, int indexForZero); //Generate the input array 
void CalculateIndices(int arraySize, int thrdCnt, int indices[MAX_THREADS][3]); //Calculate the indices to divide the array into T divisions, one division per thread
int GetRand(int min, int max);//Get a random number between min and max

//Timing functions
long GetMilliSecondTime(struct timeb timeBuf);
long GetCurrentTime(void);
void SetTime(void);
long GetTime(void);

int main(int argc, char *argv[]){

	pthread_t tid[MAX_THREADS];  
	pthread_attr_t attr[MAX_THREADS];
	int indices[MAX_THREADS][3];
	int i, indexForZero, arraySize, min;

	// Code for parsing and checking command-line arguments
	if(argc != 4){
		fprintf(stderr, "Invalid number of arguments!\n");
		exit(-1);
	}
	if((arraySize = atoi(argv[1])) <= 0 || arraySize > MAX_SIZE){
		fprintf(stderr, "Invalid Array Size\n");
		exit(-1);				
	}
	gThreadCount = atoi(argv[2]);				
	if(gThreadCount > MAX_THREADS || gThreadCount <=0){
		fprintf(stderr, "Invalid Thread Count\n");
		exit(-1);				
	}
	indexForZero = atoi(argv[3]);
	if(indexForZero < -1 || indexForZero >= arraySize){
		fprintf(stderr, "Invalid index for zero!\n");
		exit(-1);
	}

    GenerateInput(arraySize, indexForZero);

    CalculateIndices(arraySize, gThreadCount, indices); 

	// Code for the sequential part
	SetTime();
	min = SqFindMin(arraySize);
	printf("Sequential search completed in %ld ms. Min = %d\n", GetTime(), min);
	

	// Threaded with parent waiting for all child threads
	InitSharedVars();
	SetTime();

	// Write your code here
	// Initialize threads, create threads, and then let the parent wait for all threads using pthread_join
	// The thread start function is ThFindMin
	// Don't forget to properly initialize shared variables 
    
    for(int i=0; i<gThreadCount; i++) {
        pthread_attr_init(&attr[i]);
        pthread_create(&tid[i], &attr[i], ThFindMin, &indices[i]); 
	}

    for(int i=0; i<gThreadCount; i++)   {
        pthread_join(tid[i], NULL);
    }
	
    min = SearchThreadMin();
	printf("Threaded FindMin with parent waiting for all children completed in %ld ms. Min = %d\n", GetTime(), min);

	// Multi-threaded with busy waiting (parent continually checking on child threads without using semaphores)
	InitSharedVars();
	SetTime();

	// Write your code here
    // Don't use any semaphores in this part	
	// Initialize threads, create threads, and then make the parent continually check on all child threads
	// The thread start function is ThFindMin
	// Don't forget to properly initialize shared variables 
    volatile int found = 0;

    for(int j=0; j<gThreadCount; j++) {
        pthread_attr_init(&attr[j]);
        pthread_create(&tid[j], &attr[j], ThFindMin, &indices[j]);
    }
    
    while(found < gThreadCount) {
       found = 0;
       for(int k=0; k<gThreadCount; k++)    {
            if(gThreadDone[k]) {
                if(gThreadMin[k] == 0)  {
                    found=gThreadCount;
		            break;           
                }
		        found++;
           }
        }
    }
    for(int i=0; i<gThreadCount; i++)    {
        pthread_cancel(tid[i]);
    }
    min = SearchThreadMin();
	printf("Threaded FindMin with parent continually checking on children completed in %ld ms. Min = %d\n", GetTime(), min);
	

	// Multi-threaded with semaphores  

	InitSharedVars();
    // Initialize your semaphores here  
    sem_init(&completed,0,0);
    sem_init(&mutex,0,1);
	SetTime();

    // Write your code here
	// Initialize threads, create threads, and then make the parent wait on the "completed" semaphore 
	// The thread start function is ThFindMinWithSemaphore
	// Don't forget to properly initialize shared variables and semaphores using sem_init 
    
    for(int i=0; i<gThreadCount; i++)   {
        pthread_attr_init(&attr[i]);
        pthread_create(&tid[i], &attr[i], ThFindMinWithSemaphore, &indices[i]);
    }

    sem_wait(&completed);

    for(int j=0; j<gThreadCount; j++)   {
        pthread_cancel(tid[j]);
    }
	
	min = SearchThreadMin();
	printf("Threaded FindMin with parent waiting on a semaphore completed in %ld ms. Min = %d\n", GetTime(), min);
}


// Write a regular sequential function to search for the minimum value in the array gData
int SqFindMin(int size) {
    int count = 0;
    int min = gData[0];

    while(count < size) {
        if(gData[count] == 0)  {
            min = 0;
            return min;
        }
        else if(min > gData[count]) {
            min = gData[count];
            count++;
        }
        else    {
            count++;
        }
        
    }
    return min;
}

// Write a thread function that searches for the minimum value in one division of the array
// When it is done, this function should put the minimum in gThreadMin[threadNum] and set gThreadDone[threadNum] to true    
void* ThFindMin(void *param) {
	int threadNum = ((int*)param)[0];               //threadNum init to 

    int startIndex = ((int*)param)[1];
    int endIndex = ((int*)param)[2];
    
    for(int i=startIndex; i<endIndex; i++)  {
        if(gData[i] == 0)   {
                gThreadMin[threadNum] = 0;
                break;
        }
        else if(gThreadMin[threadNum] > gData[i])   {
            gThreadMin[threadNum] = gData[i];
        }
    }
    gThreadDone[threadNum] = true;
    pthread_exit(NULL);
}

// Write a thread function that searches for the minimum value in one division of the array
// When it is done, this function should put the minimum in gThreadMin[threadNum]
// If the minimum value in this division is zero, this function should post the "completed" semaphore
// If the minimum value in this division is not zero, this function should increment gDoneThreadCount and
// post the "completed" semaphore if it is the last thread to be done
// Don't forget to protect access to gDoneThreadCount with the "mutex" semaphore     
void* ThFindMinWithSemaphore(void *param) {
    int threadNum = ((int*)param)[0];
    int startIndex = ((int*)param)[1];
    int endIndex = ((int*)param)[2];
    int i=startIndex;
    while(i<=endIndex)    {
               
        if(gData[i] == 0)  {
            gThreadMin[threadNum] = 0;
            sem_post(&completed);
            i=endIndex;
            break;
        }
        else if(gThreadMin[threadNum] > gData[i])    {
            gThreadMin[threadNum] = gData[i]; 
        }
        i++;
    }
    gThreadDone[threadNum] = true;
 
    sem_wait(&mutex);
    //crit section
    gDoneThreadCount++;
    if(gDoneThreadCount == gThreadCount)    {
            sem_post(&completed);
    }
    //end crit section
    sem_post(&mutex);
    pthread_exit(NULL);
}

int SearchThreadMin() {
    int i, min = MAX_RANDOM_NUMBER + 1;
	
    for(i =0; i<gThreadCount; i++) {
                if(gThreadMin[i] == 0)
                        return 0;
		if(gThreadDone[i] == true && gThreadMin[i] < min)
			min = gThreadMin[i];
	}
	return min;
}

void InitSharedVars() {
     int i;
	 
	for(i=0; i<gThreadCount; i++){
        gThreadDone[i] = false;	
        gThreadMin[i] = MAX_RANDOM_NUMBER + 1;	
	}
    gDoneThreadCount = 0;
}

// Write a function that fills the gData array with random numbers between 1 and MAX_RANDOM_NUMBER
// If indexForZero is valid and non-negative, set the value at that index to zero 
void GenerateInput(int size, int indexForZero) {
        
        for(int i = 0; i < size; i++)   {
            if(i == indexForZero)   {
                gData[i] = 0;
            }
            else    {
                gData[i] = GetRand(1,MAX_RANDOM_NUMBER);
            }
        }
}

// Write a function that calculates the right indices to divide the array into thrdCnt equal divisions
// For each division i, indices[i][0] should be set to the division number i,
// indices[i][1] should be set to the start index, and indices[i][2] should be set to the end index 
void CalculateIndices(int arraySize, int thrdCnt, int indices[MAX_THREADS][3]) {
    int newSize = arraySize/thrdCnt;
    int startIndex = 0;
    int endIndex = newSize-1;

    for(int j=0; j < thrdCnt; j++) {
        indices[j][0] = j;
        indices[j][1] = startIndex;
        indices[j][2] = endIndex;
	
        startIndex+=newSize;
        endIndex+=newSize;
    }
}

// Get a random number in the range [x, y]
int GetRand(int x, int y) {
    int r = rand();
    r = x + r % (y-x+1);
    return r;
}

long GetMilliSecondTime(struct timeb timeBuf){
	long mliScndTime;
	mliScndTime = timeBuf.time;
	mliScndTime *= 1000;
	mliScndTime += timeBuf.millitm;
	return mliScndTime;
}

long GetCurrentTime(void){
	long crntTime=0;
	struct timeb timeBuf;
	ftime(&timeBuf);
	crntTime = GetMilliSecondTime(timeBuf);
	return crntTime;
}

void SetTime(void){
	gRefTime = GetCurrentTime();
}

long GetTime(void){
	long crntTime = GetCurrentTime();
	return (crntTime - gRefTime);
}

