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

#include "javacall_media.h"
#include "javacall_memory.h"
#include "javacall_logging.h"

#include "freertos/FreeRTOS.h"
#include <sys/stat.h>

#define JC_MEDIA_ERR_OK 0
#define JC_MEDIA_ERR_FAIL -1
#define JC_MEDIA_ERR_NOTIFY_LATER -2
#define JC_MEDIA_ERR_OUT_OF_MEMORY -3
#define JC_MEDIA_ERR_LIMIT_RES -4
#define JC_MEDIA_ERR_INVALID_ARGUMENT -5
#define JC_MEDIA_ERR_INVALID_STATE -6
#define JC_MEDIA_ERR_NOT_SUPPORTED -7
#define JC_MEDIA_ERR_TIMEOUT -8

#define JC_MEDIA_LOG_CHANNEL JC_MMA

#define JC_MEDIA_HANDLE_MIN 5000
#define JC_MAX_OPEN_MEDIA_HANDLE 10
#define JC_MAX_MEDIA_EVENT 20

#define JC_MEDIA_NATIVE_ROOT_PATH JC_NATIVE_USER_ROOT_PATH
#define JC_MEDIA_ROOT_PATH "/Phone/"
#define JC_MEDIA_EXT_ROOT_PATH "/sdcard/"
#define JC_MEDIA_TEMPFILE_PREFIX JC_MEDIA_ROOT_PATH"jcmediatmp"

static portMUX_TYPE s_media_lock = portMUX_INITIALIZER_UNLOCKED;

static void media_lock() {
    portENTER_CRITICAL(&s_media_lock);
}

static void media_unlock() {
    portEXIT_CRITICAL(&s_media_lock);
}

typedef struct _media_handle {
    int virtual_handle;
    int native_handle;
    struct _media_handle* next;
} media_handle_data;

typedef struct _media_event {
    javacall_media_event_type type;
    int handle;
    javacall_result result;
    int userdata;
    struct _media_event* next;
} media_event_data;

static int next_media_handle = JC_MEDIA_HANDLE_MIN;
static media_handle_data* g_handle_data = NULL;
static media_event_data* g_event_data = NULL;
static int event_inited = 0;

static void add_handle_data(media_handle_data* handle_data) {
    if (g_handle_data == NULL) {
        g_handle_data = handle_data;
        return;
    }

    media_handle_data* p = g_handle_data;
    while (p->next != NULL) {
        p = p->next;
    }
    p->next = handle_data;
}

static void remove_handle_data(int virtual_handle) {
    media_handle_data* p = g_handle_data;
    media_handle_data* prev = NULL;
    while (p != NULL) {
        if (p->virtual_handle == virtual_handle) {
            if (prev != NULL) {
                prev->next = p->next;
            } else {
                g_handle_data = p->next;
            }
            javacall_free(p);
            return;
        }
        prev = p;
        p = p->next;
    }
}

static int get_native_handle(int virtual_handle, int* native_handle) {
    media_handle_data* p = g_handle_data;
    while (p != NULL) {
        if (p->virtual_handle == virtual_handle) {
            *native_handle = p->native_handle;
            return 0;
        }
        p = p->next;
    }

    return -1;
}

static int get_virtual_handle(int native_handle, int* virtual_handle) {
    media_handle_data* p = g_handle_data;
    while (p != NULL) {
        if (p->native_handle == native_handle) {
            *virtual_handle = p->virtual_handle;
            return 0;
        }
        p = p->next;
    }

    return -1;
}

static int get_open_handle_count() {
    int count = 0;
    media_handle_data* p = g_handle_data;
    while (p != NULL) {
        count++;
        p = p->next;
    }
    return count;
}

#define GET_AND_CHECK_NATIVE_HANDLE(hanlde, native_handle) { \
    int ret = get_native_handle((int)handle, &native_handle); \
    if (ret < 0) { \
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_MEDIA_LOG_CHANNEL, \
            "no native handle found. already closed?\n"); \
        return JAVACALL_FAIL; \
    } \
}

static inline int convertNativeResult(int result) {
    return (result == JC_MEDIA_ERR_OK) ? JAVACALL_OK : JAVACALL_FAIL;
}

static void add_event_data(media_event_data* data) {
    if (g_event_data == NULL) {
        g_event_data = data;
        return;
    }

    media_event_data* p = g_event_data;
    while (p->next != NULL) {
        p = p->next;
    }
    p->next = data;
}

static void remove_first_event() {
    if (g_event_data == NULL) {
        return;
    }

    media_event_data* p = g_event_data;
    g_event_data = p->next;
    javacall_free(p);
}

static void clear_events() {
    media_event_data* p = g_event_data;
    media_event_data* next = NULL;
    while (p != NULL) {
        next = p->next;
        javacall_free(p);
        p = next;
    }
    g_event_data = NULL;
}

static int get_event_count() {
    int count = 0;
    media_event_data* p = g_event_data;
    while (p != NULL) {
        count++;
        p = p->next;
    }
    return count;
}

static void set_media_event_with_userdata(javacall_media_event_type type, int handle,
        javacall_result result, int userdata) {
    if (!event_inited) {
        return;
    }

    media_lock();
    do {
        if (get_event_count() >= JC_MAX_MEDIA_EVENT) {
            javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_MEDIA_LOG_CHANNEL,
                    "too many events. (> %d)\n", JC_MAX_MEDIA_EVENT);
            break;
        }

        media_event_data* ptr = (media_event_data*)javacall_malloc(sizeof(media_event_data));
        if (ptr == NULL) {
            javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_MEDIA_LOG_CHANNEL,
                "failed to create media event.\n");
            break;
        }
        memset(ptr, 0, sizeof(media_event_data));
        ptr->type = type;
        ptr->handle = handle;
        ptr->result = result;
        ptr->userdata = userdata;
        add_event_data(ptr);
        javanotify_media_event(JAVACALL_EVENT_MEDIA_COMMON, -1, JAVACALL_OK);
    } while (0);
    media_unlock();
}

static void set_media_event(javacall_media_event_type type, int handle, javacall_result result) {
    set_media_event_with_userdata(type, handle, result, 0);
}

extern int joshvm_esp32_media_create(int type, void** handle);
extern int joshvm_esp32_media_close(void* handle);
extern int joshvm_esp32_media_prepare(void* handle, void(*callback)(void*, int));
extern int joshvm_esp32_media_start(void* handle, void(*callback)(void*, int));
extern int joshvm_esp32_media_pause(void* handle);
extern int joshvm_esp32_media_stop(void* handle);
extern int joshvm_esp32_media_reset(void* handle);
extern int joshvm_esp32_media_get_state(void* handle, int* state);
extern int joshvm_esp32_media_read(void* handle, unsigned char* buffer, int size,
        int* bytesRead, void(*callback)(void*, int));
extern int joshvm_esp32_media_write(void* handle, unsigned char* buffer, int size,
        int* bytesWritten, void(*callback)(void*, int));
extern int joshvm_esp32_media_flush(void* handle);
extern int joshvm_esp32_media_get_buffsize(void* handle, int* size);
extern int joshvm_esp32_media_set_audio_sample_rate(void* handle, int value);
extern int joshvm_esp32_media_set_channel_config(void* handle, int value);
extern int joshvm_esp32_media_set_audio_bit_rate(void* handle, int value);
extern int joshvm_esp32_media_set_source(void* handle, char* source);
extern int joshvm_esp32_media_set_output_file(void* handle, char* file);
extern int joshvm_esp32_media_set_output_format(void* handle, int format);
extern int joshvm_esp32_media_get_position(void* handle, int* pos);
extern int joshvm_esp32_media_set_position(void* handle, int pos, void(*callback)(void*, int));
extern int joshvm_esp32_media_get_duration(void* handle, int* duration);

extern int joshvm_esp32_media_get_max_volume(int* volume);
extern int joshvm_esp32_media_get_volume(int* volume);
extern int joshvm_esp32_media_set_volume(int volume);
extern int joshvm_esp32_media_add_volume();
extern int joshvm_esp32_media_sub_volume();

extern int joshvm_esp32_wakeup_get_word_count();
extern int joshvm_esp32_wakeup_get_word(int pos, int* index, char* wordbuf,
        int wordlen, char* descbuf, int desclen);
extern int joshvm_esp32_wakeup_enable(void(*callback)(int));
extern int joshvm_esp32_wakeup_disable();

extern int joshvm_esp32_vad_start(void(*callback)(int));
extern int joshvm_esp32_vad_pause();
extern int joshvm_esp32_vad_resume();
extern int joshvm_esp32_vad_stop();
extern int joshvm_esp32_vad_set_timeout(int ms);

javacall_result javacall_media_open(int type, javacall_handle* handle) {
    void* native_handle;
    int result;

    if (get_open_handle_count() >= JC_MAX_OPEN_MEDIA_HANDLE) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_MEDIA_LOG_CHANNEL,
                "open too many handles. (> %d)\n", JC_MAX_OPEN_MEDIA_HANDLE);
        return JAVACALL_FAIL;
    }

    result = joshvm_esp32_media_create(type, &native_handle);
    if (result == JC_MEDIA_ERR_OK) {
        media_handle_data* ptr = (media_handle_data*)javacall_malloc(sizeof(media_handle_data));
        if (ptr == NULL) {
            return JAVACALL_OUT_OF_MEMORY;
        }
        memset(ptr, 0, sizeof(media_handle_data));
        ptr->native_handle = (int)native_handle;
        ptr->virtual_handle = next_media_handle;
        next_media_handle++;
        if (next_media_handle < JC_MEDIA_HANDLE_MIN) {
            next_media_handle = JC_MEDIA_HANDLE_MIN;
        }
        add_handle_data(ptr);
        *handle = ptr->virtual_handle;

        return JAVACALL_OK;
    }

    javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_MEDIA_LOG_CHANNEL,
        "media open failed. (%d)\n", result);
    return JAVACALL_FAIL;
}

javacall_result javacall_media_close(javacall_handle handle) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_close((void *)native_handle);
    javanotify_media_event(JAVACALL_EVENT_MEDIA_CLOSE, handle, convertNativeResult(result));
    if (result == JC_MEDIA_ERR_OK) {
        remove_handle_data(handle);
        return JAVACALL_OK;
    } else {
        return JAVACALL_FAIL;
    }
}

static void prepare_callback(void* handle, int prepare_result) {
    javacall_result result;
    int virtual_handle;

    result = get_virtual_handle((int)handle, &virtual_handle);
    if (result < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_MEDIA_LOG_CHANNEL,
            "no native handle found on prepare callback.\n");
        return JAVACALL_FAIL;
    }

    if (prepare_result == JC_MEDIA_ERR_OK) {
        result = JAVACALL_OK;
    } else {
        result = JAVACALL_FAIL;
    }

    javanotify_media_event(JAVACALL_EVENT_MEDIA_PREPARE_COMPLETED, virtual_handle, result);
    set_media_event(JAVACALL_EVENT_MEDIA_PREPARE_COMPLETED, virtual_handle, result);
}

static void end_callback(void* handle, int end_result) {
    javacall_result result;
    int virtual_handle;

    result = get_virtual_handle((int)handle, &virtual_handle);
    if (result < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_MEDIA_LOG_CHANNEL,
            "no native handle found on end callback.\n");
        return JAVACALL_FAIL;
    }

    if (end_result == JC_MEDIA_ERR_OK) {
        result = JAVACALL_OK;
    } else {
        result = JAVACALL_FAIL;
    }

    set_media_event(JAVACALL_EVENT_MEDIA_PLAY_COMPLETED, virtual_handle, result);
}

static void seek_callback(void* handle, int seek_result) {
    javacall_result result;
    int virtual_handle;

    result = get_virtual_handle((int)handle, &virtual_handle);
    if (result < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_MEDIA_LOG_CHANNEL,
            "no native handle found on seek callback.\n");
        return JAVACALL_FAIL;
    }

    if (seek_result == JC_MEDIA_ERR_OK) {
        result = JAVACALL_OK;
    } else {
        result = JAVACALL_FAIL;
    }

    set_media_event(JAVACALL_EVENT_MEDIA_SEEK_COMPLETED, virtual_handle, result);
}

javacall_result javacall_media_prepare(javacall_handle handle) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_prepare((void *)native_handle, prepare_callback);
    if (result == JC_MEDIA_ERR_NOTIFY_LATER) {
        return JAVACALL_WOULD_BLOCK;
    } else {
        javacall_result ret = (result == JC_MEDIA_ERR_OK) ? JAVACALL_OK : JAVACALL_FAIL;
        set_media_event(JAVACALL_EVENT_MEDIA_PREPARE_COMPLETED, handle, ret);
        return ret;
    }
}

javacall_result javacall_media_start(javacall_handle handle) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_start((void *)native_handle, end_callback);
    if (result == JC_MEDIA_ERR_OK) {
        return JAVACALL_OK;
    } else {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_MEDIA_LOG_CHANNEL,
            "start failed. (%d)\n", result);
        return JAVACALL_FAIL;
    }
}

javacall_result javacall_media_pause(javacall_handle handle) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_pause((void *)native_handle);
    if (result == JC_MEDIA_ERR_OK) {
        return JAVACALL_OK;
    } else {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_MEDIA_LOG_CHANNEL,
            "pause failed. (%d)\n", result);
        return JAVACALL_FAIL;
    }
}

javacall_result javacall_media_stop(javacall_handle handle) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_stop((void *)native_handle);
    if (result == JC_MEDIA_ERR_OK) {
        return JAVACALL_OK;
    } else {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_MEDIA_LOG_CHANNEL,
            "stop failed. (%d)\n", result);
        return JAVACALL_FAIL;
    }
}

javacall_result javacall_media_reset(javacall_handle handle) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_reset((void *)native_handle);
    return convertNativeResult(result);
}

javacall_result javacall_media_get_state(javacall_handle handle, javacall_media_state_type* state) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_get_state((void *)native_handle, (int*)state);
    return convertNativeResult(result);
}

static void read_callback(void* handle, int read_result) {
    javacall_result result;
    int virtual_handle;

    result = get_virtual_handle((int)handle, &virtual_handle);
    if (result < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_MEDIA_LOG_CHANNEL,
            "no native handle found on read callback.\n");
        return JAVACALL_FAIL;
    }

    if (read_result == JC_MEDIA_ERR_OK) {
        result = JAVACALL_OK;
    } else {
        result = JAVACALL_FAIL;
    }
    javanotify_media_event(JAVACALL_EVENT_MEDIA_READ, virtual_handle, result);
}

static void write_callback(void* handle, int write_result) {
    javacall_result result;
    int virtual_handle;

    result = get_virtual_handle((int)handle, &virtual_handle);
    if (result < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_MEDIA_LOG_CHANNEL,
            "no native handle found on write callback.\n");
        return JAVACALL_FAIL;
    }

    if (write_result == JC_MEDIA_ERR_OK) {
        result = JAVACALL_OK;
    } else {
        result = JAVACALL_FAIL;
    }

    javanotify_media_event(JAVACALL_EVENT_MEDIA_WRITE, virtual_handle, result);
}

javacall_result javacall_media_read(javacall_handle handle, unsigned char* buffer,
        int size, int* bytesRead) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_read((void *)native_handle, buffer, size, bytesRead, read_callback);
    if (result == JC_MEDIA_ERR_NOTIFY_LATER) {
        return JAVACALL_WOULD_BLOCK;
    } else {
        return convertNativeResult(result);
    }
}

javacall_result javacall_media_write(javacall_handle handle, unsigned char* buffer,
        int size, int* bytesWritten) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_write((void *)native_handle, buffer, size, bytesWritten, write_callback);
    if (result == JC_MEDIA_ERR_NOTIFY_LATER) {
        return JAVACALL_WOULD_BLOCK;
    } else {
        return convertNativeResult(result);
    }
}

javacall_result javacall_media_flush(javacall_handle handle) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_flush((void *)native_handle);
    return convertNativeResult(result);
}

javacall_result javacall_media_get_buffsize(javacall_handle handle, int* size) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_get_buffsize((void *)native_handle, size);
    return convertNativeResult(result);
}

javacall_result javacall_media_set_audio_sample_rate(javacall_handle handle, int value) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_set_audio_sample_rate((void *)native_handle, value);
    return convertNativeResult(result);
}

javacall_result javacall_media_set_channel_config(javacall_handle handle, int value) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_set_channel_config((void *)native_handle, value);
    return convertNativeResult(result);
}

javacall_result javacall_media_set_audio_bit_rate(javacall_handle handle, /*OUT*/ int value) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_set_audio_bit_rate((void *)native_handle, value);
    return convertNativeResult(result);
}

static int is_valid_filepath(char* path) {
    if (strncmp(path, JC_MEDIA_ROOT_PATH, strlen(JC_MEDIA_ROOT_PATH)) == 0) {
        return 1;
    }
    if (strncmp(path, JC_MEDIA_EXT_ROOT_PATH, strlen(JC_MEDIA_EXT_ROOT_PATH)) == 0) {
        return 1;
    }
    return 0;
}

static char* convert_filepath(char* path) {
    static char n_path[JAVACALL_MAX_FILE_NAME_LENGTH];

    if (strncmp(path, JC_MEDIA_ROOT_PATH, strlen(JC_MEDIA_ROOT_PATH)) == 0) {
        sprintf(n_path, "%s%s", JC_MEDIA_NATIVE_ROOT_PATH, path + strlen(JC_MEDIA_ROOT_PATH));
        return n_path;
    }
    // if (strncmp(path, JC_MEDIA_EXT_ROOT_PATH, strlen(JC_MEDIA_EXT_ROOT_PATH)) == 0) {
        // sprintf(n_path, "%s%s", JC_MEDIA_EXT_ROOT_PATH, path + strlen(JC_MEDIA_EXT_ROOT_PATH));
        // return n_path;
    // }
    return path;
}

javacall_result javacall_media_set_source(javacall_handle handle, char* source) {
    int result;
    int native_handle;

    if (source[0] == '/') {
        if (!is_valid_filepath(source)) {
            return JAVACALL_FAIL;
        }
        source = convert_filepath(source);
    }

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);
    result = joshvm_esp32_media_set_source((void *)native_handle, source);
    return convertNativeResult(result);
}

javacall_result javacall_media_set_output_file(javacall_handle handle, char* file) {
    int result;
    int native_handle;

    if (!is_valid_filepath(file)) {
        return JAVACALL_FAIL;
    }

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);
    result = joshvm_esp32_media_set_output_file((void *)native_handle, convert_filepath(file));
    return convertNativeResult(result);
}

javacall_result javacall_media_set_output_format(javacall_handle handle, int format) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_set_output_format((void *)native_handle, format);
    return convertNativeResult(result);
}

javacall_result javacall_media_get_time(javacall_handle handle, int* time) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_get_position((void *)native_handle, time);
    return convertNativeResult(result);
}

javacall_result javacall_media_set_time(javacall_handle handle, int time) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_set_position((void *)native_handle, time, seek_callback);
    if (result == JC_MEDIA_ERR_NOTIFY_LATER) {
        return JAVACALL_WOULD_BLOCK;
    } else {
        javacall_result ret = (result == JC_MEDIA_ERR_OK) ? JAVACALL_OK : JAVACALL_FAIL;
        set_media_event(JAVACALL_EVENT_MEDIA_SEEK_COMPLETED, handle, ret);
        return ret;
    }
}

javacall_result javacall_media_get_duration(javacall_handle handle, int* duration) {
    int result;
    int native_handle;

    GET_AND_CHECK_NATIVE_HANDLE(handle, native_handle);

    result = joshvm_esp32_media_get_duration((void *)native_handle, duration);
    return convertNativeResult(result);
}

javacall_result javacall_media_get_max_volume(int* volume) {
    int result = joshvm_esp32_media_get_max_volume(volume);
    return convertNativeResult(result);
}

javacall_result javacall_media_get_volume(int* volume) {
    int result = joshvm_esp32_media_get_volume(volume);
    return convertNativeResult(result);
}

javacall_result javacall_media_set_volume(int volume) {
    int result = joshvm_esp32_media_set_volume(volume);
    return convertNativeResult(result);
}


javacall_result javacall_media_event_init() {
    media_lock();
    clear_events();
    event_inited = 1;
    media_unlock();
    return JAVACALL_OK;
}

javacall_result javacall_media_event_finalize() {
    if (!event_inited) {
        return JAVACALL_OK;
    }
    media_lock();
    clear_events();
    event_inited = 0;
    media_unlock();
    javanotify_media_event(JAVACALL_EVENT_MEDIA_COMMON, -1, JAVACALL_OK);
    return JAVACALL_OK;
}

javacall_result javacall_media_event_get(javacall_media_event_type* type,
        int* handle, javacall_result* result, int* userdata) {
    if (!event_inited) {
        return JAVACALL_FAIL;
    }

    javacall_result ret = JAVACALL_FAIL;
    media_lock();
    if (g_event_data == NULL) {
        ret = JAVACALL_WOULD_BLOCK;
    } else {
        *type = g_event_data->type;
        *handle = g_event_data->handle;
        *result = g_event_data->result;
        *userdata = g_event_data->userdata;
        remove_first_event();
        ret = JAVACALL_OK;
    }
    media_unlock();

    return ret;
}

javacall_result javacall_media_get_tempfile(javacall_media_filepath_info* info) {
    static index = 100;
    sprintf(info->path, "%s%d", JC_MEDIA_TEMPFILE_PREFIX, index);
    index++;
    return JAVACALL_OK;
}

javacall_result javacall_media_remove_tempfile(char* file) {
    if ((file != NULL) && (strncmp(file, JC_MEDIA_TEMPFILE_PREFIX, strlen(JC_MEDIA_TEMPFILE_PREFIX)) == 0)) {
        unlink(convert_filepath(file));
    }
    return JAVACALL_OK;
}

javacall_result javacall_media_get_rootdir(javacall_media_filepath_info* info) {
    sprintf(info->path, "%s", JC_MEDIA_ROOT_PATH);
    return JAVACALL_OK;
}

javacall_result javacall_media_get_ext_rootdir(javacall_media_filepath_info* info) {
    sprintf(info->path, "%s", JC_MEDIA_EXT_ROOT_PATH);
    return JAVACALL_OK;
}

javacall_result javacall_media_ext_rootdir_exists() {
    struct stat st;

    if (stat(JC_MEDIA_EXT_ROOT_PATH, &st) == 0) {
        if (st.st_mode & S_IFDIR) {
            return JAVACALL_OK;
        }
    }
    return JAVACALL_FAIL;
}


static void wakeup_callback(int word_index) {
    set_media_event_with_userdata(JAVACALL_EVENT_MEDIA_WAKEUP, 0, JAVACALL_OK, word_index);
}

javacall_result javacall_media_wakeup_enable(int enable) {
    int result;
    if (enable) {
        result = joshvm_esp32_wakeup_enable(wakeup_callback);
    } else {
        result = joshvm_esp32_wakeup_disable();
    }
    return convertNativeResult(result);
}

javacall_result javacall_media_wakeup_get_count(int* count) {
    int cnt = joshvm_esp32_wakeup_get_word_count();
    if (cnt >= 0) {
        *count = cnt;
        return JAVACALL_OK;
    }
    return JAVACALL_FAIL;
}

javacall_result javacall_media_wakeup_get_word(int pos, javacall_media_word_info* info) {
    int result;

    memset(info, 0, sizeof(javacall_media_word_info));
    result = joshvm_esp32_wakeup_get_word(pos, &(info->index), &(info->word),
        (sizeof(info->word) - 1), &(info->desc), (sizeof(info->desc) - 1));
    return convertNativeResult(result);
}

static void vad_callback(int state) {
    if (state == JAVACALL_MEDIA_VAD_STATE_START) {
        set_media_event(JAVACALL_EVENT_MEDIA_VAD_BEGIN, 0, JAVACALL_OK);
    } else if (state == JAVACALL_MEDIA_VAD_STATE_STOP) {
        set_media_event(JAVACALL_EVENT_MEDIA_VAD_END, 0, JAVACALL_OK);
    }
}

javacall_result javacall_media_vad_start() {
    int result = joshvm_esp32_vad_start(vad_callback);
    return convertNativeResult(result);
}

javacall_result javacall_media_vad_pause() {
    int result = joshvm_esp32_vad_pause();
    return convertNativeResult(result);
}

javacall_result javacall_media_vad_resume() {
    int result = joshvm_esp32_vad_resume();
    return convertNativeResult(result);
}

javacall_result javacall_media_vad_stop() {
    int result = joshvm_esp32_vad_stop();
    return convertNativeResult(result);
}

javacall_result javacall_media_vad_set_timeout(int timeout) {
    int result = joshvm_esp32_vad_set_timeout(timeout);
    return convertNativeResult(result);
}
