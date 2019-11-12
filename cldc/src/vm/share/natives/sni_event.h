/*
 *   
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
 */

#ifndef _JVM_CONFIG_H_
#include "jvmconfig.h"
#endif

#ifndef _SNIEVT_H_
#define _SNIEVT_H_

#ifndef _JAVASOFT_KNI_H_
#include "kni.h"
#endif

#ifndef _JVMSPI_H_
#include "jvmspi.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** The type of signal for which a native thread is waiting. */
typedef enum SNIsignalType {
    NO_SIGNAL = 0,
    NETWORK_READ_SIGNAL,
    NETWORK_WRITE_SIGNAL,
    NETWORK_EXCEPTION_SIGNAL,
    COMM_OPEN_SIGNAL,
    COMM_READ_SIGNAL,
    COMM_WRITE_SIGNAL,
    COMM_CLOSE_SIGNAL,
    DEBUG_SIGNAL,
    VM_DEBUG_SIGNAL,
    HOST_NAME_LOOKUP_SIGNAL,
    SECURITY_CHECK_SIGNAL,
    WMA_SMS_READ_SIGNAL,
    WMA_SMS_WRITE_SIGNAL,
    PUSH_SIGNAL,
    NETWORK_UP_SIGNAL,
    NETWORK_DOWN_SIGNAL,
    GPIO_PIN_INPUT_SIGNAL,
    KEY_SIGNAL,
    BN_CALC_COMP_SIGNAL,
    MEDIA_SIGNAL,
    MEDIA_PREPARE_SIGNAL,
    MEDIA_READ_SIGNAL,
    MEDIA_WRITE_SIGNAL,
} SNIsignalType;

/**
 * Structure to hold contextual information across thread blocking, awakening, 
 * and native method reinvocation.
 * 
 * This structure is used at three different times:
 * 
 * (1) It is populated at the time the thread is blocked.
 *
 * (2) It is read at the time an event occurs and the notifying code wishes to 
 * awaken one or more threads. 
 *
 * (3) It is available to the native method when it is reinvoked.
 *
 * The waitingFor field defines the namespace within which descriptor values 
 * reside. When searching for threads to be signaled, both the waitingFor and 
 * descriptor fields must match the arguments to the signal call.
 *
 * The status field can be set at time (2) by the notifying code in order to 
 * pass status to the reinvoked native method at time (3).
 *
 * The pResult field can be set at time (1) for use at time (3). The notifying
 * code at time (2) should not set or get this field. The reason is that
 * pResult may require native memory allocation, and notification may occur on
 * zero or many threads, which makes dealing with native memory allocation 
 * difficult.
 */
typedef struct _SNIReentryData {
    SNIsignalType waitingFor;   /**< type of signal */
    int descriptor;              /**< platform specific handle */
    int status;                  /**< error code produced by the operation
                                      that unblocked the thread */
    void* pContext;               /**< platform specific context info used
                                       by thread wait functions etc. */
} SNIReentryData;


/**
 * A type for a request to schedule VM time slice.
 */
typedef void (*VmThreadTimesliceProc)(void);


/**
 * Sets the routine for implementation-specific request for a VM time slice.
 * This routine will be called every time after a VM thread is unblocked.
 * If NULL is passed as an argument, no requests will be done.
 *
 * @param p routine to request for a VM time slice
 */
void SNIEVT_set_timeslice_proc(VmThreadTimesliceProc p);

/**
 * Blocks the current Java thread. The SNIReentryData block for 
 * the current Java thread is set to the passed values.
 *
 * @param waitingFor set into SNIReentryData.waitingFor
 * @param descriptor set into SNIReentryData.descriptor
 * @param pContext set into SNIReentryData.pContext
 */
void SNIEVT_wait(
        SNIsignalType waitingFor, int descriptor, void* pContext);

/**
 * Finds and unblocks all Java threads based on what the thread is waiting 
 * for and which descriptor it is waiting on. This queries the VM for the list 
 * of blocked threads.
 * 
 * @param waitingFor used to match SNIReentryData.waitingFor
 * @param descriptor used to match SNIReentryData.descriptor
 * @param status the value stored into SNIReentryData.status for every 
 *               thread that is unblocked
 */
void SNIEVT_signal(SNIsignalType waitingFor, int descriptor, int status);

/**
 * Finds and unblocks all Java threads based on what the thread is waiting 
 * for and which descriptor it is waiting on. The list of threads to be 
 * searched is passed as a pair of parameters.
 * 
 * @param blocked_threads list of blocked threads
 * @param blocked_threads_count number of blocked threads in the list
 * @param waitingFor used to match SNIReentryData.waitingFor
 * @param descriptor used to match SNIReentryData.descriptor
 * @param status the value stored into SNIReentryData.status for every 
 *               thread that is unblocked
 */
void SNIEVT_signal_list(
        JVMSPI_BlockedThreadInfo *blocked_threads,
        int blocked_threads_count, SNIsignalType waitingFor,
        int descriptor, int status);

/**
 * A internal function that unblocks the given Java thread. This should 
 * be called in preference to calling SNI_UnblockThread directly, since 
 * depending upon how the VM is scheduled (master mode or slave mode) 
 * additional work may need to be done. In particular, in slave mode, this 
 * function will need to arrange for a VM time slice to occur quickly.
 *
 * @param thr the Java thread to unblock
 */
void SNIEVT_unblock(JVMSPI_ThreadID thr);


#ifdef __cplusplus
}
#endif

#endif

