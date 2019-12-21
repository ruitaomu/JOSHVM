#ifdef __cplusplus
extern "C" {
#endif
    
#include <esp_heap_caps.h>
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

    ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
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

    heap_caps_free(heap);
}

/**
 * Allocates memory of the given size from the private JAVACALL memory
 * pool.
 *
 * @param size Number of byte to allocate
 * @return a pointer to the newly allocated memory
 */
void* javacall_malloc(unsigned int size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

/**
 * Frees memory at the given pointer in the private JAVACALL memory pool.
 *
 * @param ptr pointer to allocated memory
 */
void  javacall_free(void* ptr) {
    heap_caps_free(ptr);
}
    
#ifdef __cplusplus
}
#endif

 
