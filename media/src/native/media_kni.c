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
#include <kni.h>
#include <kni_globals.h>
#include <sni.h>
#include <sni_event.h>

#if ENABLE_PCSL
#include "javacall_logging.h"
#include "javacall_media.h"
#include <PCSLString_Util.h>
#include <pcsl_memory.h>
#include <pcsl_string.h>

static int get_pcsl_string(jobject stringHandle, pcsl_string** string, int doThrow)
{
    if (KNI_IsNullHandle(stringHandle)) {
        if (doThrow) {
            KNI_ThrowNew(KNINullPointerException, NULL);
        }
        return -1;
    } else {
        pcsl_string* str = (pcsl_string*)pcsl_mem_malloc(sizeof(pcsl_string));
        if (str == NULL) {
            if (doThrow) {
                KNI_ThrowNew(KNIOutOfMemoryError, NULL);
            }
            return -1;
        }

        if (midp_jstring_to_pcsl_string(stringHandle, str) != PCSL_STRING_OK) {
            pcsl_mem_free(str);
            return -1;
        }

        *string = str;
    }
    return 0;
}
#endif

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_joshvm_media_PlayerImpl_open0()
{
    int handle;
#if ENABLE_PCSL
    int type;
    javacall_result result = JAVACALL_FAIL;

    type = (int)KNI_GetParameterAsInt(1);
    result = javacall_media_open(type, (javacall_handle*)&handle);
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "failed to open player.");
    }
#endif
    KNI_ReturnInt(handle);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_close0()
{
#if ENABLE_PCSL
    int handle = (int)KNI_GetParameterAsInt(1);
    javacall_result result = javacall_media_close((javacall_handle)handle);
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "failed to close player.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_finalize()
{
#if ENABLE_PCSL
    int handle = -1;
    jfieldID handle_field_id;
    jfieldID tempfile_field_id;
    pcsl_string* source = NULL;
    int status = -1;
    javacall_result result = JAVACALL_FAIL;

    KNI_StartHandles(3);
    KNI_DeclareHandle(playerObj);
    KNI_DeclareHandle(classObj);
    KNI_DeclareHandle(sourceObj);

    KNI_GetThisPointer(playerObj);
    KNI_GetObjectClass(playerObj, classObj);

    handle_field_id = KNI_GetFieldID(classObj, "handle", "I");
    tempfile_field_id = KNI_GetFieldID(classObj, "tempFile", "Ljava/lang/String;");
    if (handle_field_id) {
        handle = KNI_GetIntField(playerObj, handle_field_id);
    }
    if (handle != -1) {
        result = javacall_media_close((javacall_handle)handle);
        // javacall_printf("in finalize(). media close return %d\n", result);
    }

    if (tempfile_field_id) {
        KNI_GetObjectField(playerObj, tempfile_field_id, sourceObj);
        status = get_pcsl_string(sourceObj, &source, 0);
        if (status == 0) {
            const jbyte *ptr = pcsl_string_get_utf8_data(source);
            if (ptr != NULL) {
                result = javacall_media_remove_tempfile((char *)ptr);
                // javacall_printf("in finalize(). remove %s return %d\n", ptr, result);
                pcsl_string_release_utf8_data(ptr, source);
            }
            pcsl_string_free(source);
            pcsl_mem_free(source);
        }
    }
    KNI_EndHandles();
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_setSource0()
{
#if ENABLE_PCSL
    int handle;
    pcsl_string* source = NULL;
    int status = -1;
    javacall_result result = JAVACALL_FAIL;

    KNI_StartHandles(1);
    KNI_DeclareHandle(sourceObj);

    handle = (int)KNI_GetParameterAsInt(1);
    KNI_GetParameterAsObject(2, sourceObj);
    status = get_pcsl_string(sourceObj, &source, 1);

    if (status == 0) {
        const jbyte *ptr = pcsl_string_get_utf8_data(source);
        if (ptr != NULL) {
            result = javacall_media_set_source((javacall_handle)handle, (char *)ptr);
            if (result != JAVACALL_OK) {
                KNI_ThrowNew(KNIIOException, "failed to set source.");
            }
            pcsl_string_release_utf8_data(ptr, source);
        }
        pcsl_string_free(source);
        pcsl_mem_free(source);
    }
    KNI_EndHandles();
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_com_joshvm_media_PlayerImpl_getTempFile0()
{
#if ENABLE_PCSL
    javacall_media_filepath_info tempfile;
#endif

    KNI_StartHandles(1);
    KNI_DeclareHandle(filename);

#if ENABLE_PCSL
    javacall_result result = JAVACALL_FAIL;
    result = javacall_media_get_tempfile(&tempfile);
    if (result == JAVACALL_OK) {
        KNI_NewStringUTF(tempfile.path, filename);
    } else {
        KNI_ThrowNew(KNIIOException, "failed get tempfile.");
    }
#endif

    KNI_EndHandlesAndReturnObject(filename);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_prepare0()
{
#if ENABLE_PCSL
    int handle;
    jboolean blocking;
    javacall_result result = JAVACALL_FAIL;
    SNIReentryData* info;

    handle = (int)KNI_GetParameterAsInt(1);
    blocking = KNI_GetParameterAsBoolean(2);

    info = (SNIReentryData*)SNI_GetReentryData(NULL);
    if (info == NULL) {
        result = javacall_media_prepare((javacall_handle)handle);
    } else {
        result = (javacall_result)info->status;
    }

    if (result == JAVACALL_WOULD_BLOCK) {
        if (blocking) {
            SNIEVT_wait(MEDIA_PREPARE_SIGNAL, handle, (void*)NULL);
        }
    } else {
        if (result != JAVACALL_OK) {
            KNI_ThrowNew(KNIIOException, "failed to do prepare.");
        }
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_start0()
{
#if ENABLE_PCSL
    int handle;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    result = javacall_media_start((javacall_handle)handle);
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "failed to do start.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_pause0()
{
#if ENABLE_PCSL
    int handle;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    result = javacall_media_pause((javacall_handle)handle);
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "pause failed.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_stop0()
{
#if ENABLE_PCSL
    int handle;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    result = javacall_media_stop((javacall_handle)handle);
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "stop failed.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_reset0()
{
#if ENABLE_PCSL
    int handle;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    result = javacall_media_reset((javacall_handle)handle);
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "reset failed.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_joshvm_media_PlayerImpl_getState0()
{
    int state;
#if ENABLE_PCSL
    int handle;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    result = javacall_media_get_state((javacall_handle)handle, (javacall_media_state_type*)&state);
    if (result != JAVACALL_OK) {
        state = (int)JAVACALL_MEDIA_STATE_UNKNOWN;
    }
#endif
    KNI_ReturnInt(state);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_joshvm_media_PlayerImpl_getMediaTime0()
{
    int time;
#if ENABLE_PCSL
    int handle;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    result = javacall_media_get_time((javacall_handle)handle, &time);
    if (result != JAVACALL_OK) {
        time = -1;
    }
#endif
    KNI_ReturnInt(time);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_setMediaTime0()
{
#if ENABLE_PCSL
    int handle;
    int time;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    time = (int)KNI_GetParameterAsInt(2);

    result = javacall_media_set_time((javacall_handle)handle, time);
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "failed to set time.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_joshvm_media_PlayerImpl_getDuration0()
{
    int duration;
#if ENABLE_PCSL
    int handle;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    result = javacall_media_get_duration((javacall_handle)handle, &duration);
    if (result != JAVACALL_OK) {
        duration = -1;
    }
#endif
    KNI_ReturnInt(duration);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_setAudioSampleRate0()
{
#if ENABLE_PCSL
    int handle;
    int rate;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    rate = (int)KNI_GetParameterAsInt(2);
    result = javacall_media_set_audio_sample_rate((javacall_handle)handle, rate);
    if (result != JAVACALL_OK) {
        // KNI_ThrowNew(KNIIOException, "failed to set audio sample rate.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_setChannelConfig0()
{
#if ENABLE_PCSL
    int handle;
    int channel;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    channel = (int)KNI_GetParameterAsInt(2);
    result = javacall_media_set_channel_config((javacall_handle)handle, channel);
    if (result != JAVACALL_OK) {
        // KNI_ThrowNew(KNIIOException, "failed to set channel.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_setAudioBitRate0()
{
#if ENABLE_PCSL
    int handle;
    int rate;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    rate = (int)KNI_GetParameterAsInt(2);
    result = javacall_media_set_audio_bit_rate((javacall_handle)handle, rate);
    if (result != JAVACALL_OK) {
        // KNI_ThrowNew(KNIIOException, "failed to set audio bit rate.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_setOutputFile0()
{
#if ENABLE_PCSL
    int handle;
    pcsl_string* filename = NULL;
    int status = -1;
    javacall_result result = JAVACALL_FAIL;

    KNI_StartHandles(1);
    KNI_DeclareHandle(filenameObj);

    handle = (int)KNI_GetParameterAsInt(1);
    KNI_GetParameterAsObject(2, filenameObj);
    status = get_pcsl_string(filenameObj, &filename, 1);

    if (status == 0) {
        const jbyte *ptr = pcsl_string_get_utf8_data(filename);
        if (ptr != NULL) {
            result = javacall_media_set_output_file((javacall_handle)handle, (char *)ptr);
            if (result != JAVACALL_OK) {
                KNI_ThrowNew(KNIIOException, "failed to set output file.");
            }
            pcsl_string_release_utf8_data(ptr, filename);
        }
        pcsl_string_free(filename);
        pcsl_mem_free(filename);
    }
    KNI_EndHandles();
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_setOutputFormat0()
{
#if ENABLE_PCSL
    int handle;
    int format;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    format = (int)KNI_GetParameterAsInt(2);
    result = javacall_media_set_output_format((javacall_handle)handle, format);
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "failed to set output format.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_joshvm_media_PlayerImpl_getBufferSize0()
{
    int size;
#if ENABLE_PCSL
    int handle;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    result = javacall_media_get_buffsize((javacall_handle)handle, &size);
    if (result != JAVACALL_OK) {
        size = -1;
    }
#endif
    KNI_ReturnInt(size);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_joshvm_media_PlayerImpl_write0()
{
    int bytesWritten = -1;
#if ENABLE_PCSL
    int handle;
    int offset;
    int length;
    javacall_result result = JAVACALL_FAIL;
    SNIReentryData* info;

    handle = (int)KNI_GetParameterAsInt(1);
    offset = (int)KNI_GetParameterAsInt(3);
    length = (int)KNI_GetParameterAsInt(4);

    KNI_StartHandles(1);
    KNI_DeclareHandle(bufferObject);
    KNI_GetParameterAsObject(2, bufferObject);

    info = (SNIReentryData*)SNI_GetReentryData(NULL);
    if (info == NULL ||
        (info->status == (int)JAVACALL_OK)) {
        result = javacall_media_write((javacall_handle)handle,
            (unsigned char*)SNI_GetRawArrayPointer(bufferObject)+offset,
            length, &bytesWritten);
    }

    if (result == JAVACALL_WOULD_BLOCK) {
        SNIEVT_wait(MEDIA_WRITE_SIGNAL, handle, (void*)NULL);
    } else {
        if (result != JAVACALL_OK) {
            KNI_ThrowNew(KNIIOException, "failed to writing.");
        } else if (bytesWritten <= 0) {
            KNI_ThrowNew(KNIIOException, "can not writing.");
        }
    }
    KNI_EndHandles();
#endif
    KNI_ReturnInt(bytesWritten);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_PlayerImpl_flush0()
{
#if ENABLE_PCSL
    int handle;
    javacall_result result = JAVACALL_FAIL;

    handle = (int)KNI_GetParameterAsInt(1);
    result = javacall_media_flush((javacall_handle)handle);
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "flush failed.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_com_joshvm_media_PlayerImpl_read0()
{
    int bytesRead = -1;
#if ENABLE_PCSL
    int handle;
    int offset;
    int length;
    javacall_result result = JAVACALL_FAIL;
    SNIReentryData* info;

    handle = (int)KNI_GetParameterAsInt(1);
    offset = (int)KNI_GetParameterAsInt(3);
    length = (int)KNI_GetParameterAsInt(4);

    KNI_StartHandles(1);
    KNI_DeclareHandle(bufferObject);
    KNI_GetParameterAsObject(2, bufferObject);

    info = (SNIReentryData*)SNI_GetReentryData(NULL);
    if (info == NULL ||
        (info->status == (int)JAVACALL_OK)) {
        result = javacall_media_read((javacall_handle)handle,
            (unsigned char*)SNI_GetRawArrayPointer(bufferObject)+offset,
            length, &bytesRead);
    }
    if (result == JAVACALL_WOULD_BLOCK) {
        SNIEVT_wait(MEDIA_READ_SIGNAL, handle, (void*)NULL);
    } else {
        if (result != JAVACALL_OK) {
            KNI_ThrowNew(KNIIOException, "failed to reading.");
        } else {
            if (bytesRead == 0) {
                bytesRead = -1;
            }
        }
    }
    KNI_EndHandles();
#endif
    KNI_ReturnInt(bytesRead);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_MediaEventThread_waitingForEvent()
{
#if ENABLE_PCSL
    jfieldID handle_field_id;
    jfieldID type_field_id;
    jfieldID userdata_field_id;
    javacall_media_event_type event_type;
    int event_handle;
    javacall_result event_result;
    int userdata;
    javacall_result result;

    KNI_StartHandles(2);
    KNI_DeclareHandle(eventObj);
    KNI_DeclareHandle(classObj);

    KNI_GetParameterAsObject(1, eventObj);
    KNI_GetObjectClass(eventObj, classObj);

    result = javacall_media_event_get(&event_type, &event_handle, &event_result, &userdata);
    if (result == JAVACALL_WOULD_BLOCK) {
        SNIEVT_wait(MEDIA_SIGNAL, -1, NULL);
    } else if (result == JAVACALL_OK) {
        handle_field_id = KNI_GetFieldID(classObj, "handle", "I");
        if (handle_field_id) {
            KNI_SetIntField(eventObj, handle_field_id, (jint)event_handle);
        }
        type_field_id = KNI_GetFieldID(classObj, "type", "I");
        if (type_field_id) {
            KNI_SetIntField(eventObj, type_field_id, (jint)event_type);
        }
        userdata_field_id = KNI_GetFieldID(classObj, "intData", "I");
        if (userdata_field_id) {
            KNI_SetIntField(eventObj, userdata_field_id, (jint)userdata);
        }
    }
    KNI_EndHandles();
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_MediaEventThread_initEvent()
{
#if ENABLE_PCSL
    javacall_media_event_init();
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_com_joshvm_media_MediaEventThread_finalizeEvent()
{
#if ENABLE_PCSL
    javacall_media_event_finalize();

#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_media_AudioManager_getMaxVolume0()
{
    int volume;
#if ENABLE_PCSL
    javacall_result result = JAVACALL_FAIL;

    result = javacall_media_get_max_volume(&volume);
    if (result != JAVACALL_OK) {
        volume = -1;
    }
#endif
    KNI_ReturnInt(volume);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_media_AudioManager_getCurrentVolume0()
{
    int volume;
#if ENABLE_PCSL
    javacall_result result = JAVACALL_FAIL;

    result = javacall_media_get_volume(&volume);
    if (result != JAVACALL_OK) {
        volume = -1;
    }
#endif
    KNI_ReturnInt(volume);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_media_AudioManager_setVolume0()
{
#if ENABLE_PCSL
    int volume;
    javacall_result result = JAVACALL_FAIL;

    volume = (int)KNI_GetParameterAsInt(1);
    result = javacall_media_set_volume(volume);
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "failed to set volume.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_media_Environment_getRootDir0()
{
#if ENABLE_PCSL
    javacall_media_filepath_info rootdir;
#endif

    KNI_StartHandles(1);
    KNI_DeclareHandle(rootdirObj);

#if ENABLE_PCSL
    javacall_result result = JAVACALL_FAIL;
    result = javacall_media_get_rootdir(&rootdir);
    if (result == JAVACALL_OK) {
        KNI_NewStringUTF(rootdir.path, rootdirObj);
    } else {
        KNI_ThrowNew(KNIIOException, "failed get root dir.");
    }
#endif

    KNI_EndHandlesAndReturnObject(rootdirObj);
}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_media_Environment_getExtRootDir0()
{
#if ENABLE_PCSL
    javacall_media_filepath_info rootdir;
#endif

    KNI_StartHandles(1);
    KNI_DeclareHandle(rootdirObj);

#if ENABLE_PCSL
    javacall_result result = JAVACALL_FAIL;
    result = javacall_media_get_ext_rootdir(&rootdir);
    if (result == JAVACALL_OK) {
        KNI_NewStringUTF(rootdir.path, rootdirObj);
    } else {
        KNI_ThrowNew(KNIIOException, "failed get external root dir.");
    }
#endif

    KNI_EndHandlesAndReturnObject(rootdirObj);
}

KNIEXPORT
KNI_RETURNTYPE_BOOLEAN
Java_org_joshvm_media_Environment_getExtStorageState0()
{
    jboolean exist = KNI_FALSE;
#if ENABLE_PCSL

    exist = (JAVACALL_OK == javacall_media_ext_rootdir_exists());
#endif

    KNI_ReturnBoolean(exist);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_media_WakeUpManager_enable0()
{
#if ENABLE_PCSL
    jboolean enable;
    javacall_result result = JAVACALL_FAIL;

    enable = KNI_GetParameterAsBoolean(1);
    result = javacall_media_wakeup_enable(enable);
    if (result != JAVACALL_OK) {
        if (enable) {
            KNI_ThrowNew(KNIIOException, "failed to enable wakeup function.");
        } else {
            KNI_ThrowNew(KNIIOException, "failed to disable wakeup function.");
        }
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_media_WakeUpManager_finalize()
{
#if ENABLE_PCSL
    javacall_media_wakeup_enable(0);
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_media_WakeUpManager_getWordCount()
{
    int count;
#if ENABLE_PCSL
    javacall_result result = JAVACALL_FAIL;

    result = javacall_media_wakeup_get_count(&count);
    if (result != JAVACALL_OK) {
        count = -1;
    }
#endif
    KNI_ReturnInt(count);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_media_WakeUpManager_getWords()
{
#if ENABLE_PCSL
    javacall_media_word_info info;
    jfieldID index_field_id;
    jfieldID word_field_id;
    jfieldID desc_field_id;
    int flag = 0;
    int size;
    int i;
    javacall_result result = JAVACALL_FAIL;

    KNI_StartHandles(5);
    KNI_DeclareHandle(wordarray);
    KNI_DeclareHandle(word);
    KNI_DeclareHandle(wordClass);
    KNI_DeclareHandle(wordStr);
    KNI_DeclareHandle(descStr);

    KNI_GetParameterAsObject(1, wordarray);
    size = KNI_GetArrayLength(wordarray);
    for (i = 0; i < size; i++) {
        result = javacall_media_wakeup_get_word(i, &info);
        if (result != JAVACALL_OK) {
            continue;
        }
        KNI_GetObjectArrayElement(wordarray, i, word);
        if (flag == 0) {
            KNI_GetObjectClass(word, wordClass);
            index_field_id = KNI_GetFieldID(wordClass, "index", "I");
            word_field_id = KNI_GetFieldID(wordClass, "word", "Ljava/lang/String;");
            desc_field_id = KNI_GetFieldID(wordClass, "description", "Ljava/lang/String;");
            flag = 1;
        }
        KNI_SetIntField(word, index_field_id, info.index);
        KNI_NewStringUTF(info.word, wordStr);
        KNI_SetObjectField(word, word_field_id, wordStr);
        KNI_NewStringUTF(info.desc, descStr);
        KNI_SetObjectField(word, desc_field_id, descStr);
    }
    KNI_EndHandles();
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_media_VADController_start0()
{
#if ENABLE_PCSL
    javacall_result result = JAVACALL_FAIL;

    result = javacall_media_vad_start();
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "failed to start vad.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_media_VADController_pause0()
{
#if ENABLE_PCSL
    javacall_result result = JAVACALL_FAIL;

    result = javacall_media_vad_pause();
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "failed to pause vad.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_media_VADController_resume0()
{
#if ENABLE_PCSL
    javacall_result result = JAVACALL_FAIL;

    result = javacall_media_vad_resume();
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "failed to resume vad.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_media_VADController_stop0()
{
#if ENABLE_PCSL
    javacall_result result = JAVACALL_FAIL;

    result = javacall_media_vad_stop();
    if (result != JAVACALL_OK) {
        KNI_ThrowNew(KNIIOException, "failed to stop vad.");
    }
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_media_VADController_finalize()
{
#if ENABLE_PCSL
    javacall_media_vad_stop();
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_media_VADController_setTimeout0()
{
#if ENABLE_PCSL
    int timeout;
    javacall_result result = JAVACALL_FAIL;

    timeout = (int)KNI_GetParameterAsInt(1);
    result = javacall_media_vad_set_timeout(timeout);
    if (result != JAVACALL_OK) {
        // KNI_ThrowNew(KNIIOException, "failed to set vad timeout.");
    }
#endif

    KNI_ReturnVoid();
}
