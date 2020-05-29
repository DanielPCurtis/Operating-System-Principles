/*
 *  CSC 139
 *  Section 02
 *  Fall 2019
 *  Curtis, Daniel
 *  Fourth Assignment
 *  Tested on Linux/Athena
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define INPUT_FILE "test5.txt"
#define OUTPUT_FILE "output.txt"
#define MAX_REQ 1000


//GLOBAL VARS
char *line, *holder;
size_t sz;
int numPages, numFrames, numPageReq;
int pageArray[MAX_REQ];
//END VARS


int FindIndex(int [], int, int, int);
int FarthestAway(int [], int, int);
int Closest(int [], int, int);


void FIFO(FILE *out) {
    int index;
    int queue[MAX_REQ];
    int totalFaults =0;
    int counter = 0;
    int queue_sz = 0;
    int frontQ = 0;
    int rearQ = 0;
    fprintf(out, "First In First Out\n");
    while(counter < numPageReq) {  
        index = FindIndex(queue, pageArray[counter], frontQ, rearQ) % numFrames;
        if(index == -1) {
            if(queue_sz == numFrames) {
                int removePage = queue[frontQ];
                index = FindIndex(queue, removePage, frontQ, rearQ) % numFrames;
                queue[frontQ++];
                queue_sz--;
                fprintf(out, " Page %d unloaded from Frame %d , ", removePage, index);
            }
            queue[rearQ++] = pageArray[counter];
            index = FindIndex(queue, pageArray[counter], frontQ, rearQ) % numFrames;
            fprintf(out, " Page %d loaded into Frame %d\n", pageArray[counter], index);  
            counter++;
            queue_sz++;
            totalFaults++;
        }
        else    {
            
            fprintf(out, " Page %d already in Frame %d \n", pageArray[counter], FindIndex(queue, pageArray[counter], frontQ, rearQ) % numFrames);
            counter++;
        }  
    }
    fprintf(out, "%d page faults\n", totalFaults);
}

void OPTIMAL(FILE *out)  {
    int index;
    int optArray[MAX_REQ] = {0};
    int totalFaults =0;
    int counter = 0;
    int optCount = 0;
    fprintf(out, "\n");
    fprintf(out, "Optimal\n");
    while(counter < numPageReq) {
        index = FindIndex(optArray, pageArray[counter], 0, numFrames);
        if(index == -1) {
            if(optCount == numFrames) {
                int current = FarthestAway(optArray, numFrames, counter);
                index = FindIndex(optArray, current, 0, numFrames);
                fprintf(out, "Page %d unloaded from Frame %d, ", current, index);          
                optArray[index] = pageArray[counter];      
                fprintf(out, "Page %d loaded into Frame %d \n", pageArray[counter], index);
            }
            else    {
                optArray[optCount] = pageArray[counter];
                optCount++;
                fprintf(out, "Page %d loaded into Frame %d\n", pageArray[counter], FindIndex(optArray, pageArray[counter], 0, numFrames));
            }
            totalFaults++;
            counter++;
        }
        else    {
           fprintf(out, "Page %d already in Frame %d\n", pageArray[counter], FindIndex(optArray, pageArray[counter], 0, numFrames));
           counter++;     
        }                   
    }
    fprintf(out, "%d page faults\n", totalFaults);
}
 
void LRU(FILE *out)  {
    int index;
    int lruArray[MAX_REQ]={0};
    int totalFaults =0;
    int counter = 0;
    int lruCount = 0; 
    fprintf(out, "\n");
    fprintf(out, "LRU\n");
    while(counter < numPageReq)  {
        index = FindIndex(lruArray, pageArray[counter], 0, numFrames-1);            
        if(index == -1) {
            if(lruCount == numFrames)  {
                int current = Closest(lruArray, numFrames, counter);
                index = FindIndex(lruArray, current, 0, numFrames);
                fprintf(out, "Page %d unloaded from %d, ", current, index);
                lruArray[index] = pageArray[counter];
                fprintf(out, "Page %d loaded into Frame %d \n", pageArray[counter], index);
            }
            else    {
                lruArray[lruCount] = pageArray[counter];
                lruCount++;
                fprintf(out, "Page %d loaded into Frame %d\n", pageArray[counter], FindIndex(lruArray, pageArray[counter], 0, numFrames)); 
            }
            totalFaults++;
            counter++;
        }
        else    {
            fprintf(out, "Page %d already in Frame %d\n", pageArray[counter], FindIndex(lruArray, pageArray[counter], 0, numFrames));
            counter++;
        }
    }
    fprintf(out, "%d page faults\n", totalFaults);
}

int main(int argc, char *argv[])    {
    FILE *fp;
    FILE *o;
    int i, j;
    fp = fopen(INPUT_FILE, "r");
    o = fopen(OUTPUT_FILE, "w");
    if(fp == NULL)  {
            printf("Error opening file, please try again...");
            exit(1);
    }

    getline(&line, &sz, fp);

    sscanf(line, "%d%d%d", &numPages, &numFrames, &numPageReq);

    for(i=0; i<numPageReq; i++) {   
        getline(&holder, &sz, fp);
        pageArray[i] = atoi(holder);
    }

    FIFO(o);
    OPTIMAL(o);
    LRU(o);

    fclose(fp);
    fclose(o);
}

int FindIndex(int arr[], int findVal, int start, int end) {
    int i;
    for(i=start; i<=end; i++)  {
        if(arr[i] == findVal) {
            return i;
        }
    }
    return -1;
}

int FarthestAway(int optArray[], int count, int soFar)  {
    int i, j, k;
    int result[numFrames]; 
    for(i=0; i<count;  i++)    {
        for(j=soFar; j<numPageReq; j++) {
            if(optArray[i] == pageArray[j]) {
                result[i] = j;
                break;  
            }
            result[i] = -1;
        }
    }
    int min = result[0];
    for(k=1; k<numFrames; k++) {
        if(min == -1)   {   
            min = optArray[k-1];
            k = numFrames;
        }
        else if(result[k] > min)  {
            min = optArray[k];
        }
        else if(result[k] == -1)    {
            min = optArray[k];
            k = numFrames;
        } 
    }
    if(min == result[0])  {
        min = optArray[0];
    }
    return min;
}

int Closest(int lruArray[], int count, int soFar)  {
    int i, j, k;
    int result[numFrames];
    int smallest = 0;
    for(i=0; i<numFrames; i++)  {
        for(j=soFar; j>=0; j--)  {
            if(lruArray[i] == pageArray[j]) {
                result[i] = j;
                break;                   
            }
            result[i] = -1;           
        }
    }
    int min = result[0];
    for(k=1; k<numFrames; k++)  {
        if(result[k] < min)    {
            min = result[k];
            smallest = k;    
        }
        else if(result[k] == -1)  {
                smallest = k;
                k = numFrames;
        }
    }
    if(min == result[0])    {
        min = lruArray[0];
    }
    min = lruArray[smallest];         
    
    return min;
}
