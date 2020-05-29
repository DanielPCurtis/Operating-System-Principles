/*
 * CSC 139
 * Fall 2019
 * Third Assignment
 * S=ction 02
 * Tested on Linux/athena
 * All given tests run correctly except for test #15
 * 
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define INPUT_FILE "./input15.txt"
#define OUTPUT_FILE "./output.txt"
#define MAX_PROCESS 1000
#define MAX_COL 7

//GLOBAL VARIABLES//
int procArray[MAX_PROCESS][MAX_COL]={{0,0},{0,0}};
int queue[MAX_PROCESS] = {0};
int frontQ, rearQ, queue_sz, completedTotal, clk, processRunning, procCounter, timeQuantum;
int maxQ = MAX_PROCESS;
char *currAlg, *totalProc, *line, *holder, *temp;   
bool completed;
size_t sz;
//END VARS//



void Sort(int);                 //Sort by column
void AddtoQueue(int);           //Queue and functions
int RemovefromQueue();
void PrintQueue();
void CheckCompletion(int);      //check if completed            
void DeleteQueue();
bool ExistsInQueue(int);
void UpdateQueue();
void UpdateQueueSJF();
void UpdateQueueNoPre();
void UpdateQueuePre();
bool SmallestBurst(int);
float CalcAvgWait();
float CalcAvgWaitRR();
float CalcAvgWaitNoPre();




void RoundRobin()   {
    int currBurst;
    completed = false;
    clk = 0;
    FILE *out = fopen(OUTPUT_FILE, "w");
    DeleteQueue();
    fprintf(out, "%s",currAlg);
    Sort(1);
    UpdateQueue();

    while(!completed)   {
        
        processRunning = RemovefromQueue();
        fprintf(out, "%d  %d\n", clk, processRunning);
        currBurst = procArray[processRunning-1][2];

        if(currBurst <= timeQuantum)  {
            
            procArray[processRunning-1][4] += clk - procArray[processRunning-1][1];
            clk += currBurst;
            procArray[processRunning-1][5] = clk;
            procArray[processRunning-1][2]=0;
            completedTotal++;
            CheckCompletion(completedTotal);
        }
        else if(currBurst > timeQuantum) {
            procArray[processRunning-1][4] += clk - procArray[processRunning-1][1];
            clk += timeQuantum;
            procArray[processRunning-1][1]=clk;
            procArray[processRunning-1][2] -= timeQuantum;
            procArray[processRunning-1][6] = 1;
            UpdateQueue();   
            AddtoQueue(processRunning);
        }
        UpdateQueue();

   }
   fprintf(out, "AVG Wait Time: %0.2f\n", CalcAvgWaitRR());
   fclose(out);                        
}

void ShortestJobFirst() {
    int i, j, currBurst;
    completed = false;
    clk, completedTotal=0;
    FILE *out = fopen(OUTPUT_FILE, "w");
    DeleteQueue();  
    fprintf(out, "%s\n", currAlg);
    UpdateQueueSJF();
    while(!completed)   {
        
        processRunning = RemovefromQueue();
        fprintf(out, "%d  %d\n", clk, processRunning);
        
        for(i=0; i <= procCounter; i++)   {
            if(processRunning == procArray[i][0]) {   
                procArray[i][5] += clk-procArray[i][1];  
                            
                clk += procArray[i][2];  
                procArray[i][2]=0;
                completedTotal++;
                CheckCompletion(completedTotal);
                break;
            }
        }
        UpdateQueueSJF();

    }   
    fprintf(out, "AVG Wait Time: %0.2f\n", CalcAvgWaitNoPre());
    fclose(out);
    
}

void PriorityNoPreEmption() {
     int i;
     completed = false;
     clk, completedTotal=0;
     FILE *out = fopen(OUTPUT_FILE, "w");
     DeleteQueue();
     fprintf(out, "%s \n", currAlg);
     UpdateQueueNoPre();
     while(!completed)  {
       
        processRunning = RemovefromQueue();
        fprintf(out, "%d  %d\n", clk, processRunning);
        for(i=0; i<procCounter; i++) {

            if(processRunning == procArray[i][0])    {
                
                procArray[i][5] = clk - procArray[i][1];
                clk += procArray[i][2];
                procArray[i][2]=0;
                completedTotal++;
                completedTotal++;
                completedTotal++;
                completedTotal++;
                CheckCompletion(completedTotal);
                break;
            }
        }
        UpdateQueueNoPre();
        
               
        
    }
    fprintf(out, "Avg Wait Time: %0.2f\n", CalcAvgWaitNoPre());
    fclose(out);
}

void PriorityPreEmption()   {
    int i, j, k;
    completed = false;
    clk, completedTotal=0;
    FILE *out = fopen(OUTPUT_FILE, "w");
    DeleteQueue();
    fprintf(out, " %s\n", currAlg);
    UpdateQueuePre();
    while(!completed)   {  
        processRunning = RemovefromQueue();
        
        if(processRunning == -1)    {
                clk++;
        }
        else    {
            for(i=0; i<procCounter;i++) {
                if(processRunning == procArray[i][0])   {
                    if(processRunning!=j)   {
                            j=processRunning;
                            fprintf(out, "%d  %d\n", clk, processRunning);
                    }
                    
                    int hold = clk - procArray[i][1];
                    procArray[i][5] += hold;
                    procArray[i][2]-=1;
                    
                    if(procArray[i][2]==0)  {
                            completedTotal++;
                            CheckCompletion(completedTotal);
                    }
                    clk++;
                    procArray[i][1] = clk;
                    break;
                }
            }
        }
        UpdateQueuePre();
        
               
    }
    
    fprintf(out, "Avg Wait Time: %0.2f\n", CalcAvgWaitNoPre());
    fclose(out);        
}

int main(int argc,char *argv[]) {
    FILE *fp;
    fp = fopen(INPUT_FILE, "r");
    if(fp==NULL)    {    
            printf("Error opening File, please try again...");
            exit(1);
    }

    getline(&currAlg, &sz, fp);
    getline(&totalProc, &sz, fp);
    
    procCounter = atoi(totalProc);
    
    int i;
    for(i=0;i<procCounter;i++)  {
        getline(&line, &sz, fp);
        
        sscanf(line, "%d%d%d%d", &procArray[i][0], &procArray[i][1], &procArray[i][2], &procArray[i][3]); 
            
     }

    if(currAlg[0] == 'R')   {
        timeQuantum = (int)currAlg[3]-48;
        
        RoundRobin();   
    }
    else if(currAlg[0] == 'S')  {
        ShortestJobFirst();           
    }
    else if(currAlg[0] == 'P' && currAlg[3] == 'n')  {
        PriorityNoPreEmption();   
    }
    else if(currAlg[0] == 'P' && currAlg[3] == 'w')  {
        PriorityPreEmption();
    }
    else    {
        exit(1);
    }

    fclose(fp); 
    exit(0);
}

void Sort(int sortVal)   {
    int i, j, k, swap;
    for(i=0; i < procCounter; i++)  {
        for(j=i+1; j < procCounter; j++)    {
            if(procArray[i][sortVal] > procArray[j][sortVal])   {
                for(k=0; k<4; k++)    {
                    swap = procArray[i][k];
                    procArray[i][k] = procArray[j][k];
                    procArray[j][k] = swap;
                    
                }
            }
        }
    }
    if(sortVal == 2)    {
        for(i=0; i < procCounter; ++i)  {
            for(j=i+1; j<procCounter; ++j)  {
                    if(procArray[i][sortVal] > procArray[j][sortVal] && procArray[i][1]>procArray[j][1])    {
                                    swap = procArray[i][k];
                                    procArray[i][k] = procArray[j][k];
                                    procArray[j][k] = swap;
                            }
                    }
            }
        }
    }


void AddtoQueue(int processNum)  {
    if(queue_sz != maxQ)  {
        queue[rearQ++] = processNum;
        queue_sz++;
    }
}

int RemovefromQueue()  {
    if(frontQ == rearQ) return -1;
    
    queue_sz--;
    return queue[frontQ++];
}

void PrintQueue()   {
    int i;
    for(i=frontQ; i<rearQ; i++) 
        printf("%d\n", queue[i]);
    
}

void DeleteQueue()  {
    int i;
    for(i=frontQ; i<rearQ; i++)    {
       queue[i]=0; 
    }
    frontQ=rearQ=0;
}

bool ExistsInQueue(int val)    {
    int i;
    for(i=frontQ; i<=rearQ; i++) {
        if(val == queue[i])
            return true;
    }
    return false;
}

void UpdateQueue()  {
    int i;
    for(i=0; i < procCounter; i++)  {
            if(!ExistsInQueue(procArray[i][0]) && procArray[i][1] <= clk  && procArray[i][6] != 1  && procArray[i][2] > 0)
                AddtoQueue(procArray[i][0]); 
    }
}

void UpdateQueueSJF()    {
    int i, j;
    Sort(2);
    for(j=0; j<procCounter; j++)    {
        int dontCare = RemovefromQueue();
    }
    for(i=0; i < procCounter; i++)  {
            if(!ExistsInQueue(procArray[i][0]) && procArray[i][1] <= clk  && procArray[i][2] > 0)  {
                AddtoQueue(procArray[i][0]);
            }
    }
}

void UpdateQueueNoPre() {
    int i;
    Sort(3);
    for(i=0; i<procCounter;i++) {
            if(!ExistsInQueue(procArray[i][0]) && procArray[i][1]<=clk && procArray[i][2] > 0)   {
                AddtoQueue(procArray[i][0]);
            }
    }
}

void UpdateQueuePre()   {
    int i, j;
    Sort(3);
    for(j=0; j<procCounter; j++)   {
           int dontCare = RemovefromQueue(j);
    }
    for(i=0;i<procCounter; i++)    {
            if(!ExistsInQueue(procArray[i][0]) && procArray[i][1] <= clk  && procArray[i][2] > 0)   
               
                AddtoQueue(procArray[i][0]);
    }
}

void CheckCompletion(int asdf)  {
    if(asdf ==  procCounter)    {
        completed = true;
    }
}

float CalcAvgWaitRR() {
    float total= 0;
    int i;
    for(i=0; i<procCounter;i++) {     
        total += procArray[i][4];
    }
    return total/procCounter;
}

float CalcAvgWait() {
    float total = 0;
    int i;
    for(i=0; i<procCounter;i++) {
            total += procArray[i][5] - procArray[i][1];
    }
    return total/procCounter;
}

float CalcAvgWaitNoPre()    {
    float total=0;
    int i;
    for(i=0; i<procCounter; i++)    {
            total += procArray[i][5];
    }
    return total/procCounter;
}
