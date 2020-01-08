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

javacall_result javacall_media_open(int type, javacall_handle* handle) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_close(javacall_handle handle) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_prepare(javacall_handle handle) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_start(javacall_handle handle) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_pause(javacall_handle handle) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_stop(javacall_handle handle) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_reset(javacall_handle handle) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_get_state(javacall_handle handle, javacall_media_state_type* state) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_read(javacall_handle handle, unsigned char* buffer,
        int size, int* bytesRead) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_write(javacall_handle handle, unsigned char* buffer,
        int size, int* bytesWritten) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_flush(javacall_handle handle) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_get_buffsize(javacall_handle handle, int* size) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_set_audio_sample_rate(javacall_handle handle, int value) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_set_channel_config(javacall_handle handle, int value) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_set_audio_bit_rate(javacall_handle handle, /*OUT*/ int value) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_set_source(javacall_handle handle, char* source) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_set_output_file(javacall_handle handle, char* file) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_set_output_format(javacall_handle handle, int format) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_get_time(javacall_handle handle, int* time) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_set_time(javacall_handle handle, int time) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_get_duration(javacall_handle handle, int* duration) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_get_max_volume(int* volume) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_get_volume(int* volume) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_set_volume(int volume) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_event_init() {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_event_finalize() {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_event_get(javacall_media_event_type* type,
        int* handle, javacall_result* result, int* userdata) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_get_tempfile(javacall_media_filepath_info* info) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_remove_tempfile(char* file) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_get_rootdir(javacall_media_filepath_info* info) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_get_ext_rootdir(javacall_media_filepath_info* info) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_ext_rootdir_exists() {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_wakeup_enable(int enable) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_wakeup_get_count(int* count) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_wakeup_get_word(int pos, javacall_media_word_info* info) {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_vad_start() {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_vad_pause() {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_vad_resume() {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_vad_stop() {
    return JAVACALL_FAIL;
}

javacall_result javacall_media_vad_set_timeout(int timeout) {
    return JAVACALL_FAIL;
}
