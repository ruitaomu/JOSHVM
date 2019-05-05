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

#include "javacall_defs.h"

#if ENABLE_PCSL
#include "javacall_directui.h"
#include <PCSLString_Util.h>
#include <pcsl_memory.h>
#include <pcsl_string.h>

static int get_pcsl_string(jobject stringHandle, pcsl_string ** string)
{
    if (KNI_IsNullHandle(stringHandle)) {
        KNI_ThrowNew(KNINullPointerException, NULL);
        return -1;
    } else {
        pcsl_string * str = (pcsl_string *)pcsl_mem_malloc(sizeof(pcsl_string));

        if (str == NULL) {
            KNI_ThrowNew(KNIOutOfMemoryError, NULL);
            return -1;
        }

        if (midp_jstring_to_pcsl_string(stringHandle, str) != PCSL_STRING_OK) {
            pcsl_mem_free(str);
            return -1;
        }

        * string = str;
    }
    return 0;
}
#endif

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_getDisplayWidth0()
{
    int width = 0;
#if ENABLE_PCSL
    int height;
    javacall_directui_get_screen(&width, &height);
#endif
    KNI_ReturnInt(width);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_getDisplayHeight0()
{
    int height = 0;
#if ENABLE_PCSL
    int width;
    javacall_directui_get_screen(&width, &height);
#endif
    KNI_ReturnInt(height);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_clear0()
{
#if ENABLE_PCSL
    int rgb;
    rgb = (int)KNI_GetParameterAsInt(1);

    javacall_directui_clear(rgb);
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_update0()
{
#if ENABLE_PCSL
    int x, y;
    int w, h;
    jboolean delayed;

    KNI_StartHandles(1);
    KNI_DeclareHandle(dataObj);

    KNI_GetParameterAsObject(3, dataObj);
    x = (int)KNI_GetParameterAsInt(1);
    y = (int)KNI_GetParameterAsInt(2);
    w = (int)KNI_GetParameterAsInt(4);
    h = (int)KNI_GetParameterAsInt(5);
    delayed = KNI_GetParameterAsBoolean(6);

    SNI_BEGIN_RAW_POINTERS
    javacall_directui_drawrawdata(x, y,
        (javacall_uint8 *)SNI_GetRawArrayPointer(dataObj),
        w, h, delayed);
    SNI_END_RAW_POINTERS

    KNI_EndHandles();
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_showText0()
{
#if ENABLE_PCSL
    pcsl_string * text = NULL;
    int x, y;
    int rgb;
    int font;
    jboolean delayed;
    int status = -1;

    KNI_StartHandles(1);
    KNI_DeclareHandle(textHandle);

    x = KNI_GetParameterAsInt(1);
    y = KNI_GetParameterAsInt(2);
    KNI_GetParameterAsObject(3, textHandle);
    rgb = KNI_GetParameterAsInt(4);
    font = KNI_GetParameterAsInt(5);
    delayed = KNI_GetParameterAsBoolean(6);

    status = get_pcsl_string(textHandle, &text);
    if (status == 0) {
        GET_PCSL_STRING_DATA_AND_LENGTH(text)
        javacall_directui_textout(font, rgb, x, y, text_data, text_len, delayed);
        RELEASE_PCSL_STRING_DATA_AND_LENGTH
        pcsl_string_free(text);
        pcsl_mem_free(text);
    }

    KNI_EndHandles();
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_showImage0()
{
#if ENABLE_PCSL
    int x, y;
    int offset, len;
    int type;
    jboolean delayed;

    KNI_StartHandles(1);
    KNI_DeclareHandle(dataObj);

    KNI_GetParameterAsObject(3, dataObj);
    x = (int)KNI_GetParameterAsInt(1);
    y = (int)KNI_GetParameterAsInt(2);
    offset = (int)KNI_GetParameterAsInt(4);
    len = (int)KNI_GetParameterAsInt(5);
    type = (int)KNI_GetParameterAsInt(6);
    delayed = KNI_GetParameterAsBoolean(7);

    SNI_BEGIN_RAW_POINTERS
    javacall_directui_drawimage(x, y,
        (javacall_uint8 *)SNI_GetRawArrayPointer(dataObj)+offset,
        len, (javacall_directui_image_type)type, delayed);
    SNI_END_RAW_POINTERS

    KNI_EndHandles();
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_flush0()
{
#if ENABLE_PCSL
    javacall_directui_flush();
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_setBacklight0()
{
#if ENABLE_PCSL
    jboolean on;

    on = KNI_GetParameterAsBoolean(1);
    javacall_directui_setBacklight(on);
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_directUI_Text_getWidth0()
{
    int width = 0;
#if ENABLE_PCSL
    int height;
    int font;
    pcsl_string * text = NULL;
    int status = -1;

    KNI_StartHandles(1);
    KNI_DeclareHandle(textHandle);

    KNI_GetParameterAsObject(1, textHandle);
    font = KNI_GetParameterAsInt(2);
    status = get_pcsl_string(textHandle, &text);
    if (status == 0) {
        GET_PCSL_STRING_DATA_AND_LENGTH(text)
        javacall_directui_text_getsize(font, text_data, text_len, &width, &height);
        RELEASE_PCSL_STRING_DATA_AND_LENGTH
        pcsl_string_free(text);
        pcsl_mem_free(text);
    }

    KNI_EndHandles();
#endif
    KNI_ReturnInt(width);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_directUI_Text_getHeight0()
{
    int height = 0;
#if ENABLE_PCSL
    int width;
    int font;
    pcsl_string * text = NULL;
    int status = -1;

    KNI_StartHandles(1);
    KNI_DeclareHandle(textHandle);

    KNI_GetParameterAsObject(1, textHandle);
    font = KNI_GetParameterAsInt(2);
    status = get_pcsl_string(textHandle, &text);
    if (status == 0) {
        GET_PCSL_STRING_DATA_AND_LENGTH(text)
        javacall_directui_text_getsize(font, text_data, text_len, &width, &height);
        RELEASE_PCSL_STRING_DATA_AND_LENGTH
        pcsl_string_free(text);
        pcsl_mem_free(text);
    }

    KNI_EndHandles();
#endif
    KNI_ReturnInt(height);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_directUI_Image_getWidth0()
{
    int width = 0;
#if ENABLE_PCSL
    int height;
    int offset, len;
    int type;
    javacall_result result = JAVACALL_FAIL;

    KNI_StartHandles(1);
    KNI_DeclareHandle(dataObj);
    KNI_GetParameterAsObject(1, dataObj);
    offset = (int)KNI_GetParameterAsInt(2);
    len = (int)KNI_GetParameterAsInt(3);
    type = (int)KNI_GetParameterAsInt(4);

    SNI_BEGIN_RAW_POINTERS
    result = javacall_directui_image_getsize(
        (javacall_uint8 *)SNI_GetRawArrayPointer(dataObj)+offset,
        len, (javacall_directui_image_type)type, &width, &height);
    SNI_END_RAW_POINTERS

    if (result != JAVACALL_OK) {
        width = 0;
    }

    KNI_EndHandles();
#endif
    KNI_ReturnInt(width);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_directUI_Image_getHeight0()
{
    int height = 0;
#if ENABLE_PCSL
    int width;
    int offset, len;
    int type;
    javacall_result result = JAVACALL_FAIL;

    KNI_StartHandles(1);
    KNI_DeclareHandle(dataObj);
    KNI_GetParameterAsObject(1, dataObj);
    offset = (int)KNI_GetParameterAsInt(2);
    len = (int)KNI_GetParameterAsInt(3);
    type = (int)KNI_GetParameterAsInt(4);

    SNI_BEGIN_RAW_POINTERS
    result = javacall_directui_image_getsize(
        (javacall_uint8 *)SNI_GetRawArrayPointer(dataObj)+offset,
        len, (javacall_directui_image_type)type, &width, &height);
    SNI_END_RAW_POINTERS

    if (result != JAVACALL_OK) {
        height = 0;
    }

    KNI_EndHandles();
#endif
    KNI_ReturnInt(height);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_KeypadThread_waitingForKeyEvent()
{
    int handle = -1;
#if ENABLE_PCSL
    jfieldID value_field_id;
    jfieldID type_field_id;
    javacall_keypress_code key;
    javacall_keypress_type type;
    javacall_result result;

    KNI_StartHandles(2);
    KNI_DeclareHandle(eventObj);
    KNI_DeclareHandle(classObj);

    KNI_GetParameterAsObject(1, eventObj);
    KNI_GetObjectClass(eventObj, classObj);

    result = javacall_directui_key_event_get(&key, &type);
    if (result == JAVACALL_WOULD_BLOCK) {
        SNIEVT_wait(KEY_SIGNAL, (int)handle, (void*)NULL);
    } else if (result == JAVACALL_OK) {
        value_field_id = KNI_GetFieldID(classObj, "keyValue", "C");
        if (value_field_id) {
            KNI_SetCharField(eventObj, value_field_id, (jchar)key);
        }
        type_field_id = KNI_GetFieldID(classObj, "eventType", "I");
        if (type_field_id) {
            KNI_SetIntField(eventObj, type_field_id, (jint)type);
        }
    }
    KNI_EndHandles();
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_KeypadThread_initEvent()
{
#if ENABLE_PCSL
    javacall_directui_key_event_init();
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_KeypadThread_finalizeEvent()
{
#if ENABLE_PCSL
    javacall_directui_key_event_finalize();
#endif

    KNI_ReturnVoid();
}
