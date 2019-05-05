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
 */


#include "incls/_precompiled.incl"
#include "incls/_sni_event.cpp.incl"


static VmThreadTimesliceProc vm_thread_timeslice_proc = NULL;

/**
 * Sets the routine for implementation-specific request for a VM time slice.
 * This routine will be called every time after a VM thread is unblocked.
 * If NULL is passed as an argument, no requests will be done.
 *
 * @param p routine to request for a VM time slice
 */
void SNIEVT_set_timeslice_proc(VmThreadTimesliceProc p) {
  vm_thread_timeslice_proc = p;
}


/**
 * Blocks the current Java thread. The SNIReentryData block for 
 * the current Java thread is set to the passed values.
 * 
 * @param waitingFor set into SNIReentryData.waitingFor
 * @param descriptor set into  SNIReentryData.descriptor. If 
 *                   SNIReentryData.descriptor is -1, it means that all
 *                   descripters will be matched
 * @param pContext set into SNIReentryData.pContext
 */
void
SNIEVT_wait(SNIsignalType waitingFor, int descriptor, void* pContext)
{
    SNIReentryData* p = 
        (SNIReentryData*)SNI_GetReentryData(NULL);

    if (p == NULL) {
        p = (SNIReentryData*)
             (SNI_AllocateReentryData(sizeof (SNIReentryData)));
        if (p == NULL) {
            return;
        }
    }

    p->descriptor = descriptor;
    p->waitingFor = waitingFor;
    p->status = 0;
    p->pContext = pContext;

    SNI_BlockThread();
}

/**
 * Find and unblock all Java threads based on what the thread is waiting 
 * for and which descriptor it is waiting on.
 * 
 * @param waitingFor used to match SNIReentryData.waitingFor
 * @param descriptor used to match SNIReentryData.descriptor. If 
 *                   SNIReentryData.descriptor is -1, then it maches
 *                   descriptor anyway
 * @param status the value stored into SNIReentryData.status for every 
 *               thread that is unblocked
 */
void 
SNIEVT_signal(SNIsignalType waitingFor, int descriptor, int status)
{
    int blocked_threads_count;
    JVMSPI_BlockedThreadInfo *blocked_threads;

    blocked_threads = SNI_GetBlockedThreads(&blocked_threads_count);

    SNIEVT_signal_list(blocked_threads, blocked_threads_count, 
                            waitingFor, descriptor, status);
}

/**
 * Find and unblock all Java threads based on what the thread is waiting 
 * for and which descriptor it is waiting on.
 * 
 * @param blocked_threads list of blocked threads
 * @param blocked_threads_count number of blocked threads in the list
 * @param waitingFor used to match SNIReentryData.waitingFor
 * @param descriptor used to match SNIReentryData.descriptor. If 
 *                   SNIReentryData.descriptor is -1, then it maches
 *                   descriptor anyway
 * @param status the value stored into SNIReentryData.status for every 
 *               thread that is unblocked
 */
void 
SNIEVT_signal_list(
        JVMSPI_BlockedThreadInfo *blocked_threads,
        int blocked_threads_count, SNIsignalType waitingFor,
        int descriptor, int status)
{
    int i;
    SNIReentryData* pThreadReentryData;

    for (i = 0; i < blocked_threads_count; i++) {
        pThreadReentryData =
            (SNIReentryData*)(blocked_threads[i].reentry_data);

        if (pThreadReentryData != NULL 
                && ((pThreadReentryData->descriptor == -1) ||
                    (pThreadReentryData->descriptor == descriptor)) 
                && pThreadReentryData->waitingFor == waitingFor) {
            pThreadReentryData->status = status;
			pThreadReentryData->descriptor = descriptor;
            SNIEVT_unblock(blocked_threads[i].thread_id);
        }
    }
}

/**
 * A internal function that unblocks the given Java thread. This should 
 * be called in preference to calling SNI_UnblockThread directly, since 
 * depending upon how the VM is scheduled (master mode or slave mode) 
 * additional work may need to be done. In particular, in slave mode, this 
 * function will need to arrange for a VM time slice to occur quickly.
 *
 * @param thr the Java thread to unblock
 */
void
SNIEVT_unblock(JVMSPI_ThreadID thr) {
    /*
     * Tell the VM to unblock the thread, and then tell the platform-specific 
     * code to schedule the VM.
     */
    SNI_UnblockThread(thr);
    if (vm_thread_timeslice_proc != NULL) {
      vm_thread_timeslice_proc();
    }
}

