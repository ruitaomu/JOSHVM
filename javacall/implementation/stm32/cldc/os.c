#ifdef __cplusplus
extern "C" {
#endif

#include "javacall_os.h"

/*
 * Initialize the OS structure.
 * This is where timers and threads get started for the first
 * real_time_tick event, and where signal handlers and other I/O
 * initialization should occur.
 *
*/
void javacall_os_initialize(void){

    return;
}


/*
 * Performs a clean-up of all threads and other OS related activity
 * to allow for a clean and complete restart.  This should undo
 * all the work that initialize does.
 */
void javacall_os_dispose(){
    return;
}

/** 
 * javacall_os_flush_icache is used, for example, to flush any caches used by a
 * code segment that is deoptimized or moved during a garbage collection.
 * flush at least [address, address + size] (may flush complete icache).
 *
 * @param address   Start address to flush
 * @param size      Size to flush
 */
void javacall_os_flush_icache(unsigned char* address, int size) {
}

void javacall_os_hardware_reset() {
}

int javacall_os_set_system_time(javacall_int64 time) {
	return JAVACALL_NOT_IMPLEMENTED;
}

#ifdef __cplusplus
}
#endif



