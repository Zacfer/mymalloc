#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "mymalloc.h"

#define DEBUG 0

static int block[SIZE];
pthread_t threads[THREADS];
int i;
 
 
void *mem_ops(void *threadNo) {
	int * pointers[OPS];
	int sizes[OPS];
	int count=0;
	int allocated=0;
	int size;
	int no_of_allocs=0;
	int no_of_frees=0;
	clock_t start = clock();
	int i;
			
	//printf("Thread %d at your service!\n",*(int *)threadNo);
	
	while (count<OPS) {
			/* while there's space, let's try to eat some */
			size = rand() % (1 << (rand() % 12)) + 1;
			int * fp = mymalloc(block, size); 
			if (fp != (int*) 0) {
				pointers[allocated]=fp;
				sizes[allocated]=size;
				if (DEBUG) printf("allocated a block of size %d from thread %d\n", size, *(int *)threadNo);
				for (i = 0; i < size; i++) fp[i] = -(i + 42);
				no_of_allocs++;
				allocated++;
				count++;
			} else {
				/* no more space, lets free a block if we allocated one previously */
				if (allocated > 0) {
					int index = rand() % allocated;	
					for (i = 0; i < sizes[index]; i++) {
						if (pointers[index][i] != -(i + 42)) {
							printf("!!!! Heap corrupted !!!!\n");
							exit(EXIT_FAILURE);
						}
					}
					if (DEBUG) printf("freeing a block of size %d from thread %d\n", sizes[index], *(int *)threadNo);
					if (myfree(block, pointers[index]) == 0)
						exit(EXIT_FAILURE);
					while (++index < allocated) {
						pointers[index - 1] = pointers[index];
						sizes[index - 1] = sizes[index];
					}
					no_of_frees++;
					allocated--;
					count++;
				}
		}
	}
	
	/* cleaning up */
	int j;
	for (i=0; i<allocated; i++) {
			for (j=0; j<sizes[i]; j++) {
				if (pointers[i][j] != -(j + 42)) {
					printf("!!!! Heap corrupted !!!!\n");
					exit(EXIT_FAILURE);
				}
			}
		if (DEBUG) printf("freeing a block of size %d from thread %d\n", sizes[i], *(int *)threadNo);
		if (myfree(block, pointers[i]) == 0)
						exit(EXIT_FAILURE);	
		no_of_frees++;
	}
	
	printf("Thread %d exits: %d allocs and %d frees in %fs\n",
		*(int *)threadNo, no_of_allocs, no_of_frees,
			(( (double) clock() - start) / CLOCKS_PER_SEC) );

	pthread_exit(NULL);
}

 
int main() {
	printf("seed: %d\n",SEED);
		
	srand(SEED);
	
	if (myinit(block, SIZE) == 0)
		exit(EXIT_FAILURE);
 
	for (i=0;i<THREADS;i++) {  
	  int *threadId = (int *)malloc(sizeof(int));
	  *threadId = i;
      (void)pthread_create(&threads[i],NULL,mem_ops,threadId);
    }
	pthread_exit(NULL);
}
