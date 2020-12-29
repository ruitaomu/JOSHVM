/**
 * @file
 *
 *
 */
#include "javacall_time.h"
#include "esp_timer.h"

typedef struct {
    esp_timer_handle_t esp_timer_handle;
    javacall_callback_func func;
} javacall_timer;

static void timer_callback(void* arg) {
    javacall_timer* handle = (javacall_timer*)arg;
    javacall_callback_func func = (javacall_callback_func)handle->func;
    func(handle);
}

static javacall_int64 rtc_time_offset = 0;

/**
 *
 * Create a native timer to expire in wakeupInSeconds or less seconds.
 * If a later timer exists, cancel it and create a new timer
 *
 * @param wakeupInMilliSecondsFromNow time to wakeup in milli-seconds
 *                              relative to current time
 *                              if -1, then ignore the call
 * @param cyclic <tt>1</tt> indicates that the timer should be repeated cuclically,
 *               <tt>0</tt> indicates that this is a one-shot timer that should call the callback function once
 * @param func callback function should be called in platform's context once the timer
 *             expires
 * @param handle A pointer to the returned handle that will be associated with this timer
 *               On success.
 *
 * @return on success returns <tt>JAVACALL_OK</tt>,
 *         or <tt>JAVACALL_FAIL</tt> or negative value on failure
 */
javacall_result javacall_time_initialize_timer(
                    int                      wakeupInMilliSecondsFromNow,
                    javacall_bool            cyclic,
                    javacall_callback_func   func,
                    /*OUT*/ javacall_handle *handle) {

    if (!handle || !func) {
        return JAVACALL_INVALID_ARGUMENT;
    }

    javacall_timer* timer_handle = (javacall_timer*)javacall_malloc(sizeof(javacall_timer));
    if (timer_handle == NULL) {
        return JAVACALL_FAIL;
    }

    const esp_timer_create_args_t timer_args = {
            .callback = &timer_callback,
            .arg = timer_handle,
            .name = "jctimer"
    };

    esp_err_t result = esp_timer_create(&timer_args, &timer_handle->esp_timer_handle);
    if (result != ESP_OK) {
        javacall_free(timer_handle);
        *handle = NULL;
        return JAVACALL_FAIL;
    }

    timer_handle->func = func;
    *handle = (javacall_handle)timer_handle;
    if (cyclic) {
        esp_timer_start_periodic(timer_handle->esp_timer_handle, wakeupInMilliSecondsFromNow * 1000);
    } else {
        esp_timer_start_once(timer_handle->esp_timer_handle, wakeupInMilliSecondsFromNow * 1000);
    }

    return JAVACALL_OK;
}

/**
 *
 * Disable a set native timer
 * @param handle The handle of the timer to be finalized
 *
 * @return on success returns <tt>JAVACALL_OK</tt>,
 *         <tt>JAVACALL_FAIL</tt> or negative value on failure
 */
javacall_result javacall_time_finalize_timer(javacall_handle handle) {
    javacall_timer* timer_handle = (javacall_timer*)handle;
    esp_timer_stop(timer_handle->esp_timer_handle);
    esp_timer_delete(timer_handle->esp_timer_handle);

    return JAVACALL_OK;
}

/**
 * Return local timezone ID string. This string is maintained by this
 * function internally. Caller must NOT try to free it.
 *
 * This function should handle daylight saving time properly. For example,
 * for time zone America/Los_Angeles, during summer time, this function
 * should return GMT-07:00 and GMT-08:00 during winter time.
 *
 * @return Local timezone ID string pointer. The ID string should be in the
 *         format of GMT+/-??:??. For example, GMT-08:00 for PST.
 */
char* javacall_time_get_local_timezone(void){
    static char tzstring[128];
    snprintf(tzstring, 128, "GMT+%02d:%02d",8,0);
    return tzstring;
}


/**
 * returns number of milliseconds elapsed since midnight(00:00:00), January 1, 1970,
 *
 * @return milliseconds elapsed since midnight (00:00:00), January 1, 1970
 */
javacall_int64 /*OPTIONAL*/ javacall_time_get_milliseconds_since_1970(void){
    return rtc_time_offset + (javacall_int64)esp_timer_get_time()/1000LL;
}

javacall_result javacall_time_set_milliseconds_since_1970(javacall_int64 ms) {
    rtc_time_offset = ms - (javacall_int64)esp_timer_get_time()/1000LL;
    return JAVACALL_OK;
}

/*
 *
 * Temporarily disable timer interrupts. This is called when
 * the VM is about to sleep (when there's no Java thread to execute)
 *
 * @param handle timer handle to suspend
 *
 */
void javacall_time_suspend_ticks(javacall_handle handle) {
}

/*
 *
 * Enable  timer interrupts. This is called when the VM
 * wakes up and continues executing Java threads.
 *
 * @param handle timer handle to resume
 *
 */
void javacall_time_resume_ticks(javacall_handle handle) {
}

/*
 * Suspend the current process sleep for ms milliseconds
 */
void javacall_time_sleep(javacall_uint64 ms) {
}

