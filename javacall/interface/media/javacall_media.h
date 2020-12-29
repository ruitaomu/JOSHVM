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

#ifndef __JAVACALL_MEDIA_H
#define __JAVACALL_MEDIA_H

#ifdef __cplusplus
extern "C"{
#endif

#include "javacall_defs.h"

typedef enum {
    JAVACALL_EVENT_MEDIA_COMMON = 0,
    JAVACALL_EVENT_MEDIA_PREPARE_COMPLETED = 1000,
    JAVACALL_EVENT_MEDIA_SEEK_COMPLETED = 1001,
    JAVACALL_EVENT_MEDIA_PLAY_COMPLETED = 1002,
    JAVACALL_EVENT_MEDIA_READ = 1003,
    JAVACALL_EVENT_MEDIA_WRITE = 1004,
    JAVACALL_EVENT_MEDIA_CLOSE = 1005,
    JAVACALL_EVENT_MEDIA_WAKEUP = 1100,
    JAVACALL_EVENT_MEDIA_VAD_BEGIN = 1200,
    JAVACALL_EVENT_MEDIA_VAD_END = 1201
} javacall_media_event_type;

typedef enum {
    JAVACALL_MEDIA_STATE_STOPPED = 1,
    JAVACALL_MEDIA_STATE_PAUSED = 2,
    JAVACALL_MEDIA_STATE_STARTED = 3,
    JAVACALL_MEDIA_STATE_UNKNOWN = 99
} javacall_media_state_type;

typedef enum {
    JAVACALL_MEDIA_VAD_STATE_START = 0,
    JAVACALL_MEDIA_VAD_STATE_STOP = 1
} javacall_media_vad_state_type;

typedef struct {
    /** Must be '\0' terminated string **/
    char path[256];
} javacall_media_filepath_info;

typedef struct {
    int index;
    char word[30]; // '\0' terminated string
    char desc[100]; // '\0' terminated string
} javacall_media_word_info;

javacall_result javacall_media_open(int type, /*OUT*/ javacall_handle * handle);
javacall_result javacall_media_close(javacall_handle handle);

javacall_result javacall_media_prepare(javacall_handle handle);
javacall_result javacall_media_start(javacall_handle handle);
javacall_result javacall_media_pause(javacall_handle handle);
javacall_result javacall_media_stop(javacall_handle handle);
javacall_result javacall_media_reset(javacall_handle handle);
javacall_result javacall_media_get_state(javacall_handle handle,
        /*OUT*/ javacall_media_state_type* state);
javacall_result javacall_media_read(javacall_handle handle, unsigned char* buffer,
        int size, int* bytesRead);
javacall_result javacall_media_write(javacall_handle handle, unsigned char* buffer,
        int size, int* bytesWritten);
javacall_result javacall_media_flush(javacall_handle handle);
javacall_result javacall_media_get_buffsize(javacall_handle handle, /*OUT*/ int* size);
javacall_result javacall_media_set_audio_sample_rate(javacall_handle handle, int value);
javacall_result javacall_media_set_channel_config(javacall_handle handle, int value);
javacall_result javacall_media_set_audio_bit_rate(javacall_handle handle, int value);
javacall_result javacall_media_set_source(javacall_handle handle, char* source);
javacall_result javacall_media_set_output_file(javacall_handle handle, char* file);
javacall_result javacall_media_set_output_format(javacall_handle handle, int format);
javacall_result javacall_media_get_time(javacall_handle handle, /*OUT*/ int* time);
javacall_result javacall_media_set_time(javacall_handle handle, int time);
javacall_result javacall_media_get_duration(javacall_handle handle, /*OUT*/ int* duration);

javacall_result javacall_media_get_max_volume(/*OUT*/ int* volume);
javacall_result javacall_media_get_volume(/*OUT*/ int* volume);
javacall_result javacall_media_set_volume(int volume);

javacall_result javacall_media_event_init();
javacall_result javacall_media_event_finalize();
javacall_result javacall_media_event_get(javacall_media_event_type* type,
        int* handle, javacall_result* result, int* userdata);

javacall_result javacall_media_get_tempfile(javacall_media_filepath_info* info);
javacall_result javacall_media_remove_tempfile(char* file);
javacall_result javacall_media_get_rootdir(javacall_media_filepath_info* info);
javacall_result javacall_media_get_ext_rootdir(javacall_media_filepath_info* info);
javacall_result javacall_media_ext_rootdir_exists();

javacall_result javacall_media_wakeup_enable(int enable);
javacall_result javacall_media_wakeup_get_count(int* count);
javacall_result javacall_media_wakeup_get_word(int pos, javacall_media_word_info* info);

javacall_result javacall_media_vad_start();
javacall_result javacall_media_vad_pause();
javacall_result javacall_media_vad_resume();
javacall_result javacall_media_vad_stop();
javacall_result javacall_media_vad_set_timeout(int timeout);


void javanotify_media_event(javacall_media_event_type type,
        javacall_handle handle, javacall_result result);

#ifdef __cplusplus
}
#endif

#endif
