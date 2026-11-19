/*
 *
 * Copyright  1990-2007 Sun Microsystems, Inc. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt).
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 or visit www.sun.com if you need additional
 * information or have any questions.
 */

/**
 * @file
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>


#include "javacall_memory.h"



/**
 * Allocates large memory heap
 * VM will use this memory heap for internal memory allocation/deallocation
 * Will be called ONCE during VM startup!
 *
 * The VM stores JIT-compiled code inside this heap, so it must be
 * executable. malloc() memory is not executable when DEP is enforced,
 * which makes the VM crash as soon as it jumps into compiled code.
 *
 * @param    size required heap size in bytes
 * @param    outSize actual size of memory allocated
 * @return  a pointer to the newly allocated memory,
 *          or <tt>NULL</tt> if not available
 */
void* javacall_memory_heap_allocate(long size, long* outSize) {

    void *ptr = VirtualAlloc(NULL, (SIZE_T)size, MEM_COMMIT | MEM_RESERVE,
                             PAGE_EXECUTE_READWRITE);
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

    if (heap != NULL) {
        VirtualFree(heap, 0, MEM_RELEASE);
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
