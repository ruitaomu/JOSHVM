#ifdef __cplusplus
extern "C" {
#endif
    
    
#include <stdio.h>
#include <stdlib.h>


#include "javacall_memory.h"



/**
 * Allocates large memory heap
 * VM will use this memory heap for internal memory allocation/deallocation
 * Will be called ONCE during VM startup!
 *
 * @param    size required heap size in bytes
 * @param    outSize actual size of memory allocated
 * @return  a pointer to the newly allocated memory,
 *          or <tt>NULL</tt> if not available
 */
void* javacall_memory_heap_allocate(long size, long* outSize) {

    void *ptr = NULL;

    ptr = malloc(size);
    if (ptr == NULL){
        *outSize = 0;
    } else {
        *outSize = size;
    }
    return ptr;
}

/**
 * Free large memory heap
 * VM will call this function once when VM is shutdown to free the memory heap
 * Will be called ONCE during VM shutdown!
 *
 * @param    heap memory pointer to free
 */
void javacall_memory_heap_deallocate(void* heap) {

    free(heap);
}

/**
 * Allocates memory of the given size from the private JAVACALL memory
 * pool.
 *
 * @param size Number of byte to allocate
 * @return a pointer to the newly allocated memory
 */
void* javacall_malloc(unsigned int size) {
    return malloc(size);
}

/**
 * Frees memory at the given pointer in the private JAVACALL memory pool.
 *
 * @param ptr pointer to allocated memory
 */
void  javacall_free(void* ptr) {
    free(ptr);
}
    
#ifdef __cplusplus
}
#endif

 
