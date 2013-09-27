/**
 * Shane Girish (sg12g09)
 * Zacharias Markakis (zm1g09)
 */
 
#include "mymalloc.h"
#include <pthread.h>
//If a block points to NO_NEXT_FLAG it denotes that it does not point to another block
//in memory, and that it is the last block.
#define NO_NEXT_FLAG -1
/**
 * Initializes the given array to the required format.
 * Header information will be kept at the start of the array for the length of the array.
 * The function will also initialise a block which will point to the first block
 * that mymalloc allocates (NO_NEXT_FLAG at initialisation)
 * Every block of the array after that will be initialized to 0, which denotes a free block.
 */
int myinit(int *array, int size) {
	
	// Number of "int"s in 'pthread_mutex_t'
	int sizeOfMutex = sizeof(pthread_mutex_t)/sizeof(int);
    
    // The initialised array's header information needs 3 ints + sizeOfMutex minimum
    if(size > (3 + sizeOfMutex)) {
        array[0] = size;
		
		// The offset to the index of the very first block
		array[1] = sizeOfMutex;
		
		// The mutex lock is intialized
        pthread_mutex_init((pthread_mutex_t *)&array[2], NULL);
		
        // Stores the index of the very first block that will be allocated, NO_NEXT_FLAG at initialisation
        array[2 + sizeOfMutex] = NO_NEXT_FLAG;
		
        // Array is initialized to 0
        int i = 3 + sizeOfMutex, value = 0;
        for( ; i < size ; i++) {
            array[i] = value;
        }
        
        return 1;
    } else {
       // If the size is not greater than 4 there is not enough space to store the header + first block information.
        return 0;
    }
}
/**
 * Checks whether the given array is in the initialised format by checking if the header information
 * is correct, and whether all blocks in the array are free.
 */
int mydispose(int *array) {
    // If pointer to the very first block isn't 'NO_NEXT_FLAG'
    // then the array has allocated blocks in it
    if(array[2 + array[1]] != NO_NEXT_FLAG) return 0;
    
    // Check that all blocks except header information are free.
    int i = 3 + array[1];
    for( ; i < array[0] ; i++) {
        if(array[i] != 0) {
            // Array isn't initialized properly
            return 0;
        }
    }
    
    return 1;
}
/**
 * Traverses through the linked list of blocks in the array to find the block that needs to be freed.
 * Frees the block if successfully found and returns 1, otherwise returns 0 if the block to be freed is not found.
 */ 
int myfree(int *array, int * block) {
	
	//The mutex is used to lock the whole procedure. It will be unlocked right before the procedure ends.
	pthread_mutex_lock((pthread_mutex_t *)&array[2]);
    // previousBlock and currentBlock hold indexes of the array.
    // At initialisation previousBlock holds the index to header information about first block
    // and currentBlock holds the index to the first block
    int j, previousBlock = 2 + array[1], currentBlock = array[previousBlock];
    
    while(currentBlock != NO_NEXT_FLAG) {
        if(&array[currentBlock + 2] == block) { // currentBlock + 2 is the location of the allocated memory, check comments to mymalloc
            array[previousBlock] = array[currentBlock];
            for(j = 0 ; j < (array[currentBlock + 1] + 2) ; j++) {
                array[currentBlock + j] = 0; //Free the block
            }
			
			pthread_mutex_unlock((pthread_mutex_t *)&array[2]);
			
            return 1;
        }
        
        previousBlock = currentBlock;
        currentBlock = array[currentBlock];
    }
    
	pthread_mutex_unlock((pthread_mutex_t *)&array[2]);
	
    return 0;
}
/**
 * Attempts to find a free block inside the array that is big enough to hold 'size' + 2, and returns a pointer
 * to that block. Implementation of a linked list of blocks inside the array, using a worst-fit algorithm to allocate. 
 * Every allocated block is divided into 3 parts: 
 * The first part stores the pointer to the next block in memory
 * The second part stores the size of the allocated memory for each block (which doesns't include the size of each block's header information)
 * The third part contains the actual memory that is allocated.
 */
int * mymalloc(int *array, int size) {
	//The mutex is used to lock the whole procedure. It will be unlocked right before the procedure ends.
	pthread_mutex_lock((pthread_mutex_t *)&array[2]);
    // Return failure (null) if an invalid size is passed
    if(size < 1) return (int *)0;
    
    // 'blockSize' is the size of the required memory to allocate + the header information
    // 'result' stores the result of the function. Will be an address if this call was successful, null otherwise
    // 'currentBlock' stores the index to the start of the current block that we are working on.
    // 'nextBlock' stores the index to the block right after 'currentBlock'. Value is 'NO_NEXT_FLAG' if 'currentBlock' is the last block.
    // 'newBlock' stores the index to the new block that we are trying to create for the given 'size'
    // 'availaleSpace' stores the space between the 'currentBlock' and 'nextBlock'
    int blockSize = size + 2, *result = (int *)0, currentBlock = 2 + array[1], nextBlock = NO_NEXT_FLAG, newBlock = 0, availableSpace = 0;
	
	int selectedCurrent, selectedNew, selectedNext, selectedAvailableSpace = 0;
    
    while(currentBlock != NO_NEXT_FLAG) {
        // Calculate the index of 'newBlock' according to 'currentBlock'
        // If 'currentBlock' is (2 + array[1]) (which is the index to the array's header information for the first block), then 'newBlock' should be placed at (3 + array[1])
        // else at the index right after the 'currentBlock'
        newBlock = (currentBlock == (2 + array[1]))?(3 + array[1]):(currentBlock + 2 + array[currentBlock + 1]);
        
        // Get the index of the block right after 'currentBlock'
        nextBlock = array[currentBlock]; 
        
        // Calculate the space available to insert the 'newBlock' between 'currentBlock' and 'nextBlock'
        availableSpace = ((nextBlock == NO_NEXT_FLAG)?array[0]:nextBlock) - currentBlock - (array[currentBlock + 1] + 2);
        
        if(availableSpace < blockSize) {
            if(nextBlock == NO_NEXT_FLAG){
				if(selectedAvailableSpace == 0) {
				
					pthread_mutex_unlock((pthread_mutex_t *)&array[2]);
				
					return result;
				}
                break;
            } else {
                currentBlock = nextBlock;
                continue;
            }
        }
        
		// Picks the biggect available space to store the block.
        if(selectedAvailableSpace > availableSpace || selectedAvailableSpace == 0) {
			selectedCurrent = currentBlock;
			selectedNew = newBlock;
			selectedNext = nextBlock;
			selectedAvailableSpace = availableSpace;
		}
		
		currentBlock = nextBlock;
    }
    
	array[selectedCurrent] = selectedNew;
	array[selectedNew] = selectedNext;
	array[selectedNew + 1] = size;
	
	result = &array[selectedNew + 2];
	
	pthread_mutex_unlock((pthread_mutex_t *)&array[2]);
	
    return result;
}

