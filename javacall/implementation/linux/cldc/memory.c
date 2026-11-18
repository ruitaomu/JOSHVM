/*
 * Copyright (C) Max Mu
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * Please visit www.joshvm.org if you need additional information or
 * have any questions.
 */
#ifdef __cplusplus
extern "C" {
#endif
    
    
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>


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
/* The Java heap also holds JIT-compiled code, so it must be executable;
 * malloc memory is not on current kernels. The heap is allocated once per
 * VM run, so its size is kept in a static for munmap. */
static long heap_map_size = 0;

void* javacall_memory_heap_allocate(long size, long* outSize) {

    void *ptr = mmap(NULL, (size_t)size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (ptr == MAP_FAILED){
        *outSize = 0;
        return NULL;
    }
    heap_map_size = size;
    *outSize = size;
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

    if (heap != NULL) {
        munmap(heap, (size_t)heap_map_size);
    }
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

 
